// Copyright (c) 2022 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <smpp.hpp>

#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>

namespace asio = boost::asio;

BOOST_AUTO_TEST_SUITE(session)

BOOST_AUTO_TEST_CASE(async_send_and_receive)
{
  auto executed = 0;

  auto client = [&]() -> asio::awaitable<void>
  {
    auto executor = co_await asio::this_coro::executor;
    auto socket   = asio::ip::tcp::socket{ executor };
    co_await socket.async_connect({ asio::ip::tcp::v4(), 2775 }, asio::deferred);
    auto session = smpp::session{ std::move(socket) };

    {
      auto req_seq_num            = co_await session.async_send(smpp::submit_sm{}, asio::deferred);
      auto [pdu, seq_num, status] = co_await session.async_receive(asio::deferred);
      BOOST_CHECK(std::get<smpp::submit_sm_resp>(pdu) == smpp::submit_sm_resp{});
      BOOST_CHECK(status == smpp::command_status::rok);
      BOOST_CHECK_EQUAL(req_seq_num, seq_num);
      BOOST_CHECK(seq_num == 1);
    }
    {
      auto req_seq_num            = co_await session.async_send(smpp::deliver_sm{}, asio::deferred);
      auto [pdu, seq_num, status] = co_await session.async_receive(asio::deferred);
      BOOST_CHECK(std::get<smpp::deliver_sm_resp>(pdu) == smpp::deliver_sm_resp{});
      BOOST_CHECK(status == smpp::command_status::ralybnd);
      BOOST_CHECK_EQUAL(req_seq_num, seq_num);
      BOOST_CHECK(seq_num == 2);
    }
    {
      auto req_seq_num            = co_await session.async_send(smpp::bind_transceiver{}, asio::deferred);
      auto [pdu, seq_num, status] = co_await session.async_receive(asio::deferred);
      BOOST_CHECK(std::get<smpp::bind_transceiver_resp>(pdu) == smpp::bind_transceiver_resp{});
      BOOST_CHECK(status == smpp::command_status::rinvip);
      BOOST_CHECK_EQUAL(req_seq_num, seq_num);
      BOOST_CHECK(seq_num == 3);
    }

    executed++;
  };

  auto server = [&]() -> asio::awaitable<void>
  {
    auto executor = co_await asio::this_coro::executor;
    auto acceptor = asio::ip::tcp::acceptor(executor, { asio::ip::tcp::v4(), 2775 });
    auto session  = smpp::session{ co_await acceptor.async_accept(asio::deferred) };

    {
      auto [pdu, seq_num, status] = co_await session.async_receive(asio::deferred);
      BOOST_CHECK(std::get<smpp::submit_sm>(pdu) == smpp::submit_sm{});
      BOOST_CHECK(status == smpp::command_status::rok);
      BOOST_CHECK(seq_num == 1);
      co_await session.async_send(smpp::submit_sm_resp{}, seq_num, smpp::command_status::rok, asio::deferred);
    }
    {
      auto [pdu, seq_num, status] = co_await session.async_receive(asio::deferred);
      BOOST_CHECK(std::get<smpp::deliver_sm>(pdu) == smpp::deliver_sm{});
      BOOST_CHECK(status == smpp::command_status::rok);
      BOOST_CHECK(seq_num == 2);
      co_await session.async_send(smpp::deliver_sm_resp{}, seq_num, smpp::command_status::ralybnd, asio::deferred);
    }
    {
      auto [pdu, seq_num, status] = co_await session.async_receive(asio::deferred);
      BOOST_CHECK(std::get<smpp::bind_transceiver>(pdu) == smpp::bind_transceiver{});
      BOOST_CHECK(status == smpp::command_status::rok);
      BOOST_CHECK(seq_num == 3);
      co_await session.async_send(smpp::bind_transceiver_resp{}, seq_num, smpp::command_status::rinvip, asio::deferred);
    }

    executed++;
  };

  auto ctx = asio::io_context{};

  asio::co_spawn(ctx, server(), asio::detached);
  asio::co_spawn(ctx, client(), asio::detached);

  ctx.run_for(std::chrono::seconds{ 3 });
  BOOST_CHECK_EQUAL(executed, 2);
}

BOOST_AUTO_TEST_CASE(unbind)
{
  auto executed = 0;

  auto client = [&]() -> asio::awaitable<void>
  {
    auto executor = co_await asio::this_coro::executor;
    auto socket   = asio::ip::tcp::socket{ executor };
    co_await socket.async_connect({ asio::ip::tcp::v4(), 2775 }, asio::deferred);
    auto session = smpp::session{ std::move(socket) };

    co_await session.async_send_unbind(asio::deferred);

    try
    {
      co_await session.async_receive(asio::deferred);
    }
    catch (boost::system::system_error& e)
    {
      BOOST_CHECK(e.code() == smpp::error::unbinded);
    }

    executed++;
  };

  auto server = [&]() -> asio::awaitable<void>
  {
    auto executor = co_await asio::this_coro::executor;
    auto acceptor = asio::ip::tcp::acceptor(executor, { asio::ip::tcp::v4(), 2775 });
    auto session  = smpp::session{ co_await acceptor.async_accept(asio::deferred) };

    try
    {
      co_await session.async_receive(asio::deferred);
    }
    catch (boost::system::system_error& e)
    {
      BOOST_CHECK(e.code() == smpp::error::unbinded);
    }

    executed++;
  };

  auto ctx = asio::io_context{};

  asio::co_spawn(ctx, server(), asio::detached);
  asio::co_spawn(ctx, client(), asio::detached);

  ctx.run_for(std::chrono::seconds{ 3 });
  BOOST_CHECK_EQUAL(executed, 2);
}

BOOST_AUTO_TEST_CASE(enquire_link_timeout)
{
  auto executed = 0;

  auto client = [&]() -> asio::awaitable<void>
  {
    auto executor = co_await asio::this_coro::executor;
    auto socket   = asio::ip::tcp::socket{ executor };
    co_await socket.async_connect({ asio::ip::tcp::v4(), 2775 }, asio::deferred);
    auto session = smpp::session{ std::move(socket), std::chrono::seconds{ 1 } };

    try
    {
      co_await session.async_receive(asio::deferred);
    }
    catch (boost::system::system_error& e)
    {
      BOOST_CHECK(e.code() == smpp::error::enquire_link_timeout);
    }

    executed++;
  };

  auto server = [&]() -> asio::awaitable<void>
  {
    auto executor = co_await asio::this_coro::executor;
    auto acceptor = asio::ip::tcp::acceptor(executor, { asio::ip::tcp::v4(), 2775 });
    auto session  = smpp::session{ co_await acceptor.async_accept(asio::deferred) };
    auto timer    = asio::steady_timer{ executor, std::chrono::milliseconds{ 2500 } };

    co_await timer.async_wait(asio::deferred);

    executed++;
  };

  auto ctx = asio::io_context{};

  asio::co_spawn(ctx, server(), asio::detached);
  asio::co_spawn(ctx, client(), asio::detached);

  ctx.run_for(std::chrono::seconds{ 3 });
  BOOST_CHECK_EQUAL(executed, 2);
}

BOOST_AUTO_TEST_SUITE_END()
