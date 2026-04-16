#include <machine/riscv/visionfive2/visionfive2_hardware_clock.h>

__BEGIN_SYS

constexpr Hertz HardwareClock::FREQUENCY_LEVELS[];

unsigned int HardwareClock::_current_frequency_level = 3;

__END_SYS
