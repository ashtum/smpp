// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common.hpp>
#include <smpp/param.hpp>

namespace smpp
{
struct bind_receiver
{
  static constexpr auto command_id{ smpp::command_id::bind_receiver };

  std::string system_id;
  std::string password;
  std::string system_type;
  smpp::interface_version interface_version{ interface_version::smpp_3_4 };
  smpp::ton addr_ton{ ton::unknown };
  smpp::npi addr_npi{ npi::unknown };
  std::string address_range;

  bool operator==(const bind_receiver&) const = default;
};

namespace detail
{
template<>
inline consteval auto pdu_meta<bind_receiver>()
{
  return std::tuple{ memv<c_octet_str<16>>(&bind_receiver::system_id, "system_id"),
                     memv<c_octet_str<9>>(&bind_receiver::password, "password"),
                     memv<c_octet_str<13>>(&bind_receiver::system_type, "system_type"),
                     memv<enum_u8>(&bind_receiver::interface_version, "interface_version"),
                     memv<enum_u8>(&bind_receiver::addr_ton, "addr_ton"),
                     memv<enum_u8>(&bind_receiver::addr_npi, "addr_npi"),
                     memv<c_octet_str<41>>(&bind_receiver::address_range, "address_range") };
}
} // namespace detail
} // namespace smpp
