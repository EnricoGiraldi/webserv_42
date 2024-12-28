#include "cgi_handler.hpp"
#include <unistd.h>
#include <sstream>
#include <stdexcept>
#include <cstring>
#include <sys/wait.h>

CGIHandler::CGIHandler() {}

CGIHandler::~CGIHandler() {}

std::string CGIHandler::executeCGI(const std::string& scriptPath, const std::string& queryString,
                                   const std::map<std::string, std::string>& env) {
    int pipeIn[2];
    int pipeOut[2];
    char buffer[1024];
    std::string output;

    if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1) {
        throw std::runtime_error("Failed to create pipes.");
    }

    pid_t pid = fork();
    if (pid < 0) {
        throw std::runtime_error("Failed to fork process.");
    }

    if (pid == 0) {
        close(pipeIn[1]);
        close(pipeOut[0]);

        dup2(pipeIn[0], STDIN_FILENO);
        dup2(pipeOut[1], STDOUT_FILENO);

        close(pipeIn[0]);
        close(pipeOut[1]);

        // Set up environment variables
        char** envp = new char*[env.size() + 1];
        int i = 0;
        for (std::map<std::string, std::string>::const_iterator it = env.begin(); it != env.end(); ++it) {
            std::string envVar = it->first + "=" + it->second;
            envp[i] = new char[envVar.size() + 1];
            std::strcpy(envp[i], envVar.c_str());
            i++;
        }
        envp[i] = NULL;

        execl(scriptPath.c_str(), scriptPath.c_str(), NULL);
        _exit(1); // If execl fails
    } else {
        close(pipeIn[0]);
        close(pipeOut[1]);

        write(pipeIn[1], queryString.c_str(), queryString.size());
        close(pipeIn[1]);

        ssize_t bytesRead;
        while ((bytesRead = read(pipeOut[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            output += buffer;
        }

        close(pipeOut[0]);
        waitpid(pid, NULL, 0);
    }

    return output;
}
