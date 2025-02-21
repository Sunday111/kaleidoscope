#pragma once

#include <expected>
#include <span>
#include <string>
#include <string_view>
#include <vector>

enum class ProcessError : uint8_t
{
    FailedToCreatePipes,
    ForkFailed,
    ExitedAbnormally,
};

template <typename T, size_t extent = std::dynamic_extent>
[[nodiscard]] constexpr inline std::string_view SpanAsStringView(std::span<T, extent> span) noexcept
{
    return std::string_view(reinterpret_cast<const char*>(span.data()), span.size_bytes());  // NOLINT
}

class ProcessResult
{
public:
    std::vector<uint8_t> out;
    std::vector<uint8_t> err;
    int status = 0;
};

std::expected<ProcessResult, ProcessError> RunProcess(
    std::span<const std::string> command,
    std::string_view stdin_input = {});
