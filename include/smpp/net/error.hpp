#pragma once

#include <boost/system/error_code.hpp>

namespace smpp
{
enum class error
{
  serialization_failed = 1,
  enquire_link_timeout,
  unbinded,
};

inline const boost::system::error_category& category()
{
  static const struct : boost::system::error_category
  {
    const char* name() const noexcept override
    {
      return "smpp";
    }

    std::string message(int ev) const override
    {
      switch (static_cast<error>(ev))
      {
        case error::serialization_failed:
          return "PDU serialization failed";
        case error::enquire_link_timeout:
          return "The session was closed due to enquire_link timeout";
        case error::unbinded:
          return "The session was closed gracefully";
        default:
          return "Unknown error";
      }
    }
  } category;

  return category;
};

inline boost::system::error_code make_error_code(error e)
{
  return { static_cast<int>(e), category() };
}
} // namespace smpp

namespace boost::system
{
template<>
struct is_error_code_enum<::smpp::error>
{
  static bool const value = true;
};
} // namespace boost::system