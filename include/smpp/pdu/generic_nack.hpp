// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common.hpp>
#include <smpp/param.hpp>

namespace smpp
{
struct generic_nack
{
    static constexpr auto command_id{ smpp::command_id::generic_nack };

    bool
    operator==(const generic_nack&) const = default;
};

namespace detail
{
template<>
inline consteval auto
pdu_meta<generic_nack>()
{
    return std::tuple{};
}
} // namespace detail
} // namespace smpp
