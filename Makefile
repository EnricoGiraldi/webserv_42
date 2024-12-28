CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iinclude

# Source files in src directory
SRCS = src/cgi_handler.cpp \
       src/config_parser.cpp \
       src/error_handler.cpp \
       src/http_methods_server.cpp \
       src/request.cpp \
       src/response.cpp \
       src/router.cpp \
       src/server.cpp \
       main.cpp  # Reference the main file

# Object files will be placed in the obj directory
OBJS = $(SRCS:%.cpp=obj/%.o)

# Output binary
TARGET = webserv

# Default rule
all: $(TARGET)

# Linking the final binary
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

# Rule for creating object files
obj/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Cleaning rules
clean:
	rm -rf obj

fclean: clean
	rm -f $(TARGET)

re: fclean all
