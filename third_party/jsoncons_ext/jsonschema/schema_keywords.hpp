// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_SCHEMA_KEYWORDS_HPP
#define JSONCONS_JSONSCHEMA_SCHEMA_KEYWORDS_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/subschema.hpp>
#include <jsoncons_ext/jsonschema/format_checkers.hpp>
#include <cassert>
#include <set>
#include <sstream>
#include <iostream>
#include <cassert>
#if defined(JSONCONS_HAS_STD_REGEX)
#include <regex>
#endif

namespace jsoncons {
namespace jsonschema {

    template <class Json>
    class schema_builder
    {
    public:
        using schema_pointer = typename schema_keyword<Json>::schema_pointer;

        virtual ~schema_builder() = default;

        virtual schema_pointer build(const Json& schema,
                                     const std::vector<uri_wrapper>& uris,
                                     const std::vector<std::string>& keys) = 0;
        virtual schema_pointer make_required_keyword(const std::vector<uri_wrapper>& uris,
                                                     const std::vector<std::string>& items) = 0;

        virtual schema_pointer make_null_keyword(const std::vector<uri_wrapper>& uris) = 0;

        virtual schema_pointer make_true_keyword(const std::vector<uri_wrapper>& uris) = 0;

        virtual schema_pointer make_false_keyword(const std::vector<uri_wrapper>& uris) = 0;

        virtual schema_pointer make_object_keyword(const Json& sch, 
                                                  const std::vector<uri_wrapper>& uris) = 0;

        virtual schema_pointer make_array_keyword(const Json& sch,
                                               const std::vector<uri_wrapper>& uris) = 0;

        virtual schema_pointer make_string_keyword(const Json& sch,
                                                const std::vector<uri_wrapper>& uris) = 0;

        virtual schema_pointer make_boolean_keyword(const std::vector<uri_wrapper>& uris) = 0;

        virtual schema_pointer make_integer_keyword(const Json& sch, 
                                                 const std::vector<uri_wrapper>& uris, 
                                                 std::set<std::string>& keywords) = 0;

        virtual schema_pointer make_number_keyword(const Json& sch, 
                                                const std::vector<uri_wrapper>& uris, 
                                                std::set<std::string>& keywords) = 0;

        virtual schema_pointer make_not_keyword(const Json& schema,
                                               const std::vector<uri_wrapper>& uris) = 0;

        virtual schema_pointer make_all_of_keyword(const Json& schema,
                                                  const std::vector<uri_wrapper>& uris) = 0;

        virtual schema_pointer make_any_of_keyword(const Json& schema,
                                          const std::vector<uri_wrapper>& uris) = 0;

        virtual schema_pointer make_one_of_keyword(const Json& schema,
                                                  const std::vector<uri_wrapper>& uris) = 0;

        virtual schema_pointer make_type_keyword(const Json& schema,
                                                const std::vector<uri_wrapper>& uris) = 0;
    };

    struct collecting_error_reporter : public error_reporter
    {
        std::vector<validation_output> errors;

    private:
        void do_error(const validation_output& o) override
        {
            errors.push_back(o);
        }
    };

    template <class Json>
    void update_patch(Json& patch, const uri_wrapper& instance_location, Json&& default_value)
    {
        Json j;
        j.try_emplace("op", "add"); 
        j.try_emplace("path", instance_location.pointer()); 
        j.try_emplace("value", std::forward<Json>(default_value)); 

        patch.push_back(std::move(j));
    }

    // string schema_keyword

    template <class Json>
    void content_media_type_check(const uri_wrapper& instance_location, const Json&, 
                                  const std::string& content_media_type, const std::string& content,
                                  error_reporter& reporter)
    {
        if (content_media_type == "application/Json")
        {
            json_reader reader(content);
            std::error_code ec;
            reader.read(ec);

            if (ec)
            {
                reporter.error(validation_output(instance_location.string(), std::string("Content is not JSON: ") + ec.message(), "contentMediaType", "foo"));
            }
        }
    }

    inline
    std::string make_absolute_keyword_location(const std::vector<uri_wrapper>& uris,
                                               const std::string& keyword)
    {
        for (auto it = uris.rbegin(); it != uris.rend(); ++it)
        {
            if (!it->has_identifier() && it->is_absolute())
            {
                return it->append(keyword).string();
            }
        }
        return "";
    }

    template <class Json>
    class string_keyword : public schema_keyword<Json>
    {
        using schema_pointer = typename schema_keyword<Json>::schema_pointer;

        jsoncons::optional<std::size_t> max_length_;
        std::string absolute_max_length_location_;
        jsoncons::optional<std::size_t> min_length_;
        std::string absolute_min_length_location_;

    #if defined(JSONCONS_HAS_STD_REGEX)
        jsoncons::optional<std::regex> pattern_;
        std::string pattern_string_;
        std::string absolute_pattern_location_;
    #endif

        format_checker format_check_;
        std::string absolute_format_location_;

        jsoncons::optional<std::string> content_encoding_;
        std::string absolute_content_encoding_location_;
        jsoncons::optional<std::string> content_media_type_;
        std::string absolute_content_media_type_location_;

    public:
        string_keyword(const Json& sch, const std::vector<uri_wrapper>& uris)
            : schema_keyword<Json>((!uris.empty() && uris.back().is_absolute()) ? uris.back().string() : ""), max_length_(), min_length_(), 
    #if defined(JSONCONS_HAS_STD_REGEX)
              pattern_(),
    #endif
              content_encoding_(), content_media_type_()
        {
            auto it = sch.find("maxLength");
            if (it != sch.object_range().end()) 
            {
                max_length_ = it->value().template as<std::size_t>();
                absolute_max_length_location_ = make_absolute_keyword_location(uris, "maxLength");
            }

            it = sch.find("minLength");
            if (it != sch.object_range().end()) 
            {
                min_length_ = it->value().template as<std::size_t>();
                absolute_min_length_location_ = make_absolute_keyword_location(uris, "minLength");
            }

            it = sch.find("contentEncoding");
            if (it != sch.object_range().end()) 
            {
                content_encoding_ = it->value().template as<std::string>();
                absolute_content_encoding_location_ = make_absolute_keyword_location(uris, "contentEncoding");
                // If "contentEncoding" is set to "binary", a Json value
                // of type json_type::byte_string_value is accepted.
            }

            it = sch.find("contentMediaType");
            if (it != sch.object_range().end()) 
            {
                content_media_type_ = it->value().template as<std::string>();
                absolute_content_media_type_location_ = make_absolute_keyword_location(uris, "contentMediaType");
            }

    #if defined(JSONCONS_HAS_STD_REGEX)
            it = sch.find("pattern");
            if (it != sch.object_range().end()) 
            {
                pattern_string_ = it->value().template as<std::string>();
                pattern_ = std::regex(it->value().template as<std::string>(),std::regex::ECMAScript);
                absolute_pattern_location_ = make_absolute_keyword_location(uris, "pattern");
            }
    #endif

            it = sch.find("format");
            if (it != sch.object_range().end()) 
            {
                absolute_format_location_ = make_absolute_keyword_location(uris, "format");
                std::string format = it->value().template as<std::string>();
                if (format == "date-time")
                {
                    format_check_ = rfc3339_date_time_check;
                }
                else if (format == "date") 
                {
                    format_check_ = rfc3339_date_check;
                } 
                else if (format == "time") 
                {
                    format_check_ = rfc3339_time_check;
                } 
                else if (format == "email") 
                {
                    format_check_ = email_check;
                } 
                else if (format == "hostname") 
                {
                    format_check_ = hostname_check;
                } 
                else if (format == "ipv4") 
                {
                    format_check_ = ipv4_check;
                } 
                else if (format == "ipv6") 
                {
                    format_check_ = ipv6_check;
                } 
                else if (format == "regex") 
                {
                    format_check_ = regex_check;
                } 
                else
                {
                    // Not supported - ignore
                }
            }
        }

    private:

        void do_validate(const uri_wrapper& instance_location, 
                         const Json& instance, 
                         error_reporter& reporter,
                         Json&) const override
        {
            std::string content;
            if (content_encoding_)
            {
                if (*content_encoding_ == "base64")
                {
                    auto s = instance.template as<jsoncons::string_view>();
                    auto retval = jsoncons::decode_base64(s.begin(), s.end(), content);
                    if (retval.ec != jsoncons::conv_errc::success)
                    {
                        reporter.error(validation_output(instance_location.string(), "Content is not a base64 string", "contentEncoding", absolute_content_encoding_location_));
                        if (reporter.fail_early())
                        {
                            return;
                        }
                    }
                }
                else if (!content_encoding_->empty())
                {
                    reporter.error(validation_output(instance_location.string(), "unable to check for contentEncoding '" + *content_encoding_ + "'", "contentEncoding", absolute_content_encoding_location_));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }
            else
            {
                content = instance.template as<std::string>();
            }

            if (content_media_type_) 
            {
                content_media_type_check(instance_location, instance, *content_media_type_, content, reporter);
            } 
            else if (instance.type() == json_type::byte_string_value) 
            {
                reporter.error(validation_output(instance_location.string(), "Expected string, but is byte string", "contentMediaType", absolute_content_media_type_location_));
                if (reporter.fail_early())
                {
                    return;
                }
            }

            if (instance.type() != json_type::string_value) 
            {
                return; 
            }

            if (min_length_) 
            {
                std::size_t length = unicode_traits::count_codepoints(content.data(), content.size());
                if (length < *min_length_) 
                {
                    reporter.error(validation_output(instance_location.string(), std::string("Expected minLength: ") + std::to_string(*min_length_)
                                              + ", actual: " + std::to_string(length), "minLength", absolute_min_length_location_));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }

            if (max_length_) 
            {
                std::size_t length = unicode_traits::count_codepoints(content.data(), content.size());
                if (length > *max_length_)
                {
                    reporter.error(validation_output(instance_location.string(), std::string("Expected maxLength: ") + std::to_string(*max_length_)
                        + ", actual: " + std::to_string(length), "maxLength", absolute_max_length_location_));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }

    #if defined(JSONCONS_HAS_STD_REGEX)
            if (pattern_)
            {
                if (!std::regex_search(content, *pattern_))
                {
                    std::string message("String \"");
                    message.append(instance.template as<std::string>());
                    message.append("\" does not match pattern \"");
                    message.append(pattern_string_);
                    message.append("\"");
                    reporter.error(validation_output(instance_location.string(), std::move(message), "pattern", absolute_pattern_location_));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }

    #endif

            if (format_check_ != nullptr) 
            {
                format_check_(absolute_format_location_, instance_location, content, reporter);
                if (reporter.error_count() > 0 && reporter.fail_early())
                {
                    return;
                }
            }
        }
    };

    // not_keyword

    template <class Json>
    class not_keyword : public schema_keyword<Json>
    {
        using schema_pointer = typename schema_keyword<Json>::schema_pointer;

        schema_pointer rule_;

    public:
        not_keyword(schema_builder<Json>* builder,
                 const Json& sch,
                 const std::vector<uri_wrapper>& uris)
            : schema_keyword<Json>((!uris.empty() && uris.back().is_absolute()) ? uris.back().string() : "")
        {
            rule_ = builder->build(sch, uris, {"not"});
        }

    private:

        void do_validate(const uri_wrapper& instance_location, 
                         const Json& instance, 
                         error_reporter& reporter, 
                         Json& patch) const final
        {
            collecting_error_reporter local_reporter;
            rule_->validate(instance_location, instance, local_reporter, patch);

            if (local_reporter.errors.empty())
            {
                reporter.error(validation_output(instance_location.string(), "Instance must not be valid against schema", "not", this->absolute_keyword_location()));
            }
        }

        jsoncons::optional<Json> get_default_value(const uri_wrapper& instance_location, 
                                                   const Json& instance, 
                                                   error_reporter& reporter) const override
        {
            return rule_->get_default_value(instance_location, instance, reporter);
        }
    };

    template <class Json>
    struct all_of_criterion
    {
        static const std::string& key()
        {
            static const std::string k("allOf");
            return k;
        }

        static bool is_complete(const Json&, 
                                const uri_wrapper& instance_location, 
                                error_reporter& reporter, 
                                const collecting_error_reporter& local_reporter, 
                                std::size_t)
        {
            if (!local_reporter.errors.empty())
                reporter.error(validation_output(instance_location.string(), "At least one schema_keyword failed to match, but all are required to match. ", "allOf", "foo", local_reporter.errors));
            return !local_reporter.errors.empty();
        }
    };

    template <class Json>
    struct any_of_criterion
    {
        static const std::string& key()
        {
            static const std::string k("anyOf");
            return k;
        }

        static bool is_complete(const Json&, 
                                const uri_wrapper&, 
                                error_reporter&, 
                                const collecting_error_reporter&, 
                                std::size_t count)
        {
            return count == 1;
        }
    };

    template <class Json>
    struct one_of_criterion
    {
        static const std::string& key()
        {
            static const std::string k("oneOf");
            return k;
        }

        static bool is_complete(const Json&, 
                                const uri_wrapper& instance_location, 
                                error_reporter& reporter, 
                                const collecting_error_reporter&, 
                                std::size_t count)
        {
            if (count > 1)
            {
                std::string message(std::to_string(count));
                message.append(" subschemas matched, but exactly one is required to match");
                reporter.error(validation_output(instance_location.string(), std::move(message), "oneOf", "foo"));
            }
            return count > 1;
        }
    };

    template <class Json,class Criterion>
    class combining_keyword : public schema_keyword<Json>
    {
        using schema_pointer = typename schema_keyword<Json>::schema_pointer;

        std::vector<schema_pointer> subschemas_;

    public:
        combining_keyword(schema_builder<Json>* builder,
                       const Json& sch,
                       const std::vector<uri_wrapper>& uris)
            : schema_keyword<Json>((!uris.empty() && uris.back().is_absolute()) ? uris.back().string() : "")
        {
            size_t c = 0;
            for (const auto& subsch : sch.array_range())
            {
                subschemas_.push_back(builder->build(subsch, uris, {Criterion::key(), std::to_string(c++)}));
            }

            // Validate value of allOf, anyOf, and oneOf "MUST be a non-empty array"
        }

    private:

        void do_validate(const uri_wrapper& instance_location, 
                         const Json& instance, 
                         error_reporter& reporter, 
                         Json& patch) const final
        {
            size_t count = 0;

            collecting_error_reporter local_reporter;
            for (auto& s : subschemas_) 
            {
                std::size_t mark = local_reporter.errors.size();
                s->validate(instance_location, instance, local_reporter, patch);
                if (mark == local_reporter.errors.size())
                    count++;

                if (Criterion::is_complete(instance, instance_location, reporter, local_reporter, count))
                    return;
            }

            if (count == 0)
            {
                reporter.error(validation_output(instance_location.string(), "No schema_keyword matched, but one of them is required to match", "combined", this->absolute_keyword_location(), local_reporter.errors));
            }
        }
    };

    template <class Json,class T>
    class number_keyword : public schema_keyword<Json>
    {
        using schema_pointer = typename schema_keyword<Json>::schema_pointer;

        jsoncons::optional<T> maximum_;
        std::string absolute_maximum_location_;
        jsoncons::optional<T> minimum_;
        std::string absolute_minimum_location_;
        jsoncons::optional<T> exclusive_maximum_;
        std::string absolute_exclusive_maximum_location_;
        jsoncons::optional<T> exclusive_minimum_;
        std::string absolute_exclusive_minimum_location_;
        jsoncons::optional<double> multiple_of_;
        std::string absolute_multiple_of_location_;

    public:
        number_keyword(const Json& sch, 
                    const std::vector<uri_wrapper>& uris, 
                    std::set<std::string>& keywords)
            : schema_keyword<Json>((!uris.empty() && uris.back().is_absolute()) ? uris.back().string() : ""), 
              maximum_(), minimum_(),exclusive_maximum_(), exclusive_minimum_(), multiple_of_()
        {
            auto it = sch.find("maximum");
            if (it != sch.object_range().end()) 
            {
                maximum_ = it->value().template as<T>();
                absolute_maximum_location_ = make_absolute_keyword_location(uris,"maximum");
                keywords.insert("maximum");
            }

            it = sch.find("minimum");
            if (it != sch.object_range().end()) 
            {
                minimum_ = it->value().template as<T>();
                absolute_minimum_location_ = make_absolute_keyword_location(uris,"minimum");
                keywords.insert("minimum");
            }

            it = sch.find("exclusiveMaximum");
            if (it != sch.object_range().end()) 
            {
                exclusive_maximum_ = it->value().template as<T>();
                absolute_exclusive_maximum_location_ = make_absolute_keyword_location(uris,"exclusiveMaximum");
                keywords.insert("exclusiveMaximum");
            }

            it = sch.find("exclusiveMinimum");
            if (it != sch.object_range().end()) 
            {
                exclusive_minimum_ = it->value().template as<T>();
                absolute_exclusive_minimum_location_ = make_absolute_keyword_location(uris,"exclusiveMinimum");
                keywords.insert("exclusiveMinimum");
            }

            it = sch.find("multipleOf");
            if (it != sch.object_range().end()) 
            {
                multiple_of_ = it->value().template as<double>();
                absolute_multiple_of_location_ = make_absolute_keyword_location(uris,"multipleOf");
                keywords.insert("multipleOf");
            }
        }

    private:

        void do_validate(const uri_wrapper& instance_location, 
                         const Json& instance, 
                         error_reporter& reporter, 
                         Json&) const override
        {
            T value = instance.template as<T>(); 
            if (Json(value) != instance)
            {
                reporter.error(validation_output(instance_location.string(), "Instance is not a number", "number", this->absolute_keyword_location()));
                if (reporter.fail_early())
                {
                    return;
                }
            }

            if (multiple_of_ && value != 0) // exclude zero
            {
                if (!is_multiple_of(value, *multiple_of_))
                {
                    reporter.error(validation_output(instance_location.string(), instance.template as<std::string>() + " is not a multiple of " + std::to_string(*multiple_of_), "multipleOf", absolute_multiple_of_location_));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }

            if (maximum_)
            {
                if (value > *maximum_)
                {
                    reporter.error(validation_output(instance_location.string(), instance.template as<std::string>() + " exceeds maximum of " + std::to_string(*maximum_), "maximum", absolute_maximum_location_));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }

            if (minimum_)
            {
                if (value < *minimum_)
                {
                    reporter.error(validation_output(instance_location.string(), instance.template as<std::string>() + " is below minimum of " + std::to_string(*minimum_), "minimum", absolute_minimum_location_));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }

            if (exclusive_maximum_)
            {
                if (value >= *exclusive_maximum_)
                {
                    reporter.error(validation_output(instance_location.string(), instance.template as<std::string>() + " exceeds maximum of " + std::to_string(*exclusive_maximum_), "exclusiveMaximum", absolute_exclusive_maximum_location_));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }

            if (exclusive_minimum_)
            {
                if (value <= *exclusive_minimum_)
                {
                    reporter.error(validation_output(instance_location.string(), instance.template as<std::string>() + " is below minimum of " + std::to_string(*exclusive_minimum_), "exclusiveMinimum", absolute_exclusive_minimum_location_));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }
        }

        static bool is_multiple_of(T x, double multiple_of) 
        {
            double rem = std::remainder(x, multiple_of);
            double eps = std::nextafter(x, 0) - x;
            return std::fabs(rem) < std::fabs(eps);
        }
    };

    // null_keyword

    template <class Json>
    class null_keyword : public schema_keyword<Json>
    {
    public:
        using schema_pointer = typename schema_keyword<Json>::schema_pointer;

        null_keyword(const std::vector<uri_wrapper>& uris)
            : schema_keyword<Json>((!uris.empty() && uris.back().is_absolute()) ? uris.back().string() : "")
        {
        }
    private:
        void do_validate(const uri_wrapper& instance_location, const Json& instance, error_reporter& reporter, Json&) const override
        {
            if (!instance.is_null())
            {
                reporter.error(validation_output(instance_location.string(), "Expected to be null", "null", this->absolute_keyword_location()));
            }
        }
    };

    template <class Json>
    class boolean_keyword : public schema_keyword<Json>
    {
    public:
        using schema_pointer = typename schema_keyword<Json>::schema_pointer;

        boolean_keyword(const std::vector<uri_wrapper>& uris)
            : schema_keyword<Json>((!uris.empty() && uris.back().is_absolute()) ? uris.back().string() : "")
        {
        }
    private:
        void do_validate(const uri_wrapper&, const Json&, error_reporter&, Json&) const override
        {
        }

    };

    template <class Json>
    class true_keyword : public schema_keyword<Json>
    {
    public:
        using schema_pointer = typename schema_keyword<Json>::schema_pointer;

        true_keyword(const std::vector<uri_wrapper>& uris)
            : schema_keyword<Json>((!uris.empty() && uris.back().is_absolute()) ? uris.back().string() : "")
        {
        }
    private:
        void do_validate(const uri_wrapper&, const Json&, error_reporter&, Json&) const override
        {
        }
    };

    template <class Json>
    class false_keyword : public schema_keyword<Json>
    {
    public:
        using schema_pointer = typename schema_keyword<Json>::schema_pointer;

        false_keyword(const std::vector<uri_wrapper>& uris)
            : schema_keyword<Json>((!uris.empty() && uris.back().is_absolute()) ? uris.back().string() : "")
        {
        }
    private:
        void do_validate(const uri_wrapper& instance_location, const Json&, error_reporter& reporter, Json&) const override
        {
            reporter.error(validation_output(instance_location.string(), "False schema always fails", "false", this->absolute_keyword_location()));
        }
    };

    template <class Json>
    class required_keyword : public schema_keyword<Json>
    {
        using schema_pointer = typename schema_keyword<Json>::schema_pointer;

        std::vector<std::string> items_;

    public:
        required_keyword(const std::vector<uri_wrapper>& uris,
                         const std::vector<std::string>& items)
            : schema_keyword<Json>((!uris.empty() && uris.back().is_absolute()) ? uris.back().string() : ""), items_(items) {}
        required_keyword(const std::string& absolute_keyword_location, const std::vector<std::string>& items)
            : schema_keyword<Json>(absolute_keyword_location), items_(items) {}

        required_keyword(const required_keyword&) = delete;
        required_keyword(required_keyword&&) = default;
        required_keyword& operator=(const required_keyword&) = delete;
        required_keyword& operator=(required_keyword&&) = default;
    private:

        void do_validate(const uri_wrapper& instance_location, const Json& instance, error_reporter& reporter, Json&) const override final
        {
            for (const auto& key : items_)
            {
                if (instance.find(key) == instance.object_range().end())
                {
                    reporter.error(validation_output(instance_location.string(), "Required property \"" + key + "\" not found", "required", this->absolute_keyword_location()));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }
        }
    };

    template <class Json>
    class object_keyword : public schema_keyword<Json>
    {
        using schema_pointer = typename schema_keyword<Json>::schema_pointer;

        jsoncons::optional<std::size_t> max_properties_;
        std::string absolute_max_properties_location_;
        jsoncons::optional<std::size_t> min_properties_;
        std::string absolute_min_properties_location_;
        jsoncons::optional<required_keyword<Json>> required_;

        std::map<std::string, schema_pointer> properties_;
    #if defined(JSONCONS_HAS_STD_REGEX)
        std::vector<std::pair<std::regex, schema_pointer>> pattern_properties_;
    #endif
        schema_pointer additional_properties_;

        std::map<std::string, schema_pointer> dependencies_;

        schema_pointer property_names_;

    public:
        object_keyword(schema_builder<Json>* builder,
                    const Json& sch,
                    const std::vector<uri_wrapper>& uris)
            : schema_keyword<Json>((!uris.empty() && uris.back().is_absolute()) ? uris.back().string() : ""), 
              max_properties_(), min_properties_(), 
              additional_properties_(nullptr),
              property_names_(nullptr)
        {
            auto it = sch.find("maxProperties");
            if (it != sch.object_range().end()) 
            {
                max_properties_ = it->value().template as<std::size_t>();
                absolute_max_properties_location_ = make_absolute_keyword_location(uris, "maxProperties");
            }

            it = sch.find("minProperties");
            if (it != sch.object_range().end()) 
            {
                min_properties_ = it->value().template as<std::size_t>();
                absolute_min_properties_location_ = make_absolute_keyword_location(uris, "minProperties");
            }

            it = sch.find("required");
            if (it != sch.object_range().end()) 
            {
                auto location = make_absolute_keyword_location(uris, "required");
                required_ = required_keyword<Json>(location, 
                                                   it->value().template as<std::vector<std::string>>());
            }

            it = sch.find("properties");
            if (it != sch.object_range().end()) 
            {
                for (const auto& prop : it->value().object_range())
                    properties_.emplace(
                        std::make_pair(
                            prop.key(),
                            builder->build(prop.value(), uris, {"properties", prop.key()})));
            }

    #if defined(JSONCONS_HAS_STD_REGEX)
            it = sch.find("patternProperties");
            if (it != sch.object_range().end()) 
            {
                for (const auto& prop : it->value().object_range())
                    pattern_properties_.emplace_back(
                        std::make_pair(
                            std::regex(prop.key(), std::regex::ECMAScript),
                            builder->build(prop.value(), uris, {prop.key()})));
            }
    #endif

            it = sch.find("additionalProperties");
            if (it != sch.object_range().end()) 
            {
                additional_properties_ = builder->build(it->value(), uris, {"additionalProperties"});
            }

            it = sch.find("dependencies");
            if (it != sch.object_range().end()) 
            {
                for (const auto& dep : it->value().object_range())
                {
                    switch (dep.value().type()) 
                    {
                        case json_type::array_value:
                        {
                            auto location = make_absolute_keyword_location(uris, "required");
                            dependencies_.emplace(dep.key(),
                                                  builder->make_required_keyword({location},
                                                                                 dep.value().template as<std::vector<std::string>>()));
                            break;
                        }
                        default:
                        {
                            dependencies_.emplace(dep.key(),
                                                  builder->build(dep.value(), uris, {"dependencies", dep.key()}));
                            break;
                        }
                    }
                }
            }

            auto property_names_it = sch.find("propertyNames");
            if (property_names_it != sch.object_range().end()) 
            {
                property_names_ = builder->build(property_names_it->value(), uris, {"propertyNames"});
            }
        }
    private:

        void do_validate(const uri_wrapper& instance_location, 
                         const Json& instance, 
                         error_reporter& reporter, 
                         Json& patch) const override
        {
            if (max_properties_ && instance.size() > *max_properties_)
            {
                std::string message("Maximum properties: " + std::to_string(*max_properties_));
                message.append(", found: " + std::to_string(instance.size()));
                reporter.error(validation_output(instance_location.string(), std::move(message), "maxProperties", absolute_max_properties_location_));
                if (reporter.fail_early())
                {
                    return;
                }
            }

            if (min_properties_ && instance.size() < *min_properties_)
            {
                std::string message("Minimum properties: " + std::to_string(*min_properties_));
                message.append(", found: " + std::to_string(instance.size()));
                reporter.error(validation_output(instance_location.string(), std::move(message), "minProperties", absolute_min_properties_location_));
                if (reporter.fail_early())
                {
                    return;
                }
            }

            if (required_)
                required_->validate(instance_location, instance, reporter, patch);

            for (const auto& property : instance.object_range()) 
            {
                if (property_names_)
                    property_names_->validate(instance_location, property.key(), reporter, patch);

                bool a_prop_or_pattern_matched = false;
                auto properties_it = properties_.find(property.key());

                // check if it is in "properties"
                if (properties_it != properties_.end()) 
                {
                    a_prop_or_pattern_matched = true;
                    properties_it->second->validate(instance_location.append(property.key()), property.value(), reporter, patch);
                }

    #if defined(JSONCONS_HAS_STD_REGEX)

                // check all matching "patternProperties"
                for (auto& schema_pp : pattern_properties_)
                    if (std::regex_search(property.key(), schema_pp.first)) 
                    {
                        a_prop_or_pattern_matched = true;
                        schema_pp.second->validate(instance_location.append(property.key()), property.value(), reporter, patch);
                    }
    #endif

                // finally, check "additionalProperties" 
                if (!a_prop_or_pattern_matched && additional_properties_) 
                {
                    collecting_error_reporter local_reporter;
                    additional_properties_->validate(instance_location.append(property.key()), property.value(), local_reporter, patch);
                    if (!local_reporter.errors.empty())
                    {
                        reporter.error(validation_output(instance_location.string(), "Additional property \"" + property.key() + "\" found but was invalid.", "additionalProperties", additional_properties_->absolute_keyword_location()));
                        if (reporter.fail_early())
                        {
                            return;
                        }
                    }
                }
            }

            // reverse search
            for (auto const& prop : properties_) 
            {
                const auto finding = instance.find(prop.first);
                if (finding == instance.object_range().end()) 
                { 
                    // If property is not in instance
                    auto default_value = prop.second->get_default_value(instance_location, instance, reporter);
                    if (default_value) 
                    { 
                        // If default value is available, update patch
                        update_patch(patch, instance_location.append(prop.first), std::move(*default_value));
                    }
                }
            }

            for (const auto& dep : dependencies_) 
            {
                auto prop = instance.find(dep.first);
                if (prop != instance.object_range().end()) // if dependency-property is present in instance
                    dep.second->validate(instance_location.append(dep.first), instance, reporter, patch); // validate
            }
        }
    };

    // array_keyword

    template <class Json>
    class array_keyword : public schema_keyword<Json>
    {
        using schema_pointer = typename schema_keyword<Json>::schema_pointer;

        jsoncons::optional<std::size_t> max_items_;
        std::string absolute_max_items_location_;
        jsoncons::optional<std::size_t> min_items_;
        std::string absolute_min_items_location_;
        bool unique_items_ = false;
        schema_pointer items_schema_;
        std::vector<schema_pointer> items_;
        schema_pointer additional_items_;
        schema_pointer contains_;

    public:
        array_keyword(schema_builder<Json>* builder, 
                   const Json& sch, 
                   const std::vector<uri_wrapper>& uris)
            : schema_keyword<Json>((!uris.empty() && uris.back().is_absolute()) ? uris.back().string() : ""), 
              max_items_(), min_items_(), items_schema_(nullptr), additional_items_(nullptr), contains_(nullptr)
        {
            {
                auto it = sch.find("maxItems");
                if (it != sch.object_range().end()) 
                {
                    max_items_ = it->value().template as<std::size_t>();
                    absolute_max_items_location_ = make_absolute_keyword_location(uris, "maxItems");
                }
            }

            {
                auto it = sch.find("minItems");
                if (it != sch.object_range().end()) 
                {
                    min_items_ = it->value().template as<std::size_t>();
                    absolute_min_items_location_ = make_absolute_keyword_location(uris, "minItems");
                }
            }

            {
                auto it = sch.find("uniqueItems");
                if (it != sch.object_range().end()) 
                {
                    unique_items_ = it->value().template as<bool>();
                }
            }

            {
                auto it = sch.find("items");
                if (it != sch.object_range().end()) 
                {

                    if (it->value().type() == json_type::array_value) 
                    {
                        size_t c = 0;
                        for (const auto& subsch : it->value().array_range())
                            items_.push_back(builder->build(subsch, uris, {"items", std::to_string(c++)}));

                        auto attr_add = sch.find("additionalItems");
                        if (attr_add != sch.object_range().end()) 
                        {
                            additional_items_ = builder->build(attr_add->value(), uris, {"additionalItems"});
                        }

                    } 
                    else if (it->value().type() == json_type::object_value ||
                               it->value().type() == json_type::bool_value)
                    {
                        items_schema_ = builder->build(it->value(), uris, {"items"});
                    }

                }
            }

            {
                auto it = sch.find("contains");
                if (it != sch.object_range().end()) 
                {
                    contains_ = builder->build(it->value(), uris, {"contains"});
                }
            }
        }
    private:

        void do_validate(const uri_wrapper& instance_location, 
                         const Json& instance, 
                         error_reporter& reporter, 
                         Json& patch) const override
        {
            if (max_items_)
            {
                if (instance.size() > *max_items_)
                {
                    std::string message("Expected maximum item count: " + std::to_string(*max_items_));
                    message.append(", found: " + std::to_string(instance.size()));
                    reporter.error(validation_output(instance_location.string(), std::move(message), "maxItems", absolute_max_items_location_));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }

            if (min_items_)
            {
                if (instance.size() < *min_items_)
                {
                    std::string message("Expected minimum item count: " + std::to_string(*min_items_));
                    message.append(", found: " + std::to_string(instance.size()));
                    reporter.error(validation_output(instance_location.string(), std::move(message), "minItems", absolute_min_items_location_));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }

            if (unique_items_) 
            {
                if (!array_has_unique_items(instance))
                {
                    reporter.error(validation_output(instance_location.string(), "Array items are not unique", "uniqueItems", this->absolute_keyword_location()));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }

            size_t index = 0;
            if (items_schema_)
            {
                for (const auto& i : instance.array_range()) 
                {
                    items_schema_->validate(instance_location.append(index), i, reporter, patch);
                    index++;
                }
            }
            else 
            {
                auto item = items_.cbegin();
                for (const auto& i : instance.array_range()) 
                {
                    schema_pointer item_validator = nullptr;
                    if (item == items_.cend())
                        item_validator = additional_items_;
                    else 
                    {
                        item_validator = *item;
                        ++item;
                    }

                    if (!item_validator)
                        break;

                    item_validator->validate(instance_location.append(index), i, reporter, patch);
                }
            }

            if (contains_) 
            {
                bool contained = false;
                collecting_error_reporter local_reporter;
                for (const auto& item : instance.array_range()) 
                {
                    std::size_t mark = local_reporter.errors.size();
                    contains_->validate(instance_location, item, local_reporter, patch);
                    if (mark == local_reporter.errors.size()) 
                    {
                        contained = true;
                        break;
                    }
                }
                if (!contained)
                {
                    reporter.error(validation_output(instance_location.string(), "Expected at least one array item to match \"contains\" schema", "contains", this->absolute_keyword_location(), local_reporter.errors));
                    if (reporter.fail_early())
                    {
                        return;
                    }
                }
            }
        }

        static bool array_has_unique_items(const Json& a) 
        {
            for (auto it = a.array_range().begin(); it != a.array_range().end(); ++it) 
            {
                for (auto jt = it+1; jt != a.array_range().end(); ++jt) 
                {
                    if (*it == *jt) 
                    {
                        return false; // contains duplicates 
                    }
                }
            }
            return true; // elements are unique
        }
    };

    template <class Json>
    class conditional_keyword : public schema_keyword<Json>
    {
        using schema_pointer = typename schema_keyword<Json>::schema_pointer;

        schema_pointer if_;
        schema_pointer then_;
        schema_pointer else_;

    public:
        conditional_keyword(schema_builder<Json>* builder,
                         const Json& sch_if,
                         const Json& sch,
                         const std::vector<uri_wrapper>& uris)
            : schema_keyword<Json>((!uris.empty() && uris.back().is_absolute()) ? uris.back().string() : ""), if_(nullptr), then_(nullptr), else_(nullptr)
        {
            auto then_it = sch.find("then");
            auto else_it = sch.find("else");

            if (then_it != sch.object_range().end() || else_it != sch.object_range().end()) 
            {
                if_ = builder->build(sch_if, uris, {"if"});

                if (then_it != sch.object_range().end()) 
                {
                    then_ = builder->build(then_it->value(), uris, {"then"});
                }

                if (else_it != sch.object_range().end()) 
                {
                    else_ = builder->build(else_it->value(), uris, {"else"});
                }
            }
        }
    private:
        void do_validate(const uri_wrapper& instance_location, 
                         const Json& instance, 
                         error_reporter& reporter, 
                         Json& patch) const final
        {
            if (if_) 
            {
                collecting_error_reporter local_reporter;

                if_->validate(instance_location, instance, local_reporter, patch);
                if (local_reporter.errors.empty()) 
                {
                    if (then_)
                        then_->validate(instance_location, instance, reporter, patch);
                } 
                else 
                {
                    if (else_)
                        else_->validate(instance_location, instance, reporter, patch);
                }
            }
        }
    };

    // enum_keyword

    template <class Json>
    class enum_keyword : public schema_keyword<Json>
    {
        using schema_pointer = typename schema_keyword<Json>::schema_pointer;

        Json enum_;

    public:
        enum_keyword(const Json& sch,
                  const std::vector<uri_wrapper>& uris)
            : schema_keyword<Json>((!uris.empty() && uris.back().is_absolute()) ? uris.back().string() : ""), enum_(sch)
        {
        }
    private:
        void do_validate(const uri_wrapper& instance_location, 
                         const Json& instance, 
                         error_reporter& reporter,
                         Json&) const final
        {
            bool in_range = false;
            for (const auto& item : enum_.array_range())
            {
                if (item == instance) 
                {
                    in_range = true;
                    break;
                }
            }

            if (!in_range)
            {
                reporter.error(validation_output(instance_location.string(), instance.template as<std::string>() + " is not a valid enum value", "enum", this->absolute_keyword_location()));
                if (reporter.fail_early())
                {
                    return;
                }
            }
        }
    };

    // const_keyword

    template <class Json>
    class const_keyword : public schema_keyword<Json>
    {
        using schema_pointer = typename schema_keyword<Json>::schema_pointer;

        Json const_;

    public:
        const_keyword(const Json& sch, const std::vector<uri_wrapper>& uris)
            : schema_keyword<Json>((!uris.empty() && uris.back().is_absolute()) ? uris.back().string() : ""), const_(sch)
        {
        }
    private:
        void do_validate(const uri_wrapper& instance_location, 
                         const Json& instance, 
                         error_reporter& reporter,
                         Json&) const final
        {
            if (const_ != instance)
                reporter.error(validation_output(instance_location.string(), "Instance is not const", "const", this->absolute_keyword_location()));
        }
    };

    template <class Json>
    class type_keyword : public schema_keyword<Json>
    {
        using schema_pointer = typename schema_keyword<Json>::schema_pointer;

        Json default_value_;
        std::vector<schema_pointer> type_mapping_;
        jsoncons::optional<enum_keyword<Json>> enum_;
        jsoncons::optional<const_keyword<Json>> const_;
        std::vector<schema_pointer> combined_;
        jsoncons::optional<conditional_keyword<Json>> conditional_;
        std::vector<std::string> expected_types_;

    public:
        type_keyword(const type_keyword&) = delete;
        type_keyword& operator=(const type_keyword&) = delete;
        type_keyword(type_keyword&&) = default;
        type_keyword& operator=(type_keyword&&) = default;

        type_keyword(schema_builder<Json>* builder,
                     const Json& sch,
                     const std::vector<uri_wrapper>& uris)
            : schema_keyword<Json>((!uris.empty() && uris.back().is_absolute()) ? uris.back().string() : ""), default_value_(jsoncons::null_type()), 
              type_mapping_((uint8_t)(json_type::object_value)+1), 
              enum_(), const_()
        {
            //std::cout << uris.size() << " uris: ";
            //for (const auto& uri : uris)
            //{
            //    std::cout << uri.string() << ", ";
            //}
            //std::cout << "\n";
            std::set<std::string> known_keywords;

            auto it = sch.find("type");
            if (it == sch.object_range().end()) 
            {
                initialize_type_mapping(builder, "", sch, uris, known_keywords);
            }
            else 
            {
                switch (it->value().type()) 
                { 
                    case json_type::string_value: 
                    {
                        auto type = it->value().template as<std::string>();
                        initialize_type_mapping(builder, type, sch, uris, known_keywords);
                        expected_types_.emplace_back(std::move(type));
                        break;
                    } 

                    case json_type::array_value: // "type": ["type1", "type2"]
                    {
                        for (const auto& item : it->value().array_range())
                        {
                            auto type = item.template as<std::string>();
                            initialize_type_mapping(builder, type, sch, uris, known_keywords);
                            expected_types_.emplace_back(std::move(type));
                        }
                        break;
                    }
                    default:
                        break;
                }
            }

            const auto default_it = sch.find("default");
            if (default_it != sch.object_range().end()) 
            {
                default_value_ = default_it->value();
            }

            it = sch.find("enum");
            if (it != sch.object_range().end()) 
            {
                enum_ = enum_keyword<Json >(it->value(), uris);
            }

            it = sch.find("const");
            if (it != sch.object_range().end()) 
            {
                const_ = const_keyword<Json>(it->value(), uris);
            }

            it = sch.find("not");
            if (it != sch.object_range().end()) 
            {
                combined_.push_back(builder->make_not_keyword(it->value(), uris));
            }

            it = sch.find("allOf");
            if (it != sch.object_range().end()) 
            {
                combined_.push_back(builder->make_all_of_keyword(it->value(), uris));
            }

            it = sch.find("anyOf");
            if (it != sch.object_range().end()) 
            {
                combined_.push_back(builder->make_any_of_keyword(it->value(), uris));
            }

            it = sch.find("oneOf");
            if (it != sch.object_range().end()) 
            {
                combined_.push_back(builder->make_one_of_keyword(it->value(), uris));
            }

            it = sch.find("if");
            if (it != sch.object_range().end()) 
            {
                conditional_ = conditional_keyword<Json>(builder, it->value(), sch, uris);
            }
        }
    private:

        void do_validate(const uri_wrapper& instance_location, 
                         const Json& instance, 
                         error_reporter& reporter, 
                         Json& patch) const override final
        {
            auto type = type_mapping_[(uint8_t) instance.type()];

            if (type)
                type->validate(instance_location, instance, reporter, patch);
            else
            {
                std::ostringstream ss;
                ss << "Expected ";
                for (std::size_t i = 0; i < expected_types_.size(); ++i)
                {
                        if (i > 0)
                        { 
                            ss << ", ";
                            if (i+1 == expected_types_.size())
                            { 
                                ss << "or ";
                            }
                        }
                        ss << expected_types_[i];
                }
                ss << ", found " << instance.type();

                reporter.error(validation_output(instance_location.string(), ss.str(), "type", this->absolute_keyword_location()));
                if (reporter.fail_early())
                {
                    return;
                }
            }

            if (enum_)
            { 
                enum_->validate(instance_location, instance, reporter, patch);
                if (reporter.error_count() > 0 && reporter.fail_early())
                {
                    return;
                }
            }

            if (const_)
            { 
                const_->validate(instance_location, instance, reporter, patch);
                if (reporter.error_count() > 0 && reporter.fail_early())
                {
                    return;
                }
            }

            for (const auto& l : combined_)
            {
                l->validate(instance_location, instance, reporter, patch);
                if (reporter.error_count() > 0 && reporter.fail_early())
                {
                    return;
                }
            }


            if (conditional_)
            { 
                conditional_->validate(instance_location, instance, reporter, patch);
                if (reporter.error_count() > 0 && reporter.fail_early())
                {
                    return;
                }
            }
        }

        jsoncons::optional<Json> get_default_value(const uri_wrapper&, 
                                                   const Json&,
                                                   error_reporter&) const override
        {
            return default_value_;
        }

        void initialize_type_mapping(schema_builder<Json>* builder,
                                     const std::string& type,
                                     const Json& sch,
                                     const std::vector<uri_wrapper>& uris,
                                     std::set<std::string>& keywords)
        {
            if (type.empty() || type == "null")
            {
                type_mapping_[(uint8_t)json_type::null_value] = builder->make_null_keyword(uris);
            }
            if (type.empty() || type == "object")
            {
                type_mapping_[(uint8_t)json_type::object_value] = builder->make_object_keyword(sch, uris);
            }
            if (type.empty() || type == "array")
            {
                type_mapping_[(uint8_t)json_type::array_value] = builder->make_array_keyword(sch, uris);
            }
            if (type.empty() || type == "string")
            {
                type_mapping_[(uint8_t)json_type::string_value] = builder->make_string_keyword(sch, uris);
                // For binary types
                type_mapping_[(uint8_t) json_type::byte_string_value] = type_mapping_[(uint8_t) json_type::string_value];
            }
            if (type.empty() || type == "boolean")
            {
                type_mapping_[(uint8_t)json_type::bool_value] = builder->make_boolean_keyword(uris);
            }
            if (type.empty() || type == "integer")
            {
                type_mapping_[(uint8_t)json_type::int64_value] = builder->make_integer_keyword(sch, uris, keywords);
                type_mapping_[(uint8_t)json_type::uint64_value] = type_mapping_[(uint8_t)json_type::int64_value];
                type_mapping_[(uint8_t)json_type::double_value] = type_mapping_[(uint8_t)json_type::int64_value];
            }
            if (type.empty() || type == "number")
            {
                type_mapping_[(uint8_t)json_type::double_value] = builder->make_number_keyword(sch, uris, keywords);
                type_mapping_[(uint8_t)json_type::int64_value] = type_mapping_[(uint8_t)json_type::double_value];
                type_mapping_[(uint8_t)json_type::uint64_value] = type_mapping_[(uint8_t)json_type::double_value];
            }
        }
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_VALUE_RULES_HPP
