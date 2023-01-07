// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <smpp.hpp>

#include <boost/asio.hpp>
#include <fmt/core.h>

using pdu_request_t = std::variant<
  smpp::bind_receiver,
  smpp::bind_transceiver,
  smpp::bind_transmitter,
  smpp::query_sm,
  smpp::submit_sm,
  smpp::deliver_sm,
  smpp::replace_sm,
  smpp::cancel_sm,
  smpp::alert_notification,
  smpp::data_sm>;

using pdu_response_t = std::variant<
  smpp::generic_nack,
  smpp::bind_receiver_resp,
  smpp::bind_transceiver_resp,
  smpp::bind_transmitter_resp,
  smpp::query_sm_resp,
  smpp::submit_sm_resp,
  smpp::deliver_sm_resp,
  smpp::replace_sm_resp,
  smpp::cancel_sm_resp,
  smpp::data_sm_resp>;

auto sample_requests()
{
  std::vector<pdu_request_t> result;

  smpp::oparam sample_oparam;
  sample_oparam.set_as_string(smpp::oparam_tag::sar_total_segments, "TEST");
  sample_oparam.set_as_enum_u8(smpp::oparam_tag::message_state, smpp::message_state::expired);

  result.emplace_back(smpp::alert_notification{ .source_addr_ton = smpp::ton::abbreviated,
                                                .source_addr_npi = smpp::npi::telex,
                                                .source_addr     = "123",
                                                .esme_addr_ton   = smpp::ton::international,
                                                .esme_addr_npi   = smpp::npi::wapclient,
                                                .esme_addr       = "54321",
                                                .oparam          = sample_oparam });

  result.emplace_back(smpp::cancel_sm{ .service_type    = "HEIXY",
                                       .message_id      = "MOI123",
                                       .source_addr_ton = smpp::ton::national,
                                       .source_addr_npi = smpp::npi::privates,
                                       .source_addr     = "5463452",
                                       .dest_addr_ton   = smpp::ton::alphanumeric,
                                       .dest_addr_npi   = smpp::npi::e164,
                                       .dest_addr       = "934552" });

  result.emplace_back(smpp::data_sm{
    .service_type        = "FEYSF",
    .source_addr_ton     = smpp::ton::networkspecific,
    .source_addr_npi     = smpp::npi::ermes,
    .source_addr         = "95452343",
    .dest_addr_ton       = smpp::ton::national,
    .dest_addr_npi       = smpp::npi::telex,
    .dest_addr           = "576589434",
    .esm_class           = smpp::esm_class{ smpp::messaging_mode::forward,
                                  smpp::message_type::delivery_receipt,
                                  smpp::gsm_network_features::reply_path },
    .registered_delivery = smpp::registered_delivery{ smpp::smsc_delivery_receipt::failed,
                                                      smpp::sme_originated_ack::user_ack,
                                                      smpp::intermediate_notification::requested },
    .data_coding         = smpp::data_coding::iso2022_jp,
    .oparam              = {},
  });

  result.emplace_back(smpp::deliver_sm{
    .service_type            = "IWNM",
    .source_addr_ton         = smpp::ton::subscribernumber,
    .source_addr_npi         = smpp::npi::e212,
    .source_addr             = "UOI8745345",
    .dest_addr_ton           = smpp::ton::networkspecific,
    .dest_addr_npi           = smpp::npi::privates,
    .dest_addr               = "09877654",
    .esm_class               = smpp::esm_class{ smpp::messaging_mode::datagram,
                                  smpp::message_type::delivery_ack,
                                  smpp::gsm_network_features::uhdi },
    .protocol_id             = 5,
    .priority_flag           = smpp::priority_flag::ansi_136_urgent,
    .schedule_delivery_time  = "",
    .validity_period         = "",
    .registered_delivery     = smpp::registered_delivery{ smpp::smsc_delivery_receipt::both,
                                                      smpp::sme_originated_ack::delivery_ack,
                                                      smpp::intermediate_notification::no },
    .replace_if_present_flag = smpp::replace_if_present_flag::yes,
    .data_coding             = smpp::data_coding::iso8859_1,
    .sm_default_msg_id       = 4,
    .short_message           = "IUD&(*#IROUF&D(SUFSIFJ",
    .oparam                  = {},
  });

  result.emplace_back(smpp::query_sm{
    .message_id      = "IOPWEIU",
    .source_addr_ton = smpp::ton::networkspecific,
    .source_addr_npi = smpp::npi::e212,
    .source_addr     = "4543534253",
  });

  result.emplace_back(smpp::replace_sm{
    .message_id             = "435234MJDS53455345",
    .source_addr_ton        = smpp::ton::abbreviated,
    .source_addr_npi        = smpp::npi::wapclient,
    .source_addr            = "456462462",
    .schedule_delivery_time = "3452352345",
    .validity_period        = "5423323",
    .registered_delivery    = smpp::registered_delivery{ smpp::smsc_delivery_receipt::failed,
                                                      smpp::sme_originated_ack::both,
                                                      smpp::intermediate_notification::requested },
    .sm_default_msg_id      = 8,
    .short_message          = "sdasdase",
  });

  result.emplace_back(smpp::submit_sm{
    .service_type            = "IO",
    .source_addr_ton         = smpp::ton::subscribernumber,
    .source_addr_npi         = smpp::npi::e212,
    .source_addr             = "34234788234",
    .dest_addr_ton           = smpp::ton::subscribernumber,
    .dest_addr_npi           = smpp::npi::privates,
    .dest_addr               = "89778696747",
    .esm_class               = smpp::esm_class{ smpp::messaging_mode::store_and_forward,
                                  smpp::message_type::delivery_ack,
                                  smpp::gsm_network_features::uhdi },
    .protocol_id             = 5,
    .priority_flag           = smpp::priority_flag::ansi_136_urgent,
    .schedule_delivery_time  = "21342314324",
    .validity_period         = "43523534545",
    .registered_delivery     = smpp::registered_delivery{ smpp::smsc_delivery_receipt::failed,
                                                      smpp::sme_originated_ack::user_ack,
                                                      smpp::intermediate_notification::no },
    .replace_if_present_flag = smpp::replace_if_present_flag::no,
    .data_coding             = smpp::data_coding::jis,
    .sm_default_msg_id       = 4,
    .short_message           = "JFD:JU9458349gd;lfjgdfljg'dfgs'd",
    .oparam                  = {},
  });

  return result;
}

auto sample_responses()
{
  std::vector<pdu_response_t> result;

  result.emplace_back(smpp::cancel_sm_resp{});

  result.emplace_back(smpp::data_sm_resp{ .message_id = "JKYW0986", .oparam = {} });

  result.emplace_back(smpp::deliver_sm_resp{ .message_id = "" });

  result.emplace_back(smpp::generic_nack{});

  result.emplace_back(smpp::query_sm_resp{ .message_id    = "PKJDFL34523",
                                           .final_date    = "565464",
                                           .message_state = smpp::message_state::rejected,
                                           .error_code    = 147 });

  result.emplace_back(smpp::replace_sm_resp{});

  result.emplace_back(smpp::submit_sm_resp{ .message_id = "JSHDHSDA238904632" });

  return result;
}

int main()
{
  auto sent_requests  = sample_requests();
  auto sent_responses = sample_responses();

  std::vector<smpp::pdu_variant> received_requests;
  std::vector<smpp::pdu_variant> received_responses;

  auto server = [&]() -> asio::awaitable<void>
  {
    auto executor = co_await asio::this_coro::executor;
    auto acceptor = asio::ip::tcp::acceptor(executor, { asio::ip::tcp::v4(), 2775 });

    auto socket  = co_await acceptor.async_accept(asio::deferred);
    auto session = smpp::session{ std::move(socket) };

    for (;;)
    {
      auto [pdu, _, __] = co_await session.async_receive(asio::use_awaitable);
      received_requests.push_back(pdu);

      if (std::holds_alternative<smpp::submit_sm>(pdu)) // The last one
        break;
    }

    for (const auto& response : sent_responses)
      co_await std::visit(
        [&](const auto& pdu) { return session.async_send(pdu, 1, smpp::command_status::rok, asio::use_awaitable); },
        response);
  };

  auto client = [&]() -> asio::awaitable<void>
  {
    auto executor = co_await asio::this_coro::executor;
    auto socket   = asio::ip::tcp::socket{ executor };

    co_await socket.async_connect({ asio::ip::tcp::v4(), 2775 }, asio::deferred);
    auto session = smpp::session{ std::move(socket) };

    for (const auto& request : sent_requests)
      co_await std::visit([&](const auto& pdu) { return session.async_send(pdu, asio::use_awaitable); }, request);

    for (;;)
    {
      auto [pdu, _, __] = co_await session.async_receive(asio::use_awaitable);
      received_responses.push_back(pdu);

      if (std::holds_alternative<smpp::submit_sm_resp>(pdu)) // The last one
        break;
    }
  };

  auto ctx = asio::io_context{};

  asio::co_spawn(ctx, server(), asio::detached);
  asio::co_spawn(ctx, client(), asio::detached);

  ctx.run();

  if (received_requests.size() != sent_requests.size())
    throw std::runtime_error{ "sent and received requests sizes are not equal\n" };

  for (size_t i = 0; i < received_requests.size(); i++)
  {
    std::visit(
      [&](auto lhs, auto rhs)
      {
        if constexpr (std::is_same_v<decltype(lhs), decltype(rhs)>)
          if (lhs != rhs)
            throw std::runtime_error{ fmt::format("request_pdu comparison fail, index:{}\n", i) };
      },
      received_requests.at(i),
      sent_requests.at(i));
  }

  if (received_responses.size() != sent_responses.size())
    throw std::runtime_error{ "sent and received responses sizes are not equal\n" };

  for (size_t i = 0; i < received_responses.size(); i++)
  {
    std::visit(
      [&](auto lhs, auto rhs)
      {
        if constexpr (std::is_same_v<decltype(lhs), decltype(rhs)>)
          if (lhs != rhs)
            throw std::runtime_error{ fmt::format("response_pdu comparison fail, index:{}\n", i) };
      },
      received_responses.at(i),
      sent_responses.at(i));
  }
}
