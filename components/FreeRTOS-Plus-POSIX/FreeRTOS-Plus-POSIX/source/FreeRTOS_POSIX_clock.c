/*
 * Amazon FreeRTOS POSIX V1.1.0
 * Copyright (C) 2018 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/**
 * @file FreeRTOS_POSIX_clock.c
 * @brief Implementation of clock functions in time.h
 */

/* C standard library includes. */
#include <stddef.h>
#include <string.h>
#include <time.h>

/* FreeRTOS+POSIX includes. */
#include "FreeRTOS_POSIX.h"
#include "FreeRTOS_POSIX/errno.h"
#include "FreeRTOS_POSIX/time.h"
#include "FreeRTOS_POSIX/utils.h"

/* Declaration of snprintf. The header stdio.h is not included because it
 * includes conflicting symbols on some platforms. */
extern int snprintf( char * s,
                     size_t n,
                     const char * format,
                     ... );

/*-----------------------------------------------------------*/

clock_t clock( void )
{
    /* This function is currently unsupported. It will always return -1. */

    return ( clock_t ) -1;
}

/*-----------------------------------------------------------*/

int clock_getcpuclockid( pid_t pid,
                         clockid_t * clock_id )
{
    /* Silence warnings about unused parameters. */
    ( void ) pid;
    ( void ) clock_id;

    /* This function is currently unsupported. It will always return EPERM. */
    return EPERM;
}

/*-----------------------------------------------------------*/

int clock_nanosleep( clockid_t clock_id,
                     int flags,
                     const struct timespec * rqtp,
                     struct timespec * rmtp )
{
    int iStatus = 0;
    TickType_t xSleepTime = 0;
    struct timespec xCurrentTime = { 0 };

    /* Silence warnings about unused parameters. */
    ( void ) clock_id;
    ( void ) rmtp;
    ( void ) flags; /* This is only ignored if INCLUDE_vTaskDelayUntil is 0. */

    /* Check rqtp. */
    if( UTILS_ValidateTimespec( rqtp ) == false )
    {
        iStatus = EINVAL;
    }

    /* Get current time */
    if( ( iStatus == 0 ) && ( clock_gettime( CLOCK_REALTIME, &xCurrentTime ) != 0 ) )
    {
        iStatus = EINVAL;
    }

    if( iStatus == 0 )
    {
        /* Check for absolute time sleep. */
        if( ( flags & TIMER_ABSTIME ) == TIMER_ABSTIME )
        {
            /* Get current time */
            if( clock_gettime( CLOCK_REALTIME, &xCurrentTime ) != 0 )
            {
                iStatus = EINVAL;
            }

            /* Get number of ticks until absolute time. */
            if( ( iStatus == 0 ) && ( UTILS_AbsoluteTimespecToDeltaTicks( rqtp, &xCurrentTime, &xSleepTime ) == 0 ) )
            {
                /* Delay until absolute time if vTaskDelayUntil is available. */
                #if ( INCLUDE_vTaskDelayUntil == 1 )

                    /* Get the current tick count. This variable isn't declared
                     * at the top of the function because it's only used and needed
                     * if vTaskDelayUntil is available. */
                    TickType_t xCurrentTicks = xTaskGetTickCount();

                    /* Delay until absolute time. */
                    vTaskDelayUntil( &xCurrentTicks, xSleepTime );
                #else

                    /* If vTaskDelayUntil isn't available, ignore the TIMER_ABSTIME flag
                     * and sleep for a relative time. */
                    vTaskDelay( xSleepTime );
                #endif
            }
        }
        else
        {
            /* If TIMER_ABSTIME isn't specified, convert rqtp to ticks and
             * sleep for a relative time. */
            if( UTILS_TimespecToTicks( rqtp, &xSleepTime ) == 0 )
            {
                vTaskDelay( xSleepTime );
            }
        }
    }

    return iStatus;
}

/*-----------------------------------------------------------*/

int nanosleep( const struct timespec * rqtp,
               struct timespec * rmtp )
{
    int iStatus = 0;
    TickType_t xSleepTime = 0;

    /* Silence warnings about unused parameters. */
    ( void ) rmtp;

    /* Check rqtp. */
    if( UTILS_ValidateTimespec( rqtp ) == false )
    {
        errno = EINVAL;
        iStatus = -1;
    }

    if( iStatus == 0 )
    {
        /* Convert rqtp to ticks and delay. */
        if( UTILS_TimespecToTicks( rqtp, &xSleepTime ) == 0 )
        {
            vTaskDelay( xSleepTime );
        }
    }

    return iStatus;
}

/*-----------------------------------------------------------*/
