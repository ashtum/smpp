// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common.hpp>
#include <smpp/param.hpp>

namespace smpp
{
struct outbind
{
  static constexpr auto command_id{ smpp::command_id::outbind };

  std::string system_id{};
  std::string password{};

  bool operator==(const outbind&) const = default;
};

namespace detail
{
template<>
inline consteval auto pdu_meta<outbind>()
{
  return std::tuple{ mem<c_octet_str<16>>(&outbind::system_id, "system_id"),
                     mem<c_octet_str<9>>(&outbind::password, "password") };
}
} // namespace detail
} // namespace smpp
