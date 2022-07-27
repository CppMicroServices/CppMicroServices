// Copyright 2020 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSONSCHEMA_SCHEMA_LOADER_HPP
#define JSONCONS_JSONSCHEMA_SCHEMA_LOADER_HPP

#include <jsoncons/config/jsoncons_config.hpp>
#include <jsoncons/uri.hpp>
#include <jsoncons/json.hpp>
#include <jsoncons_ext/jsonpointer/jsonpointer.hpp>
#include <jsoncons_ext/jsonschema/subschema.hpp>
#include <jsoncons_ext/jsonschema/schema_keywords.hpp>
#include <jsoncons_ext/jsonschema/json_schema_draft7.hpp>
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
    using uri_resolver = std::function<Json(const jsoncons::uri & /*id*/)>;

    template <class Json>
    class reference_schema : public schema_keyword<Json>
    {
        using schema_pointer = typename schema_keyword<Json>::schema_pointer;

        schema_pointer referred_schema_;

    public:
        reference_schema(const std::string& id)
            : schema_keyword<Json>(id), referred_schema_(nullptr) {}

        void set_referred_schema(schema_pointer target) { referred_schema_ = target; }

    private:

        void do_validate(const uri_wrapper& instance_location, 
                         const Json& instance, 
                         error_reporter& reporter, 
                         Json& patch) const override
        {
            if (!referred_schema_)
            {
                reporter.error(validation_output(instance_location.string(), "Unresolved schema reference " + this->absolute_keyword_location(), "", this->absolute_keyword_location()));
                return;
            }

            referred_schema_->validate(instance_location, instance, reporter, patch);
        }

        jsoncons::optional<Json> get_default_value(const uri_wrapper& instance_location, 
                                                   const Json& instance, 
                                                   error_reporter& reporter) const override
        {
            if (!referred_schema_)
            {
                reporter.error(validation_output(instance_location.string(), "Unresolved schema reference " + this->absolute_keyword_location(), "", this->absolute_keyword_location()));
                return jsoncons::optional<Json>();
            }

            return referred_schema_->get_default_value(instance_location, instance, reporter);
        }
    };

    template <class Json>
    class schema_loader;

    template <class Json>
    class json_schema
    {
        using schema_pointer = typename schema_keyword<Json>::schema_pointer;

        friend class schema_loader<Json>;

        std::vector<std::unique_ptr<schema_keyword<Json>>> subschemas_;
        schema_pointer root_;
    public:
        json_schema(std::vector<std::unique_ptr<schema_keyword<Json>>>&& subschemas,
                    schema_pointer root)
            : subschemas_(std::move(subschemas)), root_(root)
        {
            if (root_ == nullptr)
                JSONCONS_THROW(schema_error("There is no root schema to validate an instance against"));
        }
    
        json_schema(const json_schema&) = delete;
        json_schema(json_schema&&) = default;
        json_schema& operator=(const json_schema&) = delete;
        json_schema& operator=(json_schema&&) = default;
    
        void validate(const uri_wrapper& instance_location, 
                      const Json& instance, 
                      error_reporter& reporter, 
                      Json& patch) const 
        {
            JSONCONS_ASSERT(root_ != nullptr);
            root_->validate(instance_location, instance, reporter, patch);
        }
    };

    template <class Json>
    struct default_uri_resolver
    {
        Json operator()(const jsoncons::uri& uri)
        {
            if (uri.path() == "/draft-07/schema") 
            {
                return jsoncons::jsonschema::json_schema_draft7<Json>::get_schema();
            }

            JSONCONS_THROW(jsonschema::schema_error("Don't know how to load JSON Schema " + std::string(uri.base())));
        }
    };

    template <class Json>
    class schema_loader : public schema_builder<Json>
    {
        using schema_pointer = typename schema_keyword<Json>::schema_pointer;

        struct subschema_registry
        {
            std::map<std::string, schema_pointer> schemas; // schemas
            std::map<std::string, reference_schema<Json>*> unresolved; // unresolved references
            std::map<std::string, Json> unprocessed_keywords;
        };

        uri_resolver<Json> resolver_;
        schema_pointer root_;

        // Owns all schemas
        std::vector<std::unique_ptr<schema_keyword<Json>>> subschemas_;

        // Map location to subschema_registry
        std::map<std::string, subschema_registry> subschema_registries_;

    public:
        schema_loader(uri_resolver<Json>&& resolver) noexcept

            : resolver_(std::move(resolver))
        {
        }

        schema_loader(const schema_loader&) = delete;
        schema_loader& operator=(const schema_loader&) = delete;
        schema_loader(schema_loader&&) = default;
        schema_loader& operator=(schema_loader&&) = default;

        std::shared_ptr<json_schema<Json>> get_schema()
        {
            return std::make_shared<json_schema<Json>>(std::move(subschemas_), root_);
        }

        schema_pointer make_required_keyword(const std::vector<uri_wrapper>& uris,
                                          const std::vector<std::string>& r) override
        {
            auto sch_orig = jsoncons::make_unique<required_keyword<Json>>(uris, r);
            auto sch = sch_orig.get();
            subschemas_.emplace_back(std::move(sch_orig));
            return sch;
        }

        schema_pointer make_null_keyword(const std::vector<uri_wrapper>& uris) override
        {
            auto sch_orig = jsoncons::make_unique<null_keyword<Json>>(uris);
            auto sch = sch_orig.get();
            subschemas_.emplace_back(std::move(sch_orig));
            return sch;
        }

        schema_pointer make_true_keyword(const std::vector<uri_wrapper>& uris) override
        {
            auto sch_orig = jsoncons::make_unique<true_keyword<Json>>(uris);
            auto sch = sch_orig.get();
            subschemas_.emplace_back(std::move(sch_orig));
            return sch;
        }

        schema_pointer make_false_keyword(const std::vector<uri_wrapper>& uris) override
        {
            auto sch_orig = jsoncons::make_unique<false_keyword<Json>>(uris);
            auto sch = sch_orig.get();
            subschemas_.emplace_back(std::move(sch_orig));
            return sch;
        }

        schema_pointer make_object_keyword(const Json& schema,
                                          const std::vector<uri_wrapper>& uris) override
        {
            auto sch_orig = jsoncons::make_unique<object_keyword<Json>>(this, schema, uris);
            auto sch = sch_orig.get();
            subschemas_.emplace_back(std::move(sch_orig));
            return sch;
        }

        schema_pointer make_array_keyword(const Json& schema,
                                       const std::vector<uri_wrapper>& uris) override
        {
            auto sch_orig = jsoncons::make_unique<array_keyword<Json>>(this, schema, uris);
            auto sch = sch_orig.get();
            subschemas_.emplace_back(std::move(sch_orig));
            return sch;
        }

        schema_pointer make_string_keyword(const Json& schema,
                                        const std::vector<uri_wrapper>& uris) override
        {
            auto sch_orig = jsoncons::make_unique<string_keyword<Json>>(schema, uris);
            auto sch = sch_orig.get();
            subschemas_.emplace_back(std::move(sch_orig));
            return sch;
        }

        schema_pointer make_boolean_keyword(const std::vector<uri_wrapper>& uris) override
        {
            auto sch_orig = jsoncons::make_unique<boolean_keyword<Json>>(uris);
            auto sch = sch_orig.get();
            subschemas_.emplace_back(std::move(sch_orig));
            return sch;
        }

        schema_pointer make_integer_keyword(const Json& schema, 
                                         const std::vector<uri_wrapper>& uris, 
                                         std::set<std::string>& keywords) override
        {
            auto sch_orig = jsoncons::make_unique<number_keyword<Json,int64_t>>(schema, uris, keywords);
            auto sch = sch_orig.get();
            subschemas_.emplace_back(std::move(sch_orig));
            return sch;
        }

        schema_pointer make_number_keyword(const Json& schema, 
                                        const std::vector<uri_wrapper>& uris, 
                                        std::set<std::string>& keywords) override
        {
            auto sch_orig = jsoncons::make_unique<number_keyword<Json,double>>(schema, uris, keywords);
            auto sch = sch_orig.get();
            subschemas_.emplace_back(std::move(sch_orig));
            return sch;
        }

        schema_pointer make_not_keyword(const Json& schema,
                                       const std::vector<uri_wrapper>& uris) override
        {
            auto sch_orig = jsoncons::make_unique<not_keyword<Json>>(this, schema, uris);
            auto sch = sch_orig.get();
            subschemas_.emplace_back(std::move(sch_orig));
            return sch;
        }

        schema_pointer make_all_of_keyword(const Json& schema,
                                          const std::vector<uri_wrapper>& uris) override
        {
            auto sch_orig = jsoncons::make_unique<combining_keyword<Json,all_of_criterion<Json>>>(this, schema, uris);
            auto sch = sch_orig.get();
            subschemas_.emplace_back(std::move(sch_orig));
            return sch;
        }

        schema_pointer make_any_of_keyword(const Json& schema,
                                          const std::vector<uri_wrapper>& uris) override
        {
            auto sch_orig = jsoncons::make_unique<combining_keyword<Json,any_of_criterion<Json>>>(this, schema, uris);
            auto sch = sch_orig.get();
            subschemas_.emplace_back(std::move(sch_orig));
            return sch;
        }

        schema_pointer make_one_of_keyword(const Json& schema,
                                          const std::vector<uri_wrapper>& uris) override
        {
            auto sch_orig = jsoncons::make_unique<combining_keyword<Json,one_of_criterion<Json>>>(this, schema, uris);
            auto sch = sch_orig.get();
            subschemas_.emplace_back(std::move(sch_orig));
            return sch;
        }

        schema_pointer make_type_keyword(const Json& schema,
                                      const std::vector<uri_wrapper>& uris) override
        {
            auto sch_orig = jsoncons::make_unique<type_keyword<Json>>(this, schema, uris);
            auto sch = sch_orig.get();
            subschemas_.emplace_back(std::move(sch_orig));
            return sch;
        }

        schema_pointer build(const Json& schema,
                             const std::vector<uri_wrapper>& uris,
                             const std::vector<std::string>& keys) override
        {
            std::vector<uri_wrapper> new_uris = update_uris(schema, uris, keys);

            schema_pointer sch = nullptr;

            switch (schema.type())
            {
                case json_type::bool_value:
                    if (schema.template as<bool>())
                    {
                        sch = make_true_keyword(new_uris);
                    }
                    else
                    {
                        sch = make_false_keyword(new_uris);
                    }
                    break;
                case json_type::object_value:
                {
                    auto it = schema.find("definitions");
                    if (it != schema.object_range().end()) 
                    {
                        for (const auto& def : it->value().object_range())
                            build(def.value(), new_uris, {"definitions", def.key()});
                    }

                    it = schema.find("$ref");
                    if (it != schema.object_range().end()) // this schema is a reference
                    { 
                        uri_wrapper relative(it->value().template as<std::string>()); 
                        uri_wrapper id = relative.resolve(new_uris.back());
                        sch = get_or_create_reference(id);
                    } 
                    else 
                    {
                        sch = make_type_keyword(schema, new_uris);
                    }
                    break;
                }
                default:
                    JSONCONS_THROW(schema_error("invalid JSON-type for a schema for " + new_uris[0].string() + ", expected: boolean or object"));
                    break;
            }

            for (const auto& uri : new_uris) 
            { 
                insert(uri, sch);

                if (schema.type() == json_type::object_value)
                {
                    for (const auto& item : schema.object_range())
                        insert_unknown_keyword(uri, item.key(), item.value()); // save unknown keywords for later reference
                }
            }
            return sch;
        }

        void load(const Json& sch)
        {
            subschema_registries_.clear();
            root_ = build(sch, {{"#"}}, {});

            // load all external schemas that have not already been loaded

            std::size_t loaded_count = 0;
            do 
            {
                loaded_count = 0;

                std::vector<std::string> locations;
                for (const auto& item : subschema_registries_)
                    locations.push_back(item.first);

                for (const auto& loc : locations) 
                {
                    if (subschema_registries_[loc].schemas.empty()) // registry for this file is empty
                    { 
                        if (resolver_) 
                        {
                            Json external_schema = resolver_(loc);
                            build(external_schema, {{loc}}, {});
                            ++loaded_count;
                        } 
                        else 
                        {
                            JSONCONS_THROW(schema_error("External schema reference '" + loc + "' needs to be loaded, but no resolver provided"));
                        }
                    }
                }
            } 
            while (loaded_count > 0);

            for (const auto &file : subschema_registries_)
            {
                if (!file.second.unresolved.empty())
                {
                    JSONCONS_THROW(schema_error("after all files have been parsed, '" +
                                                (file.first == "" ? "<root>" : file.first) +
                                                "' has still undefined references."));
                }
            }
        }

    private:

        void insert(const uri_wrapper& uri, schema_pointer s)
        {
            auto& file = get_or_create_file(std::string(uri.base()));
            auto schemas_it = file.schemas.lower_bound(std::string(uri.fragment()));
            if (schemas_it != file.schemas.end() && !(file.schemas.key_comp()(std::string(uri.fragment()), schemas_it->first))) 
            {
                JSONCONS_THROW(schema_error("schema with " + uri.string() + " already inserted"));
                return;
            }

            file.schemas.insert({std::string(uri.fragment()), s});

            // is there an unresolved reference to this newly inserted schema?
            auto unresolved_it = file.unresolved.find(std::string(uri.fragment()));
            if (unresolved_it != file.unresolved.end()) 
            {
                unresolved_it->second->set_referred_schema(s);
                file.unresolved.erase(unresolved_it);

            }
        }

        void insert_unknown_keyword(const uri_wrapper& uri, 
                                    const std::string& key, 
                                    const Json& value)
        {
            auto &file = get_or_create_file(std::string(uri.base()));
            auto new_u = uri.append(key);
            uri_wrapper new_uri(new_u);

            if (new_uri.has_json_pointer()) 
            {
                auto fragment = std::string(new_uri.fragment());
                // is there a reference looking for this unknown-keyword, which is thus no longer a unknown keyword but a schema
                auto unresolved = file.unresolved.find(fragment);
                if (unresolved != file.unresolved.end())
                    build(value, {{new_uri}}, {});
                else // no, nothing ref'd it, keep for later
                    file.unprocessed_keywords[fragment] = value;

                // recursively add possible subschemas of unknown keywords
                if (value.type() == json_type::object_value)
                    for (const auto& subsch : value.object_range())
                    {
                        insert_unknown_keyword(new_uri, subsch.key(), subsch.value());
                    }
            }
        }

        schema_pointer get_or_create_reference(const uri_wrapper& uri)
        {
            auto &file = get_or_create_file(std::string(uri.base()));

            // a schema already exists
            auto sch = file.schemas.find(std::string(uri.fragment()));
            if (sch != file.schemas.end())
                return sch->second;

            // referencing an unknown keyword, turn it into schema
            //
            // an unknown keyword can only be referenced by a JSONPointer,
            // not by a plain name identifier
            if (uri.has_json_pointer()) 
            {
                std::string fragment = std::string(uri.fragment());
                auto unprocessed_keywords_it = file.unprocessed_keywords.find(fragment);
                if (unprocessed_keywords_it != file.unprocessed_keywords.end()) 
                {
                    auto &subsch = unprocessed_keywords_it->second; 
                    auto s = build(subsch, {{uri}}, {});       //  A JSON Schema MUST be an object or a boolean.
                    file.unprocessed_keywords.erase(unprocessed_keywords_it);
                    return s;
                }
            }

            // get or create a reference_schema
            auto ref = file.unresolved.lower_bound(std::string(uri.fragment()));
            if (ref != file.unresolved.end() && !(file.unresolved.key_comp()(std::string(uri.fragment()), ref->first))) 
            {
                return ref->second; // unresolved, use existing reference
            } 
            else 
            {
                auto orig = jsoncons::make_unique<reference_schema<Json>>(uri.string());
                auto p = file.unresolved.insert(ref,
                                              {std::string(uri.fragment()), orig.get()})
                    ->second; // unresolved, create new reference
                
                subschemas_.emplace_back(std::move(orig));
                return p;
            }
        }

        subschema_registry& get_or_create_file(const std::string& loc)
        {
            auto file = subschema_registries_.lower_bound(loc);
            if (file != subschema_registries_.end() && !(subschema_registries_.key_comp()(loc, file->first)))
                return file->second;
            else
                return subschema_registries_.insert(file, {loc, {}})->second;
        }

    };

    template <class Json>
    std::shared_ptr<json_schema<Json>> make_schema(const Json& schema)
    {
        schema_loader<Json> loader{default_uri_resolver<Json>()};
        loader.load(schema);

        return loader.get_schema();
    }

    template <class Json,class URIResolver>
    typename std::enable_if<type_traits::is_unary_function_object_exact<URIResolver,Json,std::string>::value,std::shared_ptr<json_schema<Json>>>::type
    make_schema(const Json& schema, const URIResolver& resolver)
    {
        schema_loader<Json> loader(resolver);
        loader.load(schema);

        return loader.get_schema();
    }

} // namespace jsonschema
} // namespace jsoncons

#endif // JSONCONS_JSONSCHEMA_SCHEMA_LOADER_HPP
