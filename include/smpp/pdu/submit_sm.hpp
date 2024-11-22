// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common.hpp>
#include <smpp/param.hpp>

namespace smpp
{
struct submit_sm
{
    static constexpr auto command_id{ smpp::command_id::submit_sm };

    std::string service_type{};
    smpp::ton source_addr_ton{ ton::unknown };
    smpp::npi source_addr_npi{ npi::unknown };
    std::string source_addr{};
    smpp::ton dest_addr_ton{ ton::unknown };
    smpp::npi dest_addr_npi{ npi::unknown };
    std::string dest_addr{};
    smpp::esm_class esm_class{};
    uint8_t protocol_id{};
    smpp::priority_flag priority_flag{ priority_flag::gsm_non_priority };
    std::string schedule_delivery_time{};
    std::string validity_period{};
    smpp::registered_delivery registered_delivery{};
    smpp::replace_if_present_flag replace_if_present_flag{
        replace_if_present_flag::no
    };
    smpp::data_coding data_coding{ data_coding::defaults };
    uint8_t sm_default_msg_id{};
    std::string short_message{};
    smpp::oparam oparam{};

    bool
    operator==(const submit_sm&) const = default;
};

namespace detail
{
template<>
inline consteval auto
pdu_meta<submit_sm>()
{
    return std::tuple{
        mem<c_octet_str<6>>(&submit_sm::service_type, "service_type"),
        mem<enum_u8>(&submit_sm::source_addr_ton, "source_addr_ton"),
        mem<enum_u8>(&submit_sm::source_addr_npi, "source_addr_npi"),
        mem<c_octet_str<21>>(&submit_sm::source_addr, "source_addr"),
        mem<enum_u8>(&submit_sm::dest_addr_ton, "dest_addr_ton"),
        mem<enum_u8>(&submit_sm::dest_addr_npi, "dest_addr_npi"),
        mem<c_octet_str<21>>(&submit_sm::dest_addr, "dest_addr"),
        mem<enum_flag>(&submit_sm::esm_class, "esm_class"),
        mem<u8>(&submit_sm::protocol_id, "protocol_id"),
        mem<enum_u8>(&submit_sm::priority_flag, "priority_flag"),
        mem<c_octet_str<17>>(
            &submit_sm::schedule_delivery_time, "schedule_delivery_time"),
        mem<c_octet_str<17>>(&submit_sm::validity_period, "validity_period"),
        mem<enum_flag>(&submit_sm::registered_delivery, "registered_delivery"),
        mem<enum_u8>(
            &submit_sm::replace_if_present_flag, "replace_if_present_flag"),
        mem<enum_u8>(&submit_sm::data_coding, "data_coding"),
        mem<u8>(&submit_sm::sm_default_msg_id, "sm_default_msg_id"),
        mem<u8_octet_str<254>>(&submit_sm::short_message, "short_message"),
        mem<smart>(&submit_sm::oparam, "oparam")
    };
}
} // namespace detail
} // namespace smpp
