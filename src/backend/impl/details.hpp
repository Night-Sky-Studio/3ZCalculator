#pragma once

//std
#include <filesystem>
#include <memory>
#include <string>

//utl
#include "utl/json.hpp"

//library
#include "library/format.hpp"

//calc
#include "calc/calculator.hpp"

namespace fs = std::filesystem;

namespace backend::details {
    // filesystem

    using maker_func = std::function<lib::MObjectPtr(const std::string&)>;

    struct object_maker {
        maker_func func;
        bool is_recursive = false;
    };

    extern const std::unordered_map<std::string, object_maker> associated_folders;

    // TODO: make part of global scope
    using maker_iterator = const std::unordered_map<std::string, object_maker>::const_iterator;

    std::list<lib::MObjectPtr> regular_folder_iteration(
        const fs::directory_entry& entry,
        const maker_func& func);
    std::list<lib::MObjectPtr> recursive_folder_iteration(
        const fs::directory_entry& entry,
        const maker_func& func);

    // preparers

    void prepare_request_details(calc::request_t& what, const utl::Json& source);

    // TODO: remake with unordered_map or list
    void prepare_request_composed(calc::request_t& what, lib::ObjectManager& source);

    size_t prepare_object_manager(lib::ObjectManager& manager);
}
