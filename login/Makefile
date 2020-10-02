CXX             = g++
BUILD           = bin
STD             = -std=c++17
TARGET          = $(BUILD)/login 
LIBS            = -ljsoncpp -lz -lpthread -lmysqlclient -lcrypto
INC             = -I.. -I. -I../plugin/linux -I/usr/include/mysql

all: main config acceptor console cryptor db svc socket stream timer encoding acceptor gateway acceptor.login session.login auth.service
	$(CXX) $(BUILD)/main.o $(BUILD)/config.o $(BUILD)/acceptor.o $(BUILD)/console.o $(BUILD)/cryptor.o $(BUILD)/db.o $(BUILD)/service.o $(BUILD)/socket.o $(BUILD)/stream.o $(BUILD)/timer.o $(BUILD)/encoding.o $(BUILD)/acceptor.login.o $(BUILD)/gateway.o $(BUILD)/session.login.o $(BUILD)/auth.service.o $(INC) $(LIB_DIRS) $(LIBS) -o $(TARGET)

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

acceptor.login: ./model/acceptor/acceptor.login.h ./model/acceptor/acceptor.login.cpp
	$(CXX) $(STD) -c ./model/acceptor/acceptor.login.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/acceptor.login.o

gateway: ./model/gateway/gateway.h ./model/gateway/gateway.cpp
	$(CXX) $(STD) -c ./model/gateway/gateway.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/gateway.o

session.login: ./model/session/session.login.h ./model/session/session.login.cpp
	$(CXX) $(STD) -c ./model/session/session.login.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/session.login.o

auth.service: ./service/auth/auth.service.h ./service/auth/auth.service.cpp
	$(CXX) $(STD) -c ./service/auth/auth.service.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/auth.service.o

main: ./main.cpp
	$(CXX) $(STD) -c ./main.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/main.o

clean :
	rm -f $(BUILD)/*