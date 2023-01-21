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
  static auto constexpr header_length{ 4 };
  std::map<oparam_tag, std::string> oparams_;

public:
  oparam() = default;

  /// Construct an oparam from buffer
  /**
   * Constructs an oparam from buffer and is intended to be used by pdu deserializer.
   *
   * @param buf The buffer that contains optional parameters.
   */
  explicit oparam(std::span<const uint8_t>* buf)
  {
    auto deserialize_u16 = [](std::span<const uint8_t, 2> buf) -> uint16_t { return buf[0] << 8 | buf[1]; };

    while (buf->size() >= 4)
    {
      auto tag        = static_cast<oparam_tag>(deserialize_u16(buf->subspan<0, 2>()));
      auto val_length = deserialize_u16(buf->subspan<2, 2>());

      if (val_length > buf->size() - header_length)
        throw std::length_error{ "oparam val length is bigger than available buf" };

      auto val_buf = buf->subspan(header_length, val_length);
      oparams_.emplace(tag, std::string{ val_buf.begin(), val_buf.end() });

      *buf = buf->last(buf->size() - (val_length + header_length));
    }
  }

  bool operator==(const oparam&) const = default;

  /// Serialize the oparam
  /**
   * This function serialize the oparam and is intended to be used by pdu serializer.
   *
   * @param vec The vector that optional parameters would be appended to.
   */
  void serialize(std::vector<uint8_t>* vec) const
  {
    auto serialize_u16 = [&](uint16_t val)
    {
      vec->push_back((val >> 8) & 0xFF);
      vec->push_back((val >> 0) & 0xFF);
    };

    for (const auto& oparam : oparams_)
    {
      auto tag        = oparam.first;
      const auto& buf = oparam.second;

      serialize_u16(static_cast<uint16_t>(tag));
      serialize_u16(buf.size());

      vec->insert(vec->end(), buf.begin(), buf.end());
    }
  }

  /// Erase an optional parameter
  /**
   * This function erase an optional parameter by its oparam_tag.
   *
   * @return A boolean indicating if there was an optional parameter with this oparam_tag.
   *
   * @param tag The oparam_tag to be erased.
   */
  bool erase(oparam_tag tag)
  {
    return oparams_.erase(tag);
  }

  /// Finds whether an optional parameter with the given oparam_tag exists.
  /**
   * This function finds whether an optional parameter with the given oparam_tag exists.
   *
   * @return A boolean indicating if there was an optional parameter with this oparam_tag.
   *
   * @param tag The oparam_tag to be located.
   */
  bool contains(oparam_tag tag)
  {
    return oparams_.contains(tag);
  }

  /// Get an optional parameter as a string.
  /**
   * This function gets an optional parameter as a string.
   *
   * @throw std::runtime_error if optional parameter does not exist.
   *
   * @return A const reference to the string.
   *
   * @param tag The oparam_tag to be located.
   */
  const std::string& get_as_string(oparam_tag tag) const
  {
    const auto& it = oparams_.find(tag);
    if (it == oparams_.end())
      throw std::runtime_error{ "oparam does not exist" };
    return it->second;
  }

  /// Set an optional parameter as a string.
  /**
   * This function sets an optional parameter as a string.
   * If a value with the same oparam_tag already exists it will be replaced.
   *
   * @throw std::length_error if string val has length bigger than 65535.
   *
   * @param val The string to be set.
   * @param tag The oparam_tag to be located.
   */
  void set_as_string(oparam_tag tag, std::string val)
  {
    if (val.size() > 65535)
      throw std::length_error{ "oparam value length is bigger than 65535" };

    oparams_[tag] = std::move(val);
  }

  /// Get an optional parameter as an enum with an underlying type of uint8_t.
  /**
   * This function gets an optional parameter as an underlying type of uint8_t.
   *
   * @throw std::runtime_error if optional parameter does not exist.
   *
   * @return The enum value.
   *
   * @param tag The oparam_tag to be located.
   *
   * @tparam T the type of enum with an underlying type of uint8_t.
   */
  template<typename T>
  T get_as_enum_u8(oparam_tag tag) const requires(std::is_same_v<std::underlying_type_t<T>, uint8_t>)
  {
    return static_cast<T>(get_as_string(tag)[0]);
  }

  /// Set an optional parameter as an enum with an underlying type of uint8_t.
  /**
   * This function sets an optional parameter as an enum with an underlying type of uint8_t.
   * If a value with the same oparam_tag already exists it will be replaced.
   *
   * @param val The enum value to be set.
   * @param tag The oparam_tag to be located.
   *
   * @tparam T the type of enum with an underlying type of uint8_t.
   */
  template<typename T>
  void set_as_enum_u8(oparam_tag tag, T val) requires(std::is_same_v<std::underlying_type_t<T>, uint8_t>)
  {
    oparams_[tag] = static_cast<uint8_t>(val);
  }
};
} // namespace smpp
