// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <smpp.hpp>

#include <boost/asio.hpp>
#include <fmt/core.h>

namespace asio = boost::asio;

asio::awaitable<void> handle_session(asio::ip::tcp::socket socket)
{
  try
  {
    auto session = smpp::session{ std::move(socket) };

    fmt::print("Waiting for bind request...\n");
    auto [pdu, seq_num, status] = co_await session.async_receive(asio::deferred);

    auto& bind_transceiver = std::get<smpp::bind_transceiver>(pdu);
    fmt::print("bind_transceiver is received, system_id: {}\n", bind_transceiver.system_id);

    fmt::print("Sending bind_transceiver_resp...\n");
    auto bind_transceiver_resp = smpp::bind_transceiver_resp{ .system_id = "sever_01" };
    co_await session.async_send(bind_transceiver_resp, seq_num, smpp::command_status::rok, asio::deferred);

    for (;;)
    {
      auto [pdu, seq_num, status] = co_await session.async_receive(asio::deferred);

      auto& submit_sm = std::get<smpp::submit_sm>(pdu);
      fmt::print("submit_sm received, dest_addr: {}\n", submit_sm.dest_addr);

      fmt::print("Sending submit_sm_resp...\n");
      auto submit_sm_resp = smpp::submit_sm_resp{ .message_id = "123" };
      co_await session.async_send(submit_sm_resp, seq_num, smpp::command_status::rok, asio::deferred);
    }
  }
  catch (const std::exception& e)
  {
    fmt::print("Exception: {}\n", e.what());
  }
}

asio::awaitable<void> acceptor()
{
  try
  {
    auto executor = co_await asio::this_coro::executor;
    auto acceptor = asio::ip::tcp::acceptor(executor, { asio::ip::tcp::v4(), 2775 });

    for (;;)
    {
      auto socket = co_await acceptor.async_accept(asio::deferred);
      asio::co_spawn(executor, handle_session(std::move(socket)), asio::detached);
    }
  }
  catch (const std::exception& e)
  {
    fmt::print("Exception in acceptor: {}\n", e.what());
  }
}

int main(int, const char**)
{
  auto ctx = asio::io_context{};

  asio::co_spawn(ctx, acceptor(), asio::detached);

  ctx.run();
}
