//std
#include <iostream>

//toml11
#include "toml.hpp"

//library
#include "library/funcs.hpp"

//backend
#include "backend/calculator.hpp"
#include "backend/converters.hpp"
#include "backend/object_manager.hpp"

//zzz
#include "zzz/details.hpp"

size_t alloc_object_manager(backend::ObjectManager& manager) {
    size_t allocated_objects = 0;
    std::fstream file("loadable_objects.toml", std::ios::in | std::ios::binary);
    auto toml = toml::parse(file);
    file.close();

    auto add_objects_and_loader = [&](const std::string& folder, const backend::any_ptr_loader& loader) {
        manager.add_utility_funcs({
            .folder = folder,
            .loader = loader
        });
        for (const auto& it : toml.at(folder).as_array()) {
            std::string name;

            switch (it.type()) {
            case toml::value_t::integer:
                name = std::to_string(it.as_integer());
                break;
            case toml::value_t::string:
                name = it.as_string();
                break;
            default:
                throw std::runtime_error("wrong type");
            }

            allocated_objects++;
            manager.add_object(folder, folder + '/' + name);
            std::cout << lib::format("added object {}\n", folder + '/' + name);
        }
    };

    add_objects_and_loader("agents", [](const toml::value& toml) {
        auto result = std::make_shared<zzz::AgentDetails>(global::to_agent.from(toml));
        return std::static_pointer_cast<void>(result);
    });
    add_objects_and_loader("wengines", [](const toml::value& toml) {
        auto result = std::make_shared<zzz::WengineDetails>(global::to_wengine.from(toml));
        return std::static_pointer_cast<void>(result);
    });
    add_objects_and_loader("dds", [](const toml::value& toml) {
        auto result = std::make_shared<zzz::DdsDetails>(global::to_dds.from(toml));
        return std::static_pointer_cast<void>(result);
    });
    add_objects_and_loader("rotations", [](const toml::value& toml) {
        auto result = std::make_shared<zzz::rotation_details>(global::to_rotation.from(toml));
        return std::static_pointer_cast<void>(result);
    });

    return allocated_objects;
}

int main() {
    backend::ObjectManager manager;

    size_t allocated_object = alloc_object_manager(manager);
    std::cout << allocated_object << '\n';
    manager.launch();

    auto toml = lib::load_by_id("players", 1500438496);
    auto input = global::to_eval_data.from(toml);
    auto result = backend::Calculator::eval(manager, input);

    return 0;
}
