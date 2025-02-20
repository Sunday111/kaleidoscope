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
