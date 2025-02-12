#include "CGIHandler.hpp"
#include <unistd.h>     // per pipe, fork, dup2, execve, close, read, write
#include <sys/types.h>  // per pid_t
#include <sys/wait.h>   // per waitpid
#include <cstdlib>      // per exit
#include <cstring>      // per strerror, se serve
#include <string>
#include <map>

// Costruttore: implementazione vuota (o aggiungi eventuali inizializzazioni necessarie)
CGIHandler::CGIHandler() {
    // Eventuali inizializzazioni
}

// Distruttore: implementazione vuota
CGIHandler::~CGIHandler() {
    // Pulizia delle risorse, se necessaria
}

// Implementazione della funzione executeCGI.
// Esegue lo script CGI indicato da scriptPath, inviando alla sua stdin il contenuto di queryString.
// Le variabili d'ambiente (env) sono attualmente ignorate (modifica se vuoi passarle a execve).
std::string CGIHandler::executeCGI(const std::string& scriptPath, const std::string& queryString,
                                     const std::map<std::string, std::string>& env)
{
    (void)env; // Se non usi le variabili d'ambiente, castale a void per evitare warning

    int pipeIn[2];
    int pipeOut[2];
    pid_t pid;
    std::string output;

    // Creazione delle pipe per comunicazione in ingresso e in uscita
    if (pipe(pipeIn) == -1 || pipe(pipeOut) == -1) {
        // Gestione errore pipe
        return "";
    }

    pid = fork();
    if (pid < 0) {
        // Errore nella fork
        return "";
    } else if (pid == 0) {
        // Processo figlio (CGI)
        // Ridireziona lo standard input/output alle pipe
        dup2(pipeIn[0], STDIN_FILENO);
        dup2(pipeOut[1], STDOUT_FILENO);

        // Chiude le estremità non utilizzate
        close(pipeIn[1]);
        close(pipeOut[0]);

        // Prepara gli argomenti per execve
        // Nota: se lo script necessita di ulteriori argomenti, aggiungili all'array.
        char *args[] = { const_cast<char*>(scriptPath.c_str()), NULL };

        // Esegue lo script CGI; qui non passiamo un array di variabili d'ambiente.
        execve(scriptPath.c_str(), args, NULL);

        // Se execve fallisce, esce con codice di errore.
        exit(1);
    } else {
        // Processo padre
        // Chiude le estremità non utilizzate
        close(pipeIn[0]);
        close(pipeOut[1]);

        // Se è presente una queryString, la scrive sulla pipe di input per il CGI
        if (!queryString.empty()) {
            write(pipeIn[1], queryString.c_str(), queryString.size());
        }
        // Chiude l'estremità di scrittura della pipe
        close(pipeIn[1]);

        // Legge l'output del CGI dalla pipe di output
        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(pipeOut[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';  // Assicura la terminazione della stringa
            output += buffer;
        }
        // Chiude l'estremità di lettura
        close(pipeOut[0]);

        // Attende la terminazione del processo figlio
        waitpid(pid, NULL, 0);
    }

    return output;
}
