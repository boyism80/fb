CXX             = g++
BUILD           = bin
STD             = -std=c++17
TARGET          = $(BUILD)/gateway
LIBS            = -ljsoncpp -lz -lpthread -lmysqlclient -lcrypto
INC             = -I.. -I. -I../plugin/linux -I/usr/include/mysql

all: config acceptor console cryptor db svc socket stream timer encoding acceptor.gateway gateway session.gateway service.gateway main
	$(CXX) $(BUILD)/main.o $(BUILD)/config.o $(BUILD)/acceptor.o $(BUILD)/console.o $(BUILD)/cryptor.o $(BUILD)/db.o $(BUILD)/service.o $(BUILD)/socket.o $(BUILD)/stream.o $(BUILD)/timer.o $(BUILD)/encoding.o $(BUILD)/acceptor.gateway.o $(BUILD)/gateway.o $(BUILD)/session.gateway.o $(BUILD)/service.gateway.o $(INC) $(LIB_DIRS) $(LIBS) -o $(TARGET)

config: ../module/config/config.h ../module/config/config.cpp
	$(CXX) $(STD) -c ../module/config/config.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/config.o

acceptor: ../module/acceptor/acceptor.h ../module/acceptor/acceptor.hpp ../module/acceptor/acceptor.cpp
	$(CXX) $(STD) -c ../module/acceptor/acceptor.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/acceptor.o

console: ../module/console/console.h ../module/console/console.cpp
	$(CXX) $(STD) -c ../module/console/console.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/console.o

cryptor: ../module/cryptor/cryptor.h ../module/cryptor/cryptor.cpp
	$(CXX) $(STD) -c ../module/cryptor/cryptor.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/cryptor.o

db: ../module/db/db.h ../module/db/db.cpp
	$(CXX) $(STD) -c ../module/db/db.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/db.o

svc: ../module/service/service.h ../module/service/service.cpp
	$(CXX) $(STD) -c ../module/service/service.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/service.o

socket: ../module/socket/socket.h ../module/socket/socket.hpp ../module/socket/socket.cpp
	$(CXX) $(STD) -c ../module/socket/socket.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/socket.o

stream: ../module/stream/stream.h ../module/stream/stream.cpp
	$(CXX) $(STD) -c ../module/stream/stream.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/stream.o

timer: ../module/timer/timer.h ../module/timer/timer.cpp
	$(CXX) $(STD) -c ../module/timer/timer.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/timer.o

encoding: ../module/encoding/encoding.h ../module/encoding/encoding.cpp
	$(CXX) $(STD) -c ../module/encoding/encoding.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/encoding.o

acceptor.gateway: ./model/acceptor/acceptor.gateway.h ./model/acceptor/acceptor.gateway.cpp
	$(CXX) $(STD) -c ./model/acceptor/acceptor.gateway.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/acceptor.gateway.o

gateway: ./model/gateway/gateway.h ./model/gateway/gateway.cpp
	$(CXX) $(STD) -c ./model/gateway/gateway.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/gateway.o

session.gateway: ./model/session/session.gateway.h ./model/session/session.gateway.cpp
	$(CXX) $(STD) -c ./model/session/session.gateway.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/session.gateway.o

service.gateway: ./service/gateway/service.gateway.h ./service/gateway/service.gateway.cpp
	$(CXX) $(STD) -c ./service/gateway/service.gateway.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/service.gateway.o

main: ./main.cpp
	$(CXX) $(STD) -c ./main.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/main.o

clean :
	rm -f $(BUILD)/*