/**
 * @file      Util.cpp
 * @copyright Copyright 2019 MathWorks, Inc. 
 */ 

#include "Util.hpp"

namespace cppmicroservices { namespace scrimpl { namespace util {

// @brief Throws if the container T is of type @c Any or @c string or @c AnyMap and it's empty
template <>
void ThrowIfEmpty(const std::vector<cppmicroservices::Any>& value, const std::string& key)
{
  ThrowIfEmptyHelper(value, key);
}
  
template <>
void ThrowIfEmpty(const std::string& value, const std::string& key)
{
  ThrowIfEmptyHelper(value, key);
}
  
template <>
void ThrowIfEmpty(const cppmicroservices::AnyMap& value, const std::string& key)
{
  ThrowIfEmptyHelper(value, key);
}

template <>
void ThrowIfValueAbsentInChoices(const std::string& inValue, const std::vector<std::string>& choices)
{
  std::string value = inValue;
#if _WIN32
  std::transform(value.begin(), value.end(), value.begin(), tolower);
#else
  std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return static_cast<unsigned char>(std::tolower(static_cast<unsigned char>(c))); });
#endif
  if (!choices.empty() &&
      std::find(choices.begin(), choices.end(), value) == choices.end())
  {
    std::ostringstream stream;
    stream << "Invalid value '" + value + "'. ";
    stream << "The valid choices are : [";
    for (auto c = std::begin(choices); c < std::end(choices) - 1; ++c)
    {
      stream << *c << ", ";
    }
    stream << choices.back() << "].";
    throw std::out_of_range(stream.str());
  }
}

}}}
