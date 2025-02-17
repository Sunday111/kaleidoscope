from pathlib import Path
import subprocess
import shutil


ROOT_DIR = Path(__file__).parent.resolve()
SRC_DIR = ROOT_DIR / "src"
LLVM_ROOT_DIR = Path("/home/sunday/llvm/llvm-install")
CLANG_C = LLVM_ROOT_DIR / "bin/clang"
GENERATED_IR_DIR = ROOT_DIR / "ir"


def emit_llvm_ir():
    shutil.rmtree(GENERATED_IR_DIR, ignore_errors=True)

    for abs_src_file_path in SRC_DIR.rglob("*.c"):
        out_file = (
            GENERATED_IR_DIR
            / abs_src_file_path.parent.relative_to(SRC_DIR)
            / f"{abs_src_file_path.name}.ir"
        )
        out_file.parent.mkdir(parents=True, exist_ok=True)

        subprocess.check_call(
            [
                CLANG_C,
                "-O0",
                "-DNDEBUG",
                "-emit-llvm",
                *("-S", abs_src_file_path),
                *("-o", out_file),
            ]
        )


def main():
    emit_llvm_ir()


if __name__ == "__main__":
    main()
