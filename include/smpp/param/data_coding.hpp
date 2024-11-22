// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cinttypes>

namespace smpp
{
enum class data_coding : uint8_t
{
    defaults     = 0x00,
    ia5          = 0x01, // ia5 (ccitt t.50)/ascii (ansi x3.4)
    binary_alias = 0x02,
    iso8859_1    = 0x03, // latin 1
    binary       = 0x04,
    jis          = 0x05,
    iso8859_5    = 0x06, // cyrllic
    iso8859_8    = 0x07, // latin/hebrew
    ucs2         = 0x08, // ucs-2be (big endian)
    pictogram    = 0x09,
    iso2022_jp   = 0x0A, // music codes
    kanji        = 0x0D, // extended kanji jis
    ksc5601      = 0x0E
};
} // namespace smpp
