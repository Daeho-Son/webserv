CXX = c++
CXXFLAGS = -std=c++98 -Wall -Wextra -Werror -g -fsanitize=address
DEBUG = -g -fsanitize=address

NAME = webserv
INCLUDE = -I ./include
SRC_DIR = ./src/
SRC_FILE =	Conf.cpp 			\
			Client.cpp			\
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

run : all
	./webserv ./conf/test_1.conf

test1 :
	./tester http://localhost:8080

test2 :
	@python3 ./test/test.py
	@rm -f ./put_test/test_temp_output

lsof:
	@lsof -i TCP:8080 > log.txt
	@cat log.txt

siege:
	@siege -b http://localhost:8080

.PHONY : all debug clean fclean re run test1 test2 lsof siege
