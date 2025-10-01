"""
Saves every run.
"""

from subprocess import run, DEVNULL
from os.path import join
from re import findall

ITERATIONS = 100
SAVING_DIRECTORY = join('..', 'EPOS-Debug', 'outputs', 'Multicore+Multitask@experimental')
APPLICATION_ENDINGS = {'philosophers_dinner': 'The end!', 'multicore': 'Test result: OK'}
APPLICATION = 'multicore'


def handle_command_return_code(return_code: int) -> None:
    """
    Handles the return code for a command.
    """

    if return_code != 0:
        print(f"Make command failed with return code {result.returncode}")
        print(f"Error: {result.stderr}")


print(f'Beginning automatic logging with {ITERATIONS} iterations.')

ok_count = 0
failed_count = 0
exception_count = {}

for i in range(ITERATIONS):
    try:
        print(f'Running iteration {i + 1}/{ITERATIONS}.')

        result = run(['make', f'APPLICATION={APPLICATION}', 'run'], stdout=DEVNULL, check=True)
        handle_command_return_code(result.returncode)

        output = ''

        with open(join('img', f'{APPLICATION}.out'), 'r', encoding='utf-8') as output_file:
            output = output_file.read()

        exceptions = findall(r'Exception\((\d+)\)', output)

        failed = APPLICATION_ENDINGS[APPLICATION] not in output or len(exceptions) != 0

        for exception in exceptions:
            print(f'Status: Has exception {exception}.')

            if exception not in exception_count:
                exception_count[exception] = 1
            else:
                exception_count[exception] += 1

        if failed:
            print('Status: Failed.')
            failed_count += 1
            result = run(['cp',
                          join('img', f'{APPLICATION}.out'),
                          join(SAVING_DIRECTORY, f'{APPLICATION}_{i}.txt')], stdout=DEVNULL, check=True)
            handle_command_return_code(result.returncode)
        else:
            print('Status: OK.')
            ok_count += 1
    except KeyboardInterrupt:
        print('Stopping...')

print('Results:')
print(f'OK: {ok_count}/{ITERATIONS} ({ok_count / ITERATIONS:.2%})')
print(f'Failed: {failed_count}/{ITERATIONS} ({failed_count / ITERATIONS:.2%})')

for exception, count in exception_count.items():
    print(f'Exception {exception}: {count}')
