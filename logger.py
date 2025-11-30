import subprocess
import os
import signal
import sys

def main() -> None:
    APPLICATION = sys.argv[1]
    ITERATIONS = int(sys.argv[2])

    failed = 0
    sucess = 0

    print(f'[main::init] {ITERATIONS} iterations.')
    for i in range(ITERATIONS):
        lines = []
        error = False
        print(f'[main::iteration] {i}.')
        process = subprocess.Popen(
                ['make', f'APPLICATION={APPLICATION}', 'run'],
                  stdout=subprocess.PIPE,
                  stderr=subprocess.STDOUT,
                  text=True,
                  bufsize=1,
                  )
        try:
            for line in process.stdout:
                lines.append(line)
                if "Exception" in line:
                    error = True
                    failed += 1
                    print("[process::exception] killing...")
                    subprocess.run(['pkill', '-f', 'qemu'])
                if "*** EPOS is shutting down!" in line:
                    sucess += 1
                    print("[process::sucess] killing...")
                    subprocess.run(['pkill', '-f', 'qemu'])

            if error:
                with open(f"{i}.log", "w") as file:
                    file.writelines(lines)

        finally:
            if process.stdout:
                process.stdout.close()

    print(f"Sucess {sucess}.")
    print(f"Failed {failed}.")

if __name__ == '__main__':
    main()

