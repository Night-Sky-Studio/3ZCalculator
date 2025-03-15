//std
#include <fstream>

//toml11
#include "toml.hpp"

//library
#include "library/funcs.hpp"

//backend
#include "backend/calculator.hpp"
#include "backend/converters.hpp"
#include "backend/object_manager.hpp"

int main() {
    backend::ToEvalDataConverter::init();
    backend::ObjectManager manager;

    manager.add_utility_funcs({
        .folder = "agents",
        .loader = [](const std::string& path) {
            std::fstream file(path, std::ios::in | std::ios::binary);
            if (!file.is_open())
                throw std::runtime_error("file is not found");

            auto toml = toml::parse(file);
            file.close();

            auto result = std::make_shared<zzz::AgentDetails>(global::to_agent.from(toml));
            return std::static_pointer_cast<void>(result);
        }
    });

    manager.add_object("agents", 1091);

    auto agent = manager.at<zzz::AgentDetails>(1091);

    //auto toml = lib::load_by_id("players", 1500438496);
    //auto input = backend::global::to_eval_data.from(toml);
    //auto result = backend::Calculator::eval(input);

    return 0;
}
