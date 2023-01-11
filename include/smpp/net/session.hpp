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
  std::chrono::seconds enquire_link_interval_{};
  uint32_t sequence_number_{};

  static std::exception_ptr eptr_from_ec(boost::system::error_code ec)
  {
    return std::make_exception_ptr(boost::system::system_error{ ec });
  }

  uint32_t next_sequence_number()
  {
    if (++sequence_number_ > 0x7FFFFFFF)
      sequence_number_ = 1;
    return sequence_number_;
  }

  auto async_send_command(command_id command_id, uint32_t sequence_number, auto&& token)
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

          self->send_buf_.resize(header_length); // reserved for header
          auto header_buf = std::span<uint8_t, header_length>{ self->send_buf_ };
          detail::serialize_header(header_buf, header_length, command_id, sequence_number);
          auto [ec, _] = co_await asio::async_write(self->socket_, asio::buffer(self->send_buf_), deferred_tuple);

          self->send_buf_.clear();
          self->send_cv_.cancel_one();
          co_return ec;
        }),
      token,
      this,
      command_id,
      sequence_number);
  }

public:
  session(asio::ip::tcp::socket socket, std::chrono::seconds enquire_link_interval = std::chrono::seconds{ 60 })
    : socket_(std::move(socket))
    , send_cv_{ socket_.get_executor(), asio::steady_timer::time_point::max() }
    , enquire_link_timer_{ socket_.get_executor() }
    , enquire_link_interval_{ enquire_link_interval }
  {
  }

  auto async_send(const request_pdu auto& pdu, auto&& token)
  {
    return asio::async_initiate<decltype(token), void(std::exception_ptr, uint32_t)>(
      asio::experimental::co_composed<void(std::exception_ptr, uint32_t)>(
        [](auto state, auto* self, auto* const pdu) -> void
        {
          auto command_id      = std::decay_t<decltype(*pdu)>::command_id;
          auto sequence_number = self->next_sequence_number();

          while (!self->send_buf_.empty()) // ongoing send operation
          {
            auto [ec] = co_await self->send_cv_.async_wait(deferred_tuple);
            if (ec != asio::error::operation_aborted || !!state.cancelled())
              co_return { eptr_from_ec(ec), {} };
          }

          auto eptr = std::exception_ptr{};
          try
          {
            self->send_buf_.resize(header_length); // reserved for header
            serialize_to(&self->send_buf_, *pdu);
            auto header_buf = std::span<uint8_t, header_length>{ self->send_buf_ };
            detail::serialize_header(header_buf, self->send_buf_.size(), command_id, sequence_number);
            co_await asio::async_write(self->socket_, asio::buffer(self->send_buf_), asio::deferred);
          }
          catch (const std::exception& ex) // from serialize_to and asio::async_write
          {
            eptr = std::current_exception();
          }

          self->send_buf_.clear();
          self->send_cv_.cancel_one();
          co_return { eptr, sequence_number };
        }),
      token,
      this,
      &pdu);
  }

  auto async_send(const response_pdu auto& pdu, uint32_t sequence_number, command_status command_status, auto&& token)
  {
    return asio::async_initiate<decltype(token), void(std::exception_ptr)>(
      asio::experimental::co_composed<void(std::exception_ptr)>(
        [](auto state, auto* self, auto* const pdu, auto sequence_number, auto command_status) -> void
        {
          auto command_id = std::decay_t<decltype(*pdu)>::command_id;

          while (!self->send_buf_.empty()) // ongoing send operation
          {
            auto [ec] = co_await self->send_cv_.async_wait(deferred_tuple);
            if (ec != asio::error::operation_aborted || !!state.cancelled())
              co_return eptr_from_ec(ec);
          }

          auto eptr = std::exception_ptr{};
          try
          {
            self->send_buf_.resize(header_length); // reserved for header
            serialize_to(&self->send_buf_, *pdu);
            auto header_buf = std::span<uint8_t, header_length>{ self->send_buf_ };
            detail::serialize_header(header_buf, self->send_buf_.size(), command_id, sequence_number, command_status);
            co_await asio::async_write(self->socket_, asio::buffer(self->send_buf_), asio::deferred);
          }
          catch (const std::exception& ex) // from serialize_to and asio::async_write
          {
            eptr = std::current_exception();
          }

          self->send_buf_.clear();
          self->send_cv_.cancel_one();
          co_return eptr;
        }),
      token,
      this,
      &pdu,
      sequence_number,
      command_status);
  }

  auto async_send_unbind(auto&& token)
  {
    return async_send_command(command_id::unbind, next_sequence_number(), token);
  }

  auto async_receive(auto&& token)
  {
    return asio::async_initiate<decltype(token), void(std::exception_ptr, pdu_variant, uint32_t, command_status)>(
      asio::experimental::co_composed<void(std::exception_ptr, pdu_variant, uint32_t, command_status)>(
        [](auto, auto* self) -> void
        {
          for (auto needs_more = false, pending_enquire_link = false;;)
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

              needs_more = false;
              self->receive_buf_.commit(received);

              if (order[0] == 0) // receive completed first
              {
                pending_enquire_link = false;
                if (receive_ec)
                  co_return { eptr_from_ec(receive_ec), {}, {}, {} };
              }
              else if (pending_enquire_link)
              {
                co_await self->async_send_command(unbind, self->next_sequence_number(), deferred_tuple);
                auto ec = boost::system::error_code{};
                self->socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
                self->socket_.close(ec);
                co_return { std::make_exception_ptr(inactivity_error{}), {}, {}, {} };
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

            auto [command_length, command_id, command_status, sequence_number] =
              detail::deserialize_header(std::span<const uint8_t, header_length>{ self->receive_buf_ });

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
                  co_return { eptr_from_ec(ec), {}, {}, {} };
              }
              auto ec = boost::system::error_code{};
              self->socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
              self->socket_.close(ec);
              self->receive_buf_.consume(command_length);
              co_return { std::make_exception_ptr(unbinded{}), {}, {}, {} };
            }
            else
            {
              auto body_buf =
                std::span{ self->receive_buf_.begin() + header_length, self->receive_buf_.begin() + command_length };
              auto eptr = std::exception_ptr{};
              auto pdu  = pdu_variant{};
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
                eptr = std::make_exception_ptr(deserialization_error{
                  ex.what(), { self->receive_buf_.begin(), self->receive_buf_.begin() + command_length } });
              }
              self->receive_buf_.consume(command_length);
              co_return { eptr, pdu, sequence_number, command_status };
            }
          }
        }),
      token,
      this);
  }
};
} // namespace smpp
