// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common/command_id.hpp>

namespace smpp
{
inline bool constexpr is_response(command_id command_id)
{
    return static_cast<uint32_t>(command_id) & 0x80000000;
}
} // namespace smpp
