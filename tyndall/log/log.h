#pragma once

#include <string>
#include <cstring>
#include <utility>
#include <cassert>
#include <climits>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/color.h>

// interface
#define log_fatal(...) log_log(log_level_t::fatal, __VA_ARGS__)
#define log_error(...) log_log(log_level_t::error, __VA_ARGS__)
#define log_alarm(...) log_log(log_level_t::fatal, __VA_ARGS__)
#define log_warning(...) log_log(log_level_t::warning, __VA_ARGS__)
#define log_info(...) log_log(log_level_t::info, __VA_ARGS__)
#define log_debug(...) log_log(log_level_t::debug, __VA_ARGS__)
#define log_trace(...) log_log(log_level_t::trace, __VA_ARGS__)

#define log_cat_fatal(cat, ...) log_cat_log(cat, log_level_t::fatal, __VA_ARGS__)
#define log_cat_error(cat, ...) log_cat_log(cat, log_level_t::error, __VA_ARGS__)
#define log_cat_alarm(cat, ...) log_cat_log(cat, log_level_t::alarm, __VA_ARGS__)
#define log_cat_warning(cat, ...) log_cat_log(cat, log_level_t::warning, __VA_ARGS__)
#define log_cat_info(cat, ...) log_cat_log(cat, log_level_t::info, __VA_ARGS__)
#define log_cat_debug(cat, ...) log_cat_log(cat, log_level_t::debug, __VA_ARGS__)
#define log_cat_trace(cat, ...) log_cat_log(cat, log_level_t::trace, __VA_ARGS__)

// implementation

#define log_log(lvl, fmt, ...) log_cat_log("", lvl, fmt, __VA_ARGS__)
#define log_cat_log(cat, lvl, fmt, ...) log_impl(fmt, lvl, cat, __VA_ARGS__)

enum struct log_level_t : int { fatal = 0, error, alarm, warning, info, debug, trace, n_levels, invalid=INT_MAX };

static constexpr log_level_t log_level_default = log_level_t::info;

std::string log_level_to_color_string(log_level_t lvl);

/*
    Reads the log level from the LOG_LEVEL environment variable.
    Optional global / fallback level first, then key-values of categories and debug level.
    Default category is function name.
    Examples:
        LOG_LEVEL=error ./my_program
        LOG_LEVEL=1 ./my_program
        LOG_LEVEL=error,my_category:debug ./my_program
        LOG_LEVEL=my_ca*e*ry:info ./my_program
*/
log_level_t log_level(const char* cat);

#define log_impl(_fmt, lvl, category, ...) \
do { \
    const std::string cat = ((category && strlen(category)) ? category : __PRETTY_FUNCTION__); \
    \
    if(lvl <= log_level(cat.c_str())) { \
        std::string colored_cat = fmt::format(fmt::fg(fmt::color::dark_violet), cat); \
        std::string preamble = fmt::format("[{}\t{}:{} {}]\t", log_level_to_color_string(lvl), __FILE__, __LINE__, colored_cat); \
        fmt::print(preamble + (_fmt) + "\n" __VA_OPT__(,) __VA_ARGS__); \
    } \
} while(0)
