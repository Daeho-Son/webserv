CXX = c++
CXXFLAGS = -std=c++98 -Wall -Wextra -Werror
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
	./webserv ./conf/dason.conf

test1 :
	./tester http://localhost:8080

test2 :
	@python3 ./test/test.py
	@rm -f ./put_test/test_temp_output

lsof:
	@lsof -i TCP:8080

siege:
	@siege -b http://localhost:8080

netstat:
	@echo netstat -t

monitor:
	@echo "netstat -t : " `netstat -t | wc -l`
	@echo "lsof -t TCP:8080 : " `lsof -i TCP:8080 | wc -l`

.PHONY : all debug clean fclean re run test1 test2 lsof siege netstat monitor
