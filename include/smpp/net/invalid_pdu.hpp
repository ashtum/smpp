// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cinttypes>
#include <string>
#include <vector>

namespace smpp
{
struct invalid_pdu
{
    std::vector<uint8_t> buffer;
    std::string deserialization_error;
};
} // namespace smpp
