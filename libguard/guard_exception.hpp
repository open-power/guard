#pragma once

#include <exception>
#include <sstream>
#include <string>

namespace openpower
{
namespace guard
{
namespace exception
{
// TODO:Issue #3, to avoid multiple exception classes
class GuardException : public std::exception
{
  public:
    explicit GuardException(const std::string& message) : message(message){};

    const char* what() const noexcept override
    {
        return message.c_str();
    }

  private:
    std::string message;
};

class GuardFileOpenFailed : public GuardException
{
  public:
    explicit GuardFileOpenFailed(const std::string& msg) :
        GuardException(msg){};
};

class GuardFileReadFailed : public GuardException
{
  public:
    explicit GuardFileReadFailed(const std::string& msg) :
        GuardException(msg){};
};

class GuardFileWriteFailed : public GuardException
{
  public:
    explicit GuardFileWriteFailed(const std::string& msg) :
        GuardException(msg){};
};

class GuardFileSeekFailed : public GuardException
{
  public:
    explicit GuardFileSeekFailed(const std::string& msg) :
        GuardException(msg){};
};

class InvalidEntry : public GuardException
{
  public:
    explicit InvalidEntry(const std::string& msg) : GuardException(msg){};
};

class AlreadyGuarded : public GuardException
{
  public:
    explicit AlreadyGuarded(const std::string& msg) : GuardException(msg){};
};

class InvalidEntityPath : public GuardException
{
  public:
    explicit InvalidEntityPath(const std::string& msg) : GuardException(msg){};
};

class GuardFileOverFlowed : public GuardException
{
  public:
    explicit GuardFileOverFlowed(const std::string& msg) :
        GuardException(msg){};
};

} // namespace exception
} // namespace guard
} // namespace openpower
