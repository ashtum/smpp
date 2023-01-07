// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <smpp/param/esm_class.hpp>
#include <smpp/utility/data_coding_unicode.hpp>

#include <algorithm>
#include <cinttypes>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace smpp
{
class user_data_header
{
public:
  struct multi_part_data
  {
    uint16_t concat_sm_ref_num_{}; // Concatenated short message reference number
    uint8_t number_of_parts_{};    // Maximum number of short messages in the concatenated short
    uint8_t sequence_number_{};    // Sequence number of the current short message.
  };

private:
  static constexpr uint8_t concatenated_sm_8bit_ref{ 0x00 };
  static constexpr uint8_t concatenated_sm_16bit_ref{ 0x08 };

  std::vector<std::pair<uint8_t, std::string>> headers_;

public:
  user_data_header() = default;

  explicit user_data_header(std::string_view buf)
  {
    const auto constexpr header_length = 2;
    while (buf.size() >= header_length)
    {
      const auto tag        = static_cast<uint8_t>(buf[0]);
      const auto val_length = static_cast<uint8_t>(buf[1]);

      if (val_length > buf.size() - header_length)
        throw std::length_error{ "user_data_header val length is bigger than available buf" };

      const auto val_buf = buf.substr(header_length, val_length);

      if (
        (tag == concatenated_sm_8bit_ref && val_buf.size() != 3) ||
        (tag == concatenated_sm_16bit_ref && val_buf.size() != 4))
        throw std::length_error{ "user_data_header multi_part_data length is invalid" };

      headers_.emplace_back(tag, val_buf);

      buf = buf.substr(val_length + header_length);
    }
  }

  explicit user_data_header(const multi_part_data& mpd)
  {
    set_multi_part_data(mpd);
  }

  std::string serialize() const
  {
    std::string buf;
    for (const auto& [tag, val] : headers_)
    {
      buf.append({ static_cast<char>(tag), static_cast<char>(val.size()) });
      buf.insert(buf.end(), val.begin(), val.end());
    }
    return buf;
  }

  void set_multi_part_data(const multi_part_data& mpd)
  {
    if (mpd.concat_sm_ref_num_ > 0xFF)
    {
      headers_.emplace_back(
        concatenated_sm_16bit_ref,
        std::string{ { static_cast<char>((mpd.concat_sm_ref_num_ >> 8) & 0xFF),
                       static_cast<char>((mpd.concat_sm_ref_num_ >> 0) & 0xFF),
                       static_cast<char>(mpd.number_of_parts_),
                       static_cast<char>(mpd.sequence_number_) } });
    }
    else
    {
      headers_.emplace_back(
        concatenated_sm_8bit_ref,
        std::string{ { static_cast<char>(mpd.concat_sm_ref_num_),
                       static_cast<char>(mpd.number_of_parts_),
                       static_cast<char>(mpd.sequence_number_) } });
    }
  }

  multi_part_data get_multi_part_data() const
  {
    {
      const auto it = std::find_if(
        headers_.begin(), headers_.end(), [](const auto& h) { return h.first == concatenated_sm_8bit_ref; });
      if (it != headers_.end())
        return { static_cast<uint16_t>(it->second[0]),
                 static_cast<uint8_t>(it->second[1]),
                 static_cast<uint8_t>(it->second[2]) };
    }

    {
      const auto it = std::find_if(
        headers_.begin(), headers_.end(), [](const auto& h) { return h.first == concatenated_sm_16bit_ref; });
      if (it != headers_.end())
        return { static_cast<uint16_t>(
                   static_cast<uint16_t>(it->second[0]) << 8 | static_cast<uint16_t>(it->second[1])),
                 static_cast<uint8_t>(it->second[2]),
                 static_cast<uint8_t>(it->second[3]) };
    }

    return { .concat_sm_ref_num_ = 0, .number_of_parts_ = 1, .sequence_number_ = 1 };
  }
};

inline std::pair<user_data_header, std::string>
unpack_short_message(esm_class esm_class, data_coding data_coding, const std::string& short_message)
{
  if (extract_unicode(data_coding) == data_coding_unicode::ascii_8_bit && short_message.length() > 160)
    throw std::length_error{ "short_message length is larger than 160" };

  if (extract_unicode(data_coding) != data_coding_unicode::ascii_8_bit && short_message.length() > 140)
    throw std::length_error{ "short_message length is larger than 140" };

  if (
    esm_class.gsm_network_features == gsm_network_features::uhdi ||
    esm_class.gsm_network_features == gsm_network_features::both)
  {
    const auto udh_length = static_cast<uint8_t>(short_message[0]);

    if (udh_length >= short_message.length())
      throw std::length_error{ "UDH lenght is larger than short_message" };

    return { user_data_header{ short_message.substr(1, udh_length) }, short_message.substr(1 + udh_length) };
  }

  return { user_data_header{}, short_message };
}

inline std::string
pack_short_message(const user_data_header& user_data_header, std::string_view body, data_coding data_coding)
{
  std::string short_message;

  const auto serialized_udh = user_data_header.serialize();
  if (serialized_udh.length() != 0)
  {
    short_message.push_back(static_cast<char>(serialized_udh.length()));
    short_message.append(serialized_udh);
  }

  short_message.append(body);

  if (extract_unicode(data_coding) == data_coding_unicode::ascii_8_bit && short_message.length() > 160)
    throw std::length_error{ "short_message length is larger than 160" };

  if (extract_unicode(data_coding) != data_coding_unicode::ascii_8_bit && short_message.length() > 140)
    throw std::length_error{ "short_message length is larger than 140" };

  return short_message;
}
} // namespace smpp
