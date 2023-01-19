// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/net/invalid_pdu.hpp>
#include <smpp/pdu.hpp>

#include <variant>

namespace smpp
{
using pdu_variant = std::variant<
  alert_notification,
  bind_receiver,
  bind_receiver_resp,
  bind_transceiver,
  bind_transceiver_resp,
  bind_transmitter,
  bind_transmitter_resp,
  cancel_sm,
  cancel_sm_resp,
  data_sm,
  data_sm_resp,
  deliver_sm,
  deliver_sm_resp,
  generic_nack,
  query_sm,
  query_sm_resp,
  replace_sm,
  replace_sm_resp,
  submit_sm,
  submit_sm_resp,
  invalid_pdu>;
} // namespace smpp
