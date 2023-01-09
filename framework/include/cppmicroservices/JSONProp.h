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

#ifndef CPPMICROSERVICES_JSONPROP_H
#define CPPMICROSERVICES_JSONPROP_H

#include "cppmicroservices/Any.h"
#include "cppmicroservices/FrameworkConfig.h"

#ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable : 4251)
#endif

namespace cppmicroservices
{

    /// \cond
    class US_Framework_EXPORT JSONPropExpr
    {
      public:
        JSONPropExpr();
        explicit JSONPropExpr(std::string expr);

        JSONPropExpr& operator!();

        operator std::string() const;

        bool IsNull() const;

        /**
         * Convenience operator for json logical or '|'.
         *
         * Writing either
         * \code
         * JSONPropExpr expr(JSONProp("key1") == "value1");
         * expr = expr || JSONProp("key2") == "value2";
         * \endcode
         * or
         * \code
         * JSONPropExpr expr(JSONProp("key1") == "value1");
         * expr |= JSONProp("key2") == "value2";
         * \endcode
         * leads to the same string "(|(key1=value1) (key2=value2))".
         *
         * @param right A json expression object.
         * @return A json expression object.
         *
         * @{
         */
        JSONPropExpr& operator|=(JSONPropExpr const& right);
        /// @}

        /**
         * Convenience operator for json logical and '&'.
         *
         * Writing either
         * \code
         * JSONPropExpr expr(JSONProp("key1") == "value1");
         * expr = expr && JSONProp("key2") == "value2";
         * \endcode
         * or
         * \code
         * JSONPropExpr expr(JSONProp("key1") == "value1");
         * expr &= JSONProp("key2") == "value2";
         * \endcode
         * leads to the same string "(&(key1=value1) (key2=value2))".
         *
         * @param right A json expression object.
         * @return A json expression object.
         *
         * @{
         */
        JSONPropExpr& operator&=(JSONPropExpr const& right);
        /// @}

      private:
        std::string m_jsonExpr;
    };
    /// \endcond

    /**
     * \ingroup MicroServicesUtils
     * \ingroup gr_json
     *
     * A fluent API for creating json filter strings.
     *
     * Examples for creating jsonFilter objects:
     * \code
     * // This creates the filter "(&(name=Ben)(!(count=1)))"
     * jsonFilter filter(JSONProp("name") == "Ben" && !(JSONProp("count") == 1));
     *
     * // This creates the filter "(|(presence=*)(!(absence=*)))"
     * jsonFilter filter(JSONProp("presence") || !JSONProp("absence"));
     *
     * // This creates the filter "(&(ge>=-3)(approx~=hi))"
     * jsonFilter filter(JSONProp("ge") >= -3 && JSONProp("approx").Approx("hi"));
     * \endcode
     *
     * \sa jsonFilter
     */
    class US_Framework_EXPORT JSONProp
    {
      public:
        /**
         * Create a JSONProp instance for the named json property.
         *
         * @param property The name of the json property.
         */
        JSONProp(std::string property);

        /**
         * \addtogroup gr_json
         * json equality '='
         *
         * @param s A type convertible to std::string.
         * @param b A type convertible to bool
         * @return A json expression object.
         *
         * @{
         */
        JSONPropExpr operator==(std::string const& s) const;
        JSONPropExpr operator==(cppmicroservices::Any const& s) const;
        JSONPropExpr operator==(bool b) const;
        template <class T>
        JSONPropExpr
        operator==(T const& s) const
        {
            std::stringstream ss;
            ss << s;
            return JSONPropExpr(m_property + "==\'" + ss.str() + "\'");
        }
        /// @}

        operator JSONPropExpr() const;

        /**
         * States the absence of the json property.
         *
         * @return A json expression object.
         */
        JSONPropExpr operator!() const;

        /**
         * \addtogroup gr_json
         * Convenience operator for json inequality.
         *
         * Writing either
         * \code
         * JSONProp("attr") != "val"
         * \endcode
         * or
         * \code
         * !(JSONProp("attr") == "val")
         * \endcode
         * leads to the same string "(!(attr=val))".
         *
         * @param s A type convertible to std::string.
         * @return A json expression object.
         *
         * @{
         */
        JSONPropExpr operator!=(std::string const& s) const;
        JSONPropExpr operator!=(cppmicroservices::Any const& s) const;
        template <class T>
        JSONPropExpr
        operator!=(T const& s) const
        {
            std::stringstream ss;
            ss << s;
            return operator!=(ss.str());
        }
        /// @}

        /**
         * \addtogroup gr_json
         * json greater or equal '>='
         *
         * @param s A type convertible to std::string.
         * @return A json expression object.
         *
         * @{
         */
        JSONPropExpr operator>=(std::string const& s) const;
        JSONPropExpr operator>=(cppmicroservices::Any const& s) const;
        template <class T>
        JSONPropExpr
        operator>=(T const& s) const
        {
            std::stringstream ss;
            ss << s;
            return operator>=(ss.str());
        }
        /// @}

        /**
         * \addtogroup gr_json
         * json less or equal '<='
         *
         * @param s A type convertible to std::string.
         * @return A json expression object.
         *
         * @{
         */
        JSONPropExpr operator<=(std::string const& s) const;
        JSONPropExpr operator<=(cppmicroservices::Any const& s) const;
        template <class T>
        JSONPropExpr
        operator<=(T const& s) const
        {
            std::stringstream ss;
            ss << s;
            return operator<=(ss.str());
        }
        /// @}

        /**
         * \addtogroup gr_json
         * json approximation '~='
         *
         * @param s A type convertible to std::string.
         * @return A json expression object.
         *
         * @{
         */
        JSONPropExpr Approx(std::string const& s) const;
        JSONPropExpr Approx(cppmicroservices::Any const& s) const;
        template <class T>
        JSONPropExpr
        Approx(T const& s) const
        {
            std::stringstream ss;
            ss << s;
            return Approx(ss.str());
        }
        /// @}

      private:
        JSONProp& operator=(JSONProp const&) = delete;

        std::string m_property;
    };
} // namespace cppmicroservices

#ifdef _MSC_VER
#    pragma warning(pop)
#endif

/**
 * \ingroup MicroServicesUtils
 * \ingroup gr_json
 *
 * json logical and '&'
 *
 * @param left A json expression.
 * @param right A json expression.
 * @return A json expression
 */
US_Framework_EXPORT cppmicroservices::JSONPropExpr operator&&(cppmicroservices::JSONPropExpr const& left,
                                                              cppmicroservices::JSONPropExpr const& right);

/**
 * \ingroup MicroServicesUtils
 * \ingroup gr_json
 *
 * json logical or '|'
 *
 * @param left A json expression.
 * @param right A json expression.
 * @return A json expression
 */
US_Framework_EXPORT cppmicroservices::JSONPropExpr operator||(cppmicroservices::JSONPropExpr const& left,
                                                              cppmicroservices::JSONPropExpr const& right);

#endif // CPPMICROSERVICES_JSONPROP_H
