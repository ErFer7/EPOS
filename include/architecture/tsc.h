// EPOS TSC Mediator Common Package

#ifndef __tsc_h
#define __tsc_h

#include <system/config.h>

__BEGIN_SYS

class TSC_Common
{
public:
    typedef unsigned long long Time_Stamp;

protected:
    TSC_Common() {}

public:
    static Microsecond period(Hertz frequency) { return Microsecond(1000000) / Microsecond(frequency); }
    static Microsecond time(Time_Stamp ts, Hertz frequency) { return Microsecond(ts) * period(frequency); }
    static Time_Stamp ts(Microsecond time, Hertz frequency) { return (time + period(frequency) / 2) / period(frequency); }
};

__END_SYS

#endif

#if defined(__TSC_H) && !defined(__tsc_common_only__)
#include __TSC_H
#endif
