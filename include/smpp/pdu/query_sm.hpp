// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common.hpp>
#include <smpp/param.hpp>

namespace smpp
{
struct query_sm
{
    static constexpr auto command_id{ smpp::command_id::query_sm };

    std::string message_id{};
    smpp::ton source_addr_ton{ ton::unknown };
    smpp::npi source_addr_npi{ npi::unknown };
    std::string source_addr{};

    bool
    operator==(const query_sm&) const = default;
};

namespace detail
{
template<>
inline consteval auto
pdu_meta<query_sm>()
{
    return std::tuple{
        mem<c_octet_str<65>>(&query_sm::message_id, "message_id"),
        mem<enum_u8>(&query_sm::source_addr_ton, "source_addr_ton"),
        mem<enum_u8>(&query_sm::source_addr_npi, "source_addr_npi"),
        mem<c_octet_str<21>>(&query_sm::source_addr, "source_addr")
    };
}
} // namespace detail
} // namespace smpp
