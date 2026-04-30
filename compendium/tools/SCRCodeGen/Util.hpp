/*=============================================================================

  Library: CppMicroServices

  Copyright (c) The CppMicroServices developers. See the COPYRIGHT
  file at the top-level directory of this distribution and at
  https://github.com/CppMicroServices/CppMicroServices/COPYRIGHT .

  Licensed under the Apache License, Version 2.0 (the "License");
  you may not use this file except in compliance with the License.
  You may obtain a copy of the License at

  http://www.apache.org/licenses/LICENSE-2.0

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  =============================================================================*/
#ifndef UTIL_HPP
#define UTIL_HPP

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <iterator>
#include <sstream>
#include <string>

#include <rapidjson/document.h>
#include <rapidjson/error/en.h>
#include <rapidjson/istreamwrapper.h>

namespace codegen
{
    namespace util
    {

        class JsonValueValidator
        {
          public:
            template <size_t S>
            using ValidChoices = std::array<std::string, S>;

            // Throw if:
            //  - data[name] doesn't exist
            //  - data[name] is empty
            //  - data[name] is not of the type specified by type.
            // Stores a non-owning pointer into the original document tree.
            JsonValueValidator(rapidjson::Value const& data, std::string name, rapidjson::Type type)
                : jsonName(std::move(name))
                , msg("Invalid value for the name '" + jsonName + "'. Expected ")
            {
                if (!data.HasMember(jsonName.c_str()))
                {
                    std::string msg = "Mandatory name '" + jsonName + "' missing from the manifest";
                    throw std::runtime_error(msg);
                }
                jsonVal = &data[jsonName.c_str()];
                Validate(type);
            }

            // If data[name] exists, check if it is
            //  - a non-empty string, and
            //  - present in the choices array.
            // Otherwise, throw.
            // If data[name] doesn't exist, the first item in the choices array (default value)
            // will be returned by the GetString() member function via defaultStr.
            template <std::size_t S>
            JsonValueValidator(rapidjson::Value const& data, std::string name, ValidChoices<S> const& choices)
                : jsonName(std::move(name))
                , msg("Invalid value for the name '" + jsonName + "'. Expected ")
            {
                static_assert(S > 0, "Choices cannot be empty!");

                if (!data.HasMember(jsonName.c_str()))
                {
                    // No member found — store the first choice as the default string.
                    // jsonVal stays nullptr; GetString() will return defaultStr.
                    defaultStr = choices.front();
                    return;
                }
                jsonVal = &data[jsonName.c_str()];

                Validate(rapidjson::kStringType);
                auto value = std::string(jsonVal->GetString());

                // The cast is to help the compiler resolve the correct overload.
                // Refer:
                // https://stackoverflow.com/questions/7131858/stdtransform-and-toupper-no-matching-function/7131881
                std::transform(value.begin(),
                               value.end(),
                               value.begin(),
                               [](unsigned char c) { return std::tolower(c); });

                // Return true if string value == string b in a case-insensitive way.
                auto isCaseInsensitiveEqual = [&value](std::string const& b)
                {
                    auto isCaseInsensitiveEqualChar = [](auto a, auto b) { return std::tolower(a) == std::tolower(b); };

                    if (value.length() == b.length())
                    {
                        return std::equal(b.begin(), b.end(), value.begin(), isCaseInsensitiveEqualChar);
                    }
                    else
                    {
                        return false;
                    }
                };

                if (std::none_of(choices.begin(), choices.end(), isCaseInsensitiveEqual))
                {
                    std::ostringstream stream;
                    stream << "Invalid value '" + value + "' for the name '" + jsonName + "'. ";
                    stream << "The valid choices are : [";
                    std::copy(std::begin(choices),
                              std::end(choices) - 1,
                              std::ostream_iterator<std::string>(stream, ", "));
                    stream << choices.back() << "]";
                    throw std::runtime_error(stream.str());
                }
            }

            // Return a const reference to the rapidjson::Value for data[name].
            rapidjson::Value const&
            operator()() const
            {
                return *jsonVal;
            }

            // Return the string value of data[name].
            // If the member was not found (choices constructor default case),
            // returns the stored default string.
            // Throw if data[name] exists but is not of string type.
            std::string
            GetString() const
            {
                if (!jsonVal)
                {
                    return defaultStr;
                }
                if (!jsonVal->IsString())
                {
                    throw std::runtime_error("The JSON value for the name '" + jsonName + "' must be of type string");
                }
                return jsonVal->GetString();
            }

          private:
            void
            Validate(rapidjson::Type type)
            {
                switch (type)
                {
                    case rapidjson::kStringType:
                        if (!jsonVal->IsString() || std::string(jsonVal->GetString()).empty())
                        {
                            msg.append("non-empty string");
                            throw std::runtime_error(msg);
                        }
                        break;
                    case rapidjson::kArrayType:
                        if (!jsonVal->IsArray() || jsonVal->Empty())
                        {
                            msg.append("non-empty array");
                            throw std::runtime_error(msg);
                        }
                        break;
                    case rapidjson::kObjectType:
                        if (!jsonVal->IsObject() || jsonVal->ObjectEmpty())
                        {
                            msg.append("non-empty JSON object i.e. collection of name/value pairs");
                            throw std::runtime_error(msg);
                        }
                        break;
                    case rapidjson::kNumberType:
                        if (!jsonVal->IsInt())
                        {
                            msg.append("int");
                            throw std::runtime_error(msg);
                        }
                        break;
                    case rapidjson::kTrueType:
                    case rapidjson::kFalseType:
                        // rapidjson splits bool into kTrueType/kFalseType.
                        // Both fall through to the same IsBool() check.
                        if (!jsonVal->IsBool())
                        {
                            msg.append("boolean");
                            throw std::runtime_error(msg);
                        }
                        break;
                    default:
                        throw std::runtime_error("Unsupported type!");
                }
            }

            std::string jsonName;
            std::string msg;
            rapidjson::Value const* jsonVal = nullptr;
            // Stores the default string when the choices constructor
            // doesn't find the member.
            std::string defaultStr;
        };

        // Parse the manifest specified in the istream jsonStream.
        // Return the root JSON value.
        // Throw if the parsing isn't successful (such as presence of duplicate keys)
        rapidjson::Document ParseManifestOrThrow(std::istream& jsonStream);

        // Write content to a file specified by the path filePath.
        // Throw if the file can't be opened.
        void WriteToFile(std::string const& filePath, std::string const& content);

        namespace detail
        {
            inline void
            replace(std::string& fmtstr, size_t index, std::string const& s)
            {
                size_t pos = 0;
                std::string anchor = "{" + std::to_string(index) + "}";

                while (std::string::npos != pos)
                {
                    pos = fmtstr.find(anchor, pos);
                    if (std::string::npos != pos)
                    {
                        fmtstr.replace(pos, anchor.size(), s);
                        pos += s.size();
                    }
                }
            }

            inline void
            substitute(std::string&, size_t)
            {
            }

            template <typename T, typename... Args>
            void
            substitute(std::string& src, size_t index, T& t, Args&... args)
            {
                replace(src, index, t);
                ++index;
                substitute(src, index, args...);
            }
        } // namespace detail

        // Given a string "src" with placeholders of form "{0}", "{1}", "{2}"
        // etc., substitute the arguments "args" into the string where the first
        // argument of "args" is substituted in place of "{0}", the second
        // argument of "args" is substituted in place of "{1}", and so on.
        // For example, substitute("foo {0} {1}", "bar", "baz"); will return
        // the string "foo bar baz"
        template <typename... Args>
        std::string
        Substitute(std::string src, Args... args)
        {
            detail::substitute(src, 0, args...);
            return src;
        }
    } // namespace util
} // namespace codegen

#endif // UTIL_HPP
