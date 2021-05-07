CXX             = g++
BUILD           = ../dist/fb/internal
STD             = -std=c++20
TARGET          = $(BUILD)/internal
LIBS            = -ljsoncpp -lz -lpthread -lmysqlclient -lncursesw -lboost_program_options
INC             = -I.. -I. -I../plugin/linux -I/usr/include/mysql

all: $(BUILD)/config.o $(BUILD)/console.o $(BUILD)/db.o $(BUILD)/stream.o $(BUILD)/encoding.o $(BUILD)/table.base.o $(BUILD)/thread.o $(BUILD)/acceptor.internal.o $(BUILD)/session.internal.o $(BUILD)/table.internal.o $(BUILD)/main.o
	mkdir -p $(BUILD)
	$(CXX) $(BUILD)/main.o $(BUILD)/config.o $(BUILD)/console.o $(BUILD)/db.o $(BUILD)/stream.o $(BUILD)/encoding.o $(BUILD)/table.base.o $(BUILD)/thread.o $(BUILD)/acceptor.internal.o $(BUILD)/session.internal.o $(BUILD)/table.internal.o $(INC) $(LIB_DIRS) $(LIBS) -o $(TARGET)

$(BUILD)/config.o: ../module/config/config.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ../module/config/config.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/config.o

$(BUILD)/console.o: ../module/console/console.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ../module/console/console.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/console.o

$(BUILD)/db.o: ../module/db/db.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ../module/db/db.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/db.o

$(BUILD)/stream.o: ../module/stream/stream.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ../module/stream/stream.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/stream.o

$(BUILD)/encoding.o: ../module/encoding/encoding.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ../module/encoding/encoding.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/encoding.o

$(BUILD)/table.base.o: ../module/table/table.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ../module/table/table.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/table.base.o

$(BUILD)/thread.o: ../module/thread/thread.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ../module/thread/thread.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/thread.o

$(BUILD)/acceptor.internal.o: ./model/acceptor/acceptor.internal.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./model/acceptor/acceptor.internal.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/acceptor.internal.o

$(BUILD)/session.internal.o: ./model/session/session.internal.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./model/session/session.internal.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/session.internal.o

$(BUILD)/table.internal.o: ./model/table/table.internal.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./model/table/table.internal.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/table.internal.o

$(BUILD)/main.o: ./main.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./main.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/main.o

clean :
	rm -f $(BUILD)/*.o