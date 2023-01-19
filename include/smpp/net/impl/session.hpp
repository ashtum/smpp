// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common/serialization.hpp>
#include <smpp/net/detail/header_serialization.hpp>
#include <smpp/net/error.hpp>
#include <smpp/net/session.hpp>

#include <boost/asio/experimental/co_composed.hpp>
#include <boost/asio/experimental/parallel_group.hpp>
#include <boost/asio/write.hpp>

namespace smpp
{
inline session::session(asio::ip::tcp::socket socket, std::chrono::seconds enquire_link_interval)
  : socket_(std::move(socket))
  , send_cv_{ socket_.get_executor(), asio::steady_timer::time_point::max() }
  , enquire_link_timer_{ socket_.get_executor() }
  , enquire_link_interval_{ enquire_link_interval }
{
}

inline uint32_t session::next_sequence_number()
{
  if (++sequence_number_ > 0x7FFFFFFF)
    sequence_number_ = 1;
  return sequence_number_;
}

inline void session::shutdown_socket()
{
  auto ec = boost::system::error_code{};
  socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
  socket_.close(ec);
}

auto session::async_send_command(
  command_id command_id,
  uint32_t sequence_number,
  asio::completion_token_for<void(boost::system::error_code)> auto&& token)
{
  return asio::async_initiate<decltype(token), void(boost::system::error_code)>(
    asio::experimental::co_composed<void(boost::system::error_code)>(
      [](auto state, auto* self, auto command_id, auto sequence_number) -> void
      {
        while (!self->send_buf_.empty()) // ongoing send operation
        {
          auto [ec] = co_await self->send_cv_.async_wait(deferred_tuple);
          if (ec != asio::error::operation_aborted || !!state.cancelled())
            co_return ec;
        }

        state.reset_cancellation_state(asio::enable_terminal_cancellation());

        self->send_buf_.resize(header_length); // reserved for header
        auto header_buf = std::span<uint8_t, header_length>{ self->send_buf_ };
        detail::serialize_header(header_buf, header_length, command_id, sequence_number);
        auto [ec, _] = co_await asio::async_write(self->socket_, asio::buffer(self->send_buf_), deferred_tuple);

        self->send_buf_.clear();
        self->send_cv_.cancel_one();
        co_return ec;
      },
      socket_),
    token,
    this,
    command_id,
    sequence_number);
}

auto session::async_send(
  const request_pdu auto& pdu,
  asio::completion_token_for<void(boost::system::error_code, uint32_t)> auto&& token)
{
  return asio::async_initiate<decltype(token), void(boost::system::error_code, uint32_t)>(
    asio::experimental::co_composed<void(boost::system::error_code, uint32_t)>(
      [](auto state, auto* self, auto* const pdu) -> void
      {
        auto command_id      = std::decay_t<decltype(*pdu)>::command_id;
        auto sequence_number = self->next_sequence_number();

        while (!self->send_buf_.empty()) // ongoing send operation
        {
          auto [ec] = co_await self->send_cv_.async_wait(deferred_tuple);
          if (ec != asio::error::operation_aborted || !!state.cancelled())
            co_return { ec, {} };
        }

        state.reset_cancellation_state(asio::enable_terminal_cancellation());

        auto ec = boost::system::error_code{};
        try
        {
          self->send_buf_.resize(header_length); // reserved for header
          serialize_to(&self->send_buf_, *pdu);
          auto header_buf = std::span<uint8_t, header_length>{ self->send_buf_ };
          detail::serialize_header(header_buf, self->send_buf_.size(), command_id, sequence_number);
          auto [wec, _] = co_await asio::async_write(self->socket_, asio::buffer(self->send_buf_), deferred_tuple);
          ec            = wec;
        }
        catch (const std::exception&)
        {
          ec = error::serialization_failed;
        }

        self->send_buf_.clear();
        self->send_cv_.cancel_one();
        co_return { ec, sequence_number };
      },
      socket_),
    token,
    this,
    &pdu);
}

auto session::async_send(
  const response_pdu auto& pdu,
  uint32_t sequence_number,
  command_status command_status,
  asio::completion_token_for<void(boost::system::error_code)> auto&& token)
{
  return asio::async_initiate<decltype(token), void(boost::system::error_code)>(
    asio::experimental::co_composed<void(boost::system::error_code)>(
      [](auto state, auto* self, auto* const pdu, auto sequence_number, auto command_status) -> void
      {
        auto command_id = std::decay_t<decltype(*pdu)>::command_id;

        while (!self->send_buf_.empty()) // ongoing send operation
        {
          auto [ec] = co_await self->send_cv_.async_wait(deferred_tuple);
          if (ec != asio::error::operation_aborted || !!state.cancelled())
            co_return ec;
        }

        state.reset_cancellation_state(asio::enable_terminal_cancellation());

        auto ec = boost::system::error_code{};
        try
        {
          self->send_buf_.resize(header_length); // reserved for header
          serialize_to(&self->send_buf_, *pdu);
          auto header_buf = std::span<uint8_t, header_length>{ self->send_buf_ };
          detail::serialize_header(header_buf, self->send_buf_.size(), command_id, sequence_number, command_status);
          auto [wec, _] = co_await asio::async_write(self->socket_, asio::buffer(self->send_buf_), deferred_tuple);
          ec            = wec;
        }
        catch (const std::exception&)
        {
          ec = error::serialization_failed;
        }

        self->send_buf_.clear();
        self->send_cv_.cancel_one();
        co_return ec;
      },
      socket_),
    token,
    this,
    &pdu,
    sequence_number,
    command_status);
}

auto session::async_send_unbind(asio::completion_token_for<void(boost::system::error_code)> auto&& token)
{
  return async_send_command(command_id::unbind, next_sequence_number(), token);
}

auto session::async_receive(
  asio::completion_token_for<void(boost::system::error_code, pdu_variant, uint32_t, command_status)> auto&& token)
{
  return asio::async_initiate<decltype(token), void(boost::system::error_code, pdu_variant, uint32_t, command_status)>(
    asio::experimental::co_composed<void(boost::system::error_code, pdu_variant, uint32_t, command_status)>(
      [](auto state, auto* self) -> void
      {
        for (auto needs_more = false, needs_post = true, pending_enquire_link = false;;)
        {
          using enum command_id;

          if (needs_more)
          {
            self->enquire_link_timer_.expires_after(self->enquire_link_interval_);
            auto [order, receive_ec, received, timer_ec] =
              co_await asio::experimental::make_parallel_group(
                [&](auto token) { return self->socket_.async_receive(self->receive_buf_.prepare(64 * 1024), token); },
                [&](auto token) { return self->enquire_link_timer_.async_wait(token); })
                .async_wait(asio::experimental::wait_for_one(), deferred_tuple);

            needs_post = false;
            needs_more = false;
            self->receive_buf_.commit(received);

            if (order[0] == 0) // receive completed first
            {
              pending_enquire_link = false;
              if (receive_ec)
                co_return { receive_ec, {}, {}, {} };
            }
            else if (pending_enquire_link)
            {
              co_await self->async_send_command(unbind, self->next_sequence_number(), deferred_tuple);
              self->shutdown_socket();
              co_return { error::enquire_link_timeout, {}, {}, {} };
            }
            else
            {
              pending_enquire_link = true;
              co_await self->async_send_command(enquire_link, self->next_sequence_number(), deferred_tuple);
            }
          }

          if (self->receive_buf_.size() < header_length)
          {
            needs_more = true;
            continue;
          }

          auto header_buf = std::span<const uint8_t, header_length>{ self->receive_buf_ };
          auto [command_length, command_id, command_status, sequence_number] = detail::deserialize_header(header_buf);

          if (self->receive_buf_.size() < command_length)
          {
            needs_more = true;
            continue;
          }

          if (command_id == enquire_link)
          {
            co_await self->async_send_command(enquire_link_resp, sequence_number, deferred_tuple);
            self->receive_buf_.consume(command_length);
          }
          else if (command_id == enquire_link_resp)
          {
            self->receive_buf_.consume(command_length);
          }
          else if (command_id == unbind || command_id == unbind_resp)
          {
            if (command_id == unbind)
            {
              auto [ec] = co_await self->async_send_command(unbind_resp, sequence_number, deferred_tuple);
              if (ec)
                co_return { ec, {}, {}, {} };
            }
            self->shutdown_socket();
            self->receive_buf_.consume(command_length);
            co_return { error::unbinded, {}, {}, {} };
          }
          else
          {
            auto body_buf =
              std::span{ self->receive_buf_.begin() + header_length, self->receive_buf_.begin() + command_length };
            auto pdu = pdu_variant{};
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
              (std::make_index_sequence<std::variant_size_v<pdu_variant> - 1>()); // -1 because of invalid_pdu
            }
            catch (const std::exception& e)
            {
              pdu =
                invalid_pdu{ { self->receive_buf_.begin(), self->receive_buf_.begin() + command_length }, e.what() };
            }
            self->receive_buf_.consume(command_length);
            if (needs_post) // prevents stack growth
              co_await asio::post(state.get_io_executor(), asio::deferred);
            co_return { {}, std::move(pdu), sequence_number, command_status };
          }
        }
      },
      socket_),
    token,
    this);
}
} // namespace smpp
