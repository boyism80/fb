CXX             = g++ -g
DEFINE			= -DREDISCPP_HEADER_ONLY
BUILD           = ../dist/fb/game
STD             = -std=c++20
TARGET          = $(BUILD)/game 
LIBS            = -ljsoncpp -lz -lpthread -lmysqlclient -lcrypto -llua5.3 -lncursesw -lboost_program_options -fcoroutines
INC             = -I.. -I. -I../plugin/linux -I/usr/include/mysql -I../include

all: $(BUILD)/config.o $(BUILD)/console.o $(BUILD)/logger.o $(BUILD)/cryptor.o $(BUILD)/db.o $(BUILD)/encoding.o $(BUILD)/query.o $(BUILD)/stream.o $(BUILD)/table.o $(BUILD)/thread.o $(BUILD)/time.o $(BUILD)/format.o $(BUILD)/board.o $(BUILD)/clan.o $(BUILD)/model.o $(BUILD)/door.o $(BUILD)/group.o $(BUILD)/listener.o $(BUILD)/mmo.o $(BUILD)/query_game.o $(BUILD)/spell.o $(BUILD)/trade.o $(BUILD)/regex.o $(BUILD)/command.o $(BUILD)/context.o $(BUILD)/handler.o $(BUILD)/armor.o $(BUILD)/auxiliary.o $(BUILD)/bow.o $(BUILD)/cash.o $(BUILD)/consume.o $(BUILD)/equipment.o $(BUILD)/helmet.o $(BUILD)/item.o $(BUILD)/itemmix.o $(BUILD)/items.o $(BUILD)/pack.o $(BUILD)/ring.o $(BUILD)/shield.o $(BUILD)/weapon.o $(BUILD)/built_in.o $(BUILD)/lua.o $(BUILD)/map.o $(BUILD)/sector.o $(BUILD)/life.o $(BUILD)/mob.o $(BUILD)/npc.o $(BUILD)/object.o $(BUILD)/session.o $(BUILD)/dialog.o $(BUILD)/life_builtin.o $(BUILD)/mob_builtin.o $(BUILD)/npc_builtin.o $(BUILD)/object_builtin.o $(BUILD)/session_builtin.o $(BUILD)/main.o
	mkdir -p $(BUILD)
	$(CXX) $(DEFINE) $(BUILD)/config.o $(BUILD)/console.o $(BUILD)/logger.o $(BUILD)/cryptor.o $(BUILD)/db.o $(BUILD)/encoding.o $(BUILD)/query.o $(BUILD)/stream.o $(BUILD)/table.o $(BUILD)/thread.o $(BUILD)/time.o $(BUILD)/format.o $(BUILD)/board.o $(BUILD)/clan.o $(BUILD)/model.o $(BUILD)/door.o $(BUILD)/group.o $(BUILD)/listener.o $(BUILD)/mmo.o $(BUILD)/query_game.o $(BUILD)/spell.o $(BUILD)/trade.o $(BUILD)/regex.o $(BUILD)/command.o $(BUILD)/context.o $(BUILD)/handler.o $(BUILD)/armor.o $(BUILD)/auxiliary.o $(BUILD)/bow.o $(BUILD)/cash.o $(BUILD)/consume.o $(BUILD)/equipment.o $(BUILD)/helmet.o $(BUILD)/item.o $(BUILD)/itemmix.o $(BUILD)/items.o $(BUILD)/pack.o $(BUILD)/ring.o $(BUILD)/shield.o $(BUILD)/weapon.o $(BUILD)/built_in.o $(BUILD)/lua.o $(BUILD)/map.o $(BUILD)/sector.o $(BUILD)/life.o $(BUILD)/mob.o $(BUILD)/npc.o $(BUILD)/object.o $(BUILD)/session.o $(BUILD)/dialog.o $(BUILD)/life_builtin.o $(BUILD)/mob_builtin.o $(BUILD)/npc_builtin.o $(BUILD)/object_builtin.o $(BUILD)/session_builtin.o $(BUILD)/main.o $(INC) $(LIB_DIRS) $(LIBS) -o $(TARGET)

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
$(BUILD)/board.o: ./lib/board.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/board.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/board.o
$(BUILD)/clan.o: ./lib/clan.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/clan.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/clan.o
$(BUILD)/model.o: ./lib/model.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/model.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/model.o
$(BUILD)/door.o: ./lib/door.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/door.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/door.o
$(BUILD)/group.o: ./lib/group.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/group.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/group.o
$(BUILD)/listener.o: ./lib/listener.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/listener.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/listener.o
$(BUILD)/mmo.o: ./lib/mmo.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/mmo.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/mmo.o
$(BUILD)/query_game.o: ./lib/query_game.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/query_game.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/query_game.o
$(BUILD)/spell.o: ./lib/spell.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/spell.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/spell.o
$(BUILD)/trade.o: ./lib/trade.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/trade.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/trade.o
$(BUILD)/regex.o: ./lib/regex.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/regex.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/regex.o
$(BUILD)/command.o: ./lib/context/command.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/context/command.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/command.o
$(BUILD)/context.o: ./lib/context/context.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/context/context.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/context.o
$(BUILD)/handler.o: ./lib/context/handler.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/context/handler.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/handler.o
$(BUILD)/armor.o: ./lib/item/armor.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/item/armor.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/armor.o
$(BUILD)/auxiliary.o: ./lib/item/auxiliary.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/item/auxiliary.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/auxiliary.o
$(BUILD)/bow.o: ./lib/item/bow.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/item/bow.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/bow.o
$(BUILD)/cash.o: ./lib/item/cash.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/item/cash.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/cash.o
$(BUILD)/consume.o: ./lib/item/consume.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/item/consume.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/consume.o
$(BUILD)/equipment.o: ./lib/item/equipment.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/item/equipment.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/equipment.o
$(BUILD)/helmet.o: ./lib/item/helmet.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/item/helmet.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/helmet.o
$(BUILD)/item.o: ./lib/item/item.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/item/item.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/item.o
$(BUILD)/itemmix.o: ./lib/item/itemmix.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/item/itemmix.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/itemmix.o
$(BUILD)/items.o: ./lib/item/items.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/item/items.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/items.o
$(BUILD)/pack.o: ./lib/item/pack.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/item/pack.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/pack.o
$(BUILD)/ring.o: ./lib/item/ring.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/item/ring.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/ring.o
$(BUILD)/shield.o: ./lib/item/shield.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/item/shield.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/shield.o
$(BUILD)/weapon.o: ./lib/item/weapon.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/item/weapon.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/weapon.o
$(BUILD)/built_in.o: ./lib/lua/built_in.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/lua/built_in.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/built_in.o
$(BUILD)/lua.o: ./lib/lua/lua.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/lua/lua.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/lua.o
$(BUILD)/map.o: ./lib/map/map.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/map/map.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/map.o
$(BUILD)/sector.o: ./lib/map/sector.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/map/sector.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/sector.o
$(BUILD)/life.o: ./lib/object/life.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/object/life.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/life.o
$(BUILD)/life_builtin.o: ./lib/object/life.builtin.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/object/life.builtin.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/life_builtin.o
$(BUILD)/mob.o: ./lib/object/mob.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/object/mob.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/mob.o
$(BUILD)/mob_builtin.o: ./lib/object/mob.builtin.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/object/mob.builtin.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/mob_builtin.o
$(BUILD)/npc.o: ./lib/object/npc.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/object/npc.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/npc.o
$(BUILD)/npc_builtin.o: ./lib/object/npc.builtin.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/object/npc.builtin.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/npc_builtin.o
$(BUILD)/object.o: ./lib/object/object.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/object/object.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/object.o
$(BUILD)/object_builtin.o: ./lib/object/object.builtin.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/object/object.builtin.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/object_builtin.o
$(BUILD)/session.o: ./lib/object/session.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/object/session.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/session.o
$(BUILD)/dialog.o: ./lib/dialog.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/dialog.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/dialog.o
$(BUILD)/session_builtin.o: ./lib/object/session.builtin.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./lib/object/session.builtin.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/session_builtin.o

$(BUILD)/main.o: ./main.cpp
	mkdir -p $(BUILD)
	$(CXX) $(STD) -c ./main.cpp $(INC) $(LIB_DIRS) $(LIBS) -o $(BUILD)/main.o

clean :
	rm -f $(BUILD)/*.o