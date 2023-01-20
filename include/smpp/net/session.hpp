// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common/request_pdu.hpp>
#include <smpp/common/response_pdu.hpp>
#include <smpp/net/detail/static_flat_buffer.hpp>
#include <smpp/net/pdu_variant.hpp>

#include <boost/asio/as_tuple.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/ip/tcp.hpp>

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

public:
  /// Construct a session from a TCP socket
  /**
   * This constructor creates a session from a TCP socket.
   *
   * @param socket The asio::ip::tcp::socket that will be used for sending and receiving messages
   * @param enquire_link_interval The interval for detecting inactivity and enquire_link operation
   */
  explicit session(
    asio::ip::tcp::socket socket,
    std::chrono::seconds enquire_link_interval = std::chrono::seconds{ 60 });

  /// Start an asynchronous send for request PDUs
  /**
   * This function is used to asynchronously send a request PDU over the session.
   * It is an initiating function for an asynchronous_operation, and always returns immediately.
   *
   * @par Completion Signature
   * @code void(boost::system::error_code, uint32_t) @endcode
   * If the serialization of a PDU fails, operation completes with smpp::error::serialization_failed.
   * The boost::system::error_code can contains network errors and cancellation error.
   * uint32_t contains sequence_number and can be used to map the response on arrival.
   *
   * @par Per-Operation Cancellation
   * This asynchronous operation supports cancellation for the following asio::cancellation_type values:
   * @li cancellation_type::terminal
   *
   * @param pdu The request PDU
   * @param token The completion_token that will be used to produce a completion handler, which will be called when the
   * send completes
   */
  auto async_send(
    const request_pdu auto& pdu,
    asio::completion_token_for<void(boost::system::error_code, uint32_t)> auto&& token);

  /// Start an asynchronous send for response PDUs
  /**
   * This function is used to asynchronously send a response PDU over the session.
   * It is an initiating function for an asynchronous_operation, and always returns immediately.
   *
   * @par Completion Signature
   * @code void(boost::system::error_code) @endcode
   * If the serialization of a PDU fails, operation completes with smpp::error::serialization_failed.
   * The boost::system::error_code can contains network errors and cancellation error.
   *
   * @param pdu The response PDU
   * @param sequence_number The sequence_number of the request that this response belongs to
   * @param command_status The status of the response
   * @param token The completion_token that will be used to produce a completion handler, which will be called when the
   * send completes
   */
  auto async_send(
    const response_pdu auto& pdu,
    uint32_t sequence_number,
    command_status command_status,
    asio::completion_token_for<void(boost::system::error_code)> auto&& token);

  /// Start an asynchronous send for initiating unbind process
  /**
   * This function is used to asynchronously send an unbind request over the session.
   * It is an initiating function for an asynchronous_operation, and always returns immediately.
   *
   * For a graceful unbind process, after this operation completes you should continue using async_receive until it
   * completes with an error of smpp::error::unbinded, which means the peer has received unbind request and has
   * responded with an unbind_resp.
   *
   * @par Completion Signature
   * @code void(boost::system::error_code) @endcode
   * The boost::system::error_code can contains network errors and cancellation error.
   *
   * @par Per-Operation Cancellation
   * This asynchronous operation supports cancellation for the following asio::cancellation_type values:
   * @li cancellation_type::terminal
   *
   * @param token The completion_token that will be used to produce a completion handler, which will be called when the
   * send completes
   */
  auto async_send_unbind(asio::completion_token_for<void(boost::system::error_code)> auto&& token);

  /// Start an asynchronous receive
  /**
   * This function is used to asynchronously receive a PDU.
   * It is an initiating function for an asynchronous_operation, and always returns immediately.
   *
   * @par Completion Signature
   * @code void(boost::system::error_code, pdu_variant, uint32_t, command_status) @endcode
   * If the deserialization of a PDU fails, pdu_variant would contain invalid_pdu which contains the error and the buffer
   * of the PDU.
   * Upon a graceful unbind, operation completes with smpp::error::unbinded.
   * Upon an enquire_link timeout, operation completes with smpp::error::enquire_link_timeout.
   * The boost::system::error_code can contains network errors and cancellation error.
   *
   * @par Per-Operation Cancellation
   * This asynchronous operation supports cancellation for the following asio::cancellation_type values:
   * @li cancellation_type::terminal
   * @li cancellation_type::partial
   * @li cancellation_type::total
   *
   * @param token The completion_token that will be used to produce a completion handler, which will be called when the
   * receive completes
   */
  auto async_receive(
    asio::completion_token_for<void(boost::system::error_code, pdu_variant, uint32_t, command_status)> auto&& token);

private:
  uint32_t next_sequence_number();

  void shutdown_socket();

  auto async_send_command(
    command_id command_id,
    uint32_t sequence_number,
    asio::completion_token_for<void(boost::system::error_code)> auto&& token);
};
} // namespace smpp

#include <smpp/net/impl/session.hpp>
