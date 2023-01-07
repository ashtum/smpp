// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cinttypes>

namespace smpp
{
enum class smsc_delivery_receipt : uint8_t
{
  no     = 0x00,
  both   = 0x01,
  failed = 0x02
};

enum class sme_originated_ack : uint8_t
{
  no           = 0x00,
  delivery_ack = 0x04,
  user_ack     = 0x08,
  both         = 0x0C
};

enum class intermediate_notification : uint8_t
{
  no        = 0x00,
  requested = 0x10
};

struct registered_delivery
{
  smpp::smsc_delivery_receipt smsc_delivery_receipt{ smsc_delivery_receipt::no };
  smpp::sme_originated_ack sme_originated_ack{ sme_originated_ack::no };
  smpp::intermediate_notification intermediate_notification{ intermediate_notification::no };

  bool operator==(const registered_delivery&) const = default;

  static registered_delivery from_u8(uint8_t value)
  {
    return { static_cast<smpp::smsc_delivery_receipt>(value & 0x03),
             static_cast<smpp::sme_originated_ack>(value & 0x0C),
             static_cast<smpp::intermediate_notification>(value & 0x10) };
  }

  explicit operator uint8_t() const
  {
    return static_cast<uint8_t>(smsc_delivery_receipt) | static_cast<uint8_t>(sme_originated_ack) |
           static_cast<uint8_t>(intermediate_notification);
  }
};
} // namespace smpp
