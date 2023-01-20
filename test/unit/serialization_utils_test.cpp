// Copyright (c) 2022 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <smpp/common/serialization.hpp>
#include <smpp/net/detail/header_serialization.hpp>
#include <smpp/param.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(serialization_utils)

BOOST_AUTO_TEST_CASE(header)
{
  std::vector<uint8_t> buf;
  buf.resize(16);

  smpp::detail::serialize_header(
    std::span<uint8_t, 16>{ buf.begin(), buf.end() },
    42,
    smpp::command_id::unbind,
    23123546,
    smpp::command_status::max_try_count);

  auto [command_length, command_id, command_status, sequence_number] =
    smpp::detail::deserialize_header(std::span<const uint8_t, 16>{ buf.begin(), buf.end() });

  BOOST_CHECK(command_length == 42);
  BOOST_CHECK(command_id == smpp::command_id::unbind);
  BOOST_CHECK(command_status == smpp::command_status::max_try_count);
  BOOST_CHECK(sequence_number == 23123546);
}

BOOST_AUTO_TEST_CASE(enum_u8)
{
  using enum_t = smpp::priority_flag;

  std::vector<uint8_t> buf;

  smpp::detail::enum_u8::serialize_to<enum_t>(&buf, enum_t::ansi_136_bulk, "mem_name");

  BOOST_CHECK_EQUAL(buf.size(), 1);
  BOOST_CHECK(static_cast<enum_t>(buf[0]) == enum_t::ansi_136_bulk);

  {
    auto buf_v = std::span<const uint8_t>{ buf.begin(), buf.end() };
    auto val   = smpp::detail::enum_u8::deserialize<enum_t>(&buf_v, "mem_name");
    BOOST_CHECK_EQUAL(buf_v.size(), 0); // deserialize consumes buffer
    BOOST_CHECK(val == enum_t::ansi_136_bulk);
  }

  {
    auto buf_v = std::span<const uint8_t>{};
    BOOST_CHECK_THROW(smpp::detail::enum_u8::deserialize<enum_t>(&buf_v, "mem_name"), std::length_error);
  }
}

BOOST_AUTO_TEST_CASE(enum_flag)
{
  using enum_flag_t = smpp::esm_class;
  auto sample_value = smpp::esm_class{ smpp::messaging_mode::forward,
                                       smpp::message_type::delivery_receipt,
                                       smpp::gsm_network_features::reply_path };
  std::vector<uint8_t> buf;

  smpp::detail::enum_flag::serialize_to<enum_flag_t>(&buf, sample_value, "mem_name");

  BOOST_CHECK_EQUAL(buf.size(), 1);
  BOOST_CHECK(enum_flag_t::from_u8(buf[0]) == sample_value);

  {
    auto buf_v = std::span<const uint8_t>{ buf.begin(), buf.end() };
    auto val   = smpp::detail::enum_flag::deserialize<enum_flag_t>(&buf_v, "mem_name");
    BOOST_CHECK_EQUAL(buf_v.size(), 0); // deserialize consumes buffer
    BOOST_CHECK(val == sample_value);
  }

  {
    auto buf_v = std::span<const uint8_t>{};
    BOOST_CHECK_THROW(smpp::detail::enum_flag::deserialize<enum_flag_t>(&buf_v, "mem_name"), std::length_error);
  }
}

BOOST_AUTO_TEST_CASE(u8)
{
  std::vector<uint8_t> buf;

  smpp::detail::u8::serialize_to<void>(&buf, 42, "mem_name");

  BOOST_CHECK_EQUAL(buf.size(), 1);
  BOOST_CHECK_EQUAL(buf[0], 42);

  {
    auto buf_v = std::span<const uint8_t>{ buf.begin(), buf.end() };
    auto val   = smpp::detail::u8::deserialize<void>(&buf_v, "mem_name");
    BOOST_CHECK_EQUAL(buf_v.size(), 0); // deserialize consumes buffer
    BOOST_CHECK_EQUAL(val, 42);
  }

  {
    auto buf_v = std::span<const uint8_t>{};
    BOOST_CHECK_THROW(smpp::detail::u8::deserialize<void>(&buf_v, "mem_name"), std::length_error);
  }
}

BOOST_AUTO_TEST_CASE(c_octet_str)
{
  std::vector<uint8_t> buf;

  smpp::detail::c_octet_str<5>::serialize_to<void>(&buf, "TEST", "mem_name");

  BOOST_CHECK_EQUAL(buf.size(), 5);
  BOOST_CHECK_EQUAL(buf[4], '\0');

  {
    auto buf_v = std::span<const uint8_t>{ buf.begin(), buf.end() };
    auto str   = smpp::detail::c_octet_str<5>::deserialize<void>(&buf_v, "mem_name");
    BOOST_CHECK_EQUAL(buf_v.size(), 0); // deserialize consumes buffer
    BOOST_CHECK_EQUAL(str, "TEST");
  }

  {
    auto buf_v = std::span<const uint8_t>{ buf.begin(), buf.end() };
    BOOST_CHECK_THROW(smpp::detail::c_octet_str<4>::deserialize<void>(&buf_v, "mem_name"), std::length_error);
    BOOST_CHECK_EQUAL(buf_v.size(), 5); // shouldn't consume buffer on error
  }

  BOOST_CHECK_THROW(smpp::detail::c_octet_str<4>::serialize_to<void>(&buf, "TEST", "mem_name"), std::length_error);
}

BOOST_AUTO_TEST_CASE(u8_octet_str)
{
  std::vector<uint8_t> buf;

  smpp::detail::u8_octet_str<4>::serialize_to<void>(&buf, "TEST", "mem_name");

  BOOST_CHECK_EQUAL(buf.size(), 5);
  BOOST_CHECK_EQUAL(buf[0], 4);
  BOOST_CHECK_EQUAL(buf[4], 'T');

  {
    auto buf_v = std::span<const uint8_t>{ buf.begin(), buf.end() };
    auto str   = smpp::detail::u8_octet_str<4>::deserialize<void>(&buf_v, "mem_name");
    BOOST_CHECK_EQUAL(buf_v.size(), 0); // deserialize consumes buffer
    BOOST_CHECK_EQUAL(str, "TEST");
  }

  {
    auto buf_v = std::span<const uint8_t>{ buf.begin(), buf.end() };
    BOOST_CHECK_THROW(smpp::detail::u8_octet_str<3>::deserialize<void>(&buf_v, "mem_name"), std::length_error);
    BOOST_CHECK_EQUAL(buf_v.size(), 5); // shouldn't consume buffer on error
  }

  BOOST_CHECK_THROW(smpp::detail::u8_octet_str<3>::serialize_to<void>(&buf, "TEST", "mem_name"), std::length_error);
}

BOOST_AUTO_TEST_SUITE_END()
