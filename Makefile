CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

TARGET = myshell
SRCS = src/main.cpp src/jobs.cpp src/signals.cpp src/parser.cpp src/executor.cpp src/builtins.cpp 

all:
	${CXX} ${CXXFLAGS} ${SRCS} -o ${TARGET}

format:
	clang-format -i src/*.cpp src/*.h

clean: 
	rm -f ${TARGET}