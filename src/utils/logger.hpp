//
// Created by Enzo Crema on 31/12/2023.
//

#pragma once

#include "types.hpp"

#define LOG_WARN_ENABLE 1
#define LOG_INFO_ENABLE 1
#define LOG_DEBUG_ENABLE 1
#define LOG_TRACE_ENABLE 1

#ifdef NDEBUG
#define LOG_DEBUG_ENABLE 0
#define LOG_TRACE_ENABLE 0
#endif

using LogLevel = enum LogLevel : u8 {
	LOG_LEVEL_FATAL = 0,
	LOG_LEVEL_ERROR = 1,
	LOG_LEVEL_WARN = 2,
	LOG_LEVEL_INFO = 3,
	LOG_LEVEL_DEBUG = 4,
	LOG_LEVEL_TRACE = 5,
};


template <typename... Args>
constexpr void log_output(LogLevel level, const char* message, Args&&... args) {
	std::array<const char*, 6> level_string = {
	    "[FATAL]: ",
	    "[ERROR]: ",
	    "[WARN]: ",
	    "[INFO]: ",
	    "[DEBUG]: ",
	    "[TRACE]: "
	};

	fmt::print("{}{}\n", level_string.at(level), fmt::format(fmt::runtime(message), std::forward<Args>(args)...));
}

template <typename... Args>
constexpr void VKFATAL(const char* message, Args&&... args) {
	log_output(LOG_LEVEL_FATAL, message, std::forward<Args>(args)...);
}

#ifndef VKERROR
template<typename ... Args>
constexpr void VKERROR(const char* message, Args&&... args) {
	log_output(LOG_LEVEL_ERROR, message, std::forward<Args>(args)...);
}
#endif

#if LOG_WARN_ENABLE
template<typename ... Args>
constexpr void VKWARN(const char* message, Args&&... args) {
	log_output(LOG_LEVEL_WARN, message, std::forward<Args>(args)...);
}
#else
template<typename ... Args>
constexpr void VKWARN(const char* /*unused*/, Args... /*unused*/) {}
#endif

#if LOG_INFO_ENABLE
template<typename ... Args>
constexpr void VKINFO(const char* message, Args&&... args) {
	log_output(LOG_LEVEL_INFO, message, std::forward<Args>(args)...);
}
#else
template<typename ... Args>
constexpr void VKINFO(const char* /*unused*/, Args&&... /*unused*/) {}
#endif

#if LOG_DEBUG_ENABLE
template<typename ... Args>
constexpr void VKDEBUG(const char* message, Args&&... args) {
	log_output(LOG_LEVEL_DEBUG, message, std::forward<Args>(args)...);
}
#else
template<typename ... Args>
constexpr void VKDEBUG(const char* /*unused*/, Args&&... /*unused*/) {}
#endif

#if LOG_TRACE_ENABLE
template<typename ... Args>
constexpr void VKTRACE(const char* message, Args&&... args) {
	log_output(LOG_LEVEL_TRACE, message, std::forward<Args>(args)...);
}
#else
template<typename ... Args>
constexpr void VKTRACE(const char* /*unused*/, Args&&... /*unused*/) {}
#endif


#ifndef NDEBUG
#define VK_CHECK(result) do { \
    if ((result) != VK_SUCCESS) { \
        VKFATAL("Detected Vulkan error: {} in file {} at line {}\n", string_VkResult(result), __FILE__, __LINE__); \
        std::abort(); \
    } \
} while(0)
#else
#define VK_CHECK(x) x
#endif