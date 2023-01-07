// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>
#include <exception>
#include <string>
#include <vector>

namespace smpp
{
class deserialization_error : public std::exception
{
  std::string error_;
  std::vector<uint8_t> pdu_buffer_;

public:
  deserialization_error(std::string error, std::vector<uint8_t> pdu_buffer)
    : error_{ std::move(error) }
    , pdu_buffer_{ std::move(pdu_buffer) }
  {
  }

  const std::vector<uint8_t>& pdu_buffer() const noexcept
  {
    return pdu_buffer_;
  }

  const char* what() const noexcept override
  {
    return error_.data();
  }
};
} // namespace smpp
