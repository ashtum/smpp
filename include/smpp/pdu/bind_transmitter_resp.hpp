// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common.hpp>
#include <smpp/param.hpp>

namespace smpp
{
struct bind_transmitter_resp
{
    static constexpr auto command_id{ smpp::command_id::bind_transmitter_resp };

    std::string system_id{};
    smpp::oparam oparam{};

    bool
    operator==(const bind_transmitter_resp&) const = default;
};

namespace detail
{
template<>
inline consteval auto
pdu_meta<bind_transmitter_resp>()
{
    return std::tuple{ mem<c_octet_str<16>>(
                           &bind_transmitter_resp::system_id, "system_id"),
                       mem<smart>(&bind_transmitter_resp::oparam, "oparam") };
}
} // namespace detail
} // namespace smpp
