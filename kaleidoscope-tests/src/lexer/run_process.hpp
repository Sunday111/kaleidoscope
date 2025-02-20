#pragma once

#include <expected>
#include <span>
#include <string>
#include <vector>

enum class ProcessError : uint8_t
{
    FailedToCreatePipes,
    ForkFailed,
    ExitedAbnormally,
};

struct ProcessResult
{
    std::vector<uint8_t> out;
    std::vector<uint8_t> err;
    int status = 0;
};

std::expected<ProcessResult, ProcessError> RunProcess(std::span<const std::string> command);
