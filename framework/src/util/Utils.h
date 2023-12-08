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

#ifndef CPPMICROSERVICES_UTILS_H
#define CPPMICROSERVICES_UTILS_H

#include "BundleResourceContainer.h"
#include "cppmicroservices/FrameworkExport.h"

#include <exception>
#include <string>

namespace cppmicroservices
{
    namespace stringCatFast
    {

        template <typename>
        struct string_size_impl;

        template <size_t N>
        struct string_size_impl<char const[N]>
        {
            static constexpr size_t
            size(char const (&)[N])
            {
                return N - 1;
            }
        };

        template <size_t N>
        struct string_size_impl<char[N]>
        {
            static size_t
            size(char (&s)[N])
            {
                return N ? strlen(s) : 0;
            }
        };

        template <>
        struct string_size_impl<char const*>
        {
            static size_t
            size(char const* s)
            {
                return s ? strlen(s) : 0;
            }
        };

        template <>
        struct string_size_impl<char*>
        {
            static size_t
            size(char* s)
            {
                return s ? strlen(s) : 0;
            }
        };

        template <>
        struct string_size_impl<std::string>
        {
            static size_t
            size(std::string const& s)
            {
                return s.size();
            }
        };

        template <typename String>
        size_t
        string_size(String&& s)
        {
            using noref_t = typename std::remove_reference<String>::type;
            using string_t = typename std::
                conditional<std::is_array<noref_t>::value, noref_t, typename std::remove_cv<noref_t>::type>::type;
            return string_size_impl<string_t>::size(s);
        }

        template <typename...>
        struct concatenate_impl;

        template <typename String>
        struct concatenate_impl<String>
        {
            static size_t
            size(String const& s)
            {
                return string_size(s);
            }
            static void
            concatenate(std::string& result, String&& s)
            {
                result += s;
            }
        };

        template <typename String, typename... Rest>
        struct concatenate_impl<String, Rest...>
        {
            static size_t
            size(String const& s, Rest... rest)
            {
                return string_size(s) + concatenate_impl<Rest...>::size((rest)...);
            }
            static void
            concatenate(std::string& result, String&& s, Rest&&... rest)
            {
                result += s;
                concatenate_impl<Rest...>::concatenate(result, std::forward<Rest>(rest)...);
            }
        };

    } // namespace stringCatFast
    template <typename... Strings>
    std::string
    StringCatFast(Strings&&... strings)
    {
        std::string result;
        result.reserve(stringCatFast::concatenate_impl<Strings...>::size((strings)...));
        stringCatFast::concatenate_impl<Strings...>::concatenate(result, std::forward<Strings>(strings)...);
        return result;
    }

    //-------------------------------------------------------------------
    // File type checking
    //-------------------------------------------------------------------

    bool IsBundleFile(std::string const& location);

    /**
     * Return true if the bundle's zip file only contains a
     * manifest file, false otherwise.
     *
     * @param resContainer The BundleResourceContainer to check
     *
     * @return true if the bundle's zip file only contains a
     *         manifest file, false otherwise.
     * @throw std::runtime_error if the bundle manifest cannot be read.
     */
    bool OnlyContainsManifest(std::shared_ptr<BundleResourceContainer> const& resContainer);

    //-------------------------------------------------------------------
    // Framework storage
    //-------------------------------------------------------------------

    class CoreBundleContext;

    extern std::string const FWDIR_DEFAULT;

    std::string GetFrameworkDir(CoreBundleContext* ctx);

    /**
     * Optionally create and get the persistent storage path.
     *
     * @param ctx Pointer to the CoreBundleContext object.
     * @param leafDir The name of the leaf directory in the persistent storage path.
     * @param create Specify if the directory needs to be created if it doesn't already exist.
     *
     * @return A directory path or an empty string if no storage is available.
     *
     * @throw std::runtime_error if the storage directory is inaccessible
     *        or if there exists a file named @c leafDir in that directory
     *        or if the directory cannot be created when @c create is @c true.
     */
    std::string GetPersistentStoragePath(CoreBundleContext* ctx, std::string const& leafDir, bool create = true);

    //-------------------------------------------------------------------
    // Generic utility functions
    //-------------------------------------------------------------------

    void TerminateForDebug(std::exception_ptr const ex);

    namespace detail
    {
        US_Framework_EXPORT std::string GetDemangledName(std::type_info const& typeInfo);
    }

} // namespace cppmicroservices

#endif // CPPMICROSERVICES_UTILS_H
