// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <exception>

namespace smpp
{
class unbinded : public std::exception
{
public:
  const char* what() const noexcept override
  {
    return "Session is unbinded gracefully";
  }
};
} // namespace smpp
