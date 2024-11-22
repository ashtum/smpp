// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cinttypes>

namespace smpp
{
enum class priority_flag : uint8_t
{
    gsm_non_priority     = 0x00,
    gsm_priority         = 0x01,
    ansi_136_bulk        = 0x00,
    ansi_136_normal      = 0x01,
    ansi_136_urgent      = 0x02,
    ansi_136_very_urgent = 0x03,
    is_95_normal         = 0x00,
    is_95_interactive    = 0x01,
    is_95_urgent         = 0x02,
    is_95_emergency      = 0x03
};
} // namespace smpp
