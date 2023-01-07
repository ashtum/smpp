// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <algorithm>
#include <span>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace smpp
{
namespace detail
{
struct enum_u8
{
  template<typename T>
  static auto deserialize(std::span<const uint8_t>* buf, const char* name)
  {
    if (buf->empty())
      throw std::length_error{ "buf size should be at least 1, field_name:" + std::string{ name } };

    auto val = static_cast<T>(*buf->begin());

    *buf = buf->last(buf->size() - 1);

    return val;
  }

  template<typename T>
  static void serialize_to(std::vector<uint8_t>* vec, const T& val, const char*)
  {
    vec->push_back(static_cast<uint8_t>(val));
  }
};

struct enum_flag
{
  template<typename T>
  static auto deserialize(std::span<const uint8_t>* buf, const char* name)
  {
    if (buf->empty())
      throw std::length_error{ "buf size should be at least 1, field_name:" + std::string{ name } };

    auto val = T::from_u8(*buf->begin());

    *buf = buf->last(buf->size() - 1);

    return val;
  }

  template<typename T>
  static void serialize_to(std::vector<uint8_t>* vec, const T& val, const char*)
  {
    vec->push_back(static_cast<uint8_t>(val));
  }
};

struct u8
{
  template<typename T>
  static uint8_t deserialize(std::span<const uint8_t>* buf, const char* name)
  {
    if (buf->empty())
      throw std::length_error{ "buf size should be at least 1, field_name:" + std::string{ name } };

    auto val = *buf->begin();

    *buf = buf->last(buf->size() - 1);

    return val;
  }

  template<typename T>
  static void serialize_to(std::vector<uint8_t>* vec, const uint8_t& val, const char*)
  {
    vec->push_back(val);
  }
};

struct smart
{
  template<typename T>
  static auto deserialize(std::span<const uint8_t>* buf, const char*)
  {
    return T{ buf };
  }

  template<typename T>
  static void serialize_to(std::vector<uint8_t>* vec, const T& val, const char*)
  {
    val.serialize(vec);
  }
};

template<size_t MAXLEN>
struct c_octet_str
{
  template<typename T>
  static auto deserialize(std::span<const uint8_t>* buf, const char* name)
  {
    auto null_pos = std::find(buf->begin(), buf->end(), '\0');

    if (null_pos == buf->end())
      throw std::length_error{ "c_octet_str can't find null character, field_name:" + std::string{ name } };

    auto str = std::string{ buf->begin(), null_pos };

    if (str.size() >= MAXLEN)
      throw std::length_error{ "c_octet_str exceed its limit, field_name:" + std::string{ name } };

    *buf = buf->last(buf->size() - str.size() - 1); // One for null character

    return str;
  }

  template<typename T>
  static void serialize_to(std::vector<uint8_t>* vec, const std::string& val, const char* name)
  {
    if (val.size() >= MAXLEN)
      throw std::length_error{ "c_octet_str exceed its limit, field_name:" + std::string{ name } };

    vec->insert(vec->end(), val.c_str(), val.c_str() + val.size() + 1); // One for null character
  }
};

template<size_t MAXLEN>
struct u8_octet_str
{
  template<typename T>
  static auto deserialize(std::span<const uint8_t>* buf, const char* name)
  {
    if (buf->empty())
      throw std::length_error{ "buf size should be at least 1, field_name:" + std::string{ name } };

    auto length = *buf->begin();

    if (buf->size() <= length)
      throw std::length_error{ "octet_str buf is smaller than its length field, field_name:" + std::string{ name } };

    auto str = std::string{ buf->begin() + 1, buf->begin() + length + 1 }; // One for length field

    if (str.size() > MAXLEN)
      throw std::length_error{ "octet_str exceed its limit, field_name:" + std::string{ name } };

    *buf = buf->last(buf->size() - str.size() - 1); // One for length field

    return str;
  }

  template<typename T>
  static void serialize_to(std::vector<uint8_t>* vec, const std::string& val, const char* name)
  {
    if (val.size() > MAXLEN)
      throw std::length_error{ "octet_str exceed its limit, field_name:" + std::string{ name } };

    vec->push_back(static_cast<uint8_t>(val.size()));

    vec->insert(vec->end(), val.begin(), val.end());
  }
};

template<typename R, typename S, typename T>
struct memv_wrapper
{
  T S::*ptr;
  const char* name;

  auto deserialize(std::span<const uint8_t>* buf) const
  {
    return R::template deserialize<T>(buf, name);
  }

  void serialize_to(std::vector<uint8_t>* vec, const S& obj) const
  {
    R::template serialize_to<const T&>(vec, obj.*ptr, name);
  }
};

template<typename R, typename S, typename T>
inline consteval auto memv(T S::*ptr, const char* name)
{
  return memv_wrapper<R, S, T>{ ptr, name };
}

template<typename PDU>
inline consteval auto pdu_meta() = delete;

template<typename PDU>
inline constexpr auto meta_holder = detail::pdu_meta<PDU>();
} // namespace detail

template<typename PDU>
inline void serialize_to(std::vector<uint8_t>* vec, const PDU& pdu)
{
  [&]<size_t... Is>(std::index_sequence<Is...>)
  {
    (std::get<Is>(detail::meta_holder<PDU>).serialize_to(vec, pdu), ...);
  }
  (std::make_index_sequence<std::tuple_size_v<decltype(detail::meta_holder<PDU>)>>());
}

template<typename PDU>
inline auto deserialize(std::span<const uint8_t> buf)
{
  if (buf.empty())
    return PDU{};

  return [&]<size_t... Is>(std::index_sequence<Is...>)
  {
    return PDU{ std::get<Is>(detail::meta_holder<PDU>).deserialize(&buf)... };
  }
  (std::make_index_sequence<std::tuple_size_v<decltype(detail::meta_holder<PDU>)>>{});
}
} // namespace smpp
