CXX             = g++ -g
BUILD           = /dist/fb/game
STD             = -std=c++20
TARGET          = $(BUILD)/game 
LIBS            = -ljsoncpp -lz -lpthread -lmysqlclient -lcrypto -llua5.3 -lncursesw
INC             = -I.. -I. -I../plugin/linux -I/usr/include/mysql

all: $(BUILD)/config.o $(BUILD)/console.o $(BUILD)/cryptor.o $(BUILD)/database.o $(BUILD)/service.o $(BUILD)/sql.service.o $(BUILD)/stream.o $(BUILD)/timer.o $(BUILD)/encoding.o $(BUILD)/table.base.o $(BUILD)/builtin_fn.o $(BUILD)/acceptor.game.o $(BUILD)/acceptor.game.handler.o $(BUILD)/acceptor.game.command.o $(BUILD)/board.o $(BUILD)/clan.o $(BUILD)/door.o $(BUILD)/group.o $(BUILD)/item.o $(BUILD)/life.o $(BUILD)/listener.o $(BUILD)/lua.o $(BUILD)/sector.o $(BUILD)/map.o $(BUILD)/table.game.o $(BUILD)/mmo.o $(BUILD)/mob.o $(BUILD)/npc.o $(BUILD)/object.o $(BUILD)/session.game.o $(BUILD)/spell.o $(BUILD)/trade.o $(BUILD)/main.o
	$(CXX) $(BUILD)/config.o $(BUILD)/console.o $(BUILD)/cryptor.o $(BUILD)/stream.o $(BUILD)/database.o $(BUILD)/service.o $(BUILD)/sql.service.o $(BUILD)/timer.o $(BUILD)/encoding.o $(BUILD)/table.base.o $(BUILD)/acceptor.game.o $(BUILD)/acceptor.game.handler.o $(BUILD)/acceptor.game.command.o $(BUILD)/board.o $(BUILD)/clan.o $(BUILD)/door.o $(BUILD)/group.o $(BUILD)/item.o $(BUILD)/life.o $(BUILD)/listener.o $(BUILD)/lua.o $(BUILD)/sector.o $(BUILD)/map.o $(BUILD)/table.game.o $(BUILD)/mmo.o $(BUILD)/mob.o $(BUILD)/npc.o $(BUILD)/object.o $(BUILD)/session.game.o $(BUILD)/spell.o $(BUILD)/trade.o $(BUILD)/builtin_fn.o $(BUILD)/main.o $(INC) $(LIB_DIRS) $(LIBS) -o $(TARGET)

$(BUILD)/config.o: ../module/config/config.cpp
	$(CXX) $(STD) -c ../module/config/config.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/config.o

$(BUILD)/console.o: ../module/console/console.cpp
	$(CXX) $(STD) -c ../module/console/console.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/console.o

$(BUILD)/cryptor.o: ../module/cryptor/cryptor.cpp
	$(CXX) $(STD) -c ../module/cryptor/cryptor.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/cryptor.o

$(BUILD)/database.o: ../module/db/db.cpp
	$(CXX) $(STD) -c ../module/db/db.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/database.o

$(BUILD)/service.o: ../module/service/service.cpp
	$(CXX) $(STD) -c ../module/service/service.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/service.o

$(BUILD)/sql.service.o: ./service/sql/sql.service.cpp
	$(CXX) $(STD) -c ./service/sql/sql.service.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/sql.service.o

$(BUILD)/stream.o: ../module/stream/stream.cpp
	$(CXX) $(STD) -c ../module/stream/stream.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/stream.o

$(BUILD)/timer.o: ../module/timer/timer.cpp
	$(CXX) $(STD) -c ../module/timer/timer.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/timer.o

$(BUILD)/encoding.o: ../module/encoding/encoding.cpp
	$(CXX) $(STD) -c ../module/encoding/encoding.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/encoding.o

$(BUILD)/table.base.o: ../module/table/table.cpp
	$(CXX) $(STD) -c ../module/table/table.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/table.base.o

$(BUILD)/builtin_fn.o: ./builtin/builtin_function.cpp
	$(CXX) $(STD) -c ./builtin/builtin_function.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/builtin_fn.o

$(BUILD)/acceptor.game.o: ./model/acceptor/acceptor.game.cpp
	$(CXX) $(STD) -c ./model/acceptor/acceptor.game.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/acceptor.game.o

$(BUILD)/acceptor.game.handler.o: ./model/acceptor/acceptor.game.handler.cpp
	$(CXX) $(STD) -c ./model/acceptor/acceptor.game.handler.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/acceptor.game.handler.o

$(BUILD)/acceptor.game.command.o: ./model/acceptor/acceptor.game.command.cpp
	$(CXX) $(STD) -c ./model/acceptor/acceptor.game.command.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/acceptor.game.command.o

$(BUILD)/board.o: ./model/board/board.cpp
	$(CXX) $(STD) -c ./model/board/board.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/board.o

$(BUILD)/clan.o: ./model/clan/clan.cpp
	$(CXX) $(STD) -c ./model/clan/clan.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/clan.o

$(BUILD)/door.o: ./model/door/door.cpp
	$(CXX) $(STD) -c ./model/door/door.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/door.o

$(BUILD)/group.o: ./model/group/group.cpp
	$(CXX) $(STD) -c ./model/group/group.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/group.o

$(BUILD)/item.o: ./model/item/item.cpp
	$(CXX) $(STD) -c ./model/item/item.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/item.o

$(BUILD)/life.o: ./model/life/life.cpp
	$(CXX) $(STD) -c ./model/life/life.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/life.o

$(BUILD)/listener.o: ./model/listener/listener.cpp
	$(CXX) $(STD) -c ./model/listener/listener.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/listener.o

$(BUILD)/lua.o: ./model/lua/lua.cpp
	$(CXX) $(STD) -c ./model/lua/lua.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/lua.o

$(BUILD)/sector.o: ./model/map/sector.cpp
	$(CXX) $(STD) -c ./model/map/sector.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/sector.o

$(BUILD)/map.o: ./model/map/map.cpp
	$(CXX) $(STD) -c ./model/map/map.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/map.o

$(BUILD)/table.game.o: ./model/table/table.game.cpp
	$(CXX) $(STD) -c ./model/table/table.game.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/table.game.o

$(BUILD)/mmo.o: ./model/mmo/mmo.cpp
	$(CXX) $(STD) -c ./model/mmo/mmo.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/mmo.o

$(BUILD)/mob.o: ./model/mob/mob.cpp
	$(CXX) $(STD) -c ./model/mob/mob.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/mob.o

$(BUILD)/npc.o: ./model/npc/npc.cpp
	$(CXX) $(STD) -c ./model/npc/npc.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/npc.o

$(BUILD)/object.o: ./model/object/object.cpp
	$(CXX) $(STD) -c ./model/object/object.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/object.o

$(BUILD)/session.game.o: ./model/session/session.cpp
	$(CXX) $(STD) -c ./model/session/session.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/session.game.o

$(BUILD)/spell.o: ./model/spell/spell.cpp
	$(CXX) $(STD) -c ./model/spell/spell.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/spell.o

$(BUILD)/trade.o: ./model/trade/trade.cpp
	$(CXX) $(STD) -c ./model/trade/trade.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/trade.o

$(BUILD)/main.o: ./main.cpp
	$(CXX) $(STD) -c ./main.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/main.o

clean :
	rm -f $(BUILD)/*.o