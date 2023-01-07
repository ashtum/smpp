// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common.hpp>
#include <smpp/param.hpp>

namespace smpp
{
struct query_sm_resp
{
  static constexpr auto command_id{ smpp::command_id::query_sm_resp };

  std::string message_id{};
  std::string final_date{};
  smpp::message_state message_state{ message_state::unknown };
  uint8_t error_code{};

  bool operator==(const query_sm_resp&) const = default;
};

namespace detail
{
template<>
inline consteval auto pdu_meta<query_sm_resp>()
{
  return std::tuple{ memv<c_octet_str<65>>(&query_sm_resp::message_id, "message_id"),
                     memv<c_octet_str<17>>(&query_sm_resp::final_date, "final_date"),
                     memv<enum_u8>(&query_sm_resp::message_state, "message_state"),
                     memv<u8>(&query_sm_resp::error_code, "error_code") };
}
} // namespace detail
} // namespace smpp
