// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <smpp.hpp>

#include <boost/asio.hpp>

#include <iostream>

namespace asio = boost::asio;

asio::awaitable<void>
handle_session(smpp::session session)
{
    std::cout << "Waiting for bind request...\n";
    auto [pdu, seq_num, status] = co_await session.async_receive();

    std::cout << "bind_transceiver, system_id: "
              << get<smpp::bind_transceiver>(pdu).system_id << '\n';

    std::cout << "Sending bind_transceiver_resp...\n";
    auto req = smpp::bind_transceiver_resp{ .system_id = "server_01" };
    co_await session.async_send(req, seq_num, smpp::command_status::rok);

    for(;;)
    {
        auto [pdu, seq_num, status] = co_await session.async_receive();
        std::cout << "submit_sm, dest_addr: "
                  << get<smpp::submit_sm>(pdu).dest_addr << '\n';

        std::cout << "Sending submit_sm_resp...\n";
        auto resp = smpp::submit_sm_resp{ .message_id = "123" };
        co_await session.async_send(resp, seq_num, smpp::command_status::rok);
    }
}

asio::awaitable<void>
acceptor()
{
    auto executor = co_await asio::this_coro::executor;
    auto endpoint = asio::ip::tcp::endpoint{ asio::ip::tcp::v4(), 2775 };
    auto acceptor = asio::ip::tcp::acceptor(executor, endpoint);

    for(;;)
    {
        auto socket = co_await acceptor.async_accept();
        asio::co_spawn(
            executor,
            handle_session(std::move(socket)),
            [](auto eptr)
            {
                try
                {
                    if(eptr)
                        std::rethrow_exception(eptr);
                }
                catch(const std::exception& e)
                {
                    std::cerr << "Exception in session: " << e.what() << '\n';
                }
            });
    }
}

int
main(int, const char**)
{
    try
    {
        auto ctx = asio::io_context{};

        asio::co_spawn(
            ctx,
            acceptor(),
            [](auto eptr)
            {
                if(eptr)
                    std::rethrow_exception(eptr);
            });

        ctx.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << '\n';
    }
}
