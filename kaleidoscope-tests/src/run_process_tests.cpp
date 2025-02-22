#include "gtest/gtest.h"
#include "ir_expression_executor.hpp"
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

TEST(RunProcessTests, ExecuteExpressionI32)
{
    auto eval_result = IRExpressionExecutor::ExecI32(
        R"(
        %num_ptr = alloca i32, align 4
        store i32 42, ptr %num_ptr, align 4
        %num = load i32, ptr %num_ptr, align 4
    )",
        R"(%num)");

    ASSERT_TRUE(eval_result.has_value());
    ASSERT_EQ(eval_result.value(), 42);
}
