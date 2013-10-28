/*
 * This file is part of ALVAR, A Library for Virtual and Augmented Reality.
 *
 * Copyright 2007-2012 VTT Technical Research Centre of Finland
 * Copyright 2013      Sven Koehler, Hasso Plattner Institute, Potsdam
 *
 * Contact: VTT Augmented Reality Team <alvar.info@vtt.fi>
 *          <http://www.vtt.fi/multimedia/alvar.html>
 *
 * ALVAR is free software; you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2.1 of the License, or (at your option)
 * any later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with ALVAR; if not, see
 * <http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html>.
 */

#include "Timer_private.h"

#include <time.h>

#ifdef __MACH__

/*
 * Mac OS X has no clock_gettime or CLOCK_MONOTONIC.
 *
 * We use Mach functions as hinted by:
 *
 *      http://stackoverflow.com/questions/11680461/monotonic-clock-on-osx
 *
 * I'd like to receive feedback if this causes any issues at
 * Sven <dot> Koehler <att> student <dot> hpi.uni-potsdam.de
 */

#include <mach/clock.h>
#include <mach/mach.h>

#define clock_gettime mach_clock_gettime
#define CLOCK_MONOTONIC 0

static void mach_clock_gettime(int /* dummy */, timespec * tp)
{
	mach_timespec_t mts;
	clock_serv_t cclock;
	
	host_get_clock_service(mach_host_self(), SYSTEM_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
	tp->tv_nsec = mts.tv_nsec;
	tp->tv_sec = mts.tv_sec;
}

#endif


namespace alvar {

class TimerPrivateData
{
public:
    TimerPrivateData()
        : mStart()
    {
    }

    timespec mStart;
};

TimerPrivate::TimerPrivate()
    : d(new TimerPrivateData())
{
}

TimerPrivate::~TimerPrivate()
{
    delete d;
}

void TimerPrivate::start()
{
    clock_gettime(CLOCK_MONOTONIC, &d->mStart);
}

double TimerPrivate::stop()
{
    timespec stop;
    clock_gettime(CLOCK_MONOTONIC, &stop);
    return (stop.tv_sec - d->mStart.tv_sec) +
           (stop.tv_nsec - d->mStart.tv_nsec) / 1000000000.0;
}

} // namespace alvar
