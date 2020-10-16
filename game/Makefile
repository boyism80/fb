CXX             = g++ -g
BUILD           = bin
STD             = -std=c++17
TARGET          = $(BUILD)/game 
LIBS            = -ljsoncpp -lz -lpthread -lmysqlclient -lcrypto -llua5.3 -lncursesw
INC             = -I.. -I. -I../plugin/linux -I/usr/include/mysql

all: config console cryptor database svc stream timer encoding acceptor.game acceptor.game.handler board clan door group item life listener lua sector map master mmo mob npc object session.game spell trade main builtin_fn
	$(CXX) $(BUILD)/config.o $(BUILD)/console.o $(BUILD)/cryptor.o $(BUILD)/database.o $(BUILD)/service.o $(BUILD)/stream.o $(BUILD)/timer.o $(BUILD)/encoding.o $(BUILD)/acceptor.game.o $(BUILD)/acceptor.game.handler.o $(BUILD)/board.o $(BUILD)/clan.o $(BUILD)/door.o $(BUILD)/group.o $(BUILD)/item.o $(BUILD)/life.o $(BUILD)/listener.o $(BUILD)/lua.o $(BUILD)/sector.o $(BUILD)/map.o $(BUILD)/master.o $(BUILD)/mmo.o $(BUILD)/mob.o $(BUILD)/npc.o $(BUILD)/object.o $(BUILD)/session.game.o $(BUILD)/spell.o $(BUILD)/trade.o $(BUILD)/builtin_fn.o $(BUILD)/main.o $(INC) $(LIB_DIRS) $(LIBS) -o $(TARGET)

build:
	$(CXX) $(BUILD)/config.o $(BUILD)/console.o $(BUILD)/cryptor.o $(BUILD)/db.o $(BUILD)/service.o $(BUILD)/stream.o $(BUILD)/timer.o $(BUILD)/encoding.o $(BUILD)/acceptor.game.o $(BUILD)/acceptor.game.handler.o $(BUILD)/board.o $(BUILD)/clan.o $(BUILD)/door.o $(BUILD)/group.o $(BUILD)/item.o $(BUILD)/life.o $(BUILD)/listener.o $(BUILD)/lua.o $(BUILD)/sector.o $(BUILD)/map.o $(BUILD)/master.o $(BUILD)/mmo.o $(BUILD)/mob.o $(BUILD)/npc.o $(BUILD)/object.o $(BUILD)/session.game.o $(BUILD)/spell.o $(BUILD)/trade.o $(BUILD)/builtin_fn.o $(BUILD)/main.o $(INC) $(LIB_DIRS) $(LIBS) -o $(TARGET)

config: ../module/config/config.h ../module/config/config.cpp
	$(CXX) $(STD) -c ../module/config/config.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/config.o

console: ../module/console/console.h ../module/console/console.cpp
	$(CXX) $(STD) -c ../module/console/console.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/console.o

cryptor: ../module/cryptor/cryptor.h ../module/cryptor/cryptor.cpp
	$(CXX) $(STD) -c ../module/cryptor/cryptor.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/cryptor.o

database: ../module/db/db.h ../module/db/db.cpp
	$(CXX) $(STD) -c ../module/db/db.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/database.o

svc: ../module/service/service.h ../module/service/service.cpp
	$(CXX) $(STD) -c ../module/service/service.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/service.o

stream: ../module/stream/stream.h ../module/stream/stream.cpp
	$(CXX) $(STD) -c ../module/stream/stream.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/stream.o

timer: ../module/timer/timer.h ../module/timer/timer.cpp
	$(CXX) $(STD) -c ../module/timer/timer.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/timer.o

encoding: ../module/encoding/encoding.h ../module/encoding/encoding.cpp
	$(CXX) $(STD) -c ../module/encoding/encoding.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/encoding.o

builtin_fn: ./builtin/builtin_function.h ./builtin/builtin_function.hpp ./builtin/builtin_function.cpp
	$(CXX) $(STD) -c ./builtin/builtin_function.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/builtin_fn.o

acceptor.game: ./model/acceptor/acceptor.game.h ./model/acceptor/acceptor.game.cpp
	$(CXX) $(STD) -c ./model/acceptor/acceptor.game.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/acceptor.game.o

acceptor.game.handler: ./model/acceptor/acceptor.game.h ./model/acceptor/acceptor.game.handler.cpp
	$(CXX) $(STD) -c ./model/acceptor/acceptor.game.handler.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/acceptor.game.handler.o

board: ./model/board/board.h ./model/board/board.cpp
	$(CXX) $(STD) -c ./model/board/board.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/board.o

clan: ./model/clan/clan.h ./model/clan/clan.cpp
	$(CXX) $(STD) -c ./model/clan/clan.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/clan.o

door: ./model/door/door.h ./model/door/door.cpp
	$(CXX) $(STD) -c ./model/door/door.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/door.o

group: ./model/group/group.h ./model/group/group.cpp
	$(CXX) $(STD) -c ./model/group/group.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/group.o

item: ./model/item/item.h ./model/item/item.cpp
	$(CXX) $(STD) -c ./model/item/item.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/item.o

life: ./model/life/life.h ./model/life/life.cpp
	$(CXX) $(STD) -c ./model/life/life.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/life.o

listener: ./model/listener/listener.h ./model/listener/listener.cpp
	$(CXX) $(STD) -c ./model/listener/listener.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/listener.o

lua: ./model/lua/lua.h ./model/lua/lua.cpp
	$(CXX) $(STD) -c ./model/lua/lua.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/lua.o

sector: ./model/map/sector.h ./model/map/sector.cpp
	$(CXX) $(STD) -c ./model/map/sector.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/sector.o

map: ./model/map/map.h ./model/map/map.cpp
	$(CXX) $(STD) -c ./model/map/map.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/map.o

master: ./model/master/master.h ./model/master/master.cpp
	$(CXX) $(STD) -c ./model/master/master.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/master.o

mmo: ./model/mmo/mmo.h ./model/mmo/mmo.cpp
	$(CXX) $(STD) -c ./model/mmo/mmo.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/mmo.o

mob: ./model/mob/mob.h ./model/mob/mob.cpp
	$(CXX) $(STD) -c ./model/mob/mob.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/mob.o

npc: ./model/npc/npc.h ./model/npc/npc.cpp
	$(CXX) $(STD) -c ./model/npc/npc.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/npc.o

object: ./model/object/object.h ./model/object/object.cpp
	$(CXX) $(STD) -c ./model/object/object.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/object.o

session.game: ./model/session/session.h ./model/session/session.cpp
	$(CXX) $(STD) -c ./model/session/session.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/session.game.o

spell: ./model/spell/spell.h ./model/spell/spell.cpp
	$(CXX) $(STD) -c ./model/spell/spell.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/spell.o

trade: ./model/trade/trade.h ./model/trade/trade.cpp
	$(CXX) $(STD) -c ./model/trade/trade.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/trade.o

main: ./main.cpp
	$(CXX) $(STD) -c ./main.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/main.o

clean :
	rm -f $(BUILD)/*.o