#pragma once

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_DEBUG

#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/logger.h>

extern std::shared_ptr<spdlog::logger> logger;

#define LOGGER_TRACE(...) SPDLOG_LOGGER_TRACE(logger, __VA_ARGS__)
#define LOGGER_DEBUG(...) LOGGER_DEBUG(__VA_ARGS__)
#define LOGGER_INFO(...) LOGGER_INFO(__VA_ARGS__)
#define LOGGER_WARN(...) SPDLOG_LOGGER_WARN(logger, __VA_ARGS__)
#define LOGGER_ERROR(...) LOGGER_ERROR(__VA_ARGS__)
#define LOGGER_CRITICAL(...) SPDLOG_LOGGER_CRITICAL(logger, __VA_ARGS__)
