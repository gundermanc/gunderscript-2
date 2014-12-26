// Gunderscript-2 Exceptions Definitions
// (C) 2014 Christian Gunderman

#ifndef GUNDERSCRIPT_EXCEPTIONS__H__
#define GUNDERSCRIPT_EXCEPTIONS__H__

#include <exception>
#include <string>

namespace gunderscript {
namespace library {

// Gunderscript Exceptions Parent Class
// Each module's exceptions descend from this class.
class Exception : public std::exception {
 public:
  Exception () throw();
  Exception(const std::string& message) throw() { message_ = message; };
  virtual const char* what() const throw() { return message_.c_str(); }

 private:
  std::string message_;
  std::exception exception_;
};

// Not implemented exception.
class NotImplementedException : public Exception {
 public:
  NotImplementedException() : Exception("Feature not implemented.") { }
  NotImplementedException(const std::string& message) : Exception(message) { }
};

} // namespace library
} // namespace gunderscript

#endif // GUNDERSCRIPT_EXCEPTIONS__H__
