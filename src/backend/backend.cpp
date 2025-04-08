#include "backend/backend.hpp"

//std
#include <filesystem>

//library
#include "library/format.hpp"

//backend
#include "backend/impl/details.hpp"
#include "backend/impl/requests.hpp"

namespace global {
	extern std::string PATH;
}

namespace fs = std::filesystem;

namespace backend::inline v1_impl {
	std::string GET_default() {
		return "3Z Calculator Backend";
	}

	crow::response PUT_rotation(const crow::request& req) {
		crow::response response;

		try {
			fs::path ar_folder = lib::format("data/rotations/{}",
				req.url_params.get("aid"));

			if (!fs::exists(ar_folder))
				fs::create_directory(ar_folder);

			auto filename = lib::format("{}/{}.json",
				ar_folder.string(), req.url_params.get("id"));

			std::fstream file(filename, std::ios::out | std::ios::trunc);
			if (!file.is_open())
				throw FMT_RUNTIME_ERROR("file {} is not found", filename);

			auto json = utl::json::from_string(req.body);
			file << json.to_string(utl::json::Format::PRETTY);

			response.code = 200;
		} catch (const std::exception& e) {
			response = { 500, e.what() };
		}

		return response;
	}

	crow::response POST_damage(const crow::request& req, lib::ObjectManager& manager) {
		crow::response response;

		try {
			std::string type = req.url_params.get("type");

			auto json = utl::json::from_string(req.body);
			auto unpacked_request = details::json_to_request(json, manager);

			if (type.empty()) {
				response.body = details::post_damage(unpacked_request)
					.to_string(utl::json::Format::MINIMIZED);
			} else if (type == "detailed") {
				response.body = details::post_damage_detailed(unpacked_request)
					.to_string(utl::json::Format::MINIMIZED);
			} else
				throw FMT_RUNTIME_ERROR("invalid request \"/damage?type={}\"", type);

			response.code = 200;
		} catch (const std::exception& e) {
			response = { 500, e.what() };
		}

		return response;
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
		_init_object_manager();
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
	size_t Backend::_init_object_manager() {
		size_t allocated_objects = 0;
		fs::path res_path = lib::format("{}/data/", global::PATH);
		if (!exists(res_path) || !is_directory(res_path))
			throw FMT_RUNTIME_ERROR("resource folder doesn't exist at path \"{}\"", fs::absolute(res_path).string());

		for (const auto& entry : fs::directory_iterator(res_path)) {
			// ignores non directories
			if (!entry.is_directory())
				continue;

			auto maker_it = details::associated_folders.find(entry.path().filename().string());
			// ignores directories which are not associated with object creator
			if (maker_it == details::associated_folders.end())
				continue;

			auto list = maker_it->second.is_recursive
				? details::recursive_folder_iteration(entry, maker_it->second.func)
				: details::regular_folder_iteration(entry, maker_it->second.func);

			for (const auto& it : list)
				m_manager.add_object(it);
		}

		return allocated_objects;
	}
	void Backend::_init_crow_app() {
		CROW_ROUTE(m_app, "/rotation").methods("PUT"_method)([this](const crow::request& req) {
			return details::wrap_to_check_execution_time<crow::response>("POST /damage",
				[req = std::cref(req), this] {
					return PUT_rotation(req);
				});
		});

		CROW_ROUTE(m_app, "/damage").methods("POST"_method)([this](const crow::request& req) {
			return details::wrap_to_check_execution_time<crow::response>("POST /damage",
				[req = std::cref(req), this] {
					return POST_damage(req, m_manager);
				});
		});
		/*CROW_ROUTE(m_app, "/refresh").methods("POST"_method)([this] {
		    return "";
		});*/
	}
}
