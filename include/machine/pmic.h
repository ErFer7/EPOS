// EPOS PWM Mediator Common package

#ifndef __pmic_h
#define __pmic_h

#include <system/config.h>

__BEGIN_SYS

class PMIC_Common
{
protected:
    PMIC_Common() {}

public:
    static unsigned int get_cpu_voltage();
    static void set_cpu_voltage();
};

__END_SYS

#endif

#if defined(__PMIC_H) && !defined(__pmic_common_only__)
#include __PMIC_H
#endif
