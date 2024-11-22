// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/param/data_coding.hpp>

#include <cinttypes>

namespace smpp
{
enum class data_coding_unicode
{
    ascii_7_bit,
    ascii_8_bit,
    binary,
    ucs2
};

inline data_coding_unicode
extract_unicode(data_coding data_coding)
{
    constexpr uint8_t coding_group_bits_mask = 0xf0;
    constexpr uint8_t alphabet_mask          = 0x0c;
    constexpr uint8_t data_coding_mask       = 0x0f;
    // constexpr uint8_t message_class_mask = 0x10;
    // constexpr uint8_t message_class_number_mask = 0x03;
    // constexpr uint8_t compression_bit_mask = 0x20;
    constexpr uint8_t reserved_group_0                 = 0x80;
    constexpr uint8_t reserved_group_1                 = 0x90;
    constexpr uint8_t reserved_group_2                 = 0xa0;
    constexpr uint8_t reserved_group_3                 = 0xb0;
    constexpr uint8_t default_alphabet                 = 0x00;
    constexpr uint8_t data_8_bit                       = 0x04;
    constexpr uint8_t ucs2                             = 0x08;
    constexpr uint8_t reserved                         = 0x0c;
    constexpr uint8_t general_data_coding_indication_0 = 0x00;
    constexpr uint8_t general_data_coding_indication_1 = 0x10;
    constexpr uint8_t general_data_coding_indication_2 = 0x20;
    constexpr uint8_t general_data_coding_indication_3 = 0x30;
    constexpr uint8_t automatic_deletion_group_0       = 0x40;
    constexpr uint8_t automatic_deletion_group_1       = 0x50;
    constexpr uint8_t automatic_deletion_group_2       = 0x60;
    constexpr uint8_t automatic_deletion_group_3       = 0x70;
    constexpr uint8_t mwi_group_discard_message        = 0xc0;
    constexpr uint8_t mwi_group_store_message_1        = 0xd0;
    constexpr uint8_t mwi_group_store_message_2        = 0xe0;
    constexpr uint8_t data_coding_message_class        = 0xf0;

    switch(static_cast<uint8_t>(data_coding) & coding_group_bits_mask)
    {
    case general_data_coding_indication_0:
    {
        switch(static_cast<uint8_t>(data_coding) & data_coding_mask)
        {
        // ASCII
        case 0x00:
        case 0x01:
        case 0x03:
        case 0x05:
        case 0x06:
        case 0x07:
        case 0x0D:
        case 0x0E:
        // reserved
        case 0x0B:
        case 0x0C:
        case 0x0F:
            return data_coding_unicode::ascii_8_bit;
        // Binary
        case 0x02:
        case 0x04:
        case 0x09:
        case 0x0A:
            return data_coding_unicode::binary;
        // usc2
        case 0x08:
            return data_coding_unicode::ucs2;
        default:
            return data_coding_unicode::ascii_7_bit;
        }
    }
    case general_data_coding_indication_1:
    case general_data_coding_indication_2:
    case general_data_coding_indication_3:
    case automatic_deletion_group_0:
    case automatic_deletion_group_1:
    case automatic_deletion_group_2:
    case automatic_deletion_group_3:
    {
        switch(static_cast<uint8_t>(data_coding) & alphabet_mask)
        {
        case data_8_bit:
            return data_coding_unicode::binary;
        case ucs2:
            return data_coding_unicode::ucs2;
        case default_alphabet:
        case reserved:
        default:
            return data_coding_unicode::ascii_7_bit;
        }
    }
    case reserved_group_0:
    case reserved_group_1:
    case reserved_group_2:
    case reserved_group_3:
    case mwi_group_discard_message:
    case mwi_group_store_message_1:
        return data_coding_unicode::ascii_7_bit;
    case mwi_group_store_message_2:
        return data_coding_unicode::ucs2;
    case data_coding_message_class:
        // Bit2 Message coding 0->Default alphabet 1->8-bit data
        if(static_cast<uint8_t>(data_coding) & 0x04)
            return data_coding_unicode::binary;
        else
            return data_coding_unicode::ascii_7_bit;
    default:
        return data_coding_unicode::ascii_7_bit;
    }
}
} // namespace smpp
