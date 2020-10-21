CXX             = g++
BUILD           = bin
STD             = -std=c++17
TARGET          = $(BUILD)/login 
LIBS            = -ljsoncpp -lz -lpthread -lmysqlclient -lcrypto -lncursesw
INC             = -I.. -I. -I../plugin/linux -I/usr/include/mysql

all: config console cryptor db svc stream timer encoding acceptor.login gateway session.login auth.service main
	$(CXX) $(BUILD)/main.o $(BUILD)/config.o $(BUILD)/console.o $(BUILD)/cryptor.o $(BUILD)/db.o $(BUILD)/service.o $(BUILD)/stream.o $(BUILD)/timer.o $(BUILD)/encoding.o $(BUILD)/acceptor.login.o $(BUILD)/gateway.o $(BUILD)/session.login.o $(BUILD)/auth.service.o $(INC) $(LIB_DIRS) $(LIBS) -o $(TARGET)

config:
	$(CXX) $(STD) -c ../module/config/config.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/config.o

console:
	$(CXX) $(STD) -c ../module/console/console.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/console.o

cryptor:
	$(CXX) $(STD) -c ../module/cryptor/cryptor.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/cryptor.o

db:
	$(CXX) $(STD) -c ../module/db/db.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/db.o

svc:
	$(CXX) $(STD) -c ../module/service/service.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/service.o

stream:
	$(CXX) $(STD) -c ../module/stream/stream.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/stream.o

timer:
	$(CXX) $(STD) -c ../module/timer/timer.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/timer.o

encoding:
	$(CXX) $(STD) -c ../module/encoding/encoding.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/encoding.o

acceptor.login:
	$(CXX) $(STD) -c ./model/acceptor/acceptor.login.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/acceptor.login.o

gateway:
	$(CXX) $(STD) -c ./model/gateway/gateway.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/gateway.o

session.login:
	$(CXX) $(STD) -c ./model/session/session.login.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/session.login.o

auth.service:
	$(CXX) $(STD) -c ./service/auth/auth.service.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/auth.service.o

main:
	$(CXX) $(STD) -c ./main.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/main.o

clean :
	rm -f $(BUILD)/*.o