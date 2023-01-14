// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common.hpp>
#include <smpp/param.hpp>

namespace smpp
{
struct deliver_sm
{
  static constexpr auto command_id{ smpp::command_id::deliver_sm };

  std::string service_type;
  smpp::ton source_addr_ton{ ton::unknown };
  smpp::npi source_addr_npi{ npi::unknown };
  std::string source_addr;
  smpp::ton dest_addr_ton{ ton::unknown };
  smpp::npi dest_addr_npi{ npi::unknown };
  std::string dest_addr;
  smpp::esm_class esm_class;
  uint8_t protocol_id{};
  smpp::priority_flag priority_flag{ priority_flag::gsm_non_priority };
  std::string schedule_delivery_time;
  std::string validity_period;
  smpp::registered_delivery registered_delivery;
  smpp::replace_if_present_flag replace_if_present_flag{ replace_if_present_flag::no };
  smpp::data_coding data_coding{ data_coding::defaults };
  uint8_t sm_default_msg_id{};
  std::string short_message;
  smpp::oparam oparam;

  bool operator==(const deliver_sm&) const = default;
};

namespace detail
{
template<>
inline consteval auto pdu_meta<deliver_sm>()
{
  return std::tuple{ memv<c_octet_str<6>>(&deliver_sm::service_type, "service_type"),
                     memv<enum_u8>(&deliver_sm::source_addr_ton, "source_addr_ton"),
                     memv<enum_u8>(&deliver_sm::source_addr_npi, "source_addr_npi"),
                     memv<c_octet_str<21>>(&deliver_sm::source_addr, "source_addr"),
                     memv<enum_u8>(&deliver_sm::dest_addr_ton, "dest_addr_ton"),
                     memv<enum_u8>(&deliver_sm::dest_addr_npi, "dest_addr_npi"),
                     memv<c_octet_str<21>>(&deliver_sm::dest_addr, "dest_addr"),
                     memv<enum_flag>(&deliver_sm::esm_class, "esm_class"),
                     memv<u8>(&deliver_sm::protocol_id, "protocol_id"),
                     memv<enum_u8>(&deliver_sm::priority_flag, "priority_flag"),
                     memv<c_octet_str<1>>(&deliver_sm::schedule_delivery_time, "schedule_delivery_time"),
                     memv<c_octet_str<1>>(&deliver_sm::validity_period, "validity_period"),
                     memv<enum_flag>(&deliver_sm::registered_delivery, "registered_delivery"),
                     memv<enum_u8>(&deliver_sm::replace_if_present_flag, "replace_if_present_flag"),
                     memv<enum_u8>(&deliver_sm::data_coding, "data_coding"),
                     memv<u8>(&deliver_sm::sm_default_msg_id, "sm_default_msg_id"),
                     memv<u8_octet_str<254>>(&deliver_sm::short_message, "short_message"),
                     memv<smart>(&deliver_sm::oparam, "oparam") };
}
} // namespace detail
} // namespace smpp
