// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <smpp.hpp>

#include <boost/asio.hpp>
#include <fmt/core.h>

asio::awaitable<void> client()
{
  try
  {
    auto executor = co_await asio::this_coro::executor;
    auto socket   = asio::ip::tcp::socket{ executor };

    co_await socket.async_connect({ asio::ip::tcp::v4(), 2775 }, asio::deferred);
    fmt::print("Connection complete\n");

    auto session = smpp::session{ std::move(socket) };

    fmt::print("Sending bind_transceiver...\n");
    auto bind_transceiver = smpp::bind_transceiver{ .system_id = "client_01" };
    co_await session.async_send(bind_transceiver, asio::use_awaitable);

    auto [pdu, seq_num, status] = co_await session.async_receive(asio::use_awaitable);
    auto& bind_transceiver_resp = std::get<smpp::bind_transceiver_resp>(pdu);
    fmt::print("bind_transceiver_resp received, system_id: {}\n", bind_transceiver_resp.system_id);

    for (auto i = 1; i <= 3; i++)
    {
      fmt::print("Sending submit_sm...\n");
      auto submit_sm = smpp::submit_sm{ .dest_addr = fmt::format("{}", 1000 + i) };
      co_await session.async_send(submit_sm, asio::use_awaitable);

      auto [pdu, seq_num, status] = co_await session.async_receive(asio::use_awaitable);
      auto& submit_sm_resp        = std::get<smpp::submit_sm_resp>(pdu);
      fmt::print("submit_sm_resp received, message_id: {}\n", submit_sm_resp.message_id);

      co_await asio::steady_timer{ executor, std::chrono::seconds{ 1 } }.async_wait(asio::deferred);
    }

    fmt::print("Unbinding session...\n");
    co_await session.async_send(smpp::unbind{}, asio::use_awaitable);

    co_await session.async_receive(asio::use_awaitable);

    fmt::print("Gracefully disconnected from server\n");
  }
  catch (std::exception const& e)
  {
    fmt::print("Exception in client: {}\n", e.what());
  }
}

int main(int, const char**)
{
  auto ctx = asio::io_context{};

  asio::co_spawn(ctx, client(), asio::detached);

  ctx.run();
}
