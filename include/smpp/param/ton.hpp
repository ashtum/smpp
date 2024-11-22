// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cinttypes>

namespace smpp
{
enum class ton : uint8_t
{
    unknown          = 0x00,
    international    = 0x01,
    national         = 0x02,
    networkspecific  = 0x03,
    subscribernumber = 0x04,
    alphanumeric     = 0x05,
    abbreviated      = 0x06
};
} // namespace smpp
