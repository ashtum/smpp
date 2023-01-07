// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common/request_pdu.hpp>
#include <smpp/common/response_pdu.hpp>
#include <smpp/common/serialization.hpp>
#include <smpp/net/deserialize_error.hpp>
#include <smpp/net/detail/static_flat_buffer.hpp>
#include <smpp/net/pdu_variant.hpp>

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/experimental/co_composed.hpp>
#include <boost/asio/ip/tcp.hpp>
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
  uint32_t sequence_number_{ 0 };

public:
  explicit session(asio::ip::tcp::socket socket)
    : socket_(std::move(socket))
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
          const auto command_id      = std::decay_t<decltype(pdu)>::command_id;
          const auto sequence_number = next_sequence_number();

          send_buf_.clear();
          send_buf_.resize(header_length); // reserved for header

          auto eptr = std::exception_ptr{};
          try
          {
            serialize_to(&send_buf_, pdu);

            auto header = serialize_header(send_buf_.size(), command_id, sequence_number, command_status::rok);
            std::copy(header.begin(), header.end(), send_buf_.begin());

            co_await asio::async_write(socket_, asio::buffer(send_buf_), asio::deferred);
          }
          catch (const std::exception& ex)
          {
            eptr = std::current_exception();
          }

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
          const auto command_id = std::decay_t<decltype(pdu)>::command_id;
          send_buf_.clear();
          send_buf_.resize(header_length); // reserved for header

          auto eptr = std::exception_ptr{};
          try
          {
            serialize_to(&send_buf_, pdu);

            auto header = serialize_header(send_buf_.size(), command_id, sequence_number, command_status);
            std::copy(header.begin(), header.end(), send_buf_.begin());

            co_await asio::async_write(socket_, asio::buffer(send_buf_), asio::deferred);
          }
          catch (const std::exception& ex)
          {
            eptr = std::current_exception();
          }

          co_return { eptr };
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
          auto needs_receive = false;
          for (;;)
          {
            if (needs_receive)
            {
              auto [ec, received] =
                co_await socket_.async_receive(receive_buf_.prepare(64 * 1024), asio::as_tuple(asio::deferred));

              if (ec)
                co_return { std::make_exception_ptr(boost::system::system_error{ ec }), {}, {}, {} };

              receive_buf_.commit(received);
              needs_receive = false;
            }

            if (receive_buf_.size() < header_length)
            {
              needs_receive = true;
              continue;
            }

            auto header_buf = std::span{ receive_buf_ }.subspan<0, header_length>();
            auto [command_length, command_id, command_status, sequence_number] = deserialize_header(header_buf);

            if (receive_buf_.size() < command_length)
            {
              needs_receive = true;
              continue;
            }

            auto body_buf = std::span{ receive_buf_.begin() + header_length, receive_buf_.begin() + command_length };

            std::exception_ptr eptr;
            pdu_variant pdu;
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
              eptr = std::make_exception_ptr(deserialize_error{
                ex.what(),
                std::vector<uint8_t>{ receive_buf_.begin(), receive_buf_.begin() + command_length + header_length } });
            }

            receive_buf_.consume(command_length);
            co_return { eptr, pdu, sequence_number, command_status };
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

  static std::array<uint8_t, header_length> serialize_header(
    uint32_t command_length,
    command_id command_id,
    uint32_t sequence_number,
    command_status command_status)
  {
    std::array<uint8_t, header_length> buf{};

    auto s_u32 = [](std::span<uint8_t, 4> buf, uint32_t val)
    {
      buf[0] = (val >> 24) & 0xFF;
      buf[1] = (val >> 16) & 0xFF;
      buf[2] = (val >> 8) & 0xFF;
      buf[3] = (val >> 0) & 0xFF;
    };

    s_u32(std::span{ buf }.subspan<0, 4>(), command_length);
    s_u32(std::span{ buf }.subspan<4, 4>(), static_cast<uint32_t>(command_id));
    s_u32(std::span{ buf }.subspan<8, 4>(), static_cast<uint32_t>(command_status));
    s_u32(std::span{ buf }.subspan<12, 4>(), sequence_number);

    return buf;
  }

  uint32_t next_sequence_number()
  {
    if (++sequence_number_ > 0x7FFFFFFF)
      sequence_number_ = 1;
    return sequence_number_;
  }
};
} // namespace smpp
