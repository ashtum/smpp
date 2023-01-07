// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common.hpp>
#include <smpp/param.hpp>

namespace smpp
{
struct replace_sm
{
  static constexpr auto command_id{ smpp::command_id::replace_sm };

  std::string message_id{};
  smpp::ton source_addr_ton{ ton::unknown };
  smpp::npi source_addr_npi{ npi::unknown };
  std::string source_addr{};
  std::string schedule_delivery_time{};
  std::string validity_period{};
  smpp::registered_delivery registered_delivery{};
  uint8_t sm_default_msg_id{};
  std::string short_message{};

  bool operator==(const replace_sm&) const = default;
};

namespace detail
{
template<>
inline consteval auto pdu_meta<replace_sm>()
{
  return std::tuple{ memv<c_octet_str<65>>(&replace_sm::message_id, "message_id"),
                     memv<enum_u8>(&replace_sm::source_addr_ton, "source_addr_ton"),
                     memv<enum_u8>(&replace_sm::source_addr_npi, "source_addr_npi"),
                     memv<c_octet_str<21>>(&replace_sm::source_addr, "source_addr"),
                     memv<c_octet_str<17>>(&replace_sm::schedule_delivery_time, "schedule_delivery_time"),
                     memv<c_octet_str<17>>(&replace_sm::validity_period, "validity_period"),
                     memv<enum_flag>(&replace_sm::registered_delivery, "registered_delivery"),
                     memv<u8>(&replace_sm::sm_default_msg_id, "sm_default_msg_id"),
                     memv<u8_octet_str<254>>(&replace_sm::short_message, "short_message") };
}
} // namespace detail
} // namespace smpp
