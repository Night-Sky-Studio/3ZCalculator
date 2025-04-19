#include "backend/backend.hpp"

//std
#include <chrono>
#include <functional>
#include <string>

//library
#include "library/format.hpp"

//backend
#include "backend/impl/details.hpp"
#include "backend/impl/requests.hpp"

namespace global {
	extern std::string PATH;
}

namespace backend {
    template<typename TResult, typename... TArgs>
    TResult wrap_to_check_execution_time(
        std::string_view name,
        std::function<TResult()> func) {
        auto start = std::chrono::high_resolution_clock::now();
        auto result = func();
        std::chrono::duration<double, std::milli> time =
            std::chrono::high_resolution_clock::now() - start;

        CROW_LOG_INFO << lib::format("{} was dispatched in {} ms", name, time.count());

        return result;
    }
}

namespace backend {
	Backend::~Backend() {
		if (m_log_file.has_value())
			m_log_file->close();
	}

	// getters

	lib::ObjectManager& Backend::manager() {
		return m_manager;
	}

	// runners

	void Backend::run() {
		m_manager.launch();

		uint16_t max_threads = std::thread::hardware_concurrency();
		m_app.port(port)
		     .concurrency(max_threads < max_thread_load ? max_threads : max_thread_load)
		     .run();
	}

	// initializers

	void Backend::init() {
		crow::logger::setHandler(&m_logger);
		crow::logger::setLogLevel(crow::LogLevel::INFO);

		lib::ObjectManager::init_default_file_extensions();

		_init_logger(true);
		details::prepare_object_manager(m_manager);
		_init_crow_app();
	}

	void Backend::_init_logger(bool use_file) {
		m_logger.add_log_stream(std::cout);
		if (use_file) {
			m_log_file.emplace(std::fstream(lib::format("{}/logs/console.log", global::PATH),
				std::ios::out | std::ios::trunc));
			m_logger.add_log_stream(*m_log_file);
		}
	}
	void Backend::_init_crow_app() {
		CROW_ROUTE(m_app, "/rotation").methods("PUT"_method)([this](const crow::request& req) {
            return wrap_to_check_execution_time<crow::response>("POST /damage",
				[req = std::cref(req), this] {
                    return methods::put_rotation(req);
				});
		});

		CROW_ROUTE(m_app, "/damage").methods("POST"_method)([this](const crow::request& req) {
            return wrap_to_check_execution_time<crow::response>("POST /damage",
				[req = std::cref(req), this] {
                    return methods::post_damage(req, m_manager);
				});
		});
		CROW_ROUTE(m_app, "/refresh").methods("POST"_method)([this] {
            return wrap_to_check_execution_time<crow::response>("POST /refresh",
				[this] {
                    return methods::post_refresh(m_manager);
				});
		});
	}
}
