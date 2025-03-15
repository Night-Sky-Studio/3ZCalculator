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

void alloc_object_manager(backend::ObjectManager& manager) {
    std::fstream file("loadable_objects.toml", std::ios::in | std::ios::binary);
    auto toml = toml::parse(file);

    manager.add_utility_funcs({
        .folder = "agents",
        .loader = [](const toml::value& toml) {
            auto result = std::make_shared<zzz::AgentDetails>(global::to_agent.from(toml));
            return std::static_pointer_cast<void>(result);
        }
    });
    for (const auto& it : toml.at("agents").as_array())
        manager.add_object("agents", it.as_integer());

    manager.add_utility_funcs({
        .folder = "wengines",
        .loader = [](const toml::value& toml) {
            auto result = std::make_shared<zzz::WengineDetails>(global::to_wengine.from(toml));
            return std::static_pointer_cast<void>(result);
        }
    });
    for (const auto& it : toml.at("wengines").as_array())
        manager.add_object("wengines", it.as_integer());

    manager.add_utility_funcs({
        .folder = "dds",
        .loader = [](const toml::value& toml) {
            auto result = std::make_shared<zzz::DdsDetails>(global::to_dds.from(toml));
            return std::static_pointer_cast<void>(result);
        }
    });
    for (const auto& it : toml.at("dds").as_array())
        manager.add_object("dds", it.as_integer());

    manager.add_utility_funcs({
        .folder = "rotations",
        .loader = [](const toml::value& toml) {
            auto result = std::make_shared<zzz::rotation_details>(global::to_rotation.from(toml));
            return std::static_pointer_cast<void>(result);
        }
    });
    for (const auto& it : toml.at("rotations").as_array())
        manager.add_object("rotations", it.as_integer());
}

int main() {
    backend::ObjectManager manager;

    alloc_object_manager(manager);
    manager.launch();

    auto toml = lib::load_by_id("players", 1500438496);
    auto input = global::to_eval_data.from(toml);
    auto result = backend::Calculator::eval(manager, input);

    return 0;
}
