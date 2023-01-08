// Copyright (c) 2023 Mohammad Nejati
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <boost/asio/buffer.hpp>

#include <array>

namespace smpp::detail
{
template<typename T, std::size_t N>
class static_flat_buffer
{
  std::array<T, N> buf_{};

  T* in_{ buf_.begin() };
  T* out_{ buf_.begin() };
  T* last_{ buf_.begin() };

public:
  static_flat_buffer() noexcept = default;

  void clear() noexcept
  {
    in_   = buf_.begin();
    out_  = buf_.begin();
    last_ = buf_.begin();
  }

  std::size_t capacity() const noexcept
  {
    return buf_.size();
  }

  boost::asio::const_buffer data() const noexcept
  {
    return { in_, dist(in_, out_) };
  }

  const T* begin() const noexcept
  {
    return in_;
  }

  const T* end() const noexcept
  {
    return out_;
  }

  std::size_t size() const noexcept
  {
    return dist(in_, out_);
  }

  boost::asio::mutable_buffer prepare(std::size_t n)
  {
    if (n <= dist(out_, buf_.end()))
    {
      last_ = out_ + n;
      return { out_, n };
    }
    const auto len = size();
    if (n > capacity() - len)
      throw(std::length_error{ "static_flat_buffer::prepare buffer overflow" });
    if (len > 0)
      std::memmove(buf_.begin(), in_, len);
    in_   = buf_.begin();
    out_  = in_ + len;
    last_ = out_ + n;
    return { out_, n };
  }

  void commit(std::size_t n) noexcept
  {
    out_ += (std::min<std::size_t>)(n, last_ - out_);
  }

  void consume(std::size_t n) noexcept
  {
    if (n >= size())
    {
      in_  = buf_.begin();
      out_ = in_;
      return;
    }
    in_ += n;
  }

private:
  static std::size_t dist(const T* first, const T* last) noexcept
  {
    return static_cast<std::size_t>(last - first);
  }
};
} // namespace smpp::detail
