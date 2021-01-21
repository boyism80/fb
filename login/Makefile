CXX             = g++
BUILD           = /dist/fb/login
STD             = -std=c++20
TARGET          = $(BUILD)/login 
LIBS            = -ljsoncpp -lz -lpthread -lmysqlclient -lcrypto -lncursesw
INC             = -I.. -I. -I../plugin/linux -I/usr/include/mysql

all: $(BUILD)/config.o $(BUILD)/console.o $(BUILD)/cryptor.o $(BUILD)/db.o $(BUILD)/service.o $(BUILD)/stream.o $(BUILD)/timer.o $(BUILD)/encoding.o $(BUILD)/acceptor.login.o $(BUILD)/gateway.o $(BUILD)/session.login.o $(BUILD)/auth.service.o $(BUILD)/sql.service.o $(BUILD)/main.o
	$(CXX) $(BUILD)/main.o $(BUILD)/config.o $(BUILD)/console.o $(BUILD)/cryptor.o $(BUILD)/db.o $(BUILD)/service.o $(BUILD)/stream.o $(BUILD)/timer.o $(BUILD)/encoding.o $(BUILD)/acceptor.login.o $(BUILD)/gateway.o $(BUILD)/session.login.o $(BUILD)/auth.service.o $(BUILD)/sql.service.o $(INC) $(LIB_DIRS) $(LIBS) -o $(TARGET)

$(BUILD)/config.o: ../module/config/config.cpp
	$(CXX) $(STD) -c ../module/config/config.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/config.o

$(BUILD)/console.o: ../module/console/console.cpp
	$(CXX) $(STD) -c ../module/console/console.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/console.o

$(BUILD)/cryptor.o: ../module/cryptor/cryptor.cpp
	$(CXX) $(STD) -c ../module/cryptor/cryptor.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/cryptor.o

$(BUILD)/db.o: ../module/db/db.cpp
	$(CXX) $(STD) -c ../module/db/db.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/db.o

$(BUILD)/service.o: ../module/service/service.cpp
	$(CXX) $(STD) -c ../module/service/service.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/service.o

$(BUILD)/stream.o: ../module/stream/stream.cpp
	$(CXX) $(STD) -c ../module/stream/stream.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/stream.o

$(BUILD)/timer.o: ../module/timer/timer.cpp
	$(CXX) $(STD) -c ../module/timer/timer.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/timer.o

$(BUILD)/encoding.o: ../module/encoding/encoding.cpp
	$(CXX) $(STD) -c ../module/encoding/encoding.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/encoding.o

$(BUILD)/acceptor.login.o: ./model/acceptor/acceptor.login.cpp
	$(CXX) $(STD) -c ./model/acceptor/acceptor.login.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/acceptor.login.o

$(BUILD)/gateway.o: ./model/gateway/gateway.cpp
	$(CXX) $(STD) -c ./model/gateway/gateway.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/gateway.o

$(BUILD)/session.login.o: ./model/session/session.login.cpp
	$(CXX) $(STD) -c ./model/session/session.login.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/session.login.o

$(BUILD)/auth.service.o: ./service/auth/auth.service.cpp
	$(CXX) $(STD) -c ./service/auth/auth.service.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/auth.service.o

$(BUILD)/sql.service.o: ./service/sql/sql.service.cpp
	$(CXX) $(STD) -c ./service/sql/sql.service.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/sql.service.o

$(BUILD)/main.o: ./main.cpp
	$(CXX) $(STD) -c ./main.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/main.o

clean :
	rm -f $(BUILD)/*.o