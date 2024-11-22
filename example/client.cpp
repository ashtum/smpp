// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <smpp.hpp>

#include <boost/asio.hpp>

#include <iostream>

namespace asio = boost::asio;

asio::awaitable<void>
client()
{
    auto executor = co_await asio::this_coro::executor;
    auto endpoint = asio::ip::tcp::endpoint{ asio::ip::tcp::v4(), 2775 };
    auto timer    = asio::steady_timer{ executor };
    auto session  = smpp::session{ asio::ip::tcp::socket{ executor } };

    co_await session.next_layer().async_connect(endpoint);

    auto req = smpp::bind_transceiver{ .system_id = "client_01" };
    std::cout << "Sending bind_transceiver...\n";
    co_await session.async_send(req);

    auto [pdu, seq_num, status] = co_await session.async_receive();
    std::cout << "bind_transceiver_resp, system_id: "
              << get<smpp::bind_transceiver_resp>(pdu).system_id << '\n';

    for(auto i = 0; i < 3; i++)
    {
        auto req = smpp::submit_sm{ .dest_addr = std::to_string(1000 + i) };

        std::cout << "Sending submit_sm...\n";
        co_await session.async_send(req);

        auto [pdu, seq_num, status] = co_await session.async_receive();
        std::cout << "submit_sm_resp, message_id: "
                  << get<smpp::submit_sm_resp>(pdu).message_id << '\n';

        timer.expires_after(std::chrono::seconds{ 1 });
        co_await timer.async_wait();
    }

    std::cout << "Unbinding session...\n";
    co_await session.async_send_unbind();

    co_await session.async_receive();
}

int
main(int, const char**)
{
    try
    {
        auto ctx = asio::io_context{};

        asio::co_spawn(
            ctx,
            client(),
            [](auto eptr)
            {
                if(eptr)
                    std::rethrow_exception(eptr);
            });

        ctx.run();
    }
    catch(const std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
