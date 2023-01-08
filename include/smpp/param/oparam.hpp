// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/param/oparam_tag.hpp>

#include <map>
#include <memory>
#include <span>
#include <vector>

namespace smpp
{
class oparam
{
  std::map<oparam_tag, std::string> oparam_;

public:
  oparam() = default;

  bool operator==(const oparam&) const = default;

  explicit oparam(std::span<const uint8_t>* buf)
  {
    auto deserialize_u16 = [](std::span<const uint8_t, 2> buf) -> uint16_t { return buf[0] << 8 | buf[1]; };

    while (buf->size() >= 4)
    {
      auto constexpr header_length = 4;
      auto tag                     = static_cast<oparam_tag>(deserialize_u16(buf->subspan<0, 2>()));
      auto val_length              = deserialize_u16(buf->subspan<2, 2>());

      if (val_length > buf->size() - header_length)
        throw std::length_error{ "oparam val length is bigger than available buf" };

      auto val_buf = buf->subspan(header_length, val_length);
      oparam_.emplace(tag, std::string{ val_buf.begin(), val_buf.end() });

      *buf = buf->last(buf->size() - (val_length + header_length));
    }
  }

  void serialize(std::vector<uint8_t>* vec) const
  {
    auto serialize_u16 = [&](uint16_t val)
    {
      vec->push_back((val >> 8) & 0xFF);
      vec->push_back((val >> 0) & 0xFF);
    };

    for (const auto& oparam : oparam_)
    {
      auto tag        = oparam.first;
      const auto& buf = oparam.second;

      serialize_u16(static_cast<uint16_t>(tag));
      serialize_u16(buf.size());

      vec->insert(vec->end(), buf.begin(), buf.end());
    }
  }

  void erase(oparam_tag tag)
  {
    oparam_.erase(tag);
  }

  bool contains(oparam_tag tag)
  {
    return oparam_.contains(tag);
  }

  const std::string& get_as_string(oparam_tag tag) const
  {
    const auto& it = oparam_.find(tag);
    if (it == oparam_.end())
      throw std::logic_error{ "oparam doesn't exist" };
    return it->second;
  }

  void set_as_string(oparam_tag tag, std::string val)
  {
    if (val.size() > 65535)
      throw std::length_error{ "oparam value length is bigger than 65535" };

    oparam_[tag] = std::move(val);
  }

  template<typename T>
  T get_as_enum_u8(oparam_tag tag) const
  {
    return static_cast<T>(get_as_string(tag)[0]);
  }

  template<typename T>
  void set_as_enum_u8(oparam_tag tag, T val)
  {
    oparam_[tag] = static_cast<uint8_t>(val);
  }
};
} // namespace smpp
