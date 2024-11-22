// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <smpp.hpp>

#include <boost/asio.hpp>

#include <iostream>

namespace asio = boost::asio;
namespace ch   = std::chrono;

asio::awaitable<void> client()
{
  auto exec   = co_await asio::this_coro::executor;
  auto socket = asio::ip::tcp::socket{ exec };

  co_await socket.async_connect({ asio::ip::tcp::v4(), 2775 });
  std::cout << "Connection complete, sending bind_transceiver..." << std::endl;

  auto session = smpp::session{ std::move(socket) };

  auto bind_transceiver = smpp::bind_transceiver{ .system_id = "client_01" };
  co_await session.async_send(bind_transceiver);

  auto [pdu, seq_num, status] = co_await session.async_receive();
  std::cout << "bind_transceiver_resp received, system_id: "
            << std::get<smpp::bind_transceiver_resp>(pdu).system_id
            << std::endl;

  for (auto i = 1; i <= 3; i++)
  {
    std::cout << "Sending submit_sm..." << std::endl;

    auto submit_sm = smpp::submit_sm{ .dest_addr = std::to_string(1000 + i) };
    co_await session.async_send(submit_sm);

    auto [pdu, seq_num, status] = co_await session.async_receive();
    std::cout << "submit_sm_resp received, message_id: "
              << std::get<smpp::submit_sm_resp>(pdu).message_id << std::endl;

    co_await asio::steady_timer{ exec, ch::seconds{ 3 } }.async_wait();
  }

  std::cout << "Unbinding session..." << std::endl;
  co_await session.async_send_unbind();

  co_await session.async_receive();
}

int main(int, const char**)
{
  try
  {
    auto ctx = asio::io_context{};

    asio::co_spawn(
      ctx,
      client(),
      [](auto eptr)
      {
        if (eptr)
          std::rethrow_exception(eptr);
      });

    ctx.run();
  }
  catch (const std::exception& e)
  {
    std::cout << "Exception: " << e.what() << std::endl;
  }
}
