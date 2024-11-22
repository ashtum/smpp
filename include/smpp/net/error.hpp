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

inline const boost::system::error_category&
error_category()
{
    struct category : boost::system::error_category
    {
        virtual ~category() = default;

        const char*
        name() const noexcept override
        {
            return "smpp";
        }

        std::string
        message(int ev) const override
        {
            switch(static_cast<error>(ev))
            {
            case error::serialization_failed:
                return "PDU serialization failed";
            case error::enquire_link_timeout:
                return "enquire_link timeout";
            case error::unbinded:
                return "unbinded";
            default:
                return "Unknown error";
            }
        }
    };

    static const auto category_ = category{};

    return category_;
};

inline boost::system::error_code
make_error_code(error e)
{
    return { static_cast<int>(e), error_category() };
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