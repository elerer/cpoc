INCLUDES = -I../websocketpp -I../boost_1_67_0 -I../portaudio/include
SRC = main_tester.cpp
LIBS = -lboost_system-mgw63-d-x32-1_67 -lws2_32 -lboost_thread-mgw63-mt-d-x32-1_67 -lole32 -lwinmm
LIBS_PATH = -L/c/workspace/boost-build 
FLAGS =  -Wall -Werror -fpic -std=c++14 -g 


all: build

build:
	g++ $(INCLUDES) $(FLAGS) $(SRC)  /c/workspace/portaudio/lib/.libs/libportaudio.a  $(LIBS) $(LIBS_PATH) 