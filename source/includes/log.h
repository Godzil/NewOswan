/*******************************************************************************
 * NewOswan
 * log.h: C Fancy Logger
 *
 * Created by Manoël Trapier on 20/01/2009.
 * Copyright (c) 2009-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#ifndef _LOG_H
#define	_LOG_H

#ifdef	__cplusplus
extern "C" {
#endif

//#define ALLOW_COLORS

#ifdef ALLOW_COLORS
#define __C(c) "\x1B[" c "m"
#else
#define __C(c) ""
#endif

#define ANSI_COLOR __C
#define FBLACK ANSI_COLOR("30")
#define FRED ANSI_COLOR("31")
#define FGREEN ANSI_COLOR("32")
#define FYELLOW ANSI_COLOR("33")
#define FBLUE ANSI_COLOR("34")
#define FMAGENTA ANSI_COLOR("35")
#define FCYAN ANSI_COLOR("36")
#define FWHITE ANSI_COLOR("37")

#define BBLACK ANSI_COLOR("40")
#define BRED ANSI_COLOR("41")
#define BGREEN ANSI_COLOR("42")
#define BYELLOW ANSI_COLOR("43")
#define BBLUE ANSI_COLOR("44")
#define BMAGENTA ANSI_COLOR("45")
#define BCYAN ANSI_COLOR("46")
#define BWHITE ANSI_COLOR("47")

#define CNORMAL ANSI_COLOR("0")

enum
{
    TLOG_ALWAYS = -1,
    TLOG_PANIC = 0,
    TLOG_ERROR,
    TLOG_WARNING,
    TLOG_NORMAL,
    TLOG_VERBOSE,
    TLOG_DEBUG,
};

//#define LOG_ALWAYS_FFLUSH
#define DYNA_LOG_LEVEL
#define SET_DEBUG_LOG

/* Set if DYNALOG is set the maximum compiled log level */
#ifndef MAXIMUM_DEBUG_LEVEL

#ifndef SET_DEBUG_LOG
#define MAXIMUM_DEBUG_LEVEL TLOG_NORMAL
#else
#define MAXIMUM_DEBUG_LEVEL TLOG_DEBUG
#endif

#endif /* MAXIMUM_DEBUG_LEVEL */

/* Set the default log level */
#ifndef SET_DEBUG_LOG
#define DEFAULT_DEBUG_LEVEL TLOG_PANIC
#else
#define DEFAULT_DEBUG_LEVEL TLOG_DEBUG
#endif

/******************************************************************************/
/*                        DO NOT MESS AFTER THIS LINE                         */
/******************************************************************************/

#ifdef DYNA_LOG_LEVEL
# ifdef MAX_DEBUG_LEVEL
#  undef MAX_DEBUG_LEVEL
# endif
# ifdef __LOG_C_INTERNAL_
int MAX_DEBUG_LEVEL = DEFAULT_DEBUG_LEVEL;
#else
extern int MAX_DEBUG_LEVEL;
#endif
#else
# ifndef MAX_DEBUG_LEVEL
#  define MAX_DEBUG_LEVEL DEFAULT_DEBUG_LEVEL
# endif
#endif

#define Log(_level, _user, _fmt, ...)\
 if (_level <= MAXIMUM_DEBUG_LEVEL)\
  if ((_level <= MAX_DEBUG_LEVEL) || (_level <= TLOG_PANIC))\
   do { log_real(_level, _user, _fmt, ##__VA_ARGS__); } while(0)

void log_real(int level, const char *user, const char *fmt, ...);

#define LOG(_level, _str, ...) if ((_level <= MAX_DEBUG_LEVEL) || (_level <= TLOG_PANIC)) do { fputs(_str, stderr); } while(0)
#define LOGCODE(_level, _user, _code)\
   if (_level <= MAXIMUM_DEBUG_LEVEL) do{\
    Log(_level, _user, "");\
    if ((_level <= MAX_DEBUG_LEVEL) || (_level <= TLOG_PANIC))\
       do { _code; fprintf(stderr, "\n"); } while(0); } while(0)

#define INFOL(_level, _fmt) LOGCODE(_level, "INFOL", { printf _fmt; })

#define FUNC_IN() Log(TLOG_VERBOSE, NULL, ">>%s", __func__)
#define FUNC_OUT() Log(TLOG_VERBOSE, NULL, "<<%s", __func__)
#define FUNC_OUTR(out) Log(TLOG_VERBOSE, NULL, "<<%s (%d)", __func__, out)

void log_displayPanic(int signal);

#ifdef	__cplusplus
}
#endif

#endif	/* _LOG_H */

