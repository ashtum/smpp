// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cinttypes>

namespace smpp
{
enum class message_state : uint8_t
{
  enroute       = 0x01,
  delivered     = 0x02,
  expired       = 0x03,
  deleted       = 0x04,
  undeliverable = 0x05,
  accepted      = 0x06,
  unknown       = 0x07,
  rejected      = 0x08
};
} // namespace smpp
