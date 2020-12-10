#include <stdint.h>

#define SPDLOG_FMT_EXTERNAL
#define SPDLOG_EOL ""
#include <spdlog/spdlog.h>
#include <spdlog/cfg/env.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#define LOG_SPDLOG
#define LOG_FMT
#define LOG_PRINTF
#define LOG_PATH ""
#include "log.h"

#define SPDLOG_DEFAULT_PATTERN "[%^%l%$: %s:%# (%!) %H:%M:%S.%e] %v" // https://spdlog.docsforge.com/v1.x/3.custom-formatting/#pattern-flags

#define SPDLOG_DEFAULT_LEVEL spdlog::level::info

spdlog::level::level_enum to_spdlog(log_level_t lvl)
{
  switch(lvl)
  {
    case log_level_error:
      return spdlog::level::err;
    case log_level_warning:
      return spdlog::level::warn;
    default:
    case log_level_info:
      return spdlog::level::info;
    case log_level_debug:
      return spdlog::level::debug;
  }
}

const char* to_string(log_level_t lvl)
{
  switch(lvl)
  {
    case log_level_error:
      return "error";
    case log_level_warning:
      return "warning";
    case log_level_info:
      return "info";
    case log_level_debug:
      return "debug";
    default:
      return "unknown";
  }
}

struct logger_t
{
  std::shared_ptr<spdlog::logger> stdout_logger;
  std::shared_ptr<spdlog::logger> file_logger;
};

const logger_t& spdlog_get_loggers(const char *format = "", const char* file_path = "")
{
  static logger_t loggers;

  static bool first_run = true;
  if (first_run)
  {

    auto stdout_logger = spdlog::stdout_color_mt("stdout");
    //spdlog::register_logger(stdout_logger);
    //spdlog::set_default_logger(stdout_logger);
    stdout_logger->set_level(spdlog::level::trace);
    loggers.stdout_logger = std::move(stdout_logger);

    if (strlen(file_path) > 0)
    {
      auto file_logger = spdlog::basic_logger_mt("file", file_path);
      file_logger->set_level(spdlog::level::trace);
      //spdlog::register_logger(file_logger);
      loggers.file_logger = std::move(file_logger);
    }

    spdlog::set_level(spdlog::level::trace);

    if (strlen(format) > 0)
      spdlog::set_pattern(format);
    else
      spdlog::set_pattern(SPDLOG_DEFAULT_PATTERN);

    first_run = false;
  }

  return loggers;
}

void spdlog_log_init(const char *format, const char *file_path)
{
  if (format == NULL)
    format = "";
  if (file_path == NULL)
    file_path = "";
  spdlog_get_loggers(format, file_path); // first call initializes logger
}

void spdlog_log_str_impl(const char* str, log_level_t lvl, log_src_info_t* src_info)
{
  static auto loggers = spdlog_get_loggers();

  spdlog::source_loc spdlog_src_info = spdlog::source_loc{ src_info->file_name, src_info->line_number, src_info->function_name };

  auto spdlog_lvl = to_spdlog(lvl);

  if (src_info->write_stdout)
    loggers.stdout_logger->log(spdlog_src_info, spdlog_lvl, str);

  if ((loggers.file_logger) && (src_info->write_file))
    loggers.file_logger->log(spdlog_src_info, spdlog_lvl, str);
}

void spdlog_log_str(const char* str, log_level_t lvl, log_src_info_t* src_info)
{
  spdlog_log_str_impl(str, lvl, src_info);
}


log_level_t log_level(const char* cat)
{
  log_level_t ref_lvl = (log_level_t)-1;
  const char *env = getenv("LOG_LEVEL");
  if (env)
  {
    if ((cat == NULL) || strlen(cat) == 0)
    {
      for (int i=(int)log_level_debug; i <= (int)log_level_error; ++i)
        if (strncmp(env, to_string((log_level_t)i), strlen(to_string((log_level_t)i))) == 0)
          ref_lvl = (log_level_t)i;

      if ((int)ref_lvl == -1)
        ref_lvl = log_level_info;
    }
    else
    {
      for (const char* e = env; (e != NULL) && (e[1] != '\0'); e = strchr(e+1, ','))
      {
        if (*e == ',')
          ++e;

        const char *colon = strchr(e, ':');
        const char *comma = strchr(e, ',');

        bool matches = true;
        {
          const char *glob_start = e;
          const char *cat_match = cat;
          const char *glob;

          do
          {
            glob = strchr(glob_start, '*') ?: strchr(glob_start, ':') ?: strchr(glob_start, ',') ?: strchr(glob_start, '\0');

            if (glob_start == e) // first iteration
            {
              if (strncmp(glob_start, cat_match, glob - glob_start) != 0)
                matches = false;
            }
            else
            {
              bool cat_match_matches = false;
              for (const char* c = cat_match; !cat_match_matches && (*c != '\0'); ++c)
              {
                if (strncmp(c, glob_start, glob - glob_start) == 0)
                {
                  cat_match_matches = true;
                  cat_match = c;
                }
              }
              if (!cat_match_matches)
                matches = false;
            }

            cat_match += glob - glob_start;
            glob_start = glob + 1;
          }
          while (*glob == '*');

          // check tail
          if ((glob[-1] != '*') && (cat_match - cat != (int)strlen(cat)))
            matches = false;
        }

        if (matches)
        {
          if ((colon != NULL) && (colon[1] != '\0')
            && ((comma == NULL) || (comma > colon)))
          {
            for (int i=(int)log_level_debug; i <= (int)log_level_error; ++i)
              if (strncmp(colon+1, to_string((log_level_t)i), strlen(to_string((log_level_t)i))) == 0)
                ref_lvl = (log_level_t)i;
          }
        }
      }

      if ((int)ref_lvl == -1)
        ref_lvl = log_level("");
    }
  }
  else
    ref_lvl = log_level_info;

  return ref_lvl;
}
