// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common/request_pdu.hpp>
#include <smpp/common/response_pdu.hpp>
#include <smpp/common/serialization.hpp>
#include <smpp/net/deserialization_error.hpp>
#include <smpp/net/detail/header_serialization.hpp>
#include <smpp/net/detail/static_flat_buffer.hpp>
#include <smpp/net/inactivity_error.hpp>
#include <smpp/net/pdu_variant.hpp>
#include <smpp/net/unbinded.hpp>

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/experimental/co_composed.hpp>
#include <boost/asio/experimental/parallel_group.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>

namespace smpp
{
namespace asio = boost::asio;
class session
{
  static constexpr auto deferred_tuple{ asio::as_tuple(asio::deferred) };
  static constexpr auto header_length{ 16 };
  asio::ip::tcp::socket socket_;
  detail::static_flat_buffer<uint8_t, 1024 * 1024> receive_buf_{};
  std::vector<uint8_t> send_buf_{};
  asio::steady_timer send_cv_;
  asio::steady_timer enquire_link_timer_;
  std::chrono::seconds enquire_link_interval_{ 60 };
  uint8_t enquire_link_counter_{ 0 };
  uint32_t sequence_number_{ 0 };

  auto async_send_command(
    command_id command_id,
    uint32_t sequence_number,
    asio::completion_token_for<void(boost::system::error_code)> auto&& token)
  {
    return asio::async_initiate<decltype(token), void(boost::system::error_code)>(
      asio::experimental::co_composed<void(boost::system::error_code)>(
        [&, command_id, sequence_number](auto state) -> void
        {
          while (!send_buf_.empty()) // ongoing send operation
          {
            auto [ec] = co_await send_cv_.async_wait(deferred_tuple);
            if (ec != asio::error::operation_aborted || !!state.cancelled())
              co_return ec;
          }

          send_buf_.resize(header_length); // reserved for header
          detail::serialize_header(&send_buf_, header_length, command_id, sequence_number);
          auto [ec, _] = co_await asio::async_write(socket_, asio::buffer(send_buf_), deferred_tuple);

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
    , enquire_link_timer_{ socket.get_executor() }
  {
  }

  void set_enquire_link_interval(std::chrono::seconds interval)
  {
    enquire_link_interval_ = interval;
  }

  auto async_send(
    const request_pdu auto& pdu,
    asio::completion_token_for<void(std::exception_ptr, uint32_t)> auto&& token)
  {
    return asio::async_initiate<decltype(token), void(std::exception_ptr, uint32_t)>(
      asio::experimental::co_composed<void(std::exception_ptr, uint32_t)>(
        [&](auto state) -> void
        {
          auto command_id      = std::decay_t<decltype(pdu)>::command_id;
          auto sequence_number = next_sequence_number();

          while (!send_buf_.empty()) // ongoing send operation
          {
            auto [ec] = co_await send_cv_.async_wait(deferred_tuple);
            if (ec != asio::error::operation_aborted || !!state.cancelled())
              co_return { std::make_exception_ptr(boost::system::system_error{ ec }), {} };
          }

          auto eptr = std::exception_ptr{};
          try
          {
            send_buf_.resize(header_length); // reserved for header
            serialize_to(&send_buf_, pdu);
            detail::serialize_header(&send_buf_, send_buf_.size(), command_id, sequence_number);
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
        [&, sequence_number, command_status](auto state) -> void
        {
          auto command_id = std::decay_t<decltype(pdu)>::command_id;

          while (!send_buf_.empty()) // ongoing send operation
          {
            auto [ec] = co_await send_cv_.async_wait(deferred_tuple);
            if (ec != asio::error::operation_aborted || !!state.cancelled())
              co_return std::make_exception_ptr(boost::system::system_error{ ec });
          }

          auto eptr = std::exception_ptr{};
          try
          {
            send_buf_.resize(header_length); // reserved for header
            serialize_to(&send_buf_, pdu);
            detail::serialize_header(&send_buf_, send_buf_.size(), command_id, sequence_number, command_status);
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
          auto [ec] = co_await async_send_command(command_id::unbind, next_sequence_number(), deferred_tuple);
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
            using enum command_id;

            if (needs_more)
            {
              needs_more = false;
              enquire_link_timer_.expires_after(enquire_link_interval_);
              auto [order, receive_ec, received, timer_ec] =
                co_await asio::experimental::make_parallel_group(
                  [&](auto token) { return socket_.async_receive(receive_buf_.prepare(64 * 1024), token); },
                  [&](auto token) { return enquire_link_timer_.async_wait(token); })
                  .async_wait(asio::experimental::wait_for_one(), deferred_tuple);

              receive_buf_.commit(received);

              if (order[0] == 0) // receive completed first
              {
                enquire_link_counter_ = 0;
                if (receive_ec)
                  co_return { std::make_exception_ptr(boost::system::system_error{ receive_ec }), {}, {}, {} };
              }
              else
              {
                if (++enquire_link_counter_ >= 2)
                {
                  auto ec = boost::system::error_code{};
                  socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
                  socket_.close(ec);
                  co_return { std::make_exception_ptr(inactivity_error{}), {}, {}, {} };
                }
                co_await async_send_command(enquire_link, next_sequence_number(), deferred_tuple);
              }
            }

            if (receive_buf_.size() < header_length)
            {
              needs_more = true;
              continue;
            }

            auto [command_length, command_id, command_status, sequence_number] =
              detail::deserialize_header(std::span<const uint8_t, header_length>{ receive_buf_ });

            if (receive_buf_.size() < command_length)
            {
              needs_more = true;
              continue;
            }

            if (command_id == enquire_link)
            {
              co_await async_send_command(enquire_link_resp, sequence_number, deferred_tuple);
              receive_buf_.consume(command_length);
            }
            else if (command_id == enquire_link_resp)
            {
              receive_buf_.consume(command_length);
            }
            else if (command_id == unbind || command_id == unbind_resp)
            {
              if (command_id == unbind)
              {
                auto [ec] = co_await async_send_command(unbind_resp, sequence_number, deferred_tuple);
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
  uint32_t next_sequence_number()
  {
    if (++sequence_number_ > 0x7FFFFFFF)
      sequence_number_ = 1;
    return sequence_number_;
  }
};
} // namespace smpp
