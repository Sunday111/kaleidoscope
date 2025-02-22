#pragma once

#include <format>
#include <string_view>
#include <variant>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#include "fast_float/fast_float.h"  // IWYU pragma: keep
#pragma clang diagnostic pop

#include "magic_enum/magic_enum.hpp"
#include "run_process.hpp"

[[nodiscard]] inline std::expected<ProcessResult, ProcessError> RunIR(std::string_view input)
{
    using namespace std::literals;
    constexpr std::array run_ir_command{"lli-18"s};
    return RunProcess(run_ir_command, input);
}

[[nodiscard]] inline std::expected<ProcessResult, ProcessError> CToIR(std::string_view input)
{
    using namespace std::literals;
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

[[nodiscard]] inline std::expected<ProcessResult, ProcessError> CppToIR(std::string_view input)
{
    using namespace std::literals;
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

inline constexpr void Patch(std::string& s, std::string_view what, std::string_view with)
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

    using ExprEvalError = std::variant<ProcessError, std::string>;

    [[nodiscard]] static std::expected<int32_t, ExprEvalError> ExecI32(
        std::string_view expression,
        std::string_view variable_name)
    {
        auto proc_result = Exec({
            .expr = expression,
            .type = "i32",
            .var_name = variable_name,
            .format = R"(%d\00)",
            .format_length = "3",
        });

        if (!proc_result.has_value()) return std::unexpected(proc_result.error());

        const ProcessResult& r = proc_result.value();
        if (r.status != 0)
        {
            return std::unexpected(
                std::format(
                    "lli command failed with exit code {}."
                    "Stderr:\n{}",
                    r.status,
                    SpanAsStringView(std::span{r.err})));
        }

        const auto stdout_view = SpanAsStringView(std::span{r.out});
        char* begin = const_cast<char*>(stdout_view.data());        // NOLINT
        char* end = const_cast<char*>(begin + stdout_view.size());  // NOLINT

        int32_t v = 0;
        auto parse_result = fast_float::from_chars(begin, end, v, 10);
        if (parse_result.ec != std::errc())
        {
            return std::unexpected(
                std::format(
                    "Parsing error {} while trying to evaluate expression evaluation stdout.\n"
                    "Expression evaluation stdout:\n{}",
                    magic_enum::enum_name(parse_result.ec),
                    stdout_view));
        }

        return v;
    }
};
