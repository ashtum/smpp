// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common.hpp>
#include <smpp/param.hpp>

namespace smpp
{
struct data_sm
{
  static constexpr auto command_id{ smpp::command_id::data_sm };

  std::string service_type{};
  smpp::ton source_addr_ton{ ton::unknown };
  smpp::npi source_addr_npi{ npi::unknown };
  std::string source_addr{};
  smpp::ton dest_addr_ton{ ton::unknown };
  smpp::npi dest_addr_npi{ npi::unknown };
  std::string dest_addr{};
  smpp::esm_class esm_class{};
  smpp::registered_delivery registered_delivery;
  smpp::data_coding data_coding{ data_coding::defaults };
  smpp::oparam oparam{};

  bool operator==(const data_sm&) const = default;
};

namespace detail
{
template<>
inline consteval auto pdu_meta<data_sm>()
{
  return std::tuple{ memv<c_octet_str<6>>(&data_sm::service_type, "service_type"),
                     memv<enum_u8>(&data_sm::source_addr_ton, "source_addr_ton"),
                     memv<enum_u8>(&data_sm::source_addr_npi, "source_addr_npi"),
                     memv<c_octet_str<65>>(&data_sm::source_addr, "source_addr"),
                     memv<enum_u8>(&data_sm::dest_addr_ton, "dest_addr_ton"),
                     memv<enum_u8>(&data_sm::dest_addr_npi, "dest_addr_npi"),
                     memv<c_octet_str<65>>(&data_sm::dest_addr, "dest_addr"),
                     memv<enum_flag>(&data_sm::esm_class, "esm_class"),
                     memv<enum_flag>(&data_sm::registered_delivery, "registered_delivery"),
                     memv<enum_u8>(&data_sm::data_coding, "data_coding"),
                     memv<smart>(&data_sm::oparam, "oparam") };
}
} // namespace detail
} // namespace smpp
