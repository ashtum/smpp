// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common/request_pdu.hpp>
#include <smpp/common/response_pdu.hpp>
#include <smpp/common/serialization.hpp>
#include <smpp/net/detail/header_serialization.hpp>
#include <smpp/net/detail/static_flat_buffer.hpp>
#include <smpp/net/error.hpp>
#include <smpp/net/pdu_variant.hpp>
#include <smpp/net/session.hpp>

#include <boost/asio/cancel_after.hpp>
#include <boost/asio/compose.hpp>
#include <boost/asio/coroutine.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/write.hpp>

namespace smpp
{
namespace asio = boost::asio;
class session
{
    static constexpr auto header_length{ 16 };
    asio::ip::tcp::socket socket_;
    detail::static_flat_buffer<uint8_t, 128 * 1024> receive_buf_;
    std::vector<uint8_t> send_buf_;
    asio::steady_timer send_cv_;
    std::chrono::seconds enquire_link_interval_{};
    uint32_t sequence_number_{};

public:
    /// Construct a session from a TCP socket
    /**
     * This constructor creates a session from a TCP socket.
     *
     * @param socket The asio::ip::tcp::socket that will be used for sending and
     * receiving messages
     * @param enquire_link_interval The interval for detecting inactivity and
     * enquire_link operation
     */
    session(
        asio::ip::tcp::socket socket,
        std::chrono::seconds enquire_link_interval = std::chrono::seconds{
            60 });

    /// Return a reference to the next layer
    asio::ip::tcp::socket&
    next_layer() noexcept;

    /// Return a const reference to the next layer
    const asio::ip::tcp::socket&
    next_layer() const noexcept;

    /// Start an asynchronous send for request PDUs
    /**
     * This function is used to asynchronously send a request PDU over the
     * session. It is an initiating function for an asynchronous_operation, and
     * always returns immediately.
     *
     * @par Completion Signature
     * @code void(boost::system::error_code, uint32_t) @endcode
     * If the serialization of a PDU fails, operation completes with
     * smpp::error::serialization_failed. The boost::system::error_code can
     * contains network errors and cancellation error. uint32_t contains
     * sequence_number and can be used to map the response on arrival.
     *
     * @par Per-Operation Cancellation
     * This asynchronous operation supports cancellation for the following
     * asio::cancellation_type values:
     * @li cancellation_type::terminal
     *
     * @param pdu The request PDU
     * @param token The completion_token that will be used to produce a
     * completion handler, which will be called when the send completes
     */
    template<
        asio::completion_token_for<void(boost::system::error_code, uint32_t)>
            CompletionToken = asio::deferred_t>
    auto
    async_send(
        const request_pdu auto& pdu,
        CompletionToken&& token = asio::deferred_t{});

    /// Start an asynchronous send for response PDUs
    /**
     * This function is used to asynchronously send a response PDU over the
     * session. It is an initiating function for an asynchronous_operation, and
     * always returns immediately.
     *
     * @par Completion Signature
     * @code void(boost::system::error_code) @endcode
     * If the serialization of a PDU fails, operation completes with
     * smpp::error::serialization_failed. The boost::system::error_code can
     * contains network errors and cancellation error.
     *
     * @param pdu The response PDU
     * @param sequence_number The sequence_number of the request that this
     * response belongs to
     * @param command_status The status of the response
     * @param token The completion_token that will be used to produce a
     * completion handler, which will be called when the send completes
     */
    template<
        asio::completion_token_for<void(boost::system::error_code)>
            CompletionToken = asio::deferred_t>
    auto
    async_send(
        const response_pdu auto& pdu,
        uint32_t sequence_number,
        command_status command_status,
        CompletionToken&& token = asio::deferred_t{});

    /// Start an asynchronous send for initiating unbind process
    /**
     * This function is used to asynchronously send an unbind request over the
     * session. It is an initiating function for an asynchronous_operation, and
     * always returns immediately.
     *
     * For a graceful unbind process, after this operation completes you should
     * continue using async_receive until it completes with an error of
     * smpp::error::unbinded, which means the peer has received unbind request
     * and has responded with an unbind_resp.
     *
     * @par Completion Signature
     * @code void(boost::system::error_code) @endcode
     * The boost::system::error_code can contains network errors and
     * cancellation error.
     *
     * @par Per-Operation Cancellation
     * This asynchronous operation supports cancellation for the following
     * asio::cancellation_type values:
     * @li cancellation_type::terminal
     *
     * @param token The completion_token that will be used to produce a
     * completion handler, which will be called when the send completes
     */
    template<
        asio::completion_token_for<void(boost::system::error_code)>
            CompletionToken = asio::deferred_t>
    auto
    async_send_unbind(CompletionToken&& token = asio::deferred_t{});

    /// Start an asynchronous receive
    /**
     * This function is used to asynchronously receive a PDU.
     * It is an initiating function for an asynchronous_operation, and always
     * returns immediately.
     *
     * @par Completion Signature
     * @code void(boost::system::error_code, pdu_variant, uint32_t,
     * command_status) @endcode If the deserialization of a PDU fails,
     * pdu_variant would contain smpp::invalid_pdu which contains the error and
     * the buffer of the PDU. Upon a graceful unbind, operation completes with
     * smpp::error::unbinded. Upon an enquire_link timeout, operation completes
     * with smpp::error::enquire_link_timeout. The boost::system::error_code can
     * contains network errors and cancellation error.
     *
     * @par Per-Operation Cancellation
     * This asynchronous operation supports cancellation for the following
     * asio::cancellation_type values:
     * @li cancellation_type::terminal
     * @li cancellation_type::partial
     * @li cancellation_type::total
     *
     * @param token The completion_token that will be used to produce a
     * completion handler, which will be called when the receive completes
     */
    template<
        asio::completion_token_for<void(
            boost::system::error_code,
            pdu_variant,
            uint32_t,
            command_status)> CompletionToken = asio::deferred_t>
    auto
    async_receive(CompletionToken&& token = asio::deferred_t{});

private:
    uint32_t
    next_sequence_number();

    void
    shutdown_socket();

    auto
    async_send_command(
        command_id command_id,
        uint32_t sequence_number,
        asio::completion_token_for<void(boost::system::error_code)> auto&&
            token);

    class receive_op;
};

inline session::session(
    asio::ip::tcp::socket socket,
    std::chrono::seconds enquire_link_interval)
    : socket_(std::move(socket))
    , send_cv_{ socket_.get_executor(), asio::steady_timer::time_point::max() }
    , enquire_link_interval_{ enquire_link_interval }
{
}

inline asio::ip::tcp::socket&
session::next_layer() noexcept
{
    return socket_;
}

inline const asio::ip::tcp::socket&
session::next_layer() const noexcept
{
    return socket_;
}

inline uint32_t
session::next_sequence_number()
{
    if(++sequence_number_ > 0x7FFFFFFF)
        sequence_number_ = 1;
    return sequence_number_;
}

inline void
session::shutdown_socket()
{
    auto ec = boost::system::error_code{};
    socket_.shutdown(asio::ip::tcp::socket::shutdown_both, ec);
    socket_.close(ec);
}

auto
session::async_send_command(
    command_id command_id,
    uint32_t sequence_number,
    asio::completion_token_for<void(boost::system::error_code)> auto&& token)
{
    return asio::
        async_compose<decltype(token), void(boost::system::error_code)>(
            [this, command_id, sequence_number, c = asio::coroutine{}](
                auto&& self,
                boost::system::error_code ec = {},
                std::size_t                  = {}) mutable
            {
                BOOST_ASIO_CORO_REENTER(c)
                {
                    self.reset_cancellation_state(
                        asio::enable_total_cancellation());

                    while(!send_buf_.empty()) // ongoing send operation
                    {
                        BOOST_ASIO_CORO_YIELD
                        send_cv_.async_wait(std::move(self));
                        if(ec != asio::error::operation_aborted ||
                           !!self.cancelled())
                            return self.complete(ec);
                    }

                    send_buf_.resize(header_length); // reserved for header
                    detail::serialize_header(
                        std::span<uint8_t, header_length>{ send_buf_ },
                        header_length,
                        command_id,
                        sequence_number);

                    self.reset_cancellation_state(
                        asio::enable_terminal_cancellation());

                    BOOST_ASIO_CORO_YIELD
                    asio::async_write(
                        socket_,
                        asio::buffer(send_buf_),
                        asio::cancel_after(
                            enquire_link_interval_, std::move(self)));

                    if(ec == asio::error::operation_aborted &&
                       !self.cancelled())
                        return self.complete(error::enquire_link_timeout);

                    send_buf_.clear();
                    send_cv_.cancel_one();
                    self.complete(ec);
                }
            },
            token,
            socket_);
}

template<asio::completion_token_for<void(boost::system::error_code, uint32_t)>
             CompletionToken>
auto
session::async_send(const request_pdu auto& pdu, CompletionToken&& token)
{
    return asio::async_compose<
        decltype(token),
        void(boost::system::error_code, uint32_t)>(
        [this, &pdu, sequence_number = uint32_t{}, c = asio::coroutine{}](
            auto&& self,
            boost::system::error_code ec = {},
            std::size_t                  = {}) mutable
        {
            BOOST_ASIO_CORO_REENTER(c)
            {
                self.reset_cancellation_state(
                    asio::enable_total_cancellation());

                while(!send_buf_.empty()) // ongoing send operation
                {
                    BOOST_ASIO_CORO_YIELD
                    send_cv_.async_wait(std::move(self));
                    if(ec != asio::error::operation_aborted ||
                       !!self.cancelled())
                        return self.complete(ec, {});
                }

                send_buf_.resize(header_length); // reserved for header
                try
                {
                    serialize_to(&send_buf_, pdu);
                }
                catch(const std::exception&)
                {
                    send_buf_.clear();
                    return self.complete(error::serialization_failed, {});
                }
                sequence_number = next_sequence_number();
                detail::serialize_header(
                    std::span<uint8_t, header_length>{ send_buf_ },
                    send_buf_.size(),
                    std::decay_t<decltype(pdu)>::command_id,
                    sequence_number);

                self.reset_cancellation_state(
                    asio::enable_terminal_cancellation());

                BOOST_ASIO_CORO_YIELD
                asio::async_write(
                    socket_, asio::buffer(send_buf_), std::move(self));

                send_buf_.clear();
                send_cv_.cancel_one();
                self.complete(ec, sequence_number);
            }
        },
        token,
        socket_);
}

template<
    asio::completion_token_for<void(boost::system::error_code)> CompletionToken>
auto
session::async_send(
    const response_pdu auto& pdu,
    uint32_t sequence_number,
    command_status command_status,
    CompletionToken&& token)
{
    return asio::async_compose<
        decltype(token),
        void(boost::system::error_code)>(
        [this, &pdu, sequence_number, command_status, c = asio::coroutine{}](
            auto&& self,
            boost::system::error_code ec = {},
            std::size_t                  = {}) mutable
        {
            BOOST_ASIO_CORO_REENTER(c)
            {
                self.reset_cancellation_state(
                    asio::enable_total_cancellation());

                while(!send_buf_.empty()) // ongoing send operation
                {
                    BOOST_ASIO_CORO_YIELD
                    send_cv_.async_wait(std::move(self));
                    if(ec != asio::error::operation_aborted ||
                       !!self.cancelled())
                        return self.complete(ec);
                }

                send_buf_.resize(header_length); // reserved for header
                try
                {
                    serialize_to(&send_buf_, pdu);
                }
                catch(const std::exception&)
                {
                    send_buf_.clear();
                    return self.complete(error::serialization_failed);
                }
                detail::serialize_header(
                    std::span<uint8_t, header_length>{ send_buf_ },
                    send_buf_.size(),
                    std::decay_t<decltype(pdu)>::command_id,
                    sequence_number,
                    command_status);

                self.reset_cancellation_state(
                    asio::enable_terminal_cancellation());

                BOOST_ASIO_CORO_YIELD
                asio::async_write(
                    socket_, asio::buffer(send_buf_), std::move(self));

                send_buf_.clear();
                send_cv_.cancel_one();
                self.complete(ec);
            }
        },
        token,
        socket_);
}

template<
    asio::completion_token_for<void(boost::system::error_code)> CompletionToken>
auto
session::async_send_unbind(CompletionToken&& token)
{
    return async_send_command(
        command_id::unbind,
        next_sequence_number(),
        std::forward<decltype(token)>(token));
}

class session::receive_op
{
    session* s_;
    asio::coroutine c_;
    uint32_t command_length_       = {};
    command_id command_id_         = {};
    command_status command_status_ = {};
    uint32_t sequence_number_      = {};
    bool needs_more_               = false;
    bool needs_post_               = true;
    bool pending_enquire_link_     = false;

public:
    explicit receive_op(session* s)
        : s_{ s }
    {
    }

    void
    operator()(
        auto&& self,
        boost::system::error_code ec = {},
        std::size_t received         = {})
    {
        BOOST_ASIO_CORO_REENTER(c_)
        for(;;)
        {
            using enum smpp::command_id;

            if(needs_more_)
            {
                needs_more_ = false;
                needs_post_ = false;
                self.reset_cancellation_state(
                    asio::enable_total_cancellation());

                BOOST_ASIO_CORO_YIELD
                s_->socket_.async_read_some(
                    s_->receive_buf_.prepare(
                        s_->receive_buf_.capacity() - s_->receive_buf_.size()),
                    asio::cancel_after(
                        s_->enquire_link_interval_,
                        asio::cancellation_type::total,
                        std::move(self)));

                if(received != 0)
                {
                    pending_enquire_link_ = false;
                    s_->receive_buf_.commit(received);
                }

                // enquire_link timeout
                if(ec == asio::error::operation_aborted && !self.cancelled())
                {
                    if(pending_enquire_link_)
                    {
                        BOOST_ASIO_CORO_YIELD
                        s_->async_send_command(
                            unbind,
                            s_->next_sequence_number(),
                            std::move(self));
                        s_->shutdown_socket();
                        return self.complete(
                            error::enquire_link_timeout, {}, {}, {});
                    }
                    pending_enquire_link_ = true;
                    BOOST_ASIO_CORO_YIELD
                    s_->async_send_command(
                        enquire_link,
                        s_->next_sequence_number(),
                        std::move(self));
                }

                if(ec)
                    return self.complete(ec, {}, {}, {});
            }

            if(s_->receive_buf_.size() < header_length)
            {
                needs_more_ = true;
                continue;
            }

            std::tie(
                command_length_,
                command_id_,
                command_status_,
                sequence_number_) =
                detail::deserialize_header(
                    std::span<const uint8_t, header_length>{
                        s_->receive_buf_ });

            if(s_->receive_buf_.size() < command_length_)
            {
                needs_more_ = true;
                continue;
            }

            if(command_id_ == enquire_link)
            {
                BOOST_ASIO_CORO_YIELD
                s_->async_send_command(
                    enquire_link_resp, sequence_number_, std::move(self));
                s_->receive_buf_.consume(command_length_);
            }
            else if(command_id_ == enquire_link_resp)
            {
                s_->receive_buf_.consume(command_length_);
            }
            else if(command_id_ == unbind || command_id_ == unbind_resp)
            {
                if(command_id_ == unbind)
                {
                    BOOST_ASIO_CORO_YIELD
                    s_->async_send_command(
                        unbind_resp, sequence_number_, std::move(self));
                    if(ec)
                        return self.complete(ec, {}, {}, {});
                }
                s_->shutdown_socket();
                s_->receive_buf_.consume(command_length_);
                return self.complete(error::unbinded, {}, {}, {});
            }
            else
            {
                if(needs_post_) // prevents stack growth
                {
                    BOOST_ASIO_CORO_YIELD
                    asio::post(std::move(self));
                }

                auto body_buf =
                    std::span{ s_->receive_buf_.begin() + header_length,
                               s_->receive_buf_.begin() + command_length_ };
                auto pdu = pdu_variant{};
                try
                {
                    [&]<std::size_t... Is>(std::index_sequence<Is...>)
                    {
                        if(!((command_id_ ==
                                      std::decay_t<decltype(std::get<Is>(
                                          pdu_variant{}))>::command_id
                                  ? (pdu = deserialize<
                                         std::decay_t<decltype(std::get<Is>(
                                             pdu_variant{}))>>(body_buf),
                                     true)
                                  : false) ||
                             ...))
                            throw std::logic_error{ "Unknown PDU" };
                    }(std::make_index_sequence<
                        std::variant_size_v<pdu_variant> -
                        1>()); // -1 because of invalid_pdu
                }
                catch(const std::exception& e)
                {
                    pdu = invalid_pdu{ { s_->receive_buf_.begin(),
                                         s_->receive_buf_.begin() +
                                             command_length_ },
                                       e.what() };
                }
                s_->receive_buf_.consume(command_length_);
                return self.complete(
                    {}, std::move(pdu), sequence_number_, command_status_);
            }
        }
    }
};

template<asio::completion_token_for<
    void(boost::system::error_code, pdu_variant, uint32_t, command_status)>
             CompletionToken>
auto
session::async_receive(CompletionToken&& token)
{
    return asio::async_compose<
        decltype(token),
        void(boost::system::error_code, pdu_variant, uint32_t, command_status)>(
        receive_op{ this }, token, socket_);
}
} // namespace smpp
