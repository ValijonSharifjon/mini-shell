CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17

TARGET = myshell
SRC = src/main.cpp

all:
	${CXX} ${CXXFLAGS} ${SRC} -o ${TARGET}

clean: 
	rm -f ${TARGET}