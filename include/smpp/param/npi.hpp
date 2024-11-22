// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cinttypes>

namespace smpp
{
enum class npi : uint8_t
{
    unknown   = 0x00,
    e164      = 0x01,
    data      = 0x03,
    telex     = 0x04,
    e212      = 0x06,
    national  = 0x08,
    privates  = 0x09,
    ermes     = 0x0a,
    internet  = 0x0e,
    wapclient = 0x12
};
} // namespace smpp
