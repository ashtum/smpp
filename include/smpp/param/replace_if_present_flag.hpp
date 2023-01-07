// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cinttypes>

namespace smpp
{
enum class replace_if_present_flag : uint8_t
{
  no  = 0x00,
  yes = 0x01
};
} // namespace smpp
