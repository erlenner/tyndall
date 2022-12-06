#include <cstdint>
#include <cstdlib>

#include "log.h"

const char* log_level_to_string(log_level_t lvl)
{
    switch(lvl)
    {
        case log_level_t::fatal:
            return "fatal";
        case log_level_t::error:
            return "error";
        case log_level_t::alarm:
            return "alarm";
        case log_level_t::warning:
            return "warning";
        case log_level_t::info:
            return "info";
        case log_level_t::debug:
            return "debug";
        case log_level_t::trace:
            return "trace";
        default:
            return "unknown";
    }
}

std::string log_level_to_color_string(log_level_t lvl)
{
    fmt::color c;
    switch(lvl)
    {
        case log_level_t::fatal:    c = fmt::color::dark_red; break;
        case log_level_t::error:    c = fmt::color::red; break;
        case log_level_t::alarm:    c = fmt::color::dark_khaki; break;
        case log_level_t::warning:  c = fmt::color::yellow; break;
        case log_level_t::info:     c = fmt::color::light_green; break;
        case log_level_t::debug:    c = fmt::color::cyan; break;
        case log_level_t::trace:    c = fmt::color::dark_cyan; break;

        default:
            c = fmt::color::white;
    }
    return fmt::format(fmt::emphasis::bold | fmt::fg(c), log_level_to_string(lvl));
}

// parse log_level from string
// accept integers as well
log_level_t log_level_from_string(const char* string)
{
    log_level_t ret = log_level_t::invalid;

    for (int i=0; i <= (int)log_level_t::n_levels; ++i)
        if ((strncmp(string, log_level_to_string((log_level_t)i), strlen(log_level_to_string((log_level_t)i))) == 0)
            || ((string[0] == '0' + i) && (strlen(string) == 1)))
            ret = (log_level_t)i;
        else if (int integer_env = (int)strtol(string, NULL, 10); integer_env !=0)
            ret = (log_level_t)integer_env;

    return ret;
}

// loop through the different glob parts of a glob pattern (f.ex. "glob1*glob2*glob3*")
// check for match with the category (separately iterated)
// n is the size of the pattern
bool log_glob_match(const char* pat, const char* str, long n)
{
  bool ret = true;
  const char *pat_end, *pat_start = pat, *str_index = str;
  do
  {
      pat_end = strchr(pat_start, '*');
      if (pat_end == NULL)
          pat_end = pat + n;
      else
          pat_end = std::min(pat_end, pat + n);
  
      // search for the text at str_index between pat_start and pat_end
      bool pat_matches = false;
      for (const char* s = str_index; !pat_matches && (*s != '\0'); ++s)
      {
          if (strncmp(s, pat_start, static_cast<size_t>(pat_end - pat_start)) == 0)
          {
              pat_matches = true;
              str_index = s;
          }
      }
      if (!pat_matches)
          ret = false;
  
      str_index += pat_end - pat_start;
      pat_start = pat_end + 1;
  
  } while (*pat_end == '*');

  return ret;
}


log_level_t log_level(const char* cat)
{
    log_level_t ref_lvl = log_level_t::invalid;
    const char *env = getenv("LOG_LEVEL");
    if (env)
    {
        // loop through comma separated entries
        for (const char* e = env; (e != NULL) && (e[1] != '\0'); e = strchr(e+1, ','))
        {
            if (*e == ',')
                ++e;

            const char *colon = strchr(e, ':');
            const char *comma = strchr(e, ',');

            // handle "field=value" entries
            if ((colon != NULL) && (colon[1] != '\0')
                && ((comma == NULL) || (comma > colon))
                && log_glob_match(e, cat, colon - e))
            {
                ref_lvl = log_level_from_string(colon+1);
            }
            else if (ref_lvl == log_level_t::invalid)
                ref_lvl = log_level_from_string(e); // handle "value" entries, which apply to all remaining categories
        }
    }

    // set default level if there are no log levels given
    if (ref_lvl == log_level_t::invalid)
        ref_lvl = log_level_default;

    return ref_lvl;
}
