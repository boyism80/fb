CXX             = g++ -g
BUILD           = bin
STD             = -std=c++20
TARGET          = $(BUILD)/game 
LIBS            = -ljsoncpp -lz -lpthread -lmysqlclient -lcrypto -llua5.3 -lncursesw
INC             = -I.. -I. -I../plugin/linux -I/usr/include/mysql

all: config console cryptor database svc stream timer encoding table.base builtin_fn acceptor.game acceptor.game.handler board clan door group item life listener lua sector map table.game mmo mob npc object session.game spell trade main
	$(CXX) $(BUILD)/config.o $(BUILD)/console.o $(BUILD)/cryptor.o $(BUILD)/stream.o $(BUILD)/database.o $(BUILD)/service.o $(BUILD)/timer.o $(BUILD)/encoding.o $(BUILD)/table.base.o $(BUILD)/acceptor.game.o $(BUILD)/acceptor.game.handler.o $(BUILD)/board.o $(BUILD)/clan.o $(BUILD)/door.o $(BUILD)/group.o $(BUILD)/item.o $(BUILD)/life.o $(BUILD)/listener.o $(BUILD)/lua.o $(BUILD)/sector.o $(BUILD)/map.o $(BUILD)/table.game.o $(BUILD)/mmo.o $(BUILD)/mob.o $(BUILD)/npc.o $(BUILD)/object.o $(BUILD)/session.game.o $(BUILD)/spell.o $(BUILD)/trade.o $(BUILD)/builtin_fn.o $(BUILD)/main.o $(INC) $(LIB_DIRS) $(LIBS) -o $(TARGET)

config:
	$(CXX) $(STD) -c ../module/config/config.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/config.o

console:
	$(CXX) $(STD) -c ../module/console/console.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/console.o

cryptor:
	$(CXX) $(STD) -c ../module/cryptor/cryptor.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/cryptor.o

database:
	$(CXX) $(STD) -c ../module/db/db.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/database.o

svc:
	$(CXX) $(STD) -c ../module/service/service.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/service.o

stream:
	$(CXX) $(STD) -c ../module/stream/stream.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/stream.o

timer:
	$(CXX) $(STD) -c ../module/timer/timer.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/timer.o

encoding:
	$(CXX) $(STD) -c ../module/encoding/encoding.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/encoding.o

table.base:
	$(CXX) $(STD) -c ../module/table/table.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/table.base.o

builtin_fn:
	$(CXX) $(STD) -c ./builtin/builtin_function.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/builtin_fn.o

acceptor.game:
	$(CXX) $(STD) -c ./model/acceptor/acceptor.game.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/acceptor.game.o

acceptor.game.handler:
	$(CXX) $(STD) -c ./model/acceptor/acceptor.game.handler.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/acceptor.game.handler.o

board:
	$(CXX) $(STD) -c ./model/board/board.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/board.o

clan:
	$(CXX) $(STD) -c ./model/clan/clan.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/clan.o

door:
	$(CXX) $(STD) -c ./model/door/door.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/door.o

group:
	$(CXX) $(STD) -c ./model/group/group.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/group.o

item:
	$(CXX) $(STD) -c ./model/item/item.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/item.o

life:
	$(CXX) $(STD) -c ./model/life/life.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/life.o

listener:
	$(CXX) $(STD) -c ./model/listener/listener.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/listener.o

lua:
	$(CXX) $(STD) -c ./model/lua/lua.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/lua.o

sector:
	$(CXX) $(STD) -c ./model/map/sector.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/sector.o

map:
	$(CXX) $(STD) -c ./model/map/map.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/map.o

table.game:
	$(CXX) $(STD) -c ./model/table/table.game.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/table.game.o

mmo:
	$(CXX) $(STD) -c ./model/mmo/mmo.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/mmo.o

mob:
	$(CXX) $(STD) -c ./model/mob/mob.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/mob.o

npc:
	$(CXX) $(STD) -c ./model/npc/npc.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/npc.o

object:
	$(CXX) $(STD) -c ./model/object/object.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/object.o

session.game:
	$(CXX) $(STD) -c ./model/session/session.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/session.game.o

spell:
	$(CXX) $(STD) -c ./model/spell/spell.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/spell.o

trade:
	$(CXX) $(STD) -c ./model/trade/trade.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/trade.o

main:
	$(CXX) $(STD) -c ./main.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/main.o

clean :
	rm -f $(BUILD)/*.o