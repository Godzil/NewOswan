/*******************************************************************************
 * NewOswan
 * log.c: C Fancy Logger
 *
 * Created by Manoël Trapier on 20/01/2009.
 * Copyright (c) 2009-2022 986-Studio. All rights reserved.
 *
 ******************************************************************************/

#define __LOG_C_INTERNAL_

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <log.h>
#include <sys/time.h>
#include <time.h>
#include <syslog.h>

void log_displayPanic(int signal)
{
    size_t size = 0;

    //size = backtrace(array, 30);
    /* Now flood user with unusefull data. */
    fprintf(stderr, FYELLOW "\n\n ----- " FYELLOW " YICK! ERROR YICK! [bt:%zu]" FYELLOW "----- \n", size);

    // get void*'s for all entries on the stack

    // print out all the frames to stderr
    fprintf(stderr, " ----- Error:  signal: %d -----\n" FRED, signal);
    //backtrace_symbols_fd(array, size, 2);
    fprintf(stderr, FYELLOW "\n\n ----- " FYELLOW " YICK! ERROR YICK! " FYELLOW "----- " CNORMAL "\n");
}

void time_stamp_line(void)
{
    /* Time "0" will be thefirst log line */
    static char firstRun = 1;
    static struct timeval firstTime;
    struct timeval curTime;

    int cMin, cSec;
    long long cMSec;

    /* Get datetime */
    gettimeofday(&curTime, NULL);

    if (firstRun == 1)
    {
        firstRun = 0;
        firstTime.tv_sec = curTime.tv_sec;
        firstTime.tv_usec = curTime.tv_usec;
    }

    cMSec = ((curTime.tv_sec - firstTime.tv_sec) * 1000) + (curTime.tv_usec - firstTime.tv_usec) / 1000;
    cSec = (cMSec / 1000);
    cMSec %= 1000;

    cMin = cSec / 60;

    cSec %= 60;

    /* Put cursor at start of line */
    fprintf(stderr, "%c[s", 0x1B);
    fprintf(stderr, "%c[7000D", 0x1B);
    fprintf(stderr, "%c[1C", 0x1B);
    fprintf(stderr, FWHITE"[" FYELLOW "%03d" FRED "." FBLUE "%02d" FRED "." FGREEN "%03lld" FWHITE "]" CNORMAL, cMin,
            cSec, cMSec);
    fprintf(stderr, "%c[u", 0x1B);
}

void log_real(int level, const char *user, const char *fmt, ...)
{
    int i;
    va_list va;

    /* The LOG_PANIC must always be displayed */
    if ((level <= MAX_DEBUG_LEVEL) || (level <= TLOG_PANIC))
    {
        fprintf(stderr, CNORMAL);
        time_stamp_line();
        fprintf(stderr, CNORMAL " | ");

        switch (level)
        {
        case TLOG_PANIC:   fprintf(stderr, BRED FWHITE); break;
        case TLOG_ERROR:   fprintf(stderr, FRED); break;
        case TLOG_WARNING: fprintf(stderr, FYELLOW); break;
        default:
        case TLOG_NORMAL:  fprintf(stderr, FGREEN); break;
        case TLOG_VERBOSE: fprintf(stderr, FCYAN); break;
        case TLOG_DEBUG:   fprintf(stderr, BBLUE FWHITE); break;
        }

        if (user != NULL)
        {
            i = strlen(user);
            if (i < 12)
            {
                i = 12 - i;
                for (; i >= 0 ; i--)
                {
                    fprintf(stderr, " ");
                }
            }
            fprintf(stderr, "%s", user);
        }
        else
        {
            switch (level)
            {
            case TLOG_PANIC:   fprintf(stderr, "        PANIC"); break;
            case TLOG_ERROR:   fprintf(stderr, "        Error"); break;
            case TLOG_WARNING: fprintf(stderr, "      Warning"); break;
            default:
            case TLOG_NORMAL:  fprintf(stderr, "         Info"); break;
            case TLOG_VERBOSE: fprintf(stderr, "      Verbose"); break;
            case TLOG_DEBUG:   fprintf(stderr, "        Debug"); break;
            }
        }

        fprintf(stderr, CNORMAL " | ");

        va_start(va, fmt);
        vfprintf(stderr, fmt, va);
        va_end(va);

        if (fmt[0] != 0)
        {
            fprintf(stderr, "\n");
        }

#ifdef LOG_ALWAYS_FFLUSH
        /* Systematicaly flush */
        fflush(stderr);
#endif
    }
}

