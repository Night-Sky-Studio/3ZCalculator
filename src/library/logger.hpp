#pragma once

//std
#include <list>
#include <mutex>
#include <ostream>
#include <string>

//crow
#include "crow/logging.h"

namespace backend {
    class Logger : public crow::ILogHandler {
    public:
        void add_log_stream(std::ostream& stream);

        void log(std::string message, crow::LogLevel level) override;

    private:
        std::mutex _mutex;
        std::list<std::ostream*> _ostreams;
    };
}
