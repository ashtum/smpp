// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common.hpp>
#include <smpp/param.hpp>

namespace smpp
{
struct alert_notification
{
  static constexpr auto command_id{ smpp::command_id::alert_notification };

  smpp::ton source_addr_ton{ ton::unknown };
  smpp::npi source_addr_npi{ npi::unknown };
  std::string source_addr{};
  smpp::ton esme_addr_ton{ ton::unknown };
  smpp::npi esme_addr_npi{ npi::unknown };
  std::string esme_addr{};
  smpp::oparam oparam{};

  bool operator==(const alert_notification&) const = default;
};

namespace detail
{
template<>
inline consteval auto pdu_meta<alert_notification>()
{
  return std::tuple{ mem<enum_u8>(&alert_notification::source_addr_ton, "source_addr_ton"),
                     mem<enum_u8>(&alert_notification::source_addr_npi, "source_addr_npi"),
                     mem<c_octet_str<65>>(&alert_notification::source_addr, "source_addr"),
                     mem<enum_u8>(&alert_notification::esme_addr_ton, "esme_addr_ton"),
                     mem<enum_u8>(&alert_notification::esme_addr_npi, "esme_addr_npi"),
                     mem<c_octet_str<65>>(&alert_notification::esme_addr, "esme_addr"),
                     mem<smart>(&alert_notification::oparam, "oparam") };
}
} // namespace detail
} // namespace smpp
