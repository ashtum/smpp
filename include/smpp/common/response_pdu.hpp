// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/common/is_response.hpp>

namespace smpp
{
template<typename PDU>
concept response_pdu = is_response(PDU::command_id);
} // namespace smpp
