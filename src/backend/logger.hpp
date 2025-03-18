#pragma once

//std
#include <fstream>
#include <mutex>
#include <string>

//crow
#include "crow/logging.h"

namespace backend {
    class Logger : public crow::ILogHandler {
    public:
        explicit Logger(const std::string& path);

        void log(std::string message, crow::LogLevel level) override;

    private:
        std::mutex _mutex;
        std::fstream _file;
    };
}
