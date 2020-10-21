CXX             = g++
BUILD           = bin
STD             = -std=c++17
TARGET          = $(BUILD)/internal
LIBS            = -ljsoncpp -lz -lpthread -lmysqlclient -lncursesw
INC             = -I.. -I. -I../plugin/linux -I/usr/include/mysql

all: config console db stream timer encoding table session.internal acceptor.internal table.internal main
	$(CXX) $(BUILD)/main.o $(BUILD)/config.o $(BUILD)/console.o $(BUILD)/db.o $(BUILD)/stream.o $(BUILD)/timer.o $(BUILD)/encoding.o $(BUILD)/table.o $(BUILD)/acceptor.internal.o $(BUILD)/session.internal.o $(BUILD)/table.internal.o $(INC) $(LIB_DIRS) $(LIBS) -o $(TARGET)

config: ../module/config/config.h ../module/config/config.cpp
	$(CXX) $(STD) -c ../module/config/config.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/config.o

console: ../module/console/console.h ../module/console/console.cpp
	$(CXX) $(STD) -c ../module/console/console.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/console.o

db: ../module/db/db.h ../module/db/db.cpp
	$(CXX) $(STD) -c ../module/db/db.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/db.o

stream: ../module/stream/stream.h ../module/stream/stream.cpp
	$(CXX) $(STD) -c ../module/stream/stream.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/stream.o

timer: ../module/timer/timer.h ../module/timer/timer.cpp
	$(CXX) $(STD) -c ../module/timer/timer.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/timer.o

encoding: ../module/encoding/encoding.h ../module/encoding/encoding.cpp
	$(CXX) $(STD) -c ../module/encoding/encoding.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/encoding.o

table: ../module/table/table.h ../module/table/table.cpp
	$(CXX) $(STD) -c ../module/table/table.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/table.o

acceptor.internal: ./model/acceptor/acceptor.internal.h ./model/acceptor/acceptor.internal.cpp
	$(CXX) $(STD) -c ./model/acceptor/acceptor.internal.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/acceptor.internal.o

session.internal: ./model/session/session.internal.h ./model/session/session.internal.cpp
	$(CXX) $(STD) -c ./model/session/session.internal.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/session.internal.o

table.internal: ./model/table/table.internal.h ./model/table/table.internal.cpp
	$(CXX) $(STD) -c ./model/table/table.internal.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/table.internal.o

main: ./main.cpp
	$(CXX) $(STD) -c ./main.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/main.o

clean :
	rm -f $(BUILD)/*