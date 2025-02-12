NAME = webserv

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

# Lista dei file sorgente.
# Nota: "main.cpp" si trova nella directory principale, mentre gli altri in $(SRC_DIR)
SOURCES = $(SRC_DIR)/cgi_handler.cpp \
          $(SRC_DIR)/config_parser.cpp \
          $(SRC_DIR)/error_handler.cpp \
          $(SRC_DIR)/http_methods_server.cpp \
          $(SRC_DIR)/server.cpp \
          $(SRC_DIR)/router.cpp \
          main.cpp

# Suddivido i sorgenti in quelli contenuti in SRC_DIR e quelli "top-level"
SRC_SOURCES = $(filter $(SRC_DIR)/%.cpp, $(SOURCES))
TOP_SOURCES = $(filter-out $(SRC_DIR)/%.cpp, $(SOURCES))

# Generazione degli oggetti per i sorgenti in SRC_DIR.
OBJ_SRC = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_SOURCES))
# Generazione degli oggetti per i sorgenti che non sono in SRC_DIR (es. main.cpp).
OBJ_TOP = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(TOP_SOURCES))

# Oggetti totali
OBJECTS = $(OBJ_SRC) $(OBJ_TOP)

# Compilatore e flag
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -I$(INC_DIR)

# Regola principale
all: $(NAME)

# Collegamento finale per creare l'eseguibile
$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(NAME)

# Regola per compilare i sorgenti in SRC_DIR
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Regola per compilare i sorgenti in directory principale (es. main.cpp)
$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Pulizia degli oggetti compilati
clean:
	rm -rf $(OBJ_DIR)

# Pulizia completa (oggetti + eseguibile)
fclean: clean
	rm -f $(NAME)

# Ricostruzione completa
re: fclean all

.PHONY: all clean fclean re
