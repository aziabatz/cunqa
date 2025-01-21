#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include <iostream>
#include <memory>
#include <vector>
#include <spdlog/spdlog.h>
#include <spdlog/logger.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace client { inline std::shared_ptr<spdlog::logger> logger; }
namespace qpu { inline std::shared_ptr<spdlog::logger> logger; }

namespace {
    struct LoggerInitializer {
        LoggerInitializer() {
            try {
                client::logger = spdlog::get("client_logger");
                if (not client::logger) {
                    // QClient logger initialization
                    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
                    console_sink->set_level(spdlog::level::warn);

                    std::string store_path = std::getenv("STORE");
                    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(store_path + "/.api_simulator/logs/logging.log", true);
                    file_sink->set_level(spdlog::level::debug);

                    spdlog::sinks_init_list sinks = { file_sink, console_sink };
                    client::logger = std::make_shared<spdlog::logger>("client_logger", std::begin(sinks), std::end(sinks));
                    client::logger->sinks()[0]->set_pattern("[QClient] %s: %^%l: %v %$ %oms");
                    client::logger->sinks()[1]->set_pattern("%@\n\t%^%l: %v %$ %oms");

                    client::logger->set_level(spdlog::level::debug);
                    spdlog::register_logger(client::logger);
                } 

                qpu::logger = spdlog::get("qpu_logger");
                if (not qpu::logger) {
                    // QPU logger initialization
                    qpu::logger = spdlog::stdout_color_mt("qpu_logger");
                    qpu::logger->set_level(spdlog::level::debug);
                    qpu::logger->set_pattern("[QPU] %^%l: %v %$ %oms");
                }
            } catch (const spdlog::spdlog_ex &ex) {
                std::cerr << "Logger initialization failed: " << ex.what() << std::endl;
            }
        }
    };

    static LoggerInitializer logger_initializer;
}