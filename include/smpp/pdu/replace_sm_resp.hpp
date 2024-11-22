// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common.hpp>
#include <smpp/param.hpp>

namespace smpp
{
struct replace_sm_resp
{
    static constexpr auto command_id{ smpp::command_id::replace_sm_resp };

    bool
    operator==(const replace_sm_resp&) const = default;
};

namespace detail
{
template<>
inline consteval auto
pdu_meta<replace_sm_resp>()
{
    return std::tuple{};
}
} // namespace detail
} // namespace smpp
