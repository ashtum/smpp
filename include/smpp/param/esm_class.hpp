// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cinttypes>

namespace smpp
{
enum class messaging_mode : uint8_t
{
  defaults          = 0x00,
  datagram          = 0x01,
  forward           = 0x02,
  store_and_forward = 0x03
};

enum class message_type : uint8_t
{
  defaults           = 0x00,
  delivery_receipt   = 0x04,
  delivery_ack       = 0x08,
  user_ack           = 0x10,
  conv_abort         = 0x18,
  int_delivery_notif = 0x20
};

enum class gsm_network_features : uint8_t
{
  no         = 0x00,
  uhdi       = 0x40,
  reply_path = 0x80,
  both       = 0xC0,
};

struct esm_class
{
  smpp::messaging_mode messaging_mode{ messaging_mode::defaults };
  smpp::message_type message_type{ message_type::defaults };
  smpp::gsm_network_features gsm_network_features{ gsm_network_features::no };

  bool operator==(const esm_class&) const = default;

  static esm_class from_u8(uint8_t value)
  {
    return { static_cast<smpp::messaging_mode>(value & 0x03),
             static_cast<smpp::message_type>(value & 0x3C),
             static_cast<smpp::gsm_network_features>(value & 0xC0) };
  }

  explicit operator uint8_t() const
  {
    return static_cast<uint8_t>(messaging_mode) | static_cast<uint8_t>(message_type) |
           static_cast<uint8_t>(gsm_network_features);
  }
};
} // namespace smpp
