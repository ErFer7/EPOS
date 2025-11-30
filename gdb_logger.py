"""
Saves every run.
"""

from subprocess import TimeoutExpired, run, DEVNULL
from os.path import join, exists
from os import makedirs
from re import compile, DOTALL
from sys import argv

LOG_DIRECTORY = "logs"
TIMEOUT = 60  # Seconds


def handle_failure(iteration: int) -> None:
    print("Status: Failed.")

    current_log_directory = f"log_{iteration}"
    makedirs(join(LOG_DIRECTORY, current_log_directory), exist_ok=True)

    run(
        [
            "cp",
            join("img", "*.out"),
            join("img", "gdb.txt"),  # WARN: Unsafe!
            join(LOG_DIRECTORY, current_log_directory),
        ],
        stdout=DEVNULL,
        check=False,
    )


def main(application: str, iterations: str | int) -> None:
    iterations = int(iterations)

    makedirs(LOG_DIRECTORY, exist_ok=True)

    run(["make", "veryclean"], stdout=DEVNULL, check=False)

    print("Beginning tests...")

    passed = 0

    # Checks both if it has failed or is missing an OK (just to be sure)
    status_pattern = compile("^((?=.*<FAILED>)|(?!.*<OK>)).*$", DOTALL)

    for i in range(iterations):
        print(f"Running iteration {i + 1}/{iterations}.")

        try:
            run(
                ["make", f"APPLICATION={application}", "debug"],
                stdout=DEVNULL,
                check=False,
                timeout=TIMEOUT,
            )
        except TimeoutExpired:
            run(["pkill", "gdb"], stdout=DEVNULL, check=False)
            run(["pkill", "qemu-system"], stdout=DEVNULL, check=False)

            handle_failure(i)
            continue

        gdb_log_path = join("img", "gdb.txt")

        if not exists(gdb_log_path):
            handle_failure(i)
            continue

        with open(gdb_log_path, "r", encoding="utf-8") as file:
            gdb_log = file.read()

        if status_pattern.match(gdb_log):
            handle_failure(i)
        else:
            passed += 1
            print("Status: OK.")

    print("Results:")
    print(f"OK: {passed}/{iterations} ({passed / iterations:.2%})")


if __name__ == "__main__":
    main(*argv[1:])
