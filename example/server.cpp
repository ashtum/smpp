// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <smpp.hpp>

#include <boost/asio.hpp>

#include <iostream>

namespace asio = boost::asio;

asio::awaitable<void> handle_session(asio::ip::tcp::socket socket)
{
  auto session = smpp::session{ std::move(socket) };

  std::cout << "Waiting for bind request..." << std::endl;
  auto [pdu, seq_num, status] = co_await session.async_receive();

  std::cout << "bind_transceiver is received, system_id: "
            << std::get<smpp::bind_transceiver>(pdu).system_id << std::endl;

  std::cout << "Sending bind_transceiver_resp..." << std::endl;
  auto bind_transceiver_resp =
    smpp::bind_transceiver_resp{ .system_id = "sever_01" };
  co_await session.async_send(
    bind_transceiver_resp, seq_num, smpp::command_status::rok);

  for (;;)
  {
    auto [pdu, seq_num, status] = co_await session.async_receive();

    std::cout << "submit_sm received, dest_addr: "
              << std::get<smpp::submit_sm>(pdu).dest_addr << std::endl;

    std::cout << "Sending submit_sm_resp..." << std::endl;
    auto submit_sm_resp = smpp::submit_sm_resp{ .message_id = "123" };
    co_await session.async_send(
      submit_sm_resp, seq_num, smpp::command_status::rok);
  }
}

asio::awaitable<void> acceptor()
{
  auto executor = co_await asio::this_coro::executor;
  auto acceptor =
    asio::ip::tcp::acceptor(executor, { asio::ip::tcp::v4(), 2775 });

  for (;;)
  {
    auto socket = co_await acceptor.async_accept();
    asio::co_spawn(
      executor,
      handle_session(std::move(socket)),
      [](auto eptr)
      {
        try
        {
          if (eptr)
            std::rethrow_exception(eptr);
        }
        catch (const std::exception& e)
        {
          std::cout << "Exception in session: " << e.what() << std::endl;
        }
      });
  }
}

int main(int, const char**)
{
  try
  {
    auto ctx = asio::io_context{};

    asio::co_spawn(
      ctx,
      acceptor(),
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
