// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_JSON_VALIDATOR_HPP
#define JSONCONS_JSONSCHEMA_JSON_VALIDATOR_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/schema_loader.hpp>
#include <cassert>
#include <set>
#include <sstream>
#include <iostream>
#include <cassert>
#include <functional>

namespace jsoncons {
namespace jsonschema {

    class throwing_error_reporter : public error_reporter
    {
        void do_error(const validation_output& o) override
        {
            JSONCONS_THROW(validation_error(o.message()));
        }
    };

    class fail_early_reporter : public error_reporter
    {
        void do_error(const validation_output&) override
        {
        }
    public:
        fail_early_reporter()
            : error_reporter(true)
        {
        }
    };

    using error_reporter_t = std::function<void(const validation_output& o)>;

    struct error_reporter_adaptor : public error_reporter
    {
        error_reporter_t reporter_;

        error_reporter_adaptor(const error_reporter_t& reporter)
            : reporter_(reporter)
        {
        }
    private:
        void do_error(const validation_output& e) override
        {
            reporter_(e);
        }
    };

    template <class Json>
    class json_validator
    {
        std::shared_ptr<json_schema<Json>> root_;

    public:
        json_validator(std::shared_ptr<json_schema<Json>> root)
            : root_(root)
        {
        }

        json_validator(json_validator &&) = default;
        json_validator &operator=(json_validator &&) = default;

        json_validator(json_validator const &) = delete;
        json_validator &operator=(json_validator const &) = delete;

        ~json_validator() = default;

        // Validate input JSON against a JSON Schema with a default throwing error reporter
        Json validate(const Json& instance) const
        {
            throwing_error_reporter reporter;
            uri_wrapper instance_location("#");
            Json patch(json_array_arg);

            root_->validate(instance_location, instance, reporter, patch);
            return patch;
        }

        // Validate input JSON against a JSON Schema 
        bool is_valid(const Json& instance) const
        {
            fail_early_reporter reporter;
            uri_wrapper instance_location("#");
            Json patch(json_array_arg);

            root_->validate(instance_location, instance, reporter, patch);
            return reporter.error_count() == 0;
        }

        // Validate input JSON against a JSON Schema with a provided error reporter
        template <class Reporter>
        typename std::enable_if<type_traits::is_unary_function_object_exact<Reporter,void,validation_output>::value,Json>::type
        validate(const Json& instance, const Reporter& reporter) const
        {
            uri_wrapper instance_location("#");
            Json patch(json_array_arg);

            error_reporter_adaptor adaptor(reporter);
            root_->validate(instance_location, instance, adaptor, patch);
            return patch;
        }
    };

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_JSON_VALIDATOR_HPP
