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
         * Convenience operator for json logical or '||'.
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
         * leads to the same string "(key1=='value1') || (key2=='value2')".
         *
         * @param right A json expression object.
         * @return A json expression object.
         *
         * @{
         */
        JSONPropExpr& operator|=(JSONPropExpr const& right);
        /// @}

        /**
         * Convenience operator for json logical and '&&'.
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
         * leads to the same string "(key1=='value1')&&(key2=='value2')".
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
     * A fluent API for creating JSON filter strings.
     *
     * Examples for creating jsonFilter objects:
     * \code
     *
     * // This creates the filter "(name=='Ben')&&!(count==`1`)"
     * jsonFilter filter(JSONProp("name") == "Ben" && !(JSONProp("count") == 1));
     *
     * // This creates the filter "(presence==`true`)||!(absence==`true`)"
     * jsonFilter filter(JSONProp("presence") || !JSONProp("absence"));
     *
     * \endcode
     *
     * \sa LDAPFilter
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
         * \name JSONProp comparison operator ==
         *
         * @{
         * populates JMESPath expression which compares current property for equality
         * Example:
         * \code
         *
         * // Example for using JSONProp equality operator with numeric values
         * JSONPropExpr propexpr(JSONProp("property1") == 1)   // "property1==`1`"
         *
         * // Example for using JSONProp equality operator with non numeric values
         * JSONPropExpr propexpr(JSONProp("property1") == "1") // "property1=='1'"
         *
         * \endcode
         */

        JSONPropExpr operator==(std::string const& s) const;
        JSONPropExpr operator==(cppmicroservices::Any const& s) const;
        JSONPropExpr operator==(bool b) const;
        template <class T>
        JSONPropExpr
        operator==(T const& s) const
        {
            std::stringstream ss;
            std::string surroundChar = "\'";
            if (std::is_arithmetic<T>::value == true)
            {
                surroundChar = "`";
            }
            ss << s;
            return JSONPropExpr(m_property + "==" + surroundChar + ss.str() + surroundChar);
        }
        /// @}

        /**
         * Checks for existence of a key inside a JSON data
         * \note this operator only looks for 1st level keys.
         *
         * Example:
         *
         * \code
         * // Sample JSON
         * {
         *    "prop1": {
         *        "p1_prop1"
         *    },
         *
         *    "prop2":"Sample String"
         * }
         * \endcode
         *
         * \code
         * JSONPropExpr expr = (JSONPropExpr) JSONProp("prop1");
         * \endcode
         * above statement frames populates JMESPath expression:
         * <b>"keys(@, contains('prop1'))"</b>
         * which when matched against example JSON data gives `true` boolean value
         *
         * Similarly example below will frame a JMESPath which will return false
         * when tested against Sample JSON data above
         *
         * \code
         * JSONPropExpr expr = (JSONPropExpr) JSONProp("this_property_doesnot_exists");
         * \endcode
         *
         */
        operator JSONPropExpr() const;

        /**
         * Checks for existence of a value inside a JSON array
         * \note this operator only looks for 1st level keys.
         * Example:
         *
         * Sample JSON data:
         * \code
         * {
         *    "prop1": ["value1", "value2", "value3" ]
         * }
         * \endcode
         *
         * JSONPropExpr expr = JSONProp("prop1")["value"];
         * Frames below JMESPath expression
         * "'prop1'.contains(@, 'value')"
         */
        JSONPropExpr operator[](std::string const&) const;

        /**
         * States the absence of the JSON property.
         *
         * @return A JSON expression object.
         */
        JSONPropExpr operator!() const;

        /**
         * \name JSONProp inequality operator
         *
         * @{
         * Convenience operator for JSON inequality.
         *
         * \code
         * JSONProp("attr") != "val"
         * \endcode
         * leads to string "attr!='val'".
         */

        /**
         * @param s A type convertible to std::string.
         * @return A JSON expression object.
         */
        JSONPropExpr operator!=(std::string const& s) const;
        /// Overloaded != with cppmicroservices::Any
        JSONPropExpr operator!=(cppmicroservices::Any const& s) const;
        JSONPropExpr operator!=(bool const& s) const;
        /// Overloaded !=
        template <class T>
        JSONPropExpr
        operator!=(T const& s) const
        {
            std::stringstream ss;
            if constexpr (std::is_arithmetic<T>::value)
            {
                ss << s;
                return JSONPropExpr(m_property + "!=`" + ss.str() + "`");
            }
            else
            {
                std::stringstream ss;
                ss << s;
                return operator!=(ss.str());
            }
        }
        /// @}

        /**
         * @{
         *
         * \name JSONProp comparison operators >= and <=
         *
         * Convenience operator to compare arithmetic values.
         * \warning These operators are only supported for arithmetic and cppmicroservices::Any type values,
         * passing other type is not supported.
         *
         */
        JSONPropExpr operator>=(std::string const& s) const;
        JSONPropExpr operator>=(cppmicroservices::Any const& s) const;
        JSONPropExpr operator>=(bool) const = delete;
        template <class T>
        JSONPropExpr
        operator>=(T const& s) const
        {
            static_assert(std::is_arithmetic<T>::value == true);
            std::stringstream ss;
            ss << s;
            return operator>=(ss.str());
        }

        JSONPropExpr operator<=(std::string const& s) const;
        JSONPropExpr operator<=(cppmicroservices::Any const& s) const;
        JSONPropExpr operator<=(bool) const = delete;
        template <class T>
        JSONPropExpr
        operator<=(T const& s) const
        {
            static_assert(std::is_arithmetic<T>::value == true);
            std::stringstream ss;
            ss << s;
            return operator<=(ss.str());
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
 * json logical and '&&'
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
 * json logical or '||'
 *
 * @param left A json expression.
 * @param right A json expression.
 * @return A json expression
 */
US_Framework_EXPORT cppmicroservices::JSONPropExpr operator||(cppmicroservices::JSONPropExpr const& left,
                                                              cppmicroservices::JSONPropExpr const& right);

#endif // CPPMICROSERVICES_JSONPROP_H
