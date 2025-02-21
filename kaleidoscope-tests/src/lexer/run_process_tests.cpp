#include "gtest/gtest.h"
#include "run_process.hpp"

using namespace std::literals;

TEST(RunProcessTests, SuccessfullLS)
{
    constexpr std::array command{
        "ls"s,
        "-l"s,
        "/usr"s,
    };
    auto r = RunProcess(command);
    ASSERT_TRUE(r.has_value());
    ASSERT_EQ(r->status, 0);
    ASSERT_FALSE(r->out.empty());
    ASSERT_TRUE(r->err.empty());
}

TEST(RunProcessTests, FailedLS)
{
    constexpr std::array command{
        "ls"s,
        "-l"s,
        "/nonexistent_folder"s,
    };
    auto r = RunProcess(command);
    ASSERT_TRUE(r.has_value());
    ASSERT_EQ(r->status, 2);
    ASSERT_FALSE(r->err.empty());
}

[[nodiscard]] std::expected<ProcessResult, ProcessError> RunIR(std::string_view input)
{
    constexpr std::array run_ir_command{"lli-18"s};
    return RunProcess(run_ir_command, input);
}

[[nodiscard]] std::expected<ProcessResult, ProcessError> CToIR(std::string_view input)
{
    constexpr std::array command{
        "clang-18"s,
        "-O3"s,
        "-x"s,
        "c"s,           // Treat input as C source
        "-std=c23"s,    // Specify C version
        "-emit-llvm"s,  // Emit LLVM IR
        "-S"s,          // Output human-readable LLVM IR (instead of bitcode)
        "-o"s,
        "-"s,  // Write output to stdout instead of a file
        "-"s   // Read source code from stdin
    };
    return RunProcess(command, input);
}

[[nodiscard]] std::expected<ProcessResult, ProcessError> CppToIR(std::string_view input)
{
    constexpr std::array command{
        "clang++-18"s,
        "-O3"s,
        "-x"s,
        "c++"s,         // Treat input as C++ source
        "-std=c++20"s,  // Specify C++ version
        "-emit-llvm"s,  // Emit LLVM IR
        "-S"s,          // Output human-readable LLVM IR (instead of bitcode)
        "-o"s,
        "-"s,  // Write output to stdout instead of a file
        "-"s   // Read source code from stdin
    };
    return RunProcess(command, input);
}

TEST(RunProcessTests, CompileCppHelloWorld)
{
    constexpr std::string_view hello_world_code = R"(
        #include <iostream>
        int main() {
            std::cout << "Hello, World!\n";
            return 0;
        }
    )";

    auto proc_out = CppToIR(hello_world_code);
    ASSERT_TRUE(proc_out.has_value());

    ASSERT_EQ(proc_out->status, 0);

    const auto ir_data = std::move(proc_out->out);
    const auto ir_text = SpanAsStringView(std::span{ir_data});

    std::println("LLVM IR:\n{}", ir_text);

    proc_out = RunIR(ir_text);
    ASSERT_TRUE(proc_out.has_value());
    ASSERT_EQ(proc_out->status, 0);
    std::println("IR stdout: {}", SpanAsStringView(std::span{proc_out->out}));
}

TEST(RunProcessTests, CompileCHelloWorld)
{
    constexpr std::string_view hello_world_code = R"(
        #include "stdio.h"
        int main() {
            printf("Hello, world!\n");
            return 0;
        }
    )";

    auto proc_out = CToIR(hello_world_code);
    ASSERT_TRUE(proc_out.has_value());

    if (proc_out->status != 0)
    {
        std::println("stderr: {}", SpanAsStringView(std::span{proc_out->err}));
    }
    ASSERT_EQ(proc_out->status, 0);

    const auto ir_data = std::move(proc_out->out);
    const auto ir_text = SpanAsStringView(std::span{ir_data});

    std::println("LLVM IR:\n{}", ir_text);

    proc_out = RunIR(ir_text);
    ASSERT_TRUE(proc_out.has_value());
    ASSERT_EQ(proc_out->status, 0);
    std::println("IR stdout: {}", SpanAsStringView(std::span{proc_out->out}));
}

constexpr void Patch(std::string& s, std::string_view what, std::string_view with)
{
    for (size_t pos = s.find(what); pos != std::string::npos; pos = s.find(what, pos))
    {
        auto begin = std::next(s.begin(), static_cast<ssize_t>(pos));
        auto end = std::next(begin, std::ssize(what));
        s.replace_with_range(begin, end, with);
    }
}

struct IRExpressionExecutor
{
    inline static constexpr std::string_view ir_text_template = R"(
        declare i32 @printf(i8*, ...)

        @format = private constant [${format-length} x i8] c"${format}"

        define i32 @main() {
            ${expression}
            %fmt = getelementptr inbounds [${format-length} x i8], [${format-length} x i8]* @format, i32 0, i32 0
            call i32 (i8*, ...) @printf(i8* %fmt, ${variable-type} ${variable-name})
            ret i32 0
        }
    )";

    struct Params
    {
        std::string_view expr;
        std::string_view type;
        std::string_view var_name;
        std::string_view format;
        std::string_view format_length;
    };

    [[nodiscard]] static std::expected<ProcessResult, ProcessError> Exec(const Params& params)
    {
        std::string ir_text{ir_text_template};
        Patch(ir_text, "${expression}", params.expr);
        Patch(ir_text, "${variable-type}", params.type);
        Patch(ir_text, "${variable-name}", params.var_name);
        Patch(ir_text, "${format}", params.format);
        Patch(ir_text, "${format-length}", params.format_length);

        return RunIR(ir_text);
    }

    [[nodiscard]] static std::expected<ProcessResult, ProcessError> ExecI32(
        std::string_view expression,
        std::string_view variable_name)
    {
        return Exec({
            .expr = expression,
            .type = "i32",
            .var_name = variable_name,
            .format = R"(%d\00)",
            .format_length = "3",
        });
    }
};

TEST(RunProcessTests, WIP)
{
    auto proc_out = IRExpressionExecutor::ExecI32(
        R"(
        %num_ptr = alloca i32, align 4
        store i32 42, ptr %num_ptr, align 4
        %num = load i32, ptr %num_ptr, align 4
    )",
        R"(%num)");

    ASSERT_TRUE(proc_out.has_value());
    ASSERT_EQ(proc_out->status, 0) << SpanAsStringView(std::span{proc_out->err});
    std::println("IR stdout: {}", SpanAsStringView(std::span{proc_out->out}));
}
