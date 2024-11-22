// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common/command_id.hpp>
#include <smpp/common/command_status.hpp>

#include <span>

namespace smpp::detail
{
inline std::tuple<uint32_t, command_id, command_status, uint32_t>
deserialize_header(std::span<const uint8_t, 16> header_buf)
{
    auto deserialize_u32 = [](std::span<const uint8_t, 4> buf) -> uint32_t
    { return buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3]; };

    return { deserialize_u32(header_buf.subspan<0, 4>()),
             static_cast<smpp::command_id>(
                 deserialize_u32(header_buf.subspan<4, 4>())),
             static_cast<smpp::command_status>(
                 deserialize_u32(header_buf.subspan<8, 4>())),
             deserialize_u32(header_buf.subspan<12, 4>()) };
}

inline void
serialize_header(
    std::span<uint8_t, 16> header_buf,
    uint32_t command_length,
    command_id command_id,
    uint32_t sequence_number,
    command_status command_status = command_status::rok)
{
    auto serialize_u32 = [](std::span<uint8_t, 4> buf, uint32_t val)
    {
        buf[0] = (val >> 24) & 0xFF;
        buf[1] = (val >> 16) & 0xFF;
        buf[2] = (val >> 8) & 0xFF;
        buf[3] = (val >> 0) & 0xFF;
    };

    serialize_u32(header_buf.subspan<0, 4>(), command_length);
    serialize_u32(
        header_buf.subspan<4, 4>(), static_cast<uint32_t>(command_id));
    serialize_u32(
        header_buf.subspan<8, 4>(), static_cast<uint32_t>(command_status));
    serialize_u32(header_buf.subspan<12, 4>(), sequence_number);
}
} // namespace smpp::detail
