#include "run_process.hpp"

#include <sys/wait.h>

#include <array>

#include "unistd.h"

std::expected<ProcessResult, ProcessError> RunProcess(
    std::span<const std::string> command,
    std::string_view stdin_input)
{
    std::array<int, 2> stdout_pipe{}, stderr_pipe{}, stdin_pipe{};

    // Create pipes
    if (pipe(stdout_pipe.data()) == -1 || pipe(stderr_pipe.data()) == -1 || pipe(stdin_pipe.data()) == -1)
    {
        return std::unexpected(ProcessError::FailedToCreatePipes);
    }

    const pid_t pid = fork();
    if (pid == -1)
    {
        return std::unexpected(ProcessError::ForkFailed);
    }

    ProcessResult r;

    if (pid == 0)
    {
        // Child process
        close(stdout_pipe[0]);  // Close unused read end of stdout pipe
        close(stderr_pipe[0]);  // Close unused read end of stderr pipe
        close(stdin_pipe[1]);   // Close unused write end of stdin pipe

        dup2(stdout_pipe[1], STDOUT_FILENO);  // Redirect stdout to pipe
        dup2(stderr_pipe[1], STDERR_FILENO);  // Redirect stderr to pipe
        dup2(stdin_pipe[0], STDIN_FILENO);    // Redirect stdin from pipe

        close(stdout_pipe[1]);  // Close original pipe write end
        close(stderr_pipe[1]);
        close(stdin_pipe[0]);

        std::vector<char*> lp_args;
        lp_args.reserve(command.size() + 1);
        for (const auto& s : command) lp_args.push_back(const_cast<char*>(s.data()));  // NOLINT
        lp_args.push_back(nullptr);

        execvp(lp_args.front(), lp_args.data());

        // If exec fails, it will write to stderr
        perror("execlp failed");
        exit(1);
    }
    else
    {
        // Parent process
        close(stdout_pipe[1]);  // Close unused write end
        close(stderr_pipe[1]);
        close(stdin_pipe[0]);  // Close unused read end of stdin pipe

        // Write stdin data to the child process
        if (!stdin_input.empty())
        {
            write(stdin_pipe[1], stdin_input.data(), stdin_input.size());
        }
        close(stdin_pipe[1]);  // Close write end (EOF for child stdin)

        // Wait for child to finish
        int status = 0;
        waitpid(pid, &status, 0);  // NOLINT

        auto read_pipe = [](int pipe)
        {
            std::vector<uint8_t> out;
            ssize_t bytesRead = 0;
            std::array<uint8_t, 1024> tmp;  // NOLINT
            while ((bytesRead = read(pipe, tmp.data(), tmp.size())) > 0)
            {
                out.insert(out.end(), tmp.begin(), tmp.begin() + bytesRead);
            }
            return out;
        };

        r.out = read_pipe(stdout_pipe[0]);
        r.err = read_pipe(stderr_pipe[0]);

        close(stdout_pipe[0]);  // Close read ends
        close(stderr_pipe[0]);

        if (WIFEXITED(status))
        {
            r.status = WEXITSTATUS(status);
        }
        else
        {
            return std::unexpected(ProcessError::ForkFailed);
        }
    }

    return r;
}
