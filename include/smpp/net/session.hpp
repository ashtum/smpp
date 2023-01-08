// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common/request_pdu.hpp>
#include <smpp/common/response_pdu.hpp>
#include <smpp/common/serialization.hpp>
#include <smpp/net/deserialization_error.hpp>
#include <smpp/net/detail/static_flat_buffer.hpp>
#include <smpp/net/pdu_variant.hpp>
#include <smpp/net/unbinded.hpp>

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/experimental/co_composed.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/redirect_error.hpp>
#include <boost/asio/write.hpp>

namespace smpp
{
namespace asio = boost::asio;
class session
{
  static constexpr auto header_length{ 16 };
  asio::ip::tcp::socket socket_;
  detail::static_flat_buffer<uint8_t, 1024 * 1024> receive_buf_{};
  std::vector<uint8_t> send_buf_{};
  asio::steady_timer send_cv_;
  uint32_t sequence_number_{ 0 };

  auto async_send_command(
    command_id command_id,
    uint32_t sequence_number,
    asio::completion_token_for<void(boost::system::error_code)> auto&& token)
  {
    return asio::async_initiate<decltype(token), void(boost::system::error_code)>(
      asio::experimental::co_composed<void(boost::system::error_code)>(
        [&, command_id, sequence_number](auto) -> void
        {
          while (!send_buf_.empty()) // ongoing send operation
            co_await send_cv_.async_wait(asio::as_tuple(asio::deferred));

          send_buf_.resize(header_length); // reserved for header
          serialize_header(send_buf_.begin(), header_length, command_id, sequence_number, command_status::rok);
          auto [ec, _] = co_await asio::async_write(socket_, asio::buffer(send_buf_), asio::as_tuple(asio::deferred));

          send_buf_.clear();
          send_cv_.cancel_one();
          co_return ec;
        }),
      token);
  }

public:
  explicit session(asio::ip::tcp::socket socket)
    : socket_(std::move(socket))
    , send_cv_{ socket.get_executor(), asio::steady_timer::time_point::max() }
  {
  }

  auto async_send(
    const request_pdu auto& pdu,
    asio::completion_token_for<void(std::exception_ptr, uint32_t)> auto&& token)
  {
    return asio::async_initiate<decltype(token), void(std::exception_ptr, uint32_t)>(
      asio::experimental::co_composed<void(std::exception_ptr, uint32_t)>(
        [&](auto) -> void
        {
          auto command_id      = std::decay_t<decltype(pdu)>::command_id;
          auto sequence_number = next_sequence_number();

          while (!send_buf_.empty()) // ongoing send operation
            co_await send_cv_.async_wait(asio::as_tuple(asio::deferred));

          auto eptr = std::exception_ptr{};
          try
          {
            send_buf_.resize(header_length); // reserved for header
            serialize_to(&send_buf_, pdu);
            serialize_header(send_buf_.begin(), send_buf_.size(), command_id, sequence_number, command_status::rok);
            co_await asio::async_write(socket_, asio::buffer(send_buf_), asio::deferred);
          }
          catch (const std::exception& ex) // from serialize_to and asio::async_write
          {
            eptr = std::current_exception();
          }

          send_buf_.clear();
          send_cv_.cancel_one();
          co_return { eptr, sequence_number };
        }),
      token);
  }

  auto async_send(
    const response_pdu auto& pdu,
    uint32_t sequence_number,
    command_status command_status,
    asio::completion_token_for<void(std::exception_ptr)> auto&& token)
  {
    return asio::async_initiate<decltype(token), void(std::exception_ptr)>(
      asio::experimental::co_composed<void(std::exception_ptr)>(
        [&, sequence_number, command_status](auto) -> void
        {
          auto command_id = std::decay_t<decltype(pdu)>::command_id;

          while (!send_buf_.empty()) // ongoing send operation
            co_await send_cv_.async_wait(asio::as_tuple(asio::deferred));

          auto eptr = std::exception_ptr{};
          try
          {
            send_buf_.resize(header_length); // reserved for header
            serialize_to(&send_buf_, pdu);
            serialize_header(send_buf_.begin(), send_buf_.size(), command_id, sequence_number, command_status);
            co_await asio::async_write(socket_, asio::buffer(send_buf_), asio::deferred);
          }
          catch (const std::exception& ex) // from serialize_to and asio::async_write
          {
            eptr = std::current_exception();
          }

          send_buf_.clear();
          send_cv_.cancel_one();
          co_return eptr;
        }),
      token);
  }

  auto async_send_unbind(asio::completion_token_for<void(std::exception_ptr)> auto&& token)
  {
    return asio::async_initiate<decltype(token), void(std::exception_ptr)>(
      asio::experimental::co_composed<void(std::exception_ptr)>(
        [&](auto) -> void
        {
          auto [ec] =
            co_await async_send_command(command_id::unbind, next_sequence_number(), asio::as_tuple(asio::deferred));

          if (ec)
            co_return std::make_exception_ptr(boost::system::system_error{ ec });

          co_return {};
        }),
      token);
  }

  auto async_receive(
    asio::completion_token_for<void(std::exception_ptr, pdu_variant, uint32_t, command_status)> auto&& token)
  {
    return asio::async_initiate<decltype(token), void(std::exception_ptr, pdu_variant, uint32_t, command_status)>(
      asio::experimental::co_composed<void(std::exception_ptr, pdu_variant, uint32_t, command_status)>(
        [&](auto) -> void
        {
          for (auto needs_more = false;;)
          {
            if (std::exchange(needs_more, false))
            {
              auto [ec, received] =
                co_await socket_.async_receive(receive_buf_.prepare(64 * 1024), asio::as_tuple(asio::deferred));

              receive_buf_.commit(received);

              if (ec)
                co_return { std::make_exception_ptr(boost::system::system_error{ ec }), {}, {}, {} };
            }

            if (receive_buf_.size() < header_length)
            {
              needs_more = true;
              continue;
            }

            auto header_buf = std::span{ receive_buf_ }.subspan<0, header_length>();
            auto [command_length, command_id, command_status, sequence_number] = deserialize_header(header_buf);

            if (receive_buf_.size() < command_length)
            {
              needs_more = true;
              continue;
            }

            if (command_id == command_id::enquire_link)
            {
              co_await async_send_command(
                command_id::enquire_link_resp, sequence_number, asio::as_tuple(asio::deferred));
              receive_buf_.consume(command_length);
            }
            else if (command_id == command_id::enquire_link_resp)
            {
              receive_buf_.consume(command_length);
            }
            else if (command_id == command_id::unbind || command_id == command_id::unbind_resp)
            {
              if (command_id == command_id::unbind)
              {
                auto [ec] =
                  co_await async_send_command(command_id::unbind_resp, sequence_number, asio::as_tuple(asio::deferred));

                if (ec)
                  co_return { std::make_exception_ptr(boost::system::system_error{ ec }), {}, {}, {} };
              }
              auto ec = boost::system::error_code{};
              socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
              socket_.close(ec);
              receive_buf_.consume(command_length);
              co_return { std::make_exception_ptr(unbinded{}), {}, {}, {} };
            }
            else
            {
              auto body_buf = std::span{ receive_buf_.begin() + header_length, receive_buf_.begin() + command_length };
              auto eptr     = std::exception_ptr{};
              auto pdu      = pdu_variant{};
              try
              {
                [&, command_id = command_id ]<std::size_t... Is>(std::index_sequence<Is...>)
                {
                  if (!((command_id == std::decay_t<decltype(std::get<Is>(pdu_variant{}))>::command_id
                           ? (pdu = deserialize<std::decay_t<decltype(std::get<Is>(pdu_variant{}))>>(body_buf), true)
                           : false) ||
                        ...))
                    throw std::logic_error{ "Unknown PDU" };
                }
                (std::make_index_sequence<std::variant_size_v<pdu_variant>>());
              }
              catch (const std::exception& ex)
              {
                eptr = std::make_exception_ptr(
                  deserialization_error{ ex.what(), { receive_buf_.begin(), receive_buf_.begin() + command_length } });
              }
              receive_buf_.consume(command_length);
              co_return { eptr, pdu, sequence_number, command_status };
            }
          }
        }),
      token);
  }

private:
  static std::tuple<uint32_t, command_id, command_status, uint32_t> deserialize_header(
    std::span<const uint8_t, header_length> buf)
  {
    auto d_u32 = [](std::span<const uint8_t, 4> buf) -> uint32_t
    { return buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3]; };

    auto command_length  = d_u32(buf.subspan<0, 4>());
    auto command_id      = static_cast<smpp::command_id>(d_u32(buf.subspan<4, 4>()));
    auto command_status  = static_cast<smpp::command_status>(d_u32(buf.subspan<8, 4>()));
    auto sequence_number = d_u32(buf.subspan<12, 4>());

    return { command_length, command_id, command_status, sequence_number };
  }

  static void serialize_header(
    std::vector<uint8_t>::iterator it,
    uint32_t command_length,
    command_id command_id,
    uint32_t sequence_number,
    command_status command_status)
  {
    auto s_u32 = [&](uint32_t val)
    {
      *it++ = (val >> 24) & 0xFF;
      *it++ = (val >> 16) & 0xFF;
      *it++ = (val >> 8) & 0xFF;
      *it++ = (val >> 0) & 0xFF;
    };

    s_u32(command_length);
    s_u32(static_cast<uint32_t>(command_id));
    s_u32(static_cast<uint32_t>(command_status));
    s_u32(sequence_number);
  }

  uint32_t next_sequence_number()
  {
    if (++sequence_number_ > 0x7FFFFFFF)
      sequence_number_ = 1;
    return sequence_number_;
  }
};
} // namespace smpp
