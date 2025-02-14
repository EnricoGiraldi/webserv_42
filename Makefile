NAME = webserv

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = include

# Aggiornamento della lista dei file sorgente:
SOURCES = $(SRC_DIR)/cgi_handler.cpp \
          $(SRC_DIR)/config_parser.cpp \
          $(SRC_DIR)/error_handler.cpp \
          $(SRC_DIR)/server.cpp \
          $(SRC_DIR)/router.cpp \
          $(SRC_DIR)/request.cpp \
          $(SRC_DIR)/response.cpp \
          main.cpp

SRC_SOURCES = $(filter $(SRC_DIR)/%.cpp, $(SOURCES))
TOP_SOURCES = $(filter-out $(SRC_DIR)/%.cpp, $(SOURCES))

OBJ_SRC = $(patsubst $(SRC_DIR)/%.cpp,$(OBJ_DIR)/%.o,$(SRC_SOURCES))
OBJ_TOP = $(patsubst %.cpp,$(OBJ_DIR)/%.o,$(TOP_SOURCES))

OBJECTS = $(OBJ_SRC) $(OBJ_TOP)

CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -I$(INC_DIR)

all: $(NAME)

$(NAME): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
