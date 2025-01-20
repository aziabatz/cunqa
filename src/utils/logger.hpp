/*
 * Disclaimer:
 *   This was not compiled but extracted from documentation and some code.
 */

// mylibrary.h
// In library, we skip the symbol exporting part
#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include <memory>
#include <vector>
#include <spdlog/spdlog.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "spdlog/sinks/basic_file_sink.h"

namespace loggie
{

static const std::string logger_name = "loggie";

inline std::shared_ptr<spdlog::logger> get_logger()
{
    auto logger = spdlog::get(logger_name);
    if(not logger)
    {
        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_sink->set_level(spdlog::level::warn);

        std::string store_path = std::getenv("STORE");
        auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(store_path + "/.api_simulator/logs/logging.log", true);
        file_sink->set_level(spdlog::level::trace);

        spdlog::sinks_init_list sinks = { file_sink, console_sink };

        logger = std::make_shared<spdlog::logger>(logger_name,
                                                    std::begin(sinks),
                                                    std::end(sinks));
        logger->sinks()[0]->set_pattern("%s: %^%l: %v %$ %oms");
        logger->sinks()[1]->set_pattern("%@\n\t%^%l: %v %$ %oms");
        spdlog::register_logger(logger);
    }

    return logger;
}

//TODO: See how to change this and get_logger to static types
inline auto logger = get_logger();

}