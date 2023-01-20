// Copyright (c) 2022 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <smpp/pdu.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(pdu)

BOOST_AUTO_TEST_CASE(serialize_deserialize)
{

  auto check = [](const auto& pdu)
  {
    using pdu_t = std::decay_t<decltype(pdu)>;

    std::vector<uint8_t> buf;

    smpp::serialize_to(&buf, pdu);

    auto deserialzied_pdu = smpp::deserialize<pdu_t>({ buf.begin(), buf.end() });

    BOOST_CHECK(pdu == deserialzied_pdu);
  };

  auto oparam = smpp::oparam{};
  oparam.set_as_string(smpp::oparam_tag::sar_total_segments, "HJKERTUDSDS");
  oparam.set_as_enum_u8(smpp::oparam_tag::message_state, smpp::message_state::expired);

  check(smpp::bind_transceiver{ .system_id         = "L::DSFKLS:F",
                                .password          = "984DJAS9",
                                .system_type       = "JSUHGD",
                                .interface_version = smpp::interface_version::smpp_3_4,
                                .addr_ton          = smpp::ton::alphanumeric,
                                .addr_npi          = smpp::npi::e212,
                                .address_range     = "LKJSDFH:FXMCN5465465464646464sfd54" });

  check(smpp::bind_transceiver_resp{ .system_id = "SDSSIERYHFMN", .oparam = oparam });

  check(smpp::bind_transmitter{ .system_id         = "L::DSFKLS:F",
                                .password          = "984DJAS9",
                                .system_type       = "JSUHGD",
                                .interface_version = smpp::interface_version::smpp_3_4,
                                .addr_ton          = smpp::ton::alphanumeric,
                                .addr_npi          = smpp::npi::e212,
                                .address_range     = "LKJSDFH:FXMCN5465465464646464sfd54" });

  check(smpp::bind_transmitter_resp{ .system_id = "SDSSIERYHFMN", .oparam = oparam });

  check(smpp::bind_receiver{ .system_id         = "L::DSFKLS:F",
                             .password          = "984DJAS9",
                             .system_type       = "JSUHGD",
                             .interface_version = smpp::interface_version::smpp_3_4,
                             .addr_ton          = smpp::ton::alphanumeric,
                             .addr_npi          = smpp::npi::e212,
                             .address_range     = "LKJSDFH:FXMCN5465465464646464sfd54" });

  check(smpp::bind_receiver_resp{ .system_id = "SDSSIERYHFMN", .oparam = oparam });

  check(smpp::alert_notification{ .source_addr_ton = smpp::ton::abbreviated,
                                  .source_addr_npi = smpp::npi::telex,
                                  .source_addr     = "123",
                                  .esme_addr_ton   = smpp::ton::international,
                                  .esme_addr_npi   = smpp::npi::wapclient,
                                  .esme_addr       = "54321",
                                  .oparam          = oparam });

  check(smpp::outbind{ .system_id = "HEIXY", .password = "MOI123" });

  check(smpp::cancel_sm{ .service_type    = "HEIXY",
                         .message_id      = "MOI123",
                         .source_addr_ton = smpp::ton::national,
                         .source_addr_npi = smpp::npi::privates,
                         .source_addr     = "5463452",
                         .dest_addr_ton   = smpp::ton::alphanumeric,
                         .dest_addr_npi   = smpp::npi::e164,
                         .dest_addr       = "934552" });

  check(smpp::data_sm{
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
    .oparam              = oparam,
  });

  check(smpp::deliver_sm{
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
    .oparam                  = oparam,
  });

  check(smpp::query_sm{
    .message_id      = "IOPWEIU",
    .source_addr_ton = smpp::ton::networkspecific,
    .source_addr_npi = smpp::npi::e212,
    .source_addr     = "4543534253",
  });

  check(smpp::replace_sm{
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

  check(smpp::submit_sm{
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
    .oparam                  = oparam,
  });

  check(smpp::cancel_sm_resp{});

  check(smpp::data_sm_resp{ .message_id = "JKYW0986", .oparam = oparam });

  check(smpp::deliver_sm_resp{ .message_id = "" });

  check(smpp::generic_nack{});

  check(smpp::query_sm_resp{ .message_id    = "PKJDFL34523",
                             .final_date    = "565464",
                             .message_state = smpp::message_state::rejected,
                             .error_code    = 147 });

  check(smpp::replace_sm_resp{});

  check(smpp::submit_sm_resp{ .message_id = "JSHDHSDA238904632" });
}

BOOST_AUTO_TEST_SUITE_END()
