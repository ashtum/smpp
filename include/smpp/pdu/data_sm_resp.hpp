// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common.hpp>
#include <smpp/param.hpp>

namespace smpp
{
struct data_sm_resp
{
  static constexpr auto command_id{ smpp::command_id::data_sm_resp };

  std::string message_id{};
  smpp::oparam oparam{};

  bool operator==(const data_sm_resp&) const = default;
};

namespace detail
{
template<>
inline consteval auto pdu_meta<data_sm_resp>()
{
  return std::tuple{ memv<c_octet_str<65>>(&data_sm_resp::message_id, "message_id"),
                     memv<smart>(&data_sm_resp::oparam, "oparam") };
}
} // namespace detail
} // namespace smpp
