// EPOS VisionFive 2 (RISC-V) I2C Mediator Declarations

#pragma once

#include <architecture/cpu.h>
#include <machine/i2c.h>
#include <machine/pmic.h>

__BEGIN_SYS

extern OStream kout;

class PMIC : public PMIC_Common {
   public:
    typedef unsigned int Milivolts;

   private:
    typedef CPU::Reg32 Reg32;
    typedef CPU::Reg8 Reg8;

    static const Milivolts MIN_CPU_VOLTAGE = 800;
    static const Milivolts MAX_CPU_VOLTAGE = 1040;

   public:
    static const unsigned char PMIC_I2C_ADDR = 0x36;

    enum {
        REG_CHIP_ID = 0x03,
        REG_OUTPUT_CTRL1 = 0x10,   // DCDC1/2/3/4/5/6 Enable
        REG_DCDC2_VOLTAGE = 0x14,  // DCDC2 (CPU) Voltage Control
        REG_PWRON_STATUS = 0x20,
    };

   public:
    PMIC() {}

    // The most recent versions of U-boot already configure the GPIO pins in IOMUX for the PMIC connection with I2C
    static void init() {}

    static Milivolts get_cpu_voltage() {
        const char reg = REG_DCDC2_VOLTAGE;
        char val = 0;

        if (!I2C::write(PMIC_I2C_ADDR, &reg, 1, false)) {
            return 0;
        }

        if (!I2C::read(PMIC_I2C_ADDR, &val, 1, true)) {
            return 0;
        }

        unsigned int reg_val = val & 0x7F;

        if (reg_val < 71) {
            return 500 + (reg_val * 10);
        } else {
            return 1220 + ((reg_val - 71) * 20);
        }
    }

    static void set_cpu_voltage(Milivolts voltage) {
        if (voltage < MIN_CPU_VOLTAGE) voltage = MIN_CPU_VOLTAGE;
        if (voltage > MAX_CPU_VOLTAGE) voltage = MAX_CPU_VOLTAGE;

        unsigned char reg_val = 0;

        if (voltage <= 1210) {
            reg_val = (voltage - 500) / 10;
        } else {
            reg_val = 71 + ((voltage - 1220) / 20);
        }

        char payload[2];

        payload[0] = REG_DCDC2_VOLTAGE;
        payload[1] = reg_val;

        I2C::write(PMIC_I2C_ADDR, payload, 2, true);
        TSC::usleep(2);
    }

   private:
    static volatile Reg32 &reg_sys_crg(unsigned int o) {
        return reinterpret_cast<volatile Reg32 *>(0x13020000)[o / sizeof(Reg32)];
    }
};

__END_SYS
