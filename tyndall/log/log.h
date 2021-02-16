#pragma once

/*
  log.h
  log_error, log_warning etc. follows printf syntax.
  Format strings follows fmtlib format if LOG_FMT is defined,
  It uses printf format instead if LOG_PRINTF is defined.
*/

#ifdef __cplusplus
#include <string>
#include <cstring>
#include <utility>
#include <cassert>
#else
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#endif

// interface
#define log_log(lvl, fmt, ...) log_cat_log("", lvl, fmt, __VA_ARGS__)
#define log_error(...) log_log(log_level_error, __VA_ARGS__)
#define log_warning(...) log_log(log_level_warning, __VA_ARGS__)
#define log_info(...) log_log(log_level_info, __VA_ARGS__)
#define log_debug(...) log_log(log_level_debug, __VA_ARGS__)

#define log_cat_log(cat, lvl, fmt, ...) log_impl(fmt, lvl, cat, __VA_ARGS__)
#define log_cat_error(cat, ...) log_cat_log(cat, log_level_error, __VA_ARGS__)
#define log_cat_warning(cat, ...) log_cat_log(cat, log_level_warning, __VA_ARGS__)
#define log_cat_info(cat, ...) log_cat_log(cat, log_level_info, __VA_ARGS__)
#define log_cat_debug(cat, ...) log_cat_log(cat, log_level_debug, __VA_ARGS__)

typedef struct
{
  const char* pattern; // follows spdlog set_pattern style: https://github.com/gabime/spdlog/wiki/3.-Custom-formatting
  const char* file_path; // path to log output file. Will append to file.
  long flush_every_sec; // seconds between each manual fflush. 0 means no manual flush.
} log_init_params;
#define log_init log_init_impl
#define log_str log_str_impl
#define log_level log_level_impl

#define log_once(log_func, ...) do{ \
  static int run = 1; \
  if (run) \
  { \
    log_func(__VA_ARGS__); \
    run = 0; \
  } \
} while(0)

#define log_once_error(...)   log_once(log_error, __VA_ARGS__)
#define log_once_warning(...) log_once(log_warning, __VA_ARGS__)
#define log_once_info(...)    log_once(log_info, __VA_ARGS__)
#define log_once_debug(...)   log_once(log_debug, __VA_ARGS__)

#define log_cat_once_error(...)   log_once(log_cat_error, __VA_ARGS__)
#define log_cat_once_warning(...) log_once(log_cat_warning, __VA_ARGS__)
#define log_cat_once_info(...)    log_once(log_cat_info, __VA_ARGS__)
#define log_cat_once_debug(...)   log_once(log_cat_debug, __VA_ARGS__)

#define log_errno(fmt, ...) log_error("[" log_color_red log_color_bold "errno" log_color_reset ": " LOG_PATTERN_STR "] " fmt, strerror(errno) __VA_OPT__(,) __VA_ARGS__)


// colors
#define log_color_red     "\033[31m"
#define log_color_green   "\033[32m"
#define log_color_yellow  "\033[33m"
#define log_color_blue    "\033[34m"
#define log_color_magenta "\033[35m"
#define log_color_cyan    "\033[36m"
#define log_color_reset   "\033[0m"
#define log_color_bold    "\033[1m"


// implementation

typedef enum { log_level_debug, log_level_info, log_level_warning, log_level_error } log_level_t;

// struct for recording the origin of the log function call
typedef struct
{
  const char* file_name;
  const char* function_name;
  int line_number;
  bool write_stdout;
  bool write_file;
} log_src_info_t;

#ifdef __cplusplus
extern "C"
{
#endif
  /*
    Reads the log level from the LOG_LEVEL environment variable.
    Optional global / fallback level first, then key-values of categories and debug level.
    Default category is function name.
    For each category my_cat the corresponding category FILE_my_cat controls log file output.
    Examples:
      LOG_LEVEL=error ./my_program
      LOG_LEVEL=error,my_category:debug ./my_program
      LOG_LEVEL=debug,my_category:error,FILE_my_category:error ./my_program
      LOG_LEVEL=FILE*:error,my_ca*e*ry:info ./my_program
  */
  log_level_t log_level(const char* cat);
#ifdef __cplusplus
} // extern "C"
#endif

#define log_impl(fmt, lvl, category, ...) \
do { \
  static bool init = true; \
  static log_src_info_t src_info = {0}; \
  if (init) \
  { \
    const char *cat = (const char*)((category && strlen(category)) ? category : __FUNCTION__); \
    char file_cat[1024]; \
    snprintf(file_cat, sizeof(file_cat), "%s%s", "FILE_", cat); \
    \
    src_info = (log_src_info_t) { \
      .file_name = __FILE__, \
      .function_name = cat, \
      .line_number = __LINE__, \
      .write_stdout = (lvl >= log_level(cat)), \
      .write_file = (lvl >= log_level(file_cat)), \
    }; \
    init = false; \
  } \
  if (src_info.write_stdout || src_info.write_file) \
  { \
    log_format_impl(fmt, lvl, &src_info, __VA_ARGS__); \
  } \
} while(0)

#ifdef __cplusplus
extern "C"
{
#endif

  void log_init_impl(log_init_params params);

  void log_str_impl(const char* str, log_level_t lvl, log_src_info_t* src_info);
#ifdef __cplusplus
} // extern "C"
#endif

#if defined(LOG_FMT)
  #ifndef __cplusplus
    #error fmt formatting is only available for C++, not C
  #endif

  #include <fmt/format.h>
  #include <fmt/ostream.h>

  #define log_format_impl(_fmt, lvl, src_info, ...) do { \
    std::string str = fmt::format(_fmt __VA_OPT__(,) __VA_ARGS__); \
    log_str_impl(str.c_str(), lvl, src_info); \
  } while(0)

  #define LOG_PATTERN_STR "{}"

#elif defined(LOG_PRINTF)

  #define log_format_impl(fmt, lvl, src_info, ...) do { \
    char buf[1024]; \
    snprintf(buf, sizeof(buf), fmt __VA_OPT__(,) __VA_ARGS__); \
    log_str_impl(buf, lvl, src_info); \
  } while(0)

  #define LOG_PATTERN_STR "%s"

#else
  #error You need to specify either LOG_FMT or LOG_PRINTF
#endif
