CXX             = g++-12 -g
BUILD           = ../dist/fb/login
STD             = -std=c++20
TARGET          = $(BUILD)/login 
LIBS            = -ljsoncpp -lz -lpthread -lmysqlclient -lcrypto -lncursesw -lboost_program_options -lcpp_redis -ltacopie -fcoroutines
INC             = -I.. -I. -I../plugin/linux -I/usr/include/mysql -I../include

all: $(BUILD)/config.o $(BUILD)/config.o $(BUILD)/console.o $(BUILD)/logger.o $(BUILD)/cryptor.o $(BUILD)/db.o $(BUILD)/encoding.o $(BUILD)/query.o $(BUILD)/stream.o $(BUILD)/table.o $(BUILD)/thread.o $(BUILD)/time.o $(BUILD)/format.o $(BUILD)/context.o $(BUILD)/gateway.o $(BUILD)/session.o $(BUILD)/main.o
	mkdir -p $(BUILD)
	$(CXX) $(BUILD)/config.o $(BUILD)/console.o $(BUILD)/logger.o $(BUILD)/cryptor.o $(BUILD)/db.o $(BUILD)/encoding.o $(BUILD)/query.o $(BUILD)/stream.o $(BUILD)/table.o $(BUILD)/thread.o $(BUILD)/time.o $(BUILD)/format.o $(BUILD)/context.o $(BUILD)/gateway.o $(BUILD)/session.o $(BUILD)/main.o $(INC) $(LIB_DIRS) $(LIBS) -o $(TARGET)

$(BUILD)/config.o: ../core/config.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ../core/config.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/config.o
$(BUILD)/console.o: ../core/console.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ../core/console.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/console.o
$(BUILD)/logger.o: ../core/logger.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ../core/logger.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/logger.o
$(BUILD)/cryptor.o: ../core/cryptor.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ../core/cryptor.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/cryptor.o
$(BUILD)/db.o: ../core/db.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ../core/db.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/db.o
$(BUILD)/encoding.o: ../core/encoding.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ../core/encoding.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/encoding.o
$(BUILD)/query.o: ../core/query.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ../core/query.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/query.o
$(BUILD)/stream.o: ../core/stream.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ../core/stream.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/stream.o
$(BUILD)/table.o: ../core/table.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ../core/table.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/table.o
$(BUILD)/thread.o: ../core/thread.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ../core/thread.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/thread.o
$(BUILD)/time.o: ../core/time.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ../core/time.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/time.o
$(BUILD)/format.o: ../core/format.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ../core/format.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/format.o
$(BUILD)/context.o: ./lib/context.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/context.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/context.o
$(BUILD)/gateway.o: ./lib/gateway.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/gateway.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/gateway.o
$(BUILD)/session.o: ./lib/session.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/session.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/session.o
$(BUILD)/main.o: ./main.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./main.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/main.o

clean :
	rm -f $(BUILD)/*.o