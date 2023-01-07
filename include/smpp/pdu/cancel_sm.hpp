// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common.hpp>
#include <smpp/param.hpp>

namespace smpp
{
struct cancel_sm
{
  static constexpr auto command_id{ smpp::command_id::cancel_sm };

  std::string service_type{};
  std::string message_id{};
  smpp::ton source_addr_ton{ ton::unknown };
  smpp::npi source_addr_npi{ npi::unknown };
  std::string source_addr{};
  smpp::ton dest_addr_ton{ ton::unknown };
  smpp::npi dest_addr_npi{ npi::unknown };
  std::string dest_addr{};

  bool operator==(const cancel_sm&) const = default;
};

namespace detail
{
template<>
inline consteval auto pdu_meta<cancel_sm>()
{
  return std::tuple{ memv<c_octet_str<6>>(&cancel_sm::service_type, "service_type"),
                     memv<c_octet_str<65>>(&cancel_sm::message_id, "message_id"),
                     memv<enum_u8>(&cancel_sm::source_addr_ton, "source_addr_ton"),
                     memv<enum_u8>(&cancel_sm::source_addr_npi, "source_addr_npi"),
                     memv<c_octet_str<21>>(&cancel_sm::source_addr, "source_addr"),
                     memv<enum_u8>(&cancel_sm::dest_addr_ton, "dest_addr_ton"),
                     memv<enum_u8>(&cancel_sm::dest_addr_npi, "dest_addr_npi"),
                     memv<c_octet_str<21>>(&cancel_sm::dest_addr, "dest_addr") };
}
} // namespace detail
} // namespace smpp
