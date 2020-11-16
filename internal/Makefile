CXX             = g++
BUILD           = bin
STD             = -std=c++20
TARGET          = $(BUILD)/internal
LIBS            = -ljsoncpp -lz -lpthread -lmysqlclient -lncursesw
INC             = -I.. -I. -I../plugin/linux -I/usr/include/mysql

all: config console db stream timer encoding table.base acceptor.internal session.internal table.internal main
	$(CXX) $(BUILD)/main.o $(BUILD)/config.o $(BUILD)/console.o $(BUILD)/db.o $(BUILD)/stream.o $(BUILD)/timer.o $(BUILD)/encoding.o $(BUILD)/table.base.o $(BUILD)/acceptor.internal.o $(BUILD)/session.internal.o $(BUILD)/table.internal.o $(INC) $(LIB_DIRS) $(LIBS) -o $(TARGET)

config:
	$(CXX) $(STD) -c ../module/config/config.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/config.o

console:
	$(CXX) $(STD) -c ../module/console/console.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/console.o

db:
	$(CXX) $(STD) -c ../module/db/db.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/db.o

stream:
	$(CXX) $(STD) -c ../module/stream/stream.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/stream.o

timer:
	$(CXX) $(STD) -c ../module/timer/timer.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/timer.o

encoding:
	$(CXX) $(STD) -c ../module/encoding/encoding.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/encoding.o

table.base:
	$(CXX) $(STD) -c ../module/table/table.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/table.base.o

acceptor.internal:
	$(CXX) $(STD) -c ./model/acceptor/acceptor.internal.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/acceptor.internal.o

session.internal:
	$(CXX) $(STD) -c ./model/session/session.internal.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/session.internal.o

table.internal:
	$(CXX) $(STD) -c ./model/table/table.internal.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/table.internal.o

main:
	$(CXX) $(STD) -c ./main.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/main.o

clean :
	rm -f $(BUILD)/*.o