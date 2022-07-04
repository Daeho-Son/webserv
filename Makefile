CXX = c++
CXXFLAGS = -std=c++98 -Wall -Wextra -Werror -g -fsanitize=address
DEBUG = -g -fsanitize=address

NAME = webserv
INCLUDE = -I ./include
SRC_DIR = ./src/
SRC_FILE =	Conf.cpp 			\
			HttpRequest.cpp		\
			HttpResponse.cpp	\
			HttpServer.cpp		\
			LocationInfo.cpp	\
			main.cpp			\
			ServerInfo.cpp		\
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

test :
	./tester http://localhost:8080
	# bash ./scripts/test.sh

done :
	bash ./scripts/done.sh

check:
	bash ./scripts/check_byte.sh

.PHONY : all debug clean fclean re test done check
