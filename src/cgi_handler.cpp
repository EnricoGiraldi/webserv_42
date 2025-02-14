#include "CGIHandler.hpp"
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <map>

// Helper: converte la mappa di environment in un array di char* (null-terminated)
static char** convertEnvMap(const std::map<std::string, std::string>& envMap) {
    char** envp = new char*[envMap.size() + 1];
    size_t i = 0;
    for (std::map<std::string, std::string>::const_iterator it = envMap.begin(); it != envMap.end(); ++it) {
        std::string envEntry = it->first + "=" + it->second;
        envp[i] = new char[envEntry.size() + 1];
        std::strcpy(envp[i], envEntry.c_str());
        ++i;
    }
    envp[i] = NULL;
    return envp;
}

// Helper: libera l'array di environment
static void freeEnvArray(char** envp) {
    if (!envp) return;
    for (size_t i = 0; envp[i] != NULL; ++i) {
        delete[] envp[i];
    }
    delete[] envp;
}

CGIHandler::CGIHandler() {
    // eventuali inizializzazioni
}

CGIHandler::~CGIHandler() {
    // eventuale pulizia
}

std::string CGIHandler::executeCGI(const std::string& scriptPath, const std::string& queryString,
                                     const std::map<std::string, std::string>& env)
{
    int pipeIn[2];
    int pipeOut[2];
    pid_t pid;
    std::string output;

    if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1) {
        return "";
    }

    pid = fork();
    if (pid < 0) {
        return "";
    } else if (pid == 0) {
        // Processo figlio
        dup2(pipeIn[0], STDIN_FILENO);
        dup2(pipeOut[1], STDOUT_FILENO);
        close(pipeIn[1]);
        close(pipeOut[0]);

        char *args[] = { const_cast<char*>(scriptPath.c_str()), NULL };

        // Converte l'ambiente
        char** envp = convertEnvMap(env);
        execve(scriptPath.c_str(), args, envp);
        freeEnvArray(envp);
        exit(1);
    } else {
        // Processo padre
        close(pipeIn[0]);
        close(pipeOut[1]);

        if (!queryString.empty()) {
            write(pipeIn[1], queryString.c_str(), queryString.size());
        }
        close(pipeIn[1]);

        char buffer[1024];
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
