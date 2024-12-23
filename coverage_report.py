import subprocess
from pathlib import Path
import os
import shutil

ROOT_DIR = Path(__file__).parent.resolve()
LLVM = Path("/home/sunday/llvm/llvm-install/bin")
COV_REPORT_DIR = ROOT_DIR / "cov-report"
BUILD_DIR = ROOT_DIR / "build"
BIN_DIR = BUILD_DIR / "bin"
SRC_DIR = ROOT_DIR / "kaleidoscope"


def main():

    executables: list[Path] = [BIN_DIR / "kaleidoscope-tests"]

    subprocess.check_call(["cmake", "--build", BUILD_DIR])

    for executable in executables:
        profraw = executable.parent / f"{executable.name}.profraw"
        subprocess.check_call(
            args=[executable],
            env={**os.environ, "LLVM_PROFILE_FILE": profraw.as_posix()},
        )

        profdata = executable.parent / f"{executable.name}.profdata"
        subprocess.check_call(
            [
                LLVM / "llvm-profdata",
                "merge",
                "-sparse",
                profraw,
                *("-o", profdata),
            ]
        )

        report_dir = COV_REPORT_DIR / executable.name
        shutil.rmtree(report_dir, ignore_errors=True)
        report_dir.mkdir(parents=True)

        subprocess.check_call(
            [
                LLVM / "llvm-cov",
                *("show", executable),
                f"-instr-profile={profdata}",
                *("-output-dir", report_dir),
                "-format=html",
                "--show-branch-summary",
                "--show-instantiation-summary",
                "--show-mcdc-summary",
                "--show-region-summary",
                SRC_DIR,
            ]
        )


if __name__ == "__main__":
    main()
