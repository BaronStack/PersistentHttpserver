CXX=g++ -std=c++11 -lstdc++ -Wdeprecated -lrocksdb
INCLUDE="./include"

httpserver: src/http_server.cpp src/main.cpp src/mongoose.c
	${CXX} -I{INCLUDE} src/* -o httpserver

clean:
	rm -f httpserver
