NAME = plazza

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++20 -Iinclude

SRC =	src/main.cpp \
		src/Pizza.cpp \
		src/Parser.cpp \
		src/Stock.cpp	\
		src/Kitchen.cpp	\
		src/ThreadPool.cpp	\
		src/KitchenManager.cpp	\
		src/Message.cpp	\
		src/PipeIPC.cpp	\
		src/KitchenProcess.cpp	\
		src/Reception.cpp

OBJ = $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
