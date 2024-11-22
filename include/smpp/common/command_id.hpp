// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cinttypes>

namespace smpp
{
enum class command_id : uint32_t
{
    generic_nack          = 0x80000000,
    bind_receiver         = 0x00000001,
    bind_receiver_resp    = 0x80000001,
    bind_transmitter      = 0x00000002,
    bind_transmitter_resp = 0x80000002,
    query_sm              = 0x00000003,
    query_sm_resp         = 0x80000003,
    submit_sm             = 0x00000004,
    submit_sm_resp        = 0x80000004,
    deliver_sm            = 0x00000005,
    deliver_sm_resp       = 0x80000005,
    unbind                = 0x00000006,
    unbind_resp           = 0x80000006,
    replace_sm            = 0x00000007,
    replace_sm_resp       = 0x80000007,
    cancel_sm             = 0x00000008,
    cancel_sm_resp        = 0x80000008,
    bind_transceiver      = 0x00000009,
    bind_transceiver_resp = 0x80000009,
    outbind               = 0x0000000b,
    enquire_link          = 0x00000015,
    enquire_link_resp     = 0x80000015,
    submit_multi          = 0x00000021,
    submit_multi_resp     = 0x80000021,
    alert_notification    = 0x00000102,
    data_sm               = 0x00000103,
    data_sm_resp          = 0x80000103
};
} // namespace smpp
