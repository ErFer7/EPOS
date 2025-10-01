"""
Parse QEMU logs. (Only for cpu and int options)
"""

from sys import argv

CORE_COUNT = 4


def main(filepath: str) -> None:
    """
    Parse.
    """

    lines = []

    with open(filepath, 'r', encoding='utf-8') as file:
        lines = file.readlines()

    registers = {}
    last_registers_per_core = {i: {} for i in range(CORE_COUNT)}
    sanitized_log = ''
    sanitized_log_per_core = {i: '' for i in range(CORE_COUNT)}

    for i, line in enumerate(lines):
        print(f'Line {i}/{len(lines)}')

        if line.startswith('riscv'):
            entry = f'{line}\n'
            sanitized_log += entry

            core = int(line.split()[1].split(':')[1][:-1]) - 1

            if core > -1:
                sanitized_log_per_core[core] += entry
        else:
            content = line.split()

            if len(content) == 2:
                if content[0] == 'pc' and 'pc' in registers:
                    core = int(registers['mhartid']) - 1

                    if core > -1:
                        sanitized_log += f'Core {core}:\n'

                        for register, value in registers.items():
                            if register not in last_registers_per_core[core]:
                                entry = f'{register} = {value}\n'
                                sanitized_log += entry
                                sanitized_log_per_core[core] += entry
                            elif value != last_registers_per_core[core][register]:
                                entry = f'{register} = {last_registers_per_core[core][register]} -> {value}\n'
                                sanitized_log += entry
                                sanitized_log_per_core[core] += entry

                        last_registers_per_core[core] = registers.copy()
                        sanitized_log += '\n'
                        sanitized_log_per_core[core] += '\n'

                    registers = {}

                registers[content[0]] = content[1]
            else:
                for i in range(0, len(content), 2):
                    registers[content[i]] = content[i + 1]

    with open('sanitized_log.log', 'w+', encoding='utf-8') as file:
        file.write(sanitized_log)

    for core, log in sanitized_log_per_core.items():
        with open(f'sanitized_log_{core}.log', 'w+', encoding='utf-8') as file:
            file.write(log)

if __name__ == '__main__':
    main(argv[1])
