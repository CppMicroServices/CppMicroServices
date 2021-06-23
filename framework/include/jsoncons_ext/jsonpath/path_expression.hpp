// Copyright 2021 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONPATH_JSONPATH_EXPRESSION_HPP
#define JSONCONS_JSONPATH_JSONPATH_EXPRESSION_HPP

#include <string> // std::basic_string
#include <vector> // std::vector
#include <unordered_map> // std::unordered_map
#include <unordered_set> // std::unordered_set
#include <limits> // std::numeric_limits
#include <set> // std::set
#include <utility> // std::move
#if defined(JSONCONS_HAS_STD_REGEX)
#include <regex>
#endif
#include <jsoncons/json_type.hpp>
#include <jsoncons_ext/jsonpath/jsonpath_error.hpp>

namespace jsoncons { 
namespace jsonpath {

    struct reference_arg_t
    {
        explicit reference_arg_t() = default;
    };
    constexpr reference_arg_t reference_arg{};

    struct const_reference_arg_t
    {
        explicit const_reference_arg_t() = default;
    };
    constexpr const_reference_arg_t const_reference_arg{};

    struct literal_arg_t
    {
        explicit literal_arg_t() = default;
    };
    constexpr literal_arg_t literal_arg{};

    struct begin_expression_type_arg_t
    {
        explicit begin_expression_type_arg_t() = default;
    };
    constexpr begin_expression_type_arg_t begin_expression_type_arg{};

    struct end_expression_type_arg_t
    {
        explicit end_expression_type_arg_t() = default;
    };
    constexpr end_expression_type_arg_t end_expression_type_arg{};

    struct end_of_expression_arg_t
    {
        explicit end_of_expression_arg_t() = default;
    };
    constexpr end_of_expression_arg_t end_of_expression_arg{};

    struct separator_arg_t
    {
        explicit separator_arg_t() = default;
    };
    constexpr separator_arg_t separator_arg{};

    struct lparen_arg_t
    {
        explicit lparen_arg_t() = default;
    };
    constexpr lparen_arg_t lparen_arg{};

    struct rparen_arg_t
    {
        explicit rparen_arg_t() = default;
    };
    constexpr rparen_arg_t rparen_arg{};

    struct begin_union_arg_t
    {
        explicit begin_union_arg_t() = default;
    };
    constexpr begin_union_arg_t begin_union_arg{};

    struct end_union_arg_t
    {
        explicit end_union_arg_t() = default;
    };
    constexpr end_union_arg_t end_union_arg{};

    struct begin_filter_arg_t
    {
        explicit begin_filter_arg_t() = default;
    };
    constexpr begin_filter_arg_t begin_filter_arg{};

    struct end_filter_arg_t
    {
        explicit end_filter_arg_t() = default;
    };
    constexpr end_filter_arg_t end_filter_arg{};

    struct begin_expression_arg_t
    {
        explicit begin_expression_arg_t() = default;
    };
    constexpr begin_expression_arg_t begin_expression_arg{};

    struct end_index_expression_arg_t
    {
        explicit end_index_expression_arg_t() = default;
    };
    constexpr end_index_expression_arg_t end_index_expression_arg{};

    struct end_argument_expression_arg_t
    {
        explicit end_argument_expression_arg_t() = default;
    };
    constexpr end_argument_expression_arg_t end_argument_expression_arg{};

    struct current_node_arg_t
    {
        explicit current_node_arg_t() = default;
    };
    constexpr current_node_arg_t current_node_arg{};

    struct root_node_arg_t
    {
        explicit root_node_arg_t() = default;
    };
    constexpr root_node_arg_t root_node_arg{};

    struct end_function_arg_t
    {
        explicit end_function_arg_t() = default;
    };
    constexpr end_function_arg_t end_function_arg{};

    struct argument_arg_t
    {
        explicit argument_arg_t() = default;
    };
    constexpr argument_arg_t argument_arg{};

    template <class CharT>
    class path_component
    {
        enum class component_kind {root,current,identifier,index};
    public:
        using string_type = std::basic_string<CharT>;
    private:

        component_kind kind_;
        string_type identifier_;
        std::size_t index_;
    public:
        path_component(root_node_arg_t)
            : kind_(component_kind::root)
        {
            identifier_.push_back('$');
        }
        path_component(current_node_arg_t)
            : kind_(component_kind::current)
        {
            identifier_.push_back('@');
        }

        path_component(const string_type& identifier)
            : kind_(component_kind::identifier), identifier_(identifier)
        {
        }

        path_component(std::size_t index)
            : kind_(component_kind::index), index_(index)
        {
        }

        path_component(const path_component&) = default;
        path_component(path_component&&) = default;
        path_component& operator=(const path_component&) = default;
        path_component& operator=(path_component&&) = default;

        bool is_identifier() const
        {
            return kind_ == component_kind::identifier || kind_ == component_kind::root || kind_ == component_kind::current;
        }

        bool is_index() const
        {
            return kind_ == component_kind::index;
        }

        const string_type& identifier() const
        {
            return identifier_;
        }

        std::size_t index() const
        {
            return index_;
        }

        bool operator==(const path_component& other) const
        {
            if (is_identifier() && other.is_identifier())
            {
                return identifier_ == other.identifier_;
            }
            else if (is_index() && other.is_index())
            {
                return index_ == other.index_;
            }
            else
            {
                return false;
            }
        }

        bool operator<(const path_component& other) const
        {
            if (is_identifier() && other.is_identifier())
            {
                return identifier_ < other.identifier_;
            }
            else if (is_index() && other.is_index())
            {
                return index_ < other.index_;
            }
            else
            {
                return is_index() ? true : false;
            }
        }

        void to_string(string_type& buffer) const
        {
            switch (kind_)
            {
                case component_kind::root:
                    buffer.push_back('$');
                    break;
                case component_kind::current:
                    buffer.push_back('@');
                    break;
                case component_kind::identifier:
                    buffer.push_back('[');
                    buffer.push_back('\'');
                    buffer.append(identifier_);
                    buffer.push_back('\'');
                    buffer.push_back(']');
                    break;
                case component_kind::index:
                    buffer.push_back('[');
                    jsoncons::detail::from_integer(index_,buffer);
                    buffer.push_back(']');
                    break;
            }
        }
    };

    template <class CharT>
    bool operator==(const path_component<CharT>& lhs,const path_component<CharT>& rhs)
    {
        return lhs.operator==(rhs);
    }

    template <class CharT>
    bool operator!=(const path_component<CharT>& lhs, const path_component<CharT>& rhs)
    {
        return !(lhs.operator==(rhs));
    }

    template <class CharT>
    bool operator<(const path_component<CharT>& lhs,const path_component<CharT>& rhs)
    {
        return lhs.operator<(rhs);
    }

    template <class CharT>
    std::basic_string<CharT> to_string(const std::vector<path_component<CharT>>& path)
    {
        std::basic_string<CharT> buffer;
        for (const auto& component : path)
        {
            component.to_string(buffer);
        }
        return buffer;
    }

    enum class result_options {value=1,path=2,nodups=4|path,sort=8|path};

    using result_type = result_options;

    inline result_options operator~(result_options a)
    {
        return static_cast<result_options>(~static_cast<unsigned int>(a));
    }

    inline result_options operator&(result_options a, result_options b)
    {
        return static_cast<result_options>(static_cast<unsigned int>(a) & static_cast<unsigned int>(b));
    }

    inline result_options operator^(result_options a, result_options b)
    {
        return static_cast<result_options>(static_cast<unsigned int>(a) ^ static_cast<unsigned int>(b));
    }

    inline result_options operator|(result_options a, result_options b)
    {
        return static_cast<result_options>(static_cast<unsigned int>(a) | static_cast<unsigned int>(b));
    }

    inline result_options operator&=(result_options& a, result_options b)
    {
        a = a & b;
        return a;
    }

    inline result_options operator^=(result_options& a, result_options b)
    {
        a = a ^ b;
        return a;
    }

    inline result_options operator|=(result_options& a, result_options b)
    {
        a = a | b;
        return a;
    }

namespace detail {

    enum class node_type {single=1, multi};

    template <class Json,class JsonReference>
    class dynamic_resources;

    template <class Json,class JsonReference>
    struct unary_operator
    {
        std::size_t precedence_level_;
        bool is_right_associative_;

        unary_operator(std::size_t precedence_level,
                       bool is_right_associative)
            : precedence_level_(precedence_level),
              is_right_associative_(is_right_associative)
        {
        }

        virtual ~unary_operator() = default;

        std::size_t precedence_level() const 
        {
            return precedence_level_;
        }
        bool is_right_associative() const
        {
            return is_right_associative_;
        }

        virtual JsonReference evaluate(dynamic_resources<Json,JsonReference>&,
                                       JsonReference, 
                                       std::error_code&) const = 0;
    };

    template <class Json>
    bool is_false(const Json& val)
    {
        return ((val.is_array() && val.empty()) ||
                 (val.is_object() && val.empty()) ||
                 (val.is_string() && val.as_string_view().empty()) ||
                 (val.is_bool() && !val.as_bool()) ||
                 (val.is_number() && (val == Json(0))) ||
                 val.is_null());
    }

    template <class Json>
    bool is_true(const Json& val)
    {
        return !is_false(val);
    }

    template <class Json,class JsonReference>
    class unary_not_operator final : public unary_operator<Json,JsonReference>
    {
    public:
        unary_not_operator()
            : unary_operator<Json,JsonReference>(1, true)
        {}

        JsonReference evaluate(dynamic_resources<Json,JsonReference>& resources,
                             JsonReference val, 
                             std::error_code&) const override
        {
            return is_false(val) ? resources.true_value() : resources.false_value();
        }
    };

    template <class Json,class JsonReference>
    class unary_minus_operator final : public unary_operator<Json,JsonReference>
    {
    public:
        unary_minus_operator()
            : unary_operator<Json,JsonReference>(1, true)
        {}

        JsonReference evaluate(dynamic_resources<Json,JsonReference>& resources,
                             JsonReference val, 
                             std::error_code&) const override
        {
            if (val.is_int64())
            {
                return *resources.create_json(-val.template as<int64_t>());
            }
            else if (val.is_double())
            {
                return *resources.create_json(-val.as_double());
            }
            else
            {
                return resources.null_value();
            }
        }
    };

    template <class Json,class JsonReference>
    class regex_operator final : public unary_operator<Json,JsonReference>
    {
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;
        std::basic_regex<char_type> pattern_;
    public:
        regex_operator(std::basic_regex<char_type>&& pattern)
            : unary_operator<Json,JsonReference>(2, true),
              pattern_(std::move(pattern))
        {
        }

        regex_operator(regex_operator&&) = default;
        regex_operator& operator=(regex_operator&&) = default;

        JsonReference evaluate(dynamic_resources<Json,JsonReference>& resources, 
                             JsonReference val, 
                             std::error_code&) const override
        {
            if (!val.is_string())
            {
                return resources.null_value();
            }
            return std::regex_search(val.as_string(), pattern_) ? resources.true_value() : resources.false_value();
        }
    };

    template <class Json,class JsonReference>
    struct binary_operator
    {
        std::size_t precedence_level_;
        bool is_right_associative_;

        binary_operator(std::size_t precedence_level,
                        bool is_right_associative = false)
            : precedence_level_(precedence_level),
              is_right_associative_(is_right_associative)
        {
        }

        std::size_t precedence_level() const 
        {
            return precedence_level_;
        }
        bool is_right_associative() const
        {
            return is_right_associative_;
        }

        virtual JsonReference evaluate(dynamic_resources<Json,JsonReference>&,
                             JsonReference, 
                             JsonReference, 

                             std::error_code&) const = 0;

        virtual std::string to_string(int = 0) const
        {
            return "binary operator";
        }

    protected:
        ~binary_operator() = default;
    };

    // Implementations

    template <class Json,class JsonReference>
    class or_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        or_operator()
            : binary_operator<Json,JsonReference>(9)
        {
        }

        JsonReference evaluate(dynamic_resources<Json,JsonReference>& resources, JsonReference lhs, JsonReference rhs, 
                             std::error_code&) const override
        {
            if (lhs.is_null() && rhs.is_null())
            {
                return resources.null_value();
            }
            if (!is_false(lhs))
            {
                return lhs;
            }
            else
            {
                return rhs;
            }
        }
        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                //s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("or operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class and_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        and_operator()
            : binary_operator<Json,JsonReference>(8)
        {
        }

        JsonReference evaluate(dynamic_resources<Json,JsonReference>&, JsonReference lhs, JsonReference rhs, std::error_code&) const override
        {
            if (is_true(lhs))
            {
                return rhs;
            }
            else
            {
                return lhs;
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("and operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class eq_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        eq_operator()
            : binary_operator<Json,JsonReference>(6)
        {
        }

        JsonReference evaluate(dynamic_resources<Json,JsonReference>& resources, JsonReference lhs, JsonReference rhs, std::error_code&) const override 
        {
            return lhs == rhs ? resources.true_value() : resources.false_value();
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("equal operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class ne_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        ne_operator()
            : binary_operator<Json,JsonReference>(6)
        {
        }

        JsonReference evaluate(dynamic_resources<Json,JsonReference>& resources, JsonReference lhs, JsonReference rhs, std::error_code&) const override 
        {
            return lhs != rhs ? resources.true_value() : resources.false_value();
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("not equal operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class lt_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        lt_operator()
            : binary_operator<Json,JsonReference>(5)
        {
        }

        JsonReference evaluate(dynamic_resources<Json,JsonReference>& resources, JsonReference lhs, JsonReference rhs, std::error_code&) const override 
        {
            if (lhs.is_number() && rhs.is_number())
            {
                return lhs < rhs ? resources.true_value() : resources.false_value();
            }
            else if (lhs.is_string() && rhs.is_string())
            {
                return lhs < rhs ? resources.true_value() : resources.false_value();
            }
            return resources.null_value();
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("less than operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class lte_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        lte_operator()
            : binary_operator<Json,JsonReference>(5)
        {
        }

        JsonReference evaluate(dynamic_resources<Json,JsonReference>& resources, JsonReference lhs, JsonReference rhs, std::error_code&) const override 
        {
            if (lhs.is_number() && rhs.is_number())
            {
                return lhs <= rhs ? resources.true_value() : resources.false_value();
            }
            else if (lhs.is_string() && rhs.is_string())
            {
                return lhs <= rhs ? resources.true_value() : resources.false_value();
            }
            return resources.null_value();
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("less than or equal operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class gt_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        gt_operator()
            : binary_operator<Json,JsonReference>(5)
        {
        }

        JsonReference evaluate(dynamic_resources<Json,JsonReference>& resources, JsonReference lhs, JsonReference rhs, std::error_code&) const override
        {
            //std::cout << "operator> lhs: " << lhs << ", rhs: " << rhs << "\n";

            if (lhs.is_number() && rhs.is_number())
            {
                return lhs > rhs ? resources.true_value() : resources.false_value();
            }
            else if (lhs.is_string() && rhs.is_string())
            {
                return lhs > rhs ? resources.true_value() : resources.false_value();
            }
            return resources.null_value();
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("greater than operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class gte_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        gte_operator()
            : binary_operator<Json,JsonReference>(5)
        {
        }

        JsonReference evaluate(dynamic_resources<Json,JsonReference>& resources, JsonReference lhs, JsonReference rhs, std::error_code&) const override
        {
            if (lhs.is_number() && rhs.is_number())
            {
                return lhs >= rhs ? resources.true_value() : resources.false_value();
            }
            else if (lhs.is_string() && rhs.is_string())
            {
                return lhs >= rhs ? resources.true_value() : resources.false_value();
            }
            return resources.null_value();
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("greater than or equal operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class plus_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        plus_operator()
            : binary_operator<Json,JsonReference>(4)
        {
        }

        JsonReference evaluate(dynamic_resources<Json,JsonReference>& resources, JsonReference lhs, JsonReference rhs, std::error_code&) const override
        {
            if (!(lhs.is_number() && rhs.is_number()))
            {
                return resources.null_value();
            }
            else if (lhs.is_int64() && rhs.is_int64())
            {
                return *resources.create_json(((lhs.template as<int64_t>() + rhs.template as<int64_t>())));
            }
            else if (lhs.is_uint64() && rhs.is_uint64())
            {
                return *resources.create_json((lhs.template as<uint64_t>() + rhs.template as<uint64_t>()));
            }
            else
            {
                return *resources.create_json((lhs.as_double() + rhs.as_double()));
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("plus operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class minus_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        minus_operator()
            : binary_operator<Json,JsonReference>(4)
        {
        }

        JsonReference evaluate(dynamic_resources<Json,JsonReference>& resources, JsonReference lhs, JsonReference rhs, std::error_code&) const override
        {
            if (!(lhs.is_number() && rhs.is_number()))
            {
                return resources.null_value();
            }
            else if (lhs.is_int64() && rhs.is_int64())
            {
                return *resources.create_json(((lhs.template as<int64_t>() - rhs.template as<int64_t>())));
            }
            else if (lhs.is_uint64() && rhs.is_uint64())
            {
                return *resources.create_json((lhs.template as<uint64_t>() - rhs.template as<uint64_t>()));
            }
            else
            {
                return *resources.create_json((lhs.as_double() - rhs.as_double()));
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("minus operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class mult_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        mult_operator()
            : binary_operator<Json,JsonReference>(3)
        {
        }

        JsonReference evaluate(dynamic_resources<Json,JsonReference>& resources, JsonReference lhs, JsonReference rhs, std::error_code&) const override
        {
            if (!(lhs.is_number() && rhs.is_number()))
            {
                return resources.null_value();
            }
            else if (lhs.is_int64() && rhs.is_int64())
            {
                return *resources.create_json(((lhs.template as<int64_t>() * rhs.template as<int64_t>())));
            }
            else if (lhs.is_uint64() && rhs.is_uint64())
            {
                return *resources.create_json((lhs.template as<uint64_t>() * rhs.template as<uint64_t>()));
            }
            else
            {
                return *resources.create_json((lhs.as_double() * rhs.as_double()));
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("multiply operator");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class div_operator final : public binary_operator<Json,JsonReference>
    {
    public:
        div_operator()
            : binary_operator<Json,JsonReference>(3)
        {
        }

        JsonReference evaluate(dynamic_resources<Json,JsonReference>& resources, JsonReference lhs, JsonReference rhs, std::error_code&) const override
        {
            //std::cout << "operator/ lhs: " << lhs << ", rhs: " << rhs << "\n";

            if (!(lhs.is_number() && rhs.is_number()))
            {
                return resources.null_value();
            }
            else if (lhs.is_int64() && rhs.is_int64())
            {
                return *resources.create_json(((lhs.template as<int64_t>() / rhs.template as<int64_t>())));
            }
            else if (lhs.is_uint64() && rhs.is_uint64())
            {
                return *resources.create_json((lhs.template as<uint64_t>() / rhs.template as<uint64_t>()));
            }
            else
            {
                return *resources.create_json((lhs.as_double() / rhs.as_double()));
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("divide operator");
            return s;
        }
    };

    // function_base
    template <class Json,class JsonReference>
    class function_base
    {
        jsoncons::optional<std::size_t> arg_count_;
    public:
        using reference = JsonReference;
        using pointer = typename std::conditional<std::is_const<typename std::remove_reference<JsonReference>::type>::value,typename Json::const_pointer,typename Json::pointer>::type;

        function_base(jsoncons::optional<std::size_t> arg_count)
            : arg_count_(arg_count)
        {
        }

        jsoncons::optional<std::size_t> arity() const
        {
            return arg_count_;
        }

        virtual ~function_base() = default;

        virtual reference evaluate(dynamic_resources<Json,JsonReference>& resources,
                                   const std::vector<pointer>& args, 
                                   std::error_code& ec) const = 0;

        virtual std::string to_string(int level = 0) const
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("function");
            return s;
        }
    };  

    template <class Json,class JsonReference>
    class contains_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;
        using string_view_type = typename Json::string_view_type;

        contains_function()
            : function_base<Json, JsonReference>(2)
        {
        }

        reference evaluate(dynamic_resources<Json,JsonReference>& resources,
                           const std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return resources.null_value();
            }

            pointer arg0_ptr = args[0];
            pointer arg1_ptr = args[1];

            switch (arg0_ptr->type())
            {
                case json_type::array_value:
                    for (auto& j : arg0_ptr->array_range())
                    {
                        if (j == *arg1_ptr)
                        {
                            return resources.true_value();
                        }
                    }
                    return resources.false_value();
                case json_type::string_value:
                {
                    if (!arg1_ptr->is_string())
                    {
                        ec = jsonpath_errc::invalid_type;
                        return resources.null_value();
                    }
                    auto sv0 = arg0_ptr->template as<string_view_type>();
                    auto sv1 = arg1_ptr->template as<string_view_type>();
                    return sv0.find(sv1) != string_view_type::npos ? resources.true_value() : resources.false_value();
                }
                default:
                {
                    ec = jsonpath_errc::invalid_type;
                    return resources.null_value();
                }
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("contains function");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class ends_with_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;
        using string_view_type = typename Json::string_view_type;

        ends_with_function()
            : function_base<Json, JsonReference>(2)
        {
        }

        reference evaluate(dynamic_resources<Json,JsonReference>& resources,
                           const std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return resources.null_value();
            }

            pointer arg0_ptr = args[0];
            if (!arg0_ptr->is_string())
            {
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }

            pointer arg1_ptr = args[1];
            if (!arg1_ptr->is_string())
            {
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }

            auto sv0 = arg0_ptr->template as<string_view_type>();
            auto sv1 = arg1_ptr->template as<string_view_type>();

            if (sv1.length() <= sv0.length() && sv1 == sv0.substr(sv0.length() - sv1.length()))
            {
                return resources.true_value();
            }
            else
            {
                return resources.false_value();
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("ends_with function");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class starts_with_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;
        using string_view_type = typename Json::string_view_type;

        starts_with_function()
            : function_base<Json, JsonReference>(2)
        {
        }

        reference evaluate(dynamic_resources<Json,JsonReference>& resources,
                           const std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return resources.null_value();
            }

            pointer arg0_ptr = args[0];
            if (!arg0_ptr->is_string())
            {
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }

            pointer arg1_ptr = args[1];
            if (!arg1_ptr->is_string())
            {
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }

            auto sv0 = arg0_ptr->template as<string_view_type>();
            auto sv1 = arg1_ptr->template as<string_view_type>();

            if (sv1.length() <= sv0.length() && sv1 == sv0.substr(0, sv1.length()))
            {
                return resources.true_value();
            }
            else
            {
                return resources.false_value();
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("starts_with function");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class sum_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;

        sum_function()
            : function_base<Json, JsonReference>(1)
        {
        }

        reference evaluate(dynamic_resources<Json,JsonReference>& resources,
                           const std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return resources.null_value();
            }

            pointer arg0_ptr = args[0];
            if (!arg0_ptr->is_array())
            {
                //std::cout << "arg: " << *arg0_ptr << "\n";
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }
            //std::cout << "sum function arg: " << *arg0_ptr << "\n";

            double sum = 0;
            for (auto& j : arg0_ptr->array_range())
            {
                if (!j.is_number())
                {
                    ec = jsonpath_errc::invalid_type;
                    return resources.null_value();
                }
                sum += j.template as<double>();
            }

            return *resources.create_json(sum);
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("sum function");
            return s;
        }
    };

#if defined(JSONCONS_HAS_STD_REGEX)

    template <class Json,class JsonReference>
    class tokenize_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;

        tokenize_function()
            : function_base<Json, JsonReference>(2)
        {
        }

        reference evaluate(dynamic_resources<Json,JsonReference>& resources,
                           const std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return resources.null_value();
            }

            if (!args[0]->is_string() || !args[1]->is_string())
            {
                //std::cout << "arg: " << *arg0_ptr << "\n";
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }
            auto arg0 = args[0]->template as<string_type>();
            auto arg1 = args[1]->template as<string_type>();

            std::regex::flag_type options = std::regex_constants::ECMAScript; 
            std::basic_regex<char_type> pieces_regex(arg1, options);

            std::regex_token_iterator<typename string_type::const_iterator> rit ( arg0.begin(), arg0.end(), pieces_regex, -1);
            std::regex_token_iterator<typename string_type::const_iterator> rend;

            auto j = resources.create_json(json_array_arg);
            while (rit!=rend) 
            {
                j->emplace_back(rit->str());
                ++rit;
            }
            return *j;
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("tokenize function");
            return s;
        }
    };

#endif // defined(JSONCONS_HAS_STD_REGEX)

    template <class Json,class JsonReference>
    class ceil_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;

        ceil_function()
            : function_base<Json, JsonReference>(1)
        {
        }

        reference evaluate(dynamic_resources<Json,JsonReference>& resources,
                           const std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return resources.null_value();
            }

            pointer arg0_ptr = args[0];
            switch (arg0_ptr->type())
            {
                case json_type::uint64_value:
                case json_type::int64_value:
                {
                    return *resources.create_json(arg0_ptr->template as<double>());
                }
                case json_type::double_value:
                {
                    return *resources.create_json(std::ceil(arg0_ptr->template as<double>()));
                }
                default:
                    ec = jsonpath_errc::invalid_type;
                    return resources.null_value();
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("ceil function");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class floor_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;

        floor_function()
            : function_base<Json, JsonReference>(1)
        {
        }

        reference evaluate(dynamic_resources<Json,JsonReference>& resources,
                           const std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return resources.null_value();
            }

            pointer arg0_ptr = args[0];
            switch (arg0_ptr->type())
            {
                case json_type::uint64_value:
                case json_type::int64_value:
                {
                    return *resources.create_json(arg0_ptr->template as<double>());
                }
                case json_type::double_value:
                {
                    return *resources.create_json(std::floor(arg0_ptr->template as<double>()));
                }
                default:
                    ec = jsonpath_errc::invalid_type;
                    return resources.null_value();
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("floor function");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class to_number_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;

        to_number_function()
            : function_base<Json, JsonReference>(1)
        {
        }

        reference evaluate(dynamic_resources<Json,JsonReference>& resources,
                           const std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return resources.null_value();
            }

            pointer arg0_ptr = args[0];
            switch (arg0_ptr->type())
            {
                case json_type::int64_value:
                case json_type::uint64_value:
                case json_type::double_value:
                    return *arg0_ptr;
                case json_type::string_value:
                {
                    auto sv = arg0_ptr->as_string_view();
                    auto result1 = jsoncons::detail::to_integer<uint64_t>(sv.data(), sv.length());
                    if (result1)
                    {
                        return *resources.create_json(result1.value());
                    }
                    auto result2 = jsoncons::detail::to_integer<int64_t>(sv.data(), sv.length());
                    if (result2)
                    {
                        return *resources.create_json(result2.value());
                    }
                    jsoncons::detail::to_double_t to_double;
                    try
                    {
                        auto s = arg0_ptr->as_string();
                        double d = to_double(s.c_str(), s.length());
                        return *resources.create_json(d);
                    }
                    catch (const std::exception&)
                    {
                        return resources.null_value();
                    }
                }
                default:
                    ec = jsonpath_errc::invalid_type;
                    return resources.null_value();
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("to_number function");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class prod_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;

        prod_function()
            : function_base<Json, JsonReference>(1)
        {
        }

        reference evaluate(dynamic_resources<Json,JsonReference>& resources,
                           const std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return resources.null_value();
            }

            pointer arg0_ptr = args[0];
            if (!arg0_ptr->is_array() || arg0_ptr->empty())
            {
                //std::cout << "arg: " << *arg0_ptr << "\n";
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }
            double prod = 1;
            for (auto& j : arg0_ptr->array_range())
            {
                if (!j.is_number())
                {
                    ec = jsonpath_errc::invalid_type;
                    return resources.null_value();
                }
                prod *= j.template as<double>();
            }

            return *resources.create_json(prod);
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("prod function");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class avg_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;

        avg_function()
            : function_base<Json, JsonReference>(1)
        {
        }

        reference evaluate(dynamic_resources<Json,JsonReference>& resources,
                           const std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return resources.null_value();
            }

            pointer arg0_ptr = args[0];
            if (!arg0_ptr->is_array())
            {
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }
            if (arg0_ptr->empty())
            {
                return resources.null_value();
            }
            double sum = 0;
            for (auto& j : arg0_ptr->array_range())
            {
                if (!j.is_number())
                {
                    ec = jsonpath_errc::invalid_type;
                    return resources.null_value();
                }
                sum += j.template as<double>();
            }

            return *resources.create_json(sum / static_cast<double>(arg0_ptr->size()));
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("to_string function");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class min_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;

        min_function()
            : function_base<Json, JsonReference>(1)
        {
        }

        reference evaluate(dynamic_resources<Json,JsonReference>& resources,
                           const std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return resources.null_value();
            }

            pointer arg0_ptr = args[0];
            if (!arg0_ptr->is_array())
            {
                //std::cout << "arg: " << *arg0_ptr << "\n";
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }
            if (arg0_ptr->empty())
            {
                return resources.null_value();
            }
            bool is_number = arg0_ptr->at(0).is_number();
            bool is_string = arg0_ptr->at(0).is_string();
            if (!is_number && !is_string)
            {
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }

            std::size_t index = 0;
            for (std::size_t i = 1; i < arg0_ptr->size(); ++i)
            {
                if (!(arg0_ptr->at(i).is_number() == is_number && arg0_ptr->at(i).is_string() == is_string))
                {
                    ec = jsonpath_errc::invalid_type;
                    return resources.null_value();
                }
                if (arg0_ptr->at(i) < arg0_ptr->at(index))
                {
                    index = i;
                }
            }

            return arg0_ptr->at(index);
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("min function");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class max_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;

        max_function()
            : function_base<Json, JsonReference>(1)
        {
        }

        reference evaluate(dynamic_resources<Json,JsonReference>& resources,
                           const std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return resources.null_value();
            }

            pointer arg0_ptr = args[0];
            if (!arg0_ptr->is_array())
            {
                //std::cout << "arg: " << *arg0_ptr << "\n";
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }
            if (arg0_ptr->empty())
            {
                return resources.null_value();
            }

            bool is_number = arg0_ptr->at(0).is_number();
            bool is_string = arg0_ptr->at(0).is_string();
            if (!is_number && !is_string)
            {
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }

            std::size_t index = 0;
            for (std::size_t i = 1; i < arg0_ptr->size(); ++i)
            {
                if (!(arg0_ptr->at(i).is_number() == is_number && arg0_ptr->at(i).is_string() == is_string))
                {
                    ec = jsonpath_errc::invalid_type;
                    return resources.null_value();
                }
                if (arg0_ptr->at(i) > arg0_ptr->at(index))
                {
                    index = i;
                }
            }

            return arg0_ptr->at(index);
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("max function");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class abs_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;

        abs_function()
            : function_base<Json, JsonReference>(1)
        {
        }

        reference evaluate(dynamic_resources<Json,JsonReference>& resources,
                           const std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return resources.null_value();
            }

            pointer arg0_ptr = args[0];
            switch (arg0_ptr->type())
            {
                case json_type::uint64_value:
                    return *arg0_ptr;
                case json_type::int64_value:
                {
                    pointer j_ptr = arg0_ptr->template as<int64_t>() >= 0 ? arg0_ptr : resources.create_json(std::abs(arg0_ptr->template as<int64_t>()));
                    return *j_ptr;
                }
                case json_type::double_value:
                {
                    pointer j_ptr = arg0_ptr->template as<double>() >= 0 ? arg0_ptr : resources.create_json(std::abs(arg0_ptr->template as<double>()));
                    return *j_ptr;
                }
                default:
                {
                    ec = jsonpath_errc::invalid_type;
                    return resources.null_value();
                }
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("abs function");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class length_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;
        using string_view_type = typename Json::string_view_type;

        length_function()
            : function_base<Json,JsonReference>(1)
        {
        }

        reference evaluate(dynamic_resources<Json,JsonReference>& resources,
                           const std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return resources.null_value();
            }

            pointer arg0_ptr = args[0];
            //std::cout << "length function arg: " << *arg0_ptr << "\n";

            switch (arg0_ptr->type())
            {
                case json_type::object_value:
                case json_type::array_value:
                    return *resources.create_json(arg0_ptr->size());
                case json_type::string_value:
                {
                    auto sv0 = arg0_ptr->template as<string_view_type>();
                    auto length = unicode_traits::count_codepoints(sv0.data(), sv0.size());
                    return *resources.create_json(length);
                }
                default:
                {
                    ec = jsonpath_errc::invalid_type;
                    return resources.null_value();
                }
            }
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("length function");
            return s;
        }
    };

    template <class Json,class JsonReference>
    class keys_function : public function_base<Json,JsonReference>
    {
    public:
        using reference = typename function_base<Json,JsonReference>::reference;
        using pointer = typename function_base<Json,JsonReference>::pointer;
        using string_view_type = typename Json::string_view_type;

        keys_function()
            : function_base<Json,JsonReference>(1)
        {
        }

        reference evaluate(dynamic_resources<Json,JsonReference>& resources,
                           const std::vector<pointer>& args, 
                           std::error_code& ec) const override
        {
            if (args.size() != *this->arity())
            {
                ec = jsonpath_errc::invalid_arity;
                return resources.null_value();
            }

            pointer arg0_ptr = args[0];
            if (!arg0_ptr->is_object())
            {
                ec = jsonpath_errc::invalid_type;
                return resources.null_value();
            }

            auto result = resources.create_json(json_array_arg);
            result->reserve(args.size());

            for (auto& item : arg0_ptr->object_range())
            {
                result->emplace_back(item.key());
            }
            return *result;
        }

        std::string to_string(int level = 0) const override
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("keys function");
            return s;
        }
    };

    template <class Json, class JsonReference>
    struct static_resources
    {
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type>;
        using reference = JsonReference;
        using function_base_type = function_base<Json,JsonReference>;

        std::vector<std::unique_ptr<Json>> temp_json_values_;
        std::vector<std::unique_ptr<unary_operator<Json,JsonReference>>> unary_operators_;

        static_resources()
        {
        }

        const function_base_type* get_function(const string_type& name, std::error_code& ec) const
        {
            static abs_function<Json,JsonReference> abs_func;
            static contains_function<Json,JsonReference> contains_func;
            static starts_with_function<Json,JsonReference> starts_with_func;
            static ends_with_function<Json,JsonReference> ends_with_func;
            static ceil_function<Json,JsonReference> ceil_func;
            static floor_function<Json, JsonReference> floor_func;
            static to_number_function<Json,JsonReference> to_number_func;
            static sum_function<Json,JsonReference> sum_func;
            static prod_function<Json,JsonReference> prod_func;
            static avg_function<Json,JsonReference> avg_func;
            static min_function<Json,JsonReference> min_func;
            static max_function<Json,JsonReference> max_func;
            static length_function<Json,JsonReference> length_func;
            static keys_function<Json,JsonReference> keys_func;
#if defined(JSONCONS_HAS_STD_REGEX)
            static tokenize_function<Json,JsonReference> tokenize_func;
#endif

            static std::unordered_map<string_type,const function_base_type*> functions =
            {
                {string_type{'a','b','s'}, &abs_func},
                {string_type{'c','o','n','t','a','i','n','s'}, &contains_func},
                {string_type{'s','t','a','r','t','s','_','w','i','t','h'}, &starts_with_func},
                {string_type{'e','n','d','s','_','w','i','t','h'}, &ends_with_func},
                {string_type{'c','e','i','l'}, &ceil_func},
                {string_type{'f','l','o','o','r'}, &floor_func},
                {string_type{'t','o','_','n','u','m','b','e','r'}, &to_number_func},
                {string_type{'s','u','m'}, &sum_func},
                {string_type{'p','r','o', 'd'}, &prod_func},
                {string_type{'a','v','g'}, &avg_func},
                {string_type{'m','i','n'}, &min_func},
                {string_type{'m','a','x'}, &max_func},
                {string_type{'l','e','n','g','t','h'}, &length_func},
                {string_type{'k','e','y','s'}, &keys_func},
#if defined(JSONCONS_HAS_STD_REGEX)
                {string_type{'t','o','k','e','n','i','z','e'}, &tokenize_func},
#endif
                {string_type{'c','o','u','n','t'}, &length_func}
            };

            auto it = functions.find(name);
            if (it == functions.end())
            {
                ec = jsonpath_errc::unknown_function;
                return nullptr;
            }
            return it->second;
        }

        const unary_operator<Json,JsonReference>* get_unary_not() const
        {
            static unary_not_operator<Json,JsonReference> oper;
            return &oper;
        }

        const unary_operator<Json,JsonReference>* get_unary_minus() const
        {
            static unary_minus_operator<Json,JsonReference> oper;
            return &oper;
        }

        const unary_operator<Json,JsonReference>* get_regex_operator(std::basic_regex<char_type>&& pattern) 
        {
            unary_operators_.push_back(jsoncons::make_unique<regex_operator<Json,JsonReference>>(std::move(pattern)));
            return unary_operators_.back().get();
        }

        const binary_operator<Json,JsonReference>* get_or_operator() const
        {
            static or_operator<Json,JsonReference> oper;

            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_and_operator() const
        {
            static and_operator<Json,JsonReference> oper;

            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_eq_operator() const
        {
            static eq_operator<Json,JsonReference> oper;
            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_ne_operator() const
        {
            static ne_operator<Json,JsonReference> oper;
            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_lt_operator() const
        {
            static lt_operator<Json,JsonReference> oper;
            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_lte_operator() const
        {
            static lte_operator<Json,JsonReference> oper;
            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_gt_operator() const
        {
            static gt_operator<Json,JsonReference> oper;
            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_gte_operator() const
        {
            static gte_operator<Json,JsonReference> oper;
            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_plus_operator() const
        {
            static plus_operator<Json,JsonReference> oper;
            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_minus_operator() const
        {
            static minus_operator<Json,JsonReference> oper;
            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_mult_operator() const
        {
            static mult_operator<Json,JsonReference> oper;
            return &oper;
        }

        const binary_operator<Json,JsonReference>* get_div_operator() const
        {
            static div_operator<Json,JsonReference> oper;
            return &oper;
        }

        template <typename... Args>
        Json* create_json(Args&& ... args)
        {
            auto temp = jsoncons::make_unique<Json>(std::forward<Args>(args)...);
            Json* ptr = temp.get();
            temp_json_values_.emplace_back(std::move(temp));
            return ptr;
        }
    };

    enum class token_kind
    {
        root_node,
        current_node,
        lparen,
        rparen,
        begin_union,
        end_union,
        begin_filter,
        end_filter,
        begin_expression,
        end_index_expression,
        end_argument_expression,
        separator,
        literal,
        selector,
        function,
        end_function,
        argument,
        begin_expression_type,
        end_expression_type,
        end_of_expression,
        unary_operator,
        binary_operator
    };

    inline
    std::string to_string(token_kind kind)
    {
        switch (kind)
        {
            case token_kind::root_node:
                return "root_node";
            case token_kind::current_node:
                return "current_node";
            case token_kind::lparen:
                return "lparen";
            case token_kind::rparen:
                return "rparen";
            case token_kind::begin_union:
                return "begin_union";
            case token_kind::end_union:
                return "end_union";
            case token_kind::begin_filter:
                return "begin_filter";
            case token_kind::end_filter:
                return "end_filter";
            case token_kind::begin_expression:
                return "begin_expression";
            case token_kind::end_index_expression:
                return "end_index_expression";
            case token_kind::end_argument_expression:
                return "end_argument_expression";
            case token_kind::separator:
                return "separator";
            case token_kind::literal:
                return "literal";
            case token_kind::selector:
                return "selector";
            case token_kind::function:
                return "function";
            case token_kind::end_function:
                return "end_function";
            case token_kind::argument:
                return "argument";
            case token_kind::begin_expression_type:
                return "begin_expression_type";
            case token_kind::end_expression_type:
                return "end_expression_type";
            case token_kind::end_of_expression:
                return "end_of_expression";
            case token_kind::unary_operator:
                return "unary_operator";
            case token_kind::binary_operator:
                return "binary_operator";
            default:
                return "";
        }
    }

    template <class Json,class JsonReference>
    struct path_node
    {
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type,std::char_traits<char_type>>;
        using reference = JsonReference;
        using pointer = typename std::conditional<std::is_const<typename std::remove_reference<JsonReference>::type>::value,typename Json::const_pointer,typename Json::pointer>::type;
        using path_component_type = path_component<char_type>;

        std::vector<path_component_type> path;
        pointer ptr;

        path_node(const std::vector<path_component_type>& p, const pointer& valp)
            : path(p),ptr(valp)
        {
        }
        path_node(const pointer& valp)
            : ptr(valp)
        {
        }

        path_node(std::vector<path_component_type>&& p, pointer&& valp) noexcept
            : path(std::move(p)),ptr(valp)
        {
        }
        path_node(const path_node&) = default;

        path_node(path_node&& other) noexcept
            : path(std::move(other.path)), ptr(other.ptr)
        {

        }
        path_node& operator=(const path_node&) = default;

        path_node& operator=(path_node&& other) noexcept
        {
            path.swap(other.path);
            ptr = other.ptr;
            return *this;
        }
    };
 
    template <class Json,class JsonReference>
    struct path_node_less
    {
        bool operator()(const path_node<Json,JsonReference>& a,
                        const path_node<Json,JsonReference>& b) const noexcept
        {
            return a.path < b.path;
        }
    };

    template <class Json,class JsonReference>
    struct path_node_equal
    {
        bool operator()(const path_node<Json,JsonReference>& lhs,
                        const path_node<Json,JsonReference>& rhs) const noexcept
        {
            if (lhs.path.size() != rhs.path.size())
            {
                return false;
            }
            for (std::size_t i = 0; i < lhs.path.size(); ++i)
            {
                if (lhs.path[i] != rhs.path[i])
                {
                    return false;
                }
            }
            return true;
        }
    };

    template <class Json, class JsonReference>
    class dynamic_resources
    {
        std::vector<std::unique_ptr<Json>> temp_json_values_;
        std::unordered_map<std::size_t,std::pair<std::vector<path_node<Json,JsonReference>>,node_type>> cache_;
    public:

        bool is_cached(std::size_t id) const
        {
            return cache_.find(id) != cache_.end();
        }

        void add_to_cache(std::size_t id, const std::vector<path_node<Json,JsonReference>>& val, node_type ndtype) 
        {
            cache_.emplace(id,std::make_pair(val,ndtype));
        }

        void retrieve_from_cache(std::size_t id, std::vector<path_node<Json,JsonReference>>& nodes, node_type& ndtype) 
        {
            auto it = cache_.find(id);
            if (it != cache_.end())
            {
                for (auto& item : it->second.first)
                {
                    nodes.push_back(item);
                }
                ndtype = it->second.second;
            }
        }

        Json& true_value() const
        {
            static Json value(true, semantic_tag::none);
            return value;
        }

        Json& false_value() const
        {
            static Json value(false, semantic_tag::none);
            return value;
        }

        Json& null_value() const
        {
            static Json value(null_type(), semantic_tag::none);
            return value;
        }

        template <typename... Args>
        Json* create_json(Args&& ... args)
        {
            auto temp = jsoncons::make_unique<Json>(std::forward<Args>(args)...);
            Json* ptr = temp.get();
            temp_json_values_.emplace_back(std::move(temp));
            return ptr;
        }
    };

    template <class Json,class JsonReference>
    struct node_less
    {
        bool operator()(const path_node<Json,JsonReference>& a, const path_node<Json,JsonReference>& b) const
        {
            return *(a.ptr) < *(b.ptr);
        }
    };

    template <class Json,class JsonReference>
    class selector_base
    {
        bool is_path_;
        std::size_t precedence_level_;
    public:
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type,std::char_traits<char_type>>;
        using string_view_type = jsoncons::basic_string_view<char_type, std::char_traits<char_type>>;
        using reference = JsonReference;
        using path_node_type = path_node<Json,JsonReference>;
        using path_component_type = path_component<char_type>;

        selector_base(bool is_path,
                      std::size_t precedence_level = 0)
            : is_path_(is_path), 
              precedence_level_(precedence_level)
        {
        }

        virtual ~selector_base() noexcept = default;

        bool is_path() const 
        {
            return is_path_;
        }

        std::size_t precedence_level() const
        {
            return precedence_level_;
        }

        bool is_right_associative() const
        {
            return true;
        }

        static std::vector<path_component_type> generate_path(const std::vector<path_component_type>& path, 
                                                              std::size_t index, 
                                                              result_options options) 
        {
            std::vector<path_component_type> s(path);
            if ((options & result_options::path) == result_options::path)
            {
                s.emplace_back(index);
            }
            return s;
        }

        static std::vector<path_component_type> generate_path(const std::vector<path_component_type>& path, 
                                                              const string_type& identifier, 
                                                              result_options options) 
        {
            std::vector<path_component_type> s(path);
            if ((options & result_options::path) == result_options::path)
            {
                s.emplace_back(identifier);
            }
            return s;
        }

        virtual void select(dynamic_resources<Json,JsonReference>& resources,
                            const std::vector<path_component_type>& path, 
                            reference root,
                            reference val, 
                            std::vector<path_node_type>& nodes,
                            node_type& ndtype,
                            result_options options) const = 0;

        virtual void append_selector(std::unique_ptr<selector_base>&&) 
        {
        }

        virtual std::string to_string(int = 0) const
        {
            return std::string();
        }
    };

    template <class Json,class JsonReference>
    class token
    {
    public:
        using selector_base_type = selector_base<Json,JsonReference>;

        token_kind type_;

        union
        {
            std::unique_ptr<selector_base_type> selector_;
            const unary_operator<Json,JsonReference>* unary_operator_;
            const binary_operator<Json,JsonReference>* binary_operator_;
            const function_base<Json,JsonReference>* function_;
            Json value_;
        };
    public:

        token(const unary_operator<Json,JsonReference>* expression) noexcept
            : type_(token_kind::unary_operator),
              unary_operator_(expression)
        {
        }

        token(const binary_operator<Json,JsonReference>* expression) noexcept
            : type_(token_kind::binary_operator),
              binary_operator_(expression)
        {
        }

        token(current_node_arg_t) noexcept
            : type_(token_kind::current_node)
        {
        }

        token(root_node_arg_t) noexcept
            : type_(token_kind::root_node)
        {
        }

        token(end_function_arg_t) noexcept
            : type_(token_kind::end_function)
        {
        }

        token(separator_arg_t) noexcept
            : type_(token_kind::separator)
        {
        }

        token(lparen_arg_t) noexcept
            : type_(token_kind::lparen)
        {
        }

        token(rparen_arg_t) noexcept
            : type_(token_kind::rparen)
        {
        }

        token(end_of_expression_arg_t) noexcept
            : type_(token_kind::end_of_expression)
        {
        }

        token(begin_union_arg_t) noexcept
            : type_(token_kind::begin_union)
        {
        }

        token(end_union_arg_t) noexcept
            : type_(token_kind::end_union)
        {
        }

        token(begin_filter_arg_t) noexcept
            : type_(token_kind::begin_filter)
        {
        }

        token(end_filter_arg_t) noexcept
            : type_(token_kind::end_filter)
        {
        }

        token(begin_expression_arg_t) noexcept
            : type_(token_kind::begin_expression)
        {
        }

        token(end_index_expression_arg_t) noexcept
            : type_(token_kind::end_index_expression)
        {
        }

        token(end_argument_expression_arg_t) noexcept
            : type_(token_kind::end_argument_expression)
        {
        }

        token(std::unique_ptr<selector_base_type>&& expression)
            : type_(token_kind::selector)
        {
            new (&selector_) std::unique_ptr<selector_base_type>(std::move(expression));
        }

        token(const function_base<Json,JsonReference>* function) noexcept
            : type_(token_kind::function),
              function_(function)
        {
        }

        token(argument_arg_t) noexcept
            : type_(token_kind::argument)
        {
        }

        token(begin_expression_type_arg_t) noexcept
            : type_(token_kind::begin_expression_type)
        {
        }

        token(end_expression_type_arg_t) noexcept
            : type_(token_kind::end_expression_type)
        {
        }

        token(literal_arg_t, Json&& value) noexcept
            : type_(token_kind::literal), value_(std::move(value))
        {
        }

        token(token&& other) noexcept
        {
            construct(std::forward<token>(other));
        }

        const Json& get_value(const_reference_arg_t, dynamic_resources<Json,JsonReference>&) const
        {
            return value_;
        }

        Json& get_value(reference_arg_t, dynamic_resources<Json,JsonReference>& resources) const
        {
            return *resources.create_json(value_);
        }

        token& operator=(token&& other)
        {
            if (&other != this)
            {
                if (type_ == other.type_)
                {
                    switch (type_)
                    {
                        case token_kind::selector:
                            selector_ = std::move(other.selector_);
                            break;
                        case token_kind::unary_operator:
                            unary_operator_ = other.unary_operator_;
                            break;
                        case token_kind::binary_operator:
                            binary_operator_ = other.binary_operator_;
                            break;
                        case token_kind::function:
                            function_ = other.function_;
                            break;
                        case token_kind::literal:
                            value_ = std::move(other.value_);
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                    destroy();
                    construct(std::forward<token>(other));
                }
            }
            return *this;
        }

        ~token() noexcept
        {
            destroy();
        }

        token_kind type() const
        {
            return type_;
        }

        bool is_lparen() const
        {
            return type_ == token_kind::lparen; 
        }

        bool is_rparen() const
        {
            return type_ == token_kind::rparen; 
        }

        bool is_current_node() const
        {
            return type_ == token_kind::current_node; 
        }

        bool is_path() const
        {
            return type_ == token_kind::selector && selector_->is_path(); 
        }

        bool is_operator() const
        {
            return type_ == token_kind::unary_operator || 
                   type_ == token_kind::binary_operator; 
        }

        std::size_t precedence_level() const
        {
            switch(type_)
            {
                case token_kind::selector:
                    return selector_->precedence_level();
                case token_kind::unary_operator:
                    return unary_operator_->precedence_level();
                case token_kind::binary_operator:
                    return binary_operator_->precedence_level();
                default:
                    return 0;
            }
        }

        jsoncons::optional<std::size_t> arity() const
        {
            return type_ == token_kind::function ? function_->arity() : jsoncons::optional<std::size_t>();
        }

        bool is_right_associative() const
        {
            switch(type_)
            {
                case token_kind::selector:
                    return selector_->is_right_associative();
                case token_kind::unary_operator:
                    return unary_operator_->is_right_associative();
                case token_kind::binary_operator:
                    return binary_operator_->is_right_associative();
                default:
                    return false;
            }
        }

        void construct(token&& other)
        {
            type_ = other.type_;
            switch (type_)
            {
                case token_kind::selector:
                    new (&selector_) std::unique_ptr<selector_base_type>(std::move(other.selector_));
                    break;
                case token_kind::unary_operator:
                    unary_operator_ = other.unary_operator_;
                    break;
                case token_kind::binary_operator:
                    binary_operator_ = other.binary_operator_;
                    break;
                case token_kind::function:
                    function_ = other.function_;
                    break;
                case token_kind::literal:
                    new (&value_) Json(std::move(other.value_));
                    break;
                default:
                    break;
            }
        }

        void destroy() noexcept 
        {
            switch(type_)
            {
                case token_kind::selector:
                    selector_.~unique_ptr();
                    break;
                case token_kind::literal:
                    value_.~Json();
                    break;
                default:
                    break;
            }
        }

        std::string to_string(int level = 0) const
        {
            std::string s;
            switch (type_)
            {
                case token_kind::root_node:
                    if (level > 0)
                    {
                        s.append("\n");
                        s.append(level*2, ' ');
                    }
                    s.append("root node");
                    return s;
                case token_kind::current_node:
                    if (level > 0)
                    {
                        s.append("\n");
                        s.append(level*2, ' ');
                    }
                    s.append("current node");
                    return s;
                case token_kind::argument:
                    if (level > 0)
                    {
                        s.append("\n");
                        s.append(level*2, ' ');
                    }
                    s.append("argument");
                    return s;
                case token_kind::selector:
                    s.append(selector_->to_string(level));
                    break;
                case token_kind::literal:
                {
                    if (level > 0)
                    {
                        s.append("\n");
                        s.append(level*2, ' ');
                    }
                    auto sbuf = value_.to_string();
                    unicode_traits::convert(sbuf.data(), sbuf.size(), s);
                    break;
                }
                case token_kind::binary_operator:
                    s.append(binary_operator_->to_string(level));
                    break;
                case token_kind::function:
                    s.append(function_->to_string(level));
                    break;
                default:
                    if (level > 0)
                    {
                        s.append("\n");
                        s.append(level*2, ' ');
                    }
                    s.append("token kind: ");
                    s.append(jsoncons::jsonpath::detail::to_string(type_));
                    break;
            }
            //s.append("\n");
            return s;
        }
    };

    enum class node_set_tag {none,single,multi};

    template <class Json,class JsonReference>
    struct node_set
    {
        using path_node_type = path_node<Json,JsonReference>;
        using pointer = typename path_node_type::pointer;

        node_set_tag tag;
        union
        {
            path_node_type node;
            std::vector<path_node_type> nodes;
        };

        node_set() noexcept
            : tag(node_set_tag::none), node()
        {
        }
        node_set(path_node_type&& node) noexcept
            : tag(node_set_tag::single),
            node(std::move(node))
        {
        }
        node_set(std::vector<path_node_type>&& nds, node_type ndtype) noexcept
        {
            if (nds.empty())
            {
                tag = node_set_tag::none;
            }
            else if (nds.size() == 1 && (ndtype == node_type::single || ndtype == node_type()))
            {
                tag = node_set_tag::single;
                new (&node)path_node_type(nds.back());
            }
            else
            {
                tag = node_set_tag::multi;
                new (&nodes) std::vector<path_node_type>(nds);
            }
        }

        node_set(node_set&& other) noexcept
        {
            construct(std::forward<node_set>(other));
        }

        ~node_set() noexcept
        {
            destroy();
        }

        node_set& operator=(node_set&& other)
        {
            if (&other != this)
            {
                if (tag == other.tag)
                {
                    switch (tag)
                    {
                        case node_set_tag::single:
                            node = std::move(other.node);
                            break;
                        case node_set_tag::multi:
                            nodes = std::move(other.nodes);
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                    destroy();
                    construct(std::forward<node_set>(other));
                }
            }
            return *this;
        }

        pointer to_pointer(dynamic_resources<Json,JsonReference>& resources) const
        {
            switch (tag)
            {
                case node_set_tag::single:
                    return node.ptr;
                case node_set_tag::multi:
                {
                    auto j = resources.create_json(json_array_arg);
                    j->reserve(nodes.size());
                    for (auto& item : nodes)
                    {
                        j->emplace_back(*item.ptr);
                    }
                    return j;
                }
                default:
                    return &resources.null_value();
            }
        }

        void construct(node_set&& other) noexcept
        {
            tag = other.tag;
            switch (tag)
            {
                case node_set_tag::single:
                    new (&node) path_node_type(std::move(other.node));
                    break;
                case node_set_tag::multi:
                    new (&nodes) std::vector<path_node_type>(std::move(other.nodes));
                    break;
                default:
                    break;
            }
        }

        void destroy() noexcept 
        {
            switch(tag)
            {
                case node_set_tag::multi:
                    nodes.~vector();
                    break;
                case node_set_tag::single:
                    node.~path_node_type();
                    break;
                default:
                    break;
            }
        }
    };

    template <class Json,class JsonReference>
    class path_expression
    {
    public:
        using char_type = typename Json::char_type;
        using string_type = std::basic_string<char_type,std::char_traits<char_type>>;
        using string_view_type = typename Json::string_view_type;
        using path_node_type = path_node<Json,JsonReference>;
        using path_node_less_type = path_node_less<Json,JsonReference>;
        using path_node_equal_type = path_node_equal<Json,JsonReference>;
        using reference = typename path_node_type::reference;
        using pointer = typename path_node_type::pointer;
        using token_type = token<Json,JsonReference>;
        using reference_arg_type = typename std::conditional<std::is_const<typename std::remove_reference<JsonReference>::type>::value,
            const_reference_arg_t,reference_arg_t>::type;
        using path_component_type = path_component<char_type>;
    private:
        std::vector<token_type> token_list_;
    public:

        path_expression()
        {
        }

        path_expression(path_expression&& expr)
            : token_list_(std::move(expr.token_list_))
        {
        }

        path_expression(std::vector<token_type>&& token_stack)
            : token_list_(std::move(token_stack))
        {
        }

        path_expression& operator=(path_expression&& expr) = default;

        Json evaluate(dynamic_resources<Json,JsonReference>& resources, 
                      const std::vector<path_component_type>& path, 
                      reference root,
                      reference instance,
                      result_options options) const
        {
            Json result(json_array_arg);

            if ((options & result_options::value) == result_options::value)
            {
                auto callback = [&result](const std::vector<path_component_type>&, reference val)
                {
                    result.push_back(val);
                };
                evaluate(resources, path, root, instance, callback, options);
            }
            else if ((options & result_options::path) == result_options::path)
            {
                auto callback = [&result](const std::vector<path_component_type>& path, reference)
                {
                    result.emplace_back(jsoncons::jsonpath::to_string(path));
                };
                evaluate(resources, path, root, instance, callback, options);
            }

            return result;
        }

        template <class Callback>
        typename std::enable_if<type_traits::is_binary_function_object<Callback,const std::vector<path_component_type>&,reference>::value,void>::type
        evaluate(dynamic_resources<Json,JsonReference>& resources, 
                 const std::vector<path_component_type>& ipath, 
                 reference root,
                 reference current, 
                 Callback callback,
                 result_options options) const
        {
            std::error_code ec;

            std::vector<node_set<Json,JsonReference>> stack;
            std::vector<pointer> arg_stack;
            std::vector<path_component_type> path(ipath);
            Json result(json_array_arg);

            //std::cout << "EVALUATE TOKENS\n";
            //for (auto& tok : token_list_)
            //{
            //    std::cout << tok.to_string() << "\n";
            //}
            //std::cout << "\n";

            if (!token_list_.empty())
            {
                for (auto& tok : token_list_)
                {
                    //std::cout << "Token: " << tok.to_string() << "\n";
                    switch (tok.type())
                    { 
                        case token_kind::literal:
                        {
                            stack.emplace_back(path_node_type(&tok.get_value(reference_arg_type(), resources)));
                            break;
                        }
                        case token_kind::unary_operator:
                        {
                            JSONCONS_ASSERT(stack.size() >= 1);
                            pointer ptr = stack.back().to_pointer(resources);
                            stack.pop_back();

                            reference r = tok.unary_operator_->evaluate(resources, *ptr, ec);
                            stack.emplace_back(path_node_type(std::addressof(r)));
                            break;
                        }
                        case token_kind::binary_operator:
                        {
                            //std::cout << "binary operator: " << stack.size() << "\n";
                            JSONCONS_ASSERT(stack.size() >= 2);
                            pointer rhs = stack.back().to_pointer(resources);
                            //std::cout << "rhs: " << *rhs << "\n";
                            stack.pop_back();
                            pointer lhs = stack.back().to_pointer(resources);
                            //std::cout << "lhs: " << *lhs << "\n";
                            stack.pop_back();

                            reference r = tok.binary_operator_->evaluate(resources, *lhs, *rhs, ec);
                            //std::cout << "Evaluate binary expression: " << r << "\n";
                            stack.emplace_back(path_node_type(std::addressof(r)));
                            break;
                        }
                        case token_kind::root_node:
                            //std::cout << "root: " << root << "\n";
                            stack.emplace_back(path_node_type(std::addressof(root)));
                            break;
                        case token_kind::current_node:
                            //std::cout << "current: " << current << "\n";
                            stack.emplace_back(path_node_type(std::addressof(current)));
                            break;
                        case token_kind::argument:
                            JSONCONS_ASSERT(!stack.empty());
                            //std::cout << "argument stack items " << stack.size() << "\n";
                            //for (auto& item : stack)
                            //{
                            //    std::cout << *item.to_pointer(resources) << "\n";
                            //}
                            //std::cout << "\n";
                            arg_stack.push_back(stack.back().to_pointer(resources));
                            //for (auto& item : arg_stack)
                            //{
                            //    std::cout << *item << "\n";
                            //}
                            //std::cout << "\n";
                            stack.pop_back();
                            break;
                        case token_kind::function:
                        {
                            if (tok.function_->arity() && *(tok.function_->arity()) != arg_stack.size())
                            {
                                //ec = jmespath_errc::invalid_arity;
                                return;
                            }
                            //std::cout << "function arg stack:\n";
                            //for (auto& item : arg_stack)
                            //{
                            //    std::cout << *item << "\n";
                            //}
                            //std::cout << "\n";

                            reference r = tok.function_->evaluate(resources, arg_stack, ec);
                            if (ec)
                            {
                                return;
                            }
                            //std::cout << "function result: " << r << "\n";
                            arg_stack.clear();
                            stack.emplace_back(path_node_type(std::addressof(r)));
                            break;
                        }
                        case token_kind::selector:
                        {
                            if (stack.empty())
                            {
                                stack.emplace_back(path_node_type(std::addressof(current)));
                            }
                            pointer ptr = nullptr;
                            //for (auto& item : stack)
                            //{
                                //std::cout << "selector stack input:\n";
                                //switch (item.tag)
                                //{
                                //    case node_set_tag::single:
                                //        std::cout << "single: " << *(item.node.ptr) << "\n";
                                //        break;
                                //    case node_set_tag::multi:
                                //        for (auto& node : stack.back().nodes)
                                //        {
                                //            std::cout << "multi: " << *node.ptr << "\n";
                                //        }
                                //        break;
                                //    default:
                                //        break;
                            //}
                            //std::cout << "\n";
                            //}
                            switch (stack.back().tag)
                            {
                                case node_set_tag::single:
                                    ptr = stack.back().node.ptr;
                                    break;
                                case node_set_tag::multi:
                                    if (!stack.back().nodes.empty())
                                    {
                                        ptr = stack.back().nodes.back().ptr;
                                    }
                                    ptr = stack.back().to_pointer(resources);
                                    break;
                                default:
                                    break;
                            }
                            if (ptr)
                            {
                                //std::cout << "selector item: " << *ptr << "\n";
                                stack.pop_back();
                                std::vector<path_node_type> temp;
                                node_type ndtype = node_type();
                                tok.selector_->select(resources, path, root, *ptr, temp, ndtype, options);

                                if ((options & result_options::sort) == result_options::sort)
                                {
                                    std::sort(temp.begin(), temp.end(), path_node_less_type());
                                }

                                if ((options & result_options::nodups) == result_options::nodups)
                                {
                                    if ((options & result_options::sort) == result_options::sort)
                                    {
                                        auto last = std::unique(temp.begin(),temp.end(),path_node_equal_type());
                                        temp.erase(last,temp.end());
                                        stack.emplace_back(std::move(temp), ndtype);
                                    }
                                    else
                                    {
                                        std::vector<path_node_type> index(temp);
                                        std::sort(index.begin(), index.end(), path_node_less_type());
                                        auto last = std::unique(index.begin(),index.end(),path_node_equal_type());
                                        index.erase(last,index.end());

                                        std::vector<path_node_type> temp2;
                                        temp2.reserve(index.size());
                                        for (auto&& node : temp)
                                        {
                                            //std::cout << "node: " << node.path << ", " << *node.ptr << "\n";
                                            auto it = std::lower_bound(index.begin(),index.end(),node, path_node_less_type());

                                            if (it != index.end() && it->path == node.path) 
                                            {
                                                temp2.emplace_back(std::move(node));
                                                index.erase(it);
                                            }
                                        }
                                        stack.emplace_back(std::move(temp2), ndtype);
                                    }
                                }
                                else
                                {
                                    //std::cout << "selector output " << temp.size() << "\n";
                                    //for (auto& item : temp)
                                    //{
                                    //    std::cout << *item.ptr << "\n";
                                    //}
                                    //std::cout << "\n";
                                    stack.emplace_back(std::move(temp), ndtype);
                                }

                            }
                            break;
                        }
                        default:
                            break;
                    }
                }
            }
            for (auto& item : stack)
            {
                switch (item.tag)
                {
                    case node_set_tag::single:
                        //std::cout << "result: " << *item.node.ptr << "\n";
                        callback(item.node.path, *item.node.ptr);
                        break;
                    case node_set_tag::multi:
                        for (auto& node : item.nodes)
                        {
                            callback(node.path, *node.ptr);
                        }
                        break;
                    default:
                        break;
                }
            }
            //std::cout << "EVALUATE END\n";
        }

        std::string to_string(int level) const
        {
            std::string s;
            if (level > 0)
            {
                s.append("\n");
                s.append(level*2, ' ');
            }
            s.append("expression ");
            for (const auto& item : token_list_)
            {
                s.append(item.to_string(level+1));
            }

            return s;

        }
    };

} // namespace detail
} // namespace jsonpath
} // namespace jsoncons

#endif // JSONCONS_JSONPATH_JSONPATH_EXPRESSION_HPP
