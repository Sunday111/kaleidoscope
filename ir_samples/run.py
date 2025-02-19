from pathlib import Path
import subprocess
import shutil
from typing import Callable


ROOT_DIR = Path(__file__).parent.resolve()
SRC_DIR = ROOT_DIR / "src"
LLVM_ROOT_DIR = Path("/home/sunday/llvm/llvm-install")
CLANG_C = LLVM_ROOT_DIR / "bin/clang"
CLANG_CPP = LLVM_ROOT_DIR / "bin/clang++"
LLVM_LLC = LLVM_ROOT_DIR / "bin/llc"
LLVM_OPT = LLVM_ROOT_DIR / "bin/opt"
GENERATED_ROOT_DIR = ROOT_DIR / "generated"
GENERATED_IR_ROOT_DIR = GENERATED_ROOT_DIR / "IR"
GENERATED_ASM_ROOT_DIR = GENERATED_ROOT_DIR / "ASM"
GENERATED_OBJ_ROOT_DIR = GENERATED_ROOT_DIR / "OBJ"
COMPILED_ASM_ROOT_DIR = GENERATED_ROOT_DIR / "COMPILED"


def c_to_ir(source: Path, dest: Path, level: int):
    subprocess.check_call(
        [
            CLANG_C,
            "-std=c23",
            f"-O{level}",
            "-DNDEBUG",
            "-emit-llvm",
            *("-S", source),
            *("-o", dest),
        ]
    )


def cpp_to_ir(source: Path, dest: Path, level: int):
    subprocess.check_call(
        [
            CLANG_CPP,
            "-std=c++23",
            "-stdlib=libc++",
            f"-O{level}",
            "-DNDEBUG",
            "-emit-llvm",
            *("-S", source),
            *("-o", dest),
        ]
    )


def ir_to_asm(source: Path, dest: Path, level: int):
    subprocess.check_call(
        [
            CLANG_C,
            f"-O{level}",
            *("-S", source),
            *("-o", dest),
        ]
    )


def optimize_ir(source: Path, dest: Path, level: int):
    assert level >= 0
    assert level < 4
    subprocess.check_call(
        [
            LLVM_OPT,
            f"-O{level}",
            # f"-passes=default<O{level}>,loop-unroll,slp-vectorizer",
            *("-S", source),
            *("-o", dest),
        ]
    )


def compile_asm(source: Path, dest: Path, _: int):
    subprocess.check_call(
        [
            CLANG_CPP if ".cpp" in source.name else CLANG_C,
            *("-x", "assembler"),
            "-fPIC",
            *("-c", source),
            *("-o", dest),
        ]
    )


def link_file(source: Path, dest: Path, _: int):
    subprocess.check_call(
        [
            CLANG_CPP if ".cpp" in source.name else CLANG_C,
            "-fPIC",
            source,
            *("-o", dest),
        ]
    )
    subprocess.check_call(["chmod", "+x", dest])


def get_ir_dir(level: int) -> Path:
    return GENERATED_IR_ROOT_DIR / str(level)


def get_asm_dir(level: int) -> Path:
    return GENERATED_ASM_ROOT_DIR / str(level)


def get_obj_dir(level: int) -> Path:
    return GENERATED_OBJ_ROOT_DIR / str(level)


def get_program_dir(level: int) -> Path:
    return COMPILED_ASM_ROOT_DIR / str(level)


def transform_files(
    get_src_dir: Callable[[int], Path],
    src_ext: str,
    get_dst_dir: Callable[[int], Path],
    dst_ext: str,
    transformer: Callable[[str, str, int], None],
):
    src_ext = f"*{src_ext}"
    for level in range(4):
        src_dir = get_src_dir(level)
        dst_dir = get_dst_dir(level)
        for src_file in src_dir.rglob(src_ext):
            ll_file_path = (
                dst_dir
                / src_file.parent.relative_to(src_dir)
                / f"{src_file.name}{dst_ext}"
            )
            ll_file_path.parent.mkdir(parents=True, exist_ok=True)
            assert not ll_file_path.exists()
            transformer(src_file, ll_file_path, level)


def emit_llvm_ir():
    def transform(
        src_pattern: str,
        dst_ext: str,
        transformer: Callable[[str, str, int], None],
    ):
        transform_files(
            get_src_dir=lambda _: SRC_DIR,
            get_dst_dir=get_ir_dir,
            src_ext=src_pattern,
            dst_ext=dst_ext,
            transformer=transformer,
        )

    transform(src_pattern=".c", dst_ext=".ll", transformer=c_to_ir)
    transform(src_pattern=".cpp", dst_ext=".ll", transformer=cpp_to_ir)
    transform(src_pattern=".ll", dst_ext=".ll", transformer=optimize_ir)


def emit_asm():
    transform_files(
        get_src_dir=get_ir_dir,
        src_ext=".ll",
        get_dst_dir=get_asm_dir,
        dst_ext=".s",
        transformer=ir_to_asm,
    )


def emit_obj():
    transform_files(
        get_src_dir=get_asm_dir,
        src_ext=".s",
        get_dst_dir=get_obj_dir,
        dst_ext=".o",
        transformer=compile_asm,
    )


def emit_programs():
    transform_files(
        get_src_dir=get_obj_dir,
        src_ext=".o",
        get_dst_dir=get_program_dir,
        dst_ext="",
        transformer=link_file,
    )


# def emit_programs():
#     transform_files(
#         get_src_dir=get_asm_dir,
#         src_ext=".s",
#         get_dst_dir=get_program_dir,
#         dst_ext="",
#         transformer=link_file,
#     )


def main():
    shutil.rmtree(GENERATED_ROOT_DIR, ignore_errors=True)
    emit_llvm_ir()
    emit_asm()
    emit_obj()
    emit_programs()


if __name__ == "__main__":
    main()
