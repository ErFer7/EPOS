#pragma once

#include "architecture/cpu.h"
#include "machine/riscv/visionfive2/visionfive2_memory_map.h"
#include "machine/riscv/visionfive2/visionfive2_pmic.h"
#include "system/traits.h"

__BEGIN_SYS

extern OStream kout;

class HardwareClock {
    typedef CPU::Reg8 Reg8;
    typedef CPU::Reg16 Reg16;
    typedef CPU::Reg32 Reg32;
    typedef PMIC::Milivolts Milivolts;

   public:
    static const unsigned int MAX = 3;
    static const unsigned int MIN = 0;

    static constexpr Hertz FREQUENCY_LEVELS[] = {375000000, 500000000, 750000000, 1500000000};
   private:
    static const unsigned long sys_crg_base = Memory_Map::SYS_CRG_BASE;
    static const unsigned long aon_crg_base = Memory_Map::SYS_AON_BASE;
    static const unsigned long sys_con_base = Memory_Map::SYS_CON_BASE;

    static const Hertz osc = Traits<Machine>::OSC;

    static const Milivolts MAX_VOLTAGE = 1040;
    static const Milivolts MIN_VOLTAGE = 800;

    // Offsets
    enum SysCrg {
        CPU_ROOT = 0x000,
        CPU_DIV = 0x004,
        DDR_BUS = 0x0ac,
        GMAC5_AXI_64_AHB = 0x184,
        GMAC5_AXI_64_AXI = 0x188,
        GMAC_SOURCE = 0x18c,
        GMAC1_GTX = 0x190,
        GMAC1_RMII_RTX = 0x194,
        GMAC5_AXI_64_PTP = 0x198,
        GMAC5_AXI_64_RX = 0x19c,
        GMAC5_AXI_64_RX_INVERTER = 0x1a0,
        GMAC5_AXI64_TX = 0x1a4,
        GMAC5_AXI_64_TX_INVERTER = 0x1a8,
        GMAC1_GTXC = 0x1ac,
        GMAC0_GTX = 0x1b0,
        GMAC0_PTP = 0x1b4,
        GMAC_PHY = 0x1b8,
        GMAC0_GTXC = 0x1bc,
        TIMER_APB = 0x1f0,
        TIMER0 = 0x1f4,
        TIMER1 = 0x1f8,
        TIMER2 = 0x1fc,
        TIMER3 = 0x200,
        TEMPERATURE_SENSOR_APB = 0x204,
        TEMPERATURE_SENSOR = 0x208,
        RESET2_ADDRESS_SELECTOR = 0x300,
        RESET3_ADDRESS_SELECTOR = 0x304,
    };

    enum AonCrg {
        AHB_GMAC5_CLOCK = 0x08,
        AXI_GMAC5_CLOCK = 0x0c,
        GMAC0_RMII_RTX = 0x10,
        GMAC5_AXI64_CLOCK_TRANSMITTER = 0x14,
        GMAC5_AXI64_CLOCK_TRANSMISSION_INVERTER = 0x18,
        GMAC5_AXI64_CLOCK_RECEIVER = 0x1c,
        GMAC5_AXI64_CLOCK_RECEIVING_INVERTER = 0x20,
        RESET_ASSERT_ADDRESS_SELECTOR = 0x38,
    };

    enum SysSysCon {
        SAIF_SYSCFG_28 = 0x1c,
        SAIF_SYSCFG_32 = 0x20,
        SAIF_SYSCFG_36 = 0x24,
        SAIF_SYSCFG_40 = 0x28,
        SAIF_SYSCFG_44 = 0x2c,
        SAIF_SYSCFG_48 = 0x30,
        SAIF_SYSCFG_52 = 0x34,
    };

    // Bits
    enum ClockBit {
        CLOCK_ENABLE = 1 << 31,
    };

    enum ResetBit {
        TIMER_RSTN_APB = 1 << 21,
        TIMER_RSTN_TIMER0 = 1 << 22,
        TIMER_RSTN_TIMER1 = 1 << 23,
        TIMER_RSTN_TIMER2 = 1 << 24,
        TIMER_RSTN_TIMER3 = 1 << 25,
        TEMP_SENSOR_RSTN_APB = 1 << 27,
        TEMP_SENSOR_RSTN_TEMP = 1 << 28,
    };

   public:
    static void init() {
        enable();
        reset();
        set_cpu_clock_frequency_level(MAX, true);
        // reg_masked_write(sys_crg_base, DDR_BUS, 1, 24, 29);
    }

    static Hertz read_pll(unsigned int pll) {
        if (pll > 2)
            return 0;

        unsigned int pll_fbdiv_mask_low = pll == 0 ? 0 : 17;
        unsigned int pll_fbdiv_mask_high = pll_fbdiv_mask_low + 11;

        Reg32 pll_fbdiv = reg_masked_read(sys_con_base, SAIF_SYSCFG_28 + pll * 8, pll_fbdiv_mask_low, pll_fbdiv_mask_high);
        Reg32 saif_syscfg = reg(sys_con_base, SAIF_SYSCFG_32 + pll * 8);
        Reg32 pll_frac = masked(saif_syscfg, 0, 23);
        Reg32 pll_postdiv1_shift = masked(saif_syscfg, 28, 29);
        Reg32 pll_prediv = reg_masked_read(sys_con_base, SAIF_SYSCFG_36 + pll * 8, 0, 5);

        Reg32 pll_postdiv1 = 1 << pll_postdiv1_shift;
        Hertz osc_per_prediv = osc / pll_prediv;
        Hertz vco = (osc_per_prediv * pll_fbdiv) + ((osc_per_prediv * pll_frac) >> 24);

        return vco / pll_postdiv1;
    }

    static Hertz get_cpu_clock() {
        Reg8 clk_mux_sel = reg_masked_read(sys_crg_base, CPU_ROOT, 24, 29);

        Hertz base_clock = clk_mux_sel == 1U ? read_pll(0) : osc;

        Reg32 clk_cpu_div = reg_masked_read(sys_crg_base, CPU_DIV, 0, 23);

        return base_clock / clk_cpu_div;
    }

    static Hertz get_ddr_clock() {
        Reg8 clk_mux_sel = reg_masked_read(sys_crg_base, DDR_BUS, 24, 29);

        if (clk_mux_sel == 0) {
            return osc / 2;
        }

        return read_pll(1) >> clk_mux_sel;
    }

    static unsigned int get_cpu_clock_frequency_level() {
        return _current_frequency_level;
    }

    // Only multiples of the highest frequency are allowed
    static void set_cpu_clock_frequency_level(unsigned int frequency_level, bool force_vdd_change = false) {
        if (frequency_level == _current_frequency_level && !force_vdd_change) {
            return;
        }

        Hertz frequency = FREQUENCY_LEVELS[frequency_level];
        Reg8 div = FREQUENCY_LEVELS[3] / frequency;

        if (div == 0U) div = 1U;
        if (div > 7U) div = 7U;

        if (force_vdd_change) {
            if (frequency_level == MAX) {
                scale_up(frequency_level, div, MAX_VOLTAGE);

                return;
            }

            _current_frequency_level = frequency_level;
            scale_down(frequency_level, div, MIN_VOLTAGE);

            return;
        }

        if (frequency_level == MAX) {
            if (_current_frequency_level < MAX) {
                scale_up(frequency_level, div, MAX_VOLTAGE);

                return;
            }
        }

        if (_current_frequency_level == MAX) {
            scale_down(frequency_level, div, MIN_VOLTAGE);

            return;
        }

        reg(sys_crg_base, CPU_DIV) = div;

        _current_frequency_level = frequency_level;
    }

   private:
    static void enable() {
        for (int offset = GMAC5_AXI_64_AHB; offset <= GMAC0_GTXC; offset += 4) {
            reg(sys_crg_base, offset) |= CLOCK_ENABLE;
        }

        for (int offset = AHB_GMAC5_CLOCK; offset <= GMAC5_AXI64_CLOCK_RECEIVING_INVERTER; offset += 4) {
            reg(aon_crg_base, offset) |= CLOCK_ENABLE;
        }

        reg(sys_crg_base, TIMER_APB) |= CLOCK_ENABLE;
        reg(sys_crg_base, TIMER0) |= CLOCK_ENABLE;
        reg(sys_crg_base, TIMER1) |= CLOCK_ENABLE;
        reg(sys_crg_base, TIMER2) |= CLOCK_ENABLE;
        reg(sys_crg_base, TIMER3) |= CLOCK_ENABLE;

        reg(sys_crg_base, TEMPERATURE_SENSOR_APB) |= CLOCK_ENABLE;
        reg(sys_crg_base, TEMPERATURE_SENSOR) |= CLOCK_ENABLE;
    }

    static void reset() {
        reg(sys_crg_base, RESET3_ADDRESS_SELECTOR) |= TIMER_RSTN_APB;
        reg(sys_crg_base, RESET3_ADDRESS_SELECTOR) &= ~TIMER_RSTN_APB;

        reg(sys_crg_base, RESET3_ADDRESS_SELECTOR) |= TIMER_RSTN_TIMER0;
        reg(sys_crg_base, RESET3_ADDRESS_SELECTOR) &= ~TIMER_RSTN_TIMER0;

        reg(sys_crg_base, RESET3_ADDRESS_SELECTOR) |= TIMER_RSTN_TIMER1;
        reg(sys_crg_base, RESET3_ADDRESS_SELECTOR) &= ~TIMER_RSTN_TIMER1;

        reg(sys_crg_base, RESET3_ADDRESS_SELECTOR) |= TIMER_RSTN_TIMER2;
        reg(sys_crg_base, RESET3_ADDRESS_SELECTOR) &= ~TIMER_RSTN_TIMER2;

        reg(sys_crg_base, RESET3_ADDRESS_SELECTOR) |= TIMER_RSTN_TIMER3;
        reg(sys_crg_base, RESET3_ADDRESS_SELECTOR) &= ~TIMER_RSTN_TIMER3;

        reg(sys_crg_base, RESET3_ADDRESS_SELECTOR) |= TEMP_SENSOR_RSTN_APB;
        reg(sys_crg_base, RESET3_ADDRESS_SELECTOR) &= ~TEMP_SENSOR_RSTN_APB;

        reg(sys_crg_base, RESET3_ADDRESS_SELECTOR) |= TEMP_SENSOR_RSTN_TEMP;
        reg(sys_crg_base, RESET3_ADDRESS_SELECTOR) &= ~TEMP_SENSOR_RSTN_TEMP;

        // reg(sys_crg_base, RESET2_ADDRESS_SELECTOR) |= 0xC;
        // reg(sys_crg_base, RESET2_ADDRESS_SELECTOR) &= ~0xC;

        // reg(aon_crg_base, RESET_ASSERT_ADDRESS_SELECTOR) |= 0x3;
        // reg(aon_crg_base, RESET_ASSERT_ADDRESS_SELECTOR) &= ~0x3;
    }

    static volatile Reg32 &reg(unsigned long base, unsigned int offset) {
        return reinterpret_cast<volatile Reg32 *>(base)[offset / sizeof(Reg32)];
    }

    static constexpr Reg32 mask(unsigned int mask_low = 0, unsigned int mask_high = 31) {
        return ((1ULL << (mask_high - mask_low + 1)) - 1) << mask_low;
    }

    static inline Reg32 masked(Reg32 value, unsigned int mask_low = 0, unsigned int mask_high = 31) {
        return (value & mask(mask_low, mask_high)) >> mask_low;
    }

    // TODO: Use reg_read_masked
    static volatile Reg32 reg_masked_read(unsigned long base,
                                          unsigned int offset,
                                          unsigned int mask_low = 0,
                                          unsigned int mask_high = 31) {
        volatile Reg32 *address = &reinterpret_cast<volatile Reg32 *>(base)[offset / sizeof(Reg32)];

        return masked(*address, mask_low, mask_high);
    }

    static void reg_masked_write(unsigned long base,
                                 unsigned int offset,
                                 Reg32 value,
                                 unsigned int mask_low = 0,
                                 unsigned int mask_high = 31) {
        const unsigned int mask_value = mask(mask_low, mask_high);

        volatile Reg32 *address = &reinterpret_cast<volatile Reg32 *>(base)[offset / sizeof(Reg32)];

        Reg32 current = *address;
        current &= ~mask_value;
        current |= (value << mask_low) & mask_value;
        *address = current;
    }

    inline static void scale_up(unsigned int i, Reg8 div, PMIC::Milivolts voltage) {
        PMIC::set_cpu_voltage(voltage);
        reg_masked_write(sys_crg_base, CPU_DIV, div, 0, 23);

        _current_frequency_level = i;
    }

    inline static void scale_down(unsigned int i, Reg8 div, PMIC::Milivolts voltage) {
        reg_masked_write(sys_crg_base, CPU_DIV, div, 0, 23);
        PMIC::set_cpu_voltage(voltage);

        _current_frequency_level = i;
    }

   private:
    static unsigned int _current_frequency_level;
};

__END_SYS
