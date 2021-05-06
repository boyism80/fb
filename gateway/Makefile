CXX             = g++
BUILD           = /dist/fb/gateway
STD             = -std=c++20
TARGET          = $(BUILD)/gateway
LIBS            = -ljsoncpp -lz -lpthread -lmysqlclient -lcrypto -lncursesw -lboost_program_options
INC             = -I.. -I. -I../plugin/linux -I/usr/include/mysql

all: $(BUILD)/config.o $(BUILD)/console.o $(BUILD)/cryptor.o $(BUILD)/db.o $(BUILD)/service.o $(BUILD)/stream.o $(BUILD)/encoding.o $(BUILD)/thread.o $(BUILD)/acceptor.gateway.o $(BUILD)/gateway.o $(BUILD)/session.gateway.o $(BUILD)/service.gateway.o $(BUILD)/main.o
	$(CXX) $(BUILD)/main.o $(BUILD)/config.o $(BUILD)/console.o $(BUILD)/cryptor.o $(BUILD)/db.o $(BUILD)/service.o $(BUILD)/stream.o $(BUILD)/encoding.o $(BUILD)/thread.o $(BUILD)/acceptor.gateway.o $(BUILD)/gateway.o $(BUILD)/session.gateway.o $(BUILD)/service.gateway.o $(INC) $(LIB_DIRS) $(LIBS) -o $(TARGET)

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

$(BUILD)/encoding.o: ../module/encoding/encoding.cpp
	$(CXX) $(STD) -c ../module/encoding/encoding.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/encoding.o

$(BUILD)/thread.o: ../module/thread/thread.cpp
	$(CXX) $(STD) -c ../module/thread/thread.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/thread.o

$(BUILD)/acceptor.gateway.o: ./model/acceptor/acceptor.gateway.cpp
	$(CXX) $(STD) -c ./model/acceptor/acceptor.gateway.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/acceptor.gateway.o

$(BUILD)/gateway.o: ./model/gateway/gateway.cpp
	$(CXX) $(STD) -c ./model/gateway/gateway.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/gateway.o

$(BUILD)/session.gateway.o: ./model/session/session.gateway.cpp
	$(CXX) $(STD) -c ./model/session/session.gateway.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/session.gateway.o

$(BUILD)/service.gateway.o: ./service/gateway/service.gateway.cpp
	$(CXX) $(STD) -c ./service/gateway/service.gateway.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/service.gateway.o

$(BUILD)/main.o: ./main.cpp
	$(CXX) $(STD) -c ./main.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/main.o

clean :
	rm -f $(BUILD)/*.o