CXX = c++
CXXFLAGS = -std=c++98 -Wall -Wextra -Werror
DEUBG = -g -fsanitize=address

NAME = webserv
INCLUDE = -I ./include
SRC_DIR = ./src/
SRC_FILE =	main.cpp \
			Conf.cpp \
			HttpServer.cpp	\
			HttpRequest.cpp	\
			HttpResponse.cpp\
			util.cpp

SRCS = $(addprefix $(SRC_DIR), $(SRC_FILE))
OBJS = $(SRCS:.cpp=.o)

%.o : %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCLUDE) $< -o $@

all : $(NAME)

$(NAME) : $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(NAME)

debug :
	$(CXX) $(DEBUG) $(INCLUDE) $(SRCS) -o webserv

clean :
	rm -f $(OBJS)

fclean : clean
	rm -f $(NAME)

re : fclean all

.PHONY : all debug clean fclean re
