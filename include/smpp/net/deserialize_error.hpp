// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <cstdint>
#include <exception>
#include <span>
#include <string>
#include <vector>

namespace smpp
{
class deserialize_error : public std::exception
{
  std::string error_;
  std::vector<uint8_t> buffer_;

public:
  deserialize_error(std::string error, std::vector<uint8_t> buffer)
    : error_{ std::move(error) }
    , buffer_{ std::move(buffer) }
  {
  }

  std::span<const uint8_t> buffer() const noexcept
  {
    return buffer_;
  }

  const char* what() const noexcept override
  {
    return error_.data();
  }
};
} // namespace smpp
