// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common.hpp>
#include <smpp/param.hpp>

namespace smpp
{
struct bind_transmitter
{
  static constexpr auto command_id{ smpp::command_id::bind_transmitter };

  std::string system_id{};
  std::string password{};
  std::string system_type{};
  smpp::interface_version interface_version{ interface_version::smpp_3_4 };
  smpp::ton addr_ton{ ton::unknown };
  smpp::npi addr_npi{ npi::unknown };
  std::string address_range{};

  bool operator==(const bind_transmitter&) const = default;
};

namespace detail
{
template<>
inline consteval auto pdu_meta<bind_transmitter>()
{
  return std::tuple{ mem<c_octet_str<16>>(&bind_transmitter::system_id, "system_id"),
                     mem<c_octet_str<9>>(&bind_transmitter::password, "password"),
                     mem<c_octet_str<13>>(&bind_transmitter::system_type, "system_type"),
                     mem<enum_u8>(&bind_transmitter::interface_version, "interface_version"),
                     mem<enum_u8>(&bind_transmitter::addr_ton, "addr_ton"),
                     mem<enum_u8>(&bind_transmitter::addr_npi, "addr_npi"),
                     mem<c_octet_str<41>>(&bind_transmitter::address_range, "address_range") };
}
} // namespace detail
} // namespace smpp
