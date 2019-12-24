#include "fb_game.h"

using namespace fb::game;

fb::game::acceptor* fb::game::acceptor::_instance = NULL;

acceptor::acceptor(uint16_t port) : fb_acceptor<fb::game::session>(port)
{
	this->convert_item_file("db/item_db.txt");
	//this->convert_npc_file();
	//this->convert_npc_spawn_file();
	//this->convert_mob_file();

	this->load_maps("db/map.json");
	this->load_warp("db/warp.json");
	this->load_items("db/item.json");
	this->load_itemmix("db/itemmix.json");
	this->load_npc("db/npc.json");
	this->load_mob("db/mob.json");
	this->load_drop_item("db/item_drop.json");

	//this->convert_npc_spawn_file();
	this->load_npc_spawn("db/npc_spawn.json");

	//this->convert_mob_spawn_file();
	this->load_mob_spawn("db/mob_spawn.json");


	//this->convert_class_file();
	this->load_class();

	this->register_handle(0x10, &acceptor::handle_login);				// 게임서버 접속 핸들러
	this->register_handle(0x11, &acceptor::handle_direction);			// 방향전환 핸들러
	this->register_handle(0x06, &acceptor::handle_update_move);			// 이동과 맵 데이터 업데이트 핸들러
	this->register_handle(0x32, &acceptor::handle_move);				// 이동 핸들러
	this->register_handle(0x13, &acceptor::handle_attack);				// 공격 핸들러
	this->register_handle(0x07, &acceptor::handle_pickup);				// 아이템 줍기 핸들러
	this->register_handle(0x1D, &acceptor::handle_emotion);				// 감정표현 핸들러
	this->register_handle(0x05, &acceptor::handle_update_map);			// 맵 데이터 업데이트 핸들러
	this->register_handle(0x38, &acceptor::handle_refresh);				// 새로고침 핸들러
	this->register_handle(0x1C, &acceptor::handle_active_item);			// 아이템 사용 핸들러
	this->register_handle(0x1F, &acceptor::handle_inactive_item);		// 아이템 장착 해제 핸들러
	this->register_handle(0x08, &acceptor::handle_drop_item);			// 아이템 버리기 핸들러
	this->register_handle(0x24, &acceptor::handle_drop_cash);			// 금전 버리기 핸들러
	this->register_handle(0x09, &acceptor::handle_front_info);			// 앞방향 정보 핸들러
	this->register_handle(0x2D, &acceptor::handle_self_info);			// 나 자신의 정보 핸들러
	this->register_handle(0x1B, &acceptor::handle_option_changed);		// 옵션 설정 핸들러
	this->register_handle(0x43, &acceptor::handle_click_object);		// 오브젝트 클릭 핸들러
	this->register_handle(0x66, &acceptor::handle_item_info);			// 인벤토리 우클릭 핸들러
	this->register_handle(0x6B, &acceptor::handle_itemmix);				// 아이템 조합 핸들러
	this->register_handle(0x4A, &acceptor::handle_trade);				// 교환 핸들러
	this->register_handle(0x2E, &acceptor::handle_group);				// 그룹 핸들러

	this->register_timer(100, &acceptor::handle_mob_action);			// 몹 행동 타이머
	this->register_timer(1000, &acceptor::handle_mob_respawn);			// 몹 리젠 타이머
}

acceptor::~acceptor()
{
	for(auto i = this->_maps.begin(); i != this->_maps.end(); i++)
		delete i->second;

	for(auto i = this->_items.begin(); i != this->_items.end(); i++)
		delete i->second;

	for(auto i = this->_npcs.begin(); i != this->_npcs.end(); i++)
		delete i->second;

	for(auto i = this->_mobs.begin(); i != this->_mobs.end(); i++)
		delete i->second;

	for(auto i = this->_classes.begin(); i != this->_classes.end(); i++)
		delete *i;

	for(auto i = this->_itemmixes.begin(); i != this->_itemmixes.end(); i++)
		delete *i;
}

bool fb::game::acceptor::convert_map_file(const std::string& db_fname)
{
	Json::Value db_map;

	std::ifstream ifstream(db_fname);
	if(ifstream.is_open() == false)
		return false;

	std::string line;
	while(std::getline(ifstream, line))
	{
		if(isdigit(line[0]) == false)
			continue;

		std::stringstream sstream(line);
		
		std::string id;
		std::getline(sstream, id, '\t');

		std::string parent;
		std::getline(sstream, parent, '\t');

		std::string name;
		std::getline(sstream, name, '\t');

		std::string bgm;
		std::getline(sstream, bgm, '\t');

		std::string effect;
		std::getline(sstream, effect, '\t');

		std::string attr;
		std::getline(sstream, attr, '\t');

		db_map[id] = Json::Value();
		db_map[id]["name"] = name;
		db_map[id]["parent"] = parent;
		db_map[id]["bgm"] = std::stoi(bgm);

		uint32_t i_attr = std::stoi(attr);
		db_map[id]["option"]                        = Json::Value();
		db_map[id]["option"]["build in"]            =  (bool(i_attr & 0x01));
		db_map[id]["option"]["enabled talk"]        = !(bool(i_attr & 0x02));
		db_map[id]["option"]["enabled whisper"]     = !(bool(i_attr & 0x04));
		db_map[id]["option"]["enabled magic"]       = !(bool(i_attr & 0x08));
		db_map[id]["option"]["hunting ground"]      =  (bool(i_attr & 0x10));
		db_map[id]["option"]["enabled pk"]          =  (bool(i_attr & 0x20));
		db_map[id]["option"]["enabled die penalty"] =  (bool(i_attr & 0x30));

		uint32_t i_effect = std::stoi(effect);
		if(i_effect == 0x01)
			db_map[id]["effect"] = "hot wave";
		else if(i_effect == 0x02)
			db_map[id]["effect"] = "shipwreck";
	}
	ifstream.close();

	std::ofstream ofstream;
	ofstream.open("db/map.json");
	
	Json::StyledWriter writer;
	ofstream << writer.write(db_map);
	ofstream.close();
	return true;
}

bool fb::game::acceptor::convert_block_file(const map* map, const char* ext)
{
	char fname[256] = {0, };
	sprintf(fname, "map/%06d.txt", map->id());
	std::ifstream ifstream(fname);
	if(ifstream.is_open() == false)
		return false;

	sprintf(fname, "map/%06d.%s", map->id(), ext);
	std::ofstream ofstream(fname);
	if(ofstream.is_open() == false)
		return false;

	std::string line;
	Json::Value block_json(Json::arrayValue);
	while(std::getline(ifstream, line))
	{
		std::stringstream sstream(line);
		std::string token;
		std::getline(sstream, token, '\t');
		std::getline(sstream, token, '\t');
		uint16_t offset = std::stoi(token);

		Json::Value json_value;
		json_value["x"] = offset % map->width();
		json_value["y"] = offset / map->width();
		block_json.append(json_value);
	}

	Json::FastWriter writer;
	ofstream << writer.write(block_json);

	ifstream.close();
	ofstream.close();
	return true;
}

bool fb::game::acceptor::convert_item_file(const std::string& db_fname)
{
	Json::Value db_map;

	std::ifstream ifstream(db_fname);
	if(ifstream.is_open() == false)
		return false;

	std::ifstream ifstream_ext("db/item_db_ext.txt");
	if(ifstream_ext.is_open() == false)
		return false;
	Json::Value json_ext;
	Json::Reader reader;
	if(reader.parse(ifstream_ext, json_ext) == false)
		return false;

	ifstream_ext.close();
	

	std::string line;
	Json::Value db_item;
	while(std::getline(ifstream, line))
	{
		if(isdigit(line[0]) == false)
			continue;

		Json::Value json_value;
		std::stringstream sstream(line);

		std::string id;
		std::getline(sstream, id, '\t');

		std::string name;
		std::getline(sstream, name, '\t');
		json_value["name"] = name;

		if(name == "타라의남자옷")
		{
			std::cout << std::endl;
		}

		std::string types;
		std::getline(sstream, types, '\t');
		switch(std::stoi(types))
		{
		case 0:
			json_value["type"] = "weapon";
			break;

		case 1:
			json_value["type"] = "armor";
			break;

		case 2:
			json_value["type"] = "shield";
			break;

		case 3:
			json_value["type"] = "helmet";
			break;

		case 4:
			json_value["type"] = "ring";
			break;

		case 5:
			json_value["type"] = "auxiliary";
			break;

		case 6: // 동동주 같은거
			json_value["type"] = "consume";// 사용하면 사라진다.
											// bundle type이 반드시 필요
			json_value["bundle type"] = "package";  // 하나의 아이템에 다수개 소비
			break;

		case 7: // 토끼고기 같은거
			json_value["type"] = "consume"; // 사용하면 사라진다.
			json_value["bundle type"] = "bundle"; // 그냥 여러개를 가질 수 있음
			break;

		case 8: // script
			json_value["type"] = "stuff";
			json_value["bundle type"] = "default";
			json_value["available"] = true;
			break;

		case 9:
			json_value["type"] = "stuff";
			json_value["available"] = false;
			break;

		case 10: // 딱 하나만 가지면서 사용도 못함
			json_value["type"] = "stuff";
			json_value["available"] = false;
			break;

		case 11: // 여러개 가지면서 사용은 못함
			json_value["type"] = "stuff";
			json_value["bundle type"] = "bundle";
			json_value["available"] = false;
			break;

		case 12:
			json_value["type"] = "arrow"; // 기본으로 bundle type, consumable임
			break;

		default:
			std::cout << "what?? : " << types << std::endl;
			continue;
		}

		bool equipment = (json_value["type"] == "weapon" || 
			json_value["type"] == "armor" || 
			json_value["type"] == "shield" ||
			json_value["type"] == "helmet" ||
			json_value["type"] == "ring" ||
			json_value["type"] == "auxiliary");

		Json::Value equipment_option;

		std::string icon;
		std::getline(sstream, icon, '\t');
		json_value["icon"] = std::stoi(icon);

		std::string look;
		std::getline(sstream, look, '\t');
		if(equipment)
			equipment_option["look"] = std::stoi(look);

		std::string color;
		std::getline(sstream, color, '\t');
		if(std::stoi(color) != 0)
			json_value["color"] = std::stoi(color);

		std::string max_count;
		std::getline(sstream, max_count, '\t');
		if(std::stoi(max_count) == 0)
		{}
		else if(std::stoi(max_count) == 1 && json_value.isMember("bundle type") && json_value["bundle type"].asString() == "bundle")
		{
			json_value.removeMember("bundle type");
		}
		else
		{
			json_value["capacity"] = std::stoi(max_count);
		}

		std::string durability;
		std::getline(sstream, durability, '\t');
		if(std::stoi(durability) != 0)
		{
			if(std::stoi(types) == 6)
				json_value["capacity"] = std::stoi(durability);
			else
				equipment_option["durability"] = std::stoi(durability);
		}

		std::string price;
		std::getline(sstream, price, '\t');
		if(std::stoi(price) != 0)
			json_value["price"] = std::stoi(price);


		// 제한 형식
		Json::Value limit;
		std::string strength_limit;
		std::getline(sstream, strength_limit, '\t');
		if(std::stoi(strength_limit) != 0)
			limit["strength"] = std::stoi(strength_limit);

		std::string dexteritry_limit;
		std::getline(sstream, dexteritry_limit, '\t');
		if(std::stoi(dexteritry_limit) != 0)
			limit["dexteritry"] = std::stoi(dexteritry_limit);

		std::string intelligence_limit;
		std::getline(sstream, intelligence_limit, '\t');
		if(std::stoi(intelligence_limit) != 0)
			limit["intelligence"] = std::stoi(intelligence_limit);

		std::string sex_limit;
		std::getline(sstream, sex_limit, '\t');
		if(std::stoi(sex_limit) == 1)
			limit["sex"] = "man";
		else if(std::stoi(sex_limit) == 2)
			limit["sex"] = "woman";

		std::string level_limit;
		std::getline(sstream, level_limit, '\t');
		if(std::stoi(level_limit) != 0)
			limit["level"] = std::stoi(level_limit);

		std::string class_limit;
		std::getline(sstream, class_limit, '\t');
		if(std::stoi(class_limit) != 0)
			limit["level"] = std::stoi(class_limit);

		std::string promotion_limit;
		std::getline(sstream, promotion_limit, '\t');
		if(std::stoi(promotion_limit) != 0)
			limit["promotion"] = std::stoi(promotion_limit);

		if(limit.size() != 0)
			json_value["limit"] = limit;

		std::string die_penelty;
		std::getline(sstream, die_penelty, '\t');
		if(std::stoi(die_penelty) == 0)
			json_value["die penelty"] = "drop";
		else if(std::stoi(die_penelty) == 1)
			json_value["die penelty"] = "destroy";

		std::string tradable;
		std::getline(sstream, tradable, '\t');
		json_value["tradable"] = bool(std::stoi(tradable));

		std::string repairable;
		std::getline(sstream, repairable, '\t');
		if(equipment)
			equipment_option["repairable"] = bool(std::stoi(repairable));

		std::string repair_price;
		std::getline(sstream, repair_price, '\t');
		if(bool(std::stoi(repairable)) && std::stoi(repair_price) != 0)
			equipment_option["repair price"] = std::stoi(repair_price);

		std::string entrust_price;
		std::getline(sstream, entrust_price, '\t');
		if(std::stoi(entrust_price) != 0)
			json_value["entrust price"] = std::stoi(entrust_price);

		std::string rename_price;
		std::getline(sstream, rename_price, '\t');
		if(std::stoi(rename_price) != 0)
			equipment_option["rename price"] = std::stoi(rename_price);


		Json::Value script;
		std::string dress_script;
		std::getline(sstream, dress_script, '\t');
		if(dress_script != "-")
			script["active"] = dress_script;

		Json::Value random_damage;
		if(json_ext.isMember(dress_script))
		{
			if(json_ext[dress_script].isMember("s_mindam") && json_ext[dress_script].isMember("s_maxdam") && json_ext[dress_script].isMember("l_mindam") && json_ext[dress_script].isMember("l_maxdam"))
			{
				random_damage["small"] = Json::Value();
				random_damage["small"]["min"] = json_ext[dress_script]["s_mindam"].asInt();
				random_damage["small"]["max"] = json_ext[dress_script]["s_maxdam"].asInt();
				random_damage["large"] = Json::Value();
				random_damage["large"]["min"] = json_ext[dress_script]["l_mindam"].asInt();
				random_damage["large"]["max"] = json_ext[dress_script]["l_maxdam"].asInt();

				equipment_option["damage range"] = random_damage;
			}

			Json::Value defensive;
			if(json_ext[dress_script].isMember("defensive physical"))
				defensive["physical"] = json_ext[dress_script]["defensive physical"].asInt();

			if(json_ext[dress_script].isMember("defensive magical"))
				defensive["magical"] = json_ext[dress_script]["defensive magical"].asInt();

			if(defensive.size() != 0)
				equipment_option["defensive"] = defensive;

			if(json_ext[dress_script].isMember("hit"))
				equipment_option["hit"] = json_ext[dress_script]["hit"].asInt();

			if(json_ext[dress_script].isMember("damage"))
				equipment_option["damage"] = json_ext[dress_script]["damage"].asInt();

			if(json_ext[dress_script].isMember("strength"))
				equipment_option["strength"] = json_ext[dress_script]["strength"].asInt();

			if(json_ext[dress_script].isMember("intelligence"))
				equipment_option["intelligence"] = json_ext[dress_script]["intelligence"].asInt();

			if(json_ext[dress_script].isMember("dexteritry"))
				equipment_option["dexteritry"] = json_ext[dress_script]["dexteritry"].asInt();

			if(json_ext[dress_script].isMember("hp"))
				equipment_option["hp"] = json_ext[dress_script]["hp"].asInt();

			if(json_ext[dress_script].isMember("mp"))
				equipment_option["mp"] = json_ext[dress_script]["mp"].asInt();

			if(json_ext[dress_script].isMember("sound"))
				equipment_option["sound"] = json_ext[dress_script]["sound"].asInt();

			if(json_ext[dress_script].isMember("hp percent"))
				equipment_option["hp percent"] = json_ext[dress_script]["hp percent"].asInt();

			if(json_ext[dress_script].isMember("mp percent"))
				equipment_option["mp percent"] = json_ext[dress_script]["mp percent"].asInt();

			if(json_ext[dress_script].isMember("healing cycle"))
				equipment_option["healing cycle"] = json_ext[dress_script]["healing cycle"].asInt();

			if(json_ext[dress_script].isMember("spell"))
				equipment_option["spell"] = json_ext[dress_script]["spell"].asString();
		}

		std::string undress_script;
		std::getline(sstream, undress_script, '\t');
		if(undress_script != "-")
			script["inactive"] = undress_script;

		std::string action_script;
		std::getline(sstream, action_script, '\t');
		if(action_script != "-")
			script["active"] = action_script;
		if(script.size() != 0)
			json_value["script"] = script;

		std::string tooltip;
		std::getline(sstream, tooltip, '\t');
		if(tooltip != "-")
			json_value["tooltip"] = tooltip;

		std::string desc;
		std::getline(sstream, desc, '\t');
		if(desc != "-")
			json_value["desc"] = desc;

		if(equipment_option.size() != 0)
			json_value["equipment option"] = equipment_option;

		db_item[id] = json_value;
	}
	ifstream.close();

	std::ofstream ofstream;
	ofstream.open("db/item.json");

	Json::StyledWriter writer;
	ofstream << writer.write(db_item);
	ofstream.close();
	return true;
}

bool fb::game::acceptor::convert_npc_file(const std::string& db_fname)
{
	Json::Value db_map;

	std::ifstream ifstream(db_fname);
	if(ifstream.is_open() == false)
		return false;

	std::string line;
	Json::Value db_npc;
	char buffer[256];
	while(std::getline(ifstream, line))
	{
		if(isdigit(line[0]) == false)
			continue;

		Json::Value json_value;
		std::stringstream sstream(line);

		std::string id;
		std::getline(sstream, id, '\t');
		sprintf(buffer, "%05d", std::stoi(id));

		std::string name;
		std::getline(sstream, name, '\t');
		json_value["name"] = name;

		std::string look;
		std::getline(sstream, look, '\t');
		json_value["look"] = std::stoi(look);

		std::string color;
		std::getline(sstream, color, '\t');
		json_value["color"] = std::stoi(color);

		db_npc[buffer] = json_value;
	}
	ifstream.close();

	std::ofstream ofstream;
	ofstream.open("db/npc.json");

	Json::StyledWriter writer;
	ofstream << writer.write(db_npc);
	ofstream.close();
	return true;
}

bool fb::game::acceptor::convert_npc_spawn_file(const std::string& db_fname)
{
	Json::Value db_map;

	std::ifstream ifstream(db_fname);
	if(ifstream.is_open() == false)
		return false;

	std::string line;
	Json::Value db(Json::arrayValue);
	while(std::getline(ifstream, line))
	{
		if(isdigit(line[0]) == false)
			continue;

		Json::Value json_value;
		std::stringstream sstream(line);

		std::string id_str;
		std::getline(sstream, id_str, '\t');
		uint16_t id = std::stoi(id_str);

		if(this->_npcs.find(id) == this->_npcs.end())
		{
			std::cout << "invalid npc id : " << id << std::endl;
			continue;
		}
		npc::core* npc = this->_npcs[id];
		json_value["npc"] = npc->name();

		std::string map_id_str;
		std::getline(sstream, map_id_str, '\t');
		uint16_t map_id = std::stoi(map_id_str);
		if(this->_maps.find(map_id) == this->_maps.end())
		{
			std::cout << "invalid map id : " << map_id << std::endl;
			continue;
		}
		fb::game::map* map = this->_maps[map_id];
		json_value["map"] = map->name();

		std::string x;
		std::getline(sstream, x, '\t');

		std::string y;
		std::getline(sstream, y, '\t');
		json_value["position"] = Json::Value();
		json_value["position"]["x"] = std::stoi(x);
		json_value["position"]["y"] = std::stoi(y);

		std::string direction;
		std::getline(sstream, direction, '\t');
		switch(std::stoi(direction))
		{
		case 0:
			json_value["direction"] = "top";
			break;

		case 1:
			json_value["direction"] = "right";
			break;

		case 2:
			json_value["direction"] = "bottom";
			break;

		case 3:
			json_value["direction"] = "left";
			break;
		}

		std::string script;
		std::getline(sstream, script, '\t');
		json_value["script"] = script;

		db.append(json_value);
	}
	ifstream.close();

	std::ofstream ofstream;
	ofstream.open("db/npc_spawn.json");

	Json::StyledWriter writer;
	ofstream << writer.write(db);
	ofstream.close();
	return true;
}

bool fb::game::acceptor::convert_mob_file(const std::string& db_fname)
{
	Json::Value db_mob;

	std::ifstream ifstream(db_fname);
	if(ifstream.is_open() == false)
		return false;

	std::string line;
	Json::Value db;
	char buffer[256];
	while(std::getline(ifstream, line))
	{
		if(isdigit(line[0]) == false)
			continue;

		Json::Value json_value;
		std::stringstream sstream(line);

		std::string id;
		std::getline(sstream, id, '\t');
		sprintf(buffer, "%06d", std::stoi(id));

		std::string name;
		std::getline(sstream, name, '\t');
		json_value["name"] = name;

		std::string look;
		std::getline(sstream, look, '\t');
		json_value["look"] = std::stoi(look);

		std::string color;
		std::getline(sstream, color, '\t');
		json_value["color"] = std::stoi(color);

		std::string base_hp;
		std::getline(sstream, base_hp, '\t');
		json_value["hp"] = std::stoi(base_hp);

		std::string experience;
		std::getline(sstream, experience, '\t');
		json_value["experience"] = std::stoi(experience);

		json_value["defensive"] = Json::Value();

		std::string defensive_physical;
		std::getline(sstream, defensive_physical, '\t');
		json_value["defensive"]["physical"] = std::stoi(defensive_physical);

		std::string size;
		std::getline(sstream, size, '\t');
		if(size == "s")
			size = "small";
		else if(size == "l")
			size = "large";
		json_value["size"] = size;

		std::string offensive;
		std::getline(sstream, offensive, '\t');
		uint32_t offensive_int = std::stoi(offensive);
		//json_value["offensive"] = offensive;
		switch(offensive_int)
		{
		case 0:
			json_value["offensive"] = "containment";
			break;

		case 1:
			json_value["offensive"] = "counter";
			break;

		case 2:
			json_value["offensive"] = "none";
			break;

		case 3:
			json_value["offensive"] = "non move";
			break;

		case 4:
			json_value["offensive"] = "run away";
			break;

		default:
			json_value["offensive"] = "unknown";
			break;
		}

		std::string defensive_magical;
		std::getline(sstream, defensive_magical, '\t');
		json_value["defensive"]["magical"] = std::stoi(defensive_magical);

		std::string min;
		std::getline(sstream, min, '\t');

		std::string max;
		std::getline(sstream, max, '\t');

		json_value["damage"] = Json::Value();
		json_value["damage"]["min"] = std::stoi(min);
		json_value["damage"]["max"] = std::stoi(max);


		std::string script_attack;
		std::getline(sstream, script_attack, '\t');

		std::string script_die;
		std::getline(sstream, script_die, '\t');

		json_value["script"] = Json::Value();
		if(script_attack != "-")
			json_value["script"]["attack"] = script_attack;

		if(script_die != "-")
			json_value["script"]["die"] = script_die;

		if(json_value.size() == 0)
			json_value.removeMember("script");

		std::string speed;
		std::getline(sstream, speed, '\t');
		json_value["speed"] = std::stoi(speed);

		db[buffer] = json_value;
	}
	ifstream.close();

	std::ofstream ofstream;
	ofstream.open("db/mob.json");

	Json::StyledWriter writer;
	ofstream << writer.write(db);
	ofstream.close();
	return true;
}

bool fb::game::acceptor::convert_mob_spawn_file(const std::string& db_fname)
{
	std::ifstream ifstream(db_fname);
	if(ifstream.is_open() == false)
		return false;

	std::string line;
	Json::Value db;
	while(std::getline(ifstream, line))
	{
		if(isdigit(line[0]) == false)
			continue;

		Json::Value spawns;
		std::stringstream sstream(line);

		std::string mob_id_str;
		std::getline(sstream, mob_id_str, '\t');
		uint16_t mob_id = std::stoi(mob_id_str);

		if(this->_mobs.find(mob_id) == this->_mobs.end())
		{
			std::cout << "invalid mob id : " << mob_id << std::endl;
			continue;
		}
		fb::game::mob::core* mob_core = this->_mobs[mob_id];


		std::string map_id_str;
		std::getline(sstream, map_id_str, '\t');
		uint16_t map_id = std::stoi(map_id_str);

		if(this->_maps.find(map_id) == this->_maps.end())
		{
			std::cout << "invalid map id : " << map_id << std::endl;
			continue;
		}
		fb::game::map* map = this->_maps[map_id];

		std::string x0_str;
		std::getline(sstream, x0_str, '\t');
		uint16_t x0 = std::stoi(x0_str);

		std::string x1_str;
		std::getline(sstream, x1_str, '\t');
		uint16_t x1 = std::stoi(x1_str);

		std::string y0_str;
		std::getline(sstream, y0_str, '\t');
		uint16_t y0 = std::stoi(y0_str);

		std::string y1_str;
		std::getline(sstream, y1_str, '\t');
		uint16_t y1 = std::stoi(y1_str);

		std::string count_str;
		std::getline(sstream, count_str, '\t');
		uint16_t count = std::stoi(count_str);

		std::string rezen_str;
		std::getline(sstream, rezen_str, '\t');
		uint16_t rezen = std::stoi(rezen_str);

		if(db.isMember(map->name()))
			spawns = db[map->name()];
		else
			db[map->name()] = Json::Value(Json::arrayValue);

		Json::Value spawn;
		spawn["name"] = mob_core->name();
		spawn["area"] = Json::Value();
		spawn["area"]["x0"] = x0;
		spawn["area"]["x1"] = x1;
		spawn["area"]["y0"] = y0;
		spawn["area"]["y1"] = y1;
		spawn["count"] = count;
		spawn["rezen time"] = rezen;
		db[map->name()].append(spawn);
	}
	ifstream.close();

	std::ofstream ofstream;
	ofstream.open("db/mob_spawn.json");

	Json::StyledWriter writer;
	ofstream << writer.write(db);
	ofstream.close();
	return true;
}

bool fb::game::acceptor::convert_class_file(const std::string& db_fname)
{
	std::ifstream ifstream(db_fname);
	if(ifstream.is_open() == false)
		return false;

	std::string line;
	Json::Value db;
	while(std::getline(ifstream, line))
	{
		if(isdigit(line[0]) == false)
			continue;

		std::stringstream sstream(line);

		std::string class_id_str;
		std::getline(sstream, class_id_str, '\t');
		uint8_t class_id = std::stoi(class_id_str);

		std::string class_name;
		switch(class_id)
		{
		case 0:
			class_name = "평민";
			break;
		case 1:
			class_name = "전사";
			break;
		case 2:
			class_name = "도적";
			break;
		case 3:
			class_name = "주술사";
			break;
		case 4:
			class_name = "도사";
			break;
		}
		//const std::string* class_name = this->class2name(class_id, 0);
		//if(class_name == NULL)
		//	throw std::runtime_error("cannot find id");

		std::string level_str;
		std::getline(sstream, level_str, '\t');
		uint8_t level = std::stoi(level_str);

		std::string require_exp_str;
		std::getline(sstream, require_exp_str, '\t');
		uint32_t require_exp = std::strtoul(require_exp_str.c_str(), nullptr, 0);

		if(db.isMember(class_id_str) == false)
		{
			db[class_id_str] = Json::Value();
			db[class_id_str]["levels"] = Json::Value(Json::arrayValue);
			db[class_id_str]["promotions"] = Json::Value(Json::arrayValue);

			switch(class_id)
			{
			case 0:
				db[class_id_str]["promotions"].append("평민");
				break;

			case 1:
				db[class_id_str]["promotions"].append("전사");
				db[class_id_str]["promotions"].append("검객");
				db[class_id_str]["promotions"].append("검제");
				db[class_id_str]["promotions"].append("검황");
				db[class_id_str]["promotions"].append("검성");
				break;

			case 2:
				db[class_id_str]["promotions"].append("도적");
				db[class_id_str]["promotions"].append("자객");
				db[class_id_str]["promotions"].append("진검");
				db[class_id_str]["promotions"].append("귀객");
				db[class_id_str]["promotions"].append("태성");
				break;

			case 3:
				db[class_id_str]["promotions"].append("주술사");
				db[class_id_str]["promotions"].append("술사");
				db[class_id_str]["promotions"].append("현사");
				db[class_id_str]["promotions"].append("현인");
				db[class_id_str]["promotions"].append("현자");
				break;

			case 4:
				db[class_id_str]["promotions"].append("도사");
				db[class_id_str]["promotions"].append("도인");
				db[class_id_str]["promotions"].append("명인");
				db[class_id_str]["promotions"].append("진인");
				db[class_id_str]["promotions"].append("진선");
				break;
			}
		}

		Json::Value json_ability;
		json_ability["exp"] = require_exp;
		json_ability["strength"] = 0;
		json_ability["intelligence"] = 0;
		json_ability["dexteritry"] = 0;
		json_ability["hp"] = 0;
		json_ability["mp"] = 0;

		std::stringstream comment_stream;
		comment_stream << "// Level " << std::to_string(level);
		db[class_id_str]["levels"].append(json_ability).setComment(comment_stream.str(), Json::CommentPlacement::commentAfterOnSameLine);
	}

	ifstream.close();

	Json::Value db_array(Json::arrayValue);
	std::stringstream comment_stream;
	for(auto i = db.begin(); i != db.end(); i++)
	{
		comment_stream.str("");
		comment_stream << "// class id " << i.key().asString();
		db_array.append((*i)).setComment(comment_stream.str(), Json::CommentPlacement::commentAfterOnSameLine);
	}


	std::ofstream ofstream;
	ofstream.open("db/class.json");

	Json::StyledWriter writer;
	ofstream << writer.write(db_array);
	ofstream.close();

	return true;
}

bool fb::game::acceptor::load_maps(const std::string& db_fname)
{
	std::ifstream ifstream;
	ifstream.open(db_fname);
	if(ifstream.is_open() == false)
		return false;

	Json::Value json_maps;
	Json::Reader reader;
	if(reader.parse(ifstream, json_maps) == false)
		return false;
	ifstream.close();

	
	char fname[256] = {0, };
	for(auto i = json_maps.begin(); i != json_maps.end(); i++)
	{
		Json::Value json = *i;

		uint16_t            id      = std::stoi(i.key().asString());
		uint16_t            parent  = std::stoi(json["parent"].asString());
		uint8_t             bgm     = json["bgm"].asInt();
		std::string         name    = json["name"].asString();

		map::effects        effect;
		if(json.isMember("effect") == false)
			effect = map::effects::NO_EFFECT;
		else if(json["effect"].asString() == "hot wave")
			effect = map::effects::HOT_WAVE;
		else if(json["effect"].asString() == "shipwreck")
			effect = map::effects::SHIPWRECT;
		else
			effect = map::effects::NO_EFFECT;

		map::options option = map::options::NO_OPTION;
		if(json["option"]["build in"].asBool())
			option = (map::options)(option | map::options::BUILD_IN);

		if(json["option"]["enabled talk"].asBool() == false)
			option = (map::options)(option | map::options::DISABLE_TALK);

		if(json["option"]["enabled whisper"].asBool() == false)
			option = (map::options)(option | map::options::DISABLE_WHISPER);

		if(json["option"]["enabled magic"].asBool() == false)
			option = (map::options)(option | map::options::DISABLE_MAGIC);

		if(json["option"]["hunting ground"].asBool())
			option = (map::options)(option | map::options::HUNTING_GROUND);

		if(json["option"]["enabled pk"].asBool())
			option = (map::options)(option | map::options::ENABLE_PK);

		if(json["option"]["enabled die penalty"].asBool())
			option = (map::options)(option | map::options::DISABLE_DIE_PENALTY);

		sprintf(fname, "map/%06d.map", id);
		std::ifstream ifstream_map(fname, std::ios::binary);
		if(ifstream_map.is_open() == false)
			continue;

		std::vector<char> map_buffer(std::istreambuf_iterator<char>(ifstream_map), {});
		ifstream_map.close();

		try
		{
			map* created_map = new map(id, parent, bgm, name, option, effect, map_buffer.data(), map_buffer.size());
			//this->convert_block_file(created_map, "block");

			sprintf(fname, "map/%06d.block", id);
			std::ifstream ifstream_block(fname);
			if(ifstream_block.is_open() == false)
				throw std::runtime_error("cannot load block file");

			Json::Reader reader;
			Json::Value block_json;
			if(reader.parse(ifstream_block, block_json) == false)
				throw std::runtime_error("cannot parse block data");

			for(auto i = block_json.begin(); i != block_json.end(); i++)
				created_map->block((*i)["x"].asInt(), (*i)["y"].asInt(), true);

			ifstream_block.close();
			this->_maps.insert(std::make_pair(id, created_map));
		}
		catch(std::exception e)
		{
			std::cout << e.what() << std::endl;
		}
	}
	
	return true;
}

bool fb::game::acceptor::load_items(const std::string& db_fname)
{
	std::ifstream ifstream;
	ifstream.open(db_fname);
	if(ifstream.is_open() == false)
		return false;

	Json::Value json_items;
	Json::Reader reader;
	if(reader.parse(ifstream, json_items) == false)
		return false;
	ifstream.close();


	for(auto i = json_items.begin(); i != json_items.end(); i++)
	{
		Json::Value         json            = *i;

		uint16_t            code            = std::stoi(i.key().asString());
		std::string         name            = json["name"].asString();
		std::string         types           = json["type"].asString();
		
		uint16_t            icon            = json["icon"].asInt() + 0xBFFF;
		uint8_t             color           = json["color"].asInt();

		bool                trade           = json["tradable"].asBool();
		std::string         action_script   = json["action script"].asString();
		
		uint32_t            price           = json["price"].asInt();
		uint32_t            entrust			= json["entrust price"].asInt();

		std::string         tooltip         = json["tooltip"].asString();
		std::string         desc            = json["desc"].asString();

		bool                equipment       = types == "weapon" || types == "armor" || types == "helmet" || types == "ring" || types == "auxiliary" || types == "arrow";

		fb::game::item::item_limit limit;
		if(json.isMember("limit"))
		{
			if(json["limit"].isMember("level"))
				limit.level = json["limit"]["level"].asInt();

			if(json["limit"].isMember("strength"))
				limit.strength = json["limit"]["strength"].asInt();

			if(json["limit"].isMember("dexteritry"))
				limit.dexteritry = json["limit"]["dexteritry"].asInt();

			if(json["limit"].isMember("intelligence"))
				limit.intelligence = json["limit"]["intelligence"].asInt();

			if(json["limit"].isMember("cls"))
				limit.cls = json["limit"]["cls"].asInt();

			if(json["limit"].isMember("promotion"))
				limit.promotion = json["limit"]["promotion"].asInt();

			if(json["limit"].isMember("sex"))
			{
				std::string sex_limit = json["limit"]["sex"].asString();
				if(sex_limit == "man")
				{
					limit.sex = fb::game::sex::MAN;
				}
				else if(sex_limit == "woman")
				{
					limit.sex = fb::game::sex::WOMAN;
				}
				else
				{
					std::cout << "invalid sex limit : " << name << std::endl;
					continue;
				}
			}
		}

		fb::game::item::penalties penalty;
		if(json.isMember("die penelty") == false)
		{
			penalty = fb::game::item::penalties::NONE;
		}
		else if(json["die penelty"].asString() == "drop")
		{
			penalty = fb::game::item::penalties::DROP;
		}
		else if(json["die penelty"].asString() == "destroy")
		{
			penalty = fb::game::item::penalties::DESTRUCTION;
		}
		else
		{
			std::cout << "invalid die penelty : " << name << std::endl;
			continue;
		}

		std::string active_script;
		std::string inactive_script;
		if(json.isMember("script"))
		{
			if(json["script"].isMember("active"))
				active_script = json["script"]["active"].asString();

			if(json["script"].isMember("inactive"))
				inactive_script = json["script"]["inactive"].asString();
		}

		uint32_t look = 0;
		uint32_t durability = 0;
		bool repairable = true;
		uint32_t repair_price = 0;
		uint32_t rename_price = 0;

		uint32_t damage_small_min = 0, damage_small_max = 0;
		uint32_t damage_large_min = 0, damage_large_max = 0;
		uint8_t hit = 0, damage = 0;
		uint8_t sound = 0;
		uint8_t strength = 0, intelligence = 0, dexteritry = 0;
		uint32_t base_hp = 0, base_mp = 0;
		float hp_percent = 0.0f, mp_percent = 0.0f;
		uint8_t healing_cycle = 0;
		std::string spell;

		int16_t defensive_physical = 0, defensive_magical = 0;

		if(json.isMember("equipment option"))
		{
			look = json["equipment option"]["look"].asInt();
			durability = json["equipment option"]["durability"].asInt();
			repairable = json["equipment option"]["repairable"].asBool();
			repair_price = json["equipment option"]["repair price"].asInt();
			rename_price = json["equipment option"]["rename price"].asInt();

			if(json["equipment option"].isMember("damage range"))
			{
				damage_small_min = json["equipment option"]["damage range"]["small"]["min"].asInt();
				damage_small_max = json["equipment option"]["damage range"]["small"]["max"].asInt();

				damage_large_min = json["equipment option"]["damage range"]["large"]["min"].asInt();
				damage_large_max = json["equipment option"]["damage range"]["large"]["max"].asInt();
			}

			hit = json["equipment option"]["hit"].asInt();
			damage = json["equipment option"]["damage"].asInt();
			strength = json["equipment option"]["strength"].asInt();
			intelligence = json["equipment option"]["intelligence"].asInt();
			base_hp = json["equipment option"]["hp"].asInt();
			base_mp = json["equipment option"]["mp"].asInt();
			sound = json["equipment option"]["sound"].asInt();
			hp_percent = json["equipment option"]["hp percent"].asInt();
			mp_percent = json["equipment option"]["mp percent"].asInt();
			healing_cycle = json["equipment option"]["healing cycle"].asInt();
			spell = json["equipment option"]["spell"].asString();
			
			if(json["equipment option"].isMember("defensive"))
			{
				defensive_physical = json["equipment option"]["defensive"]["physical"].asInt();
				defensive_magical = json["equipment option"]["defensive"]["magical"].asInt();
			}
		}
		

		fb::game::item::core*               item = NULL;
		if(types == "stuff")
		{
			bool            available       = json["available"].asBool();       // default인 경우 일반아이템, bundle인 경우 일반번들
			std::string     bundle_type     = json["bundle type"].asString();

			item = new fb::game::item::core(name, icon, color);
			if(json.isMember("capacity"))
				item->capacity(json["capacity"].asInt());

			// price, limit, die_penalty, trade, entrust_price, tooltip, desc, active_script
		}
		else if(types == "consume")
		{
			std::string     bundle_type     = json["bundle type"].asString();   // package인 경우 동동주형식, bundle 형식인 경우 일반번들
			uint32_t        capacity        = json["capacity"].asInt();

			if(bundle_type == "package")
				item = new fb::game::pack::core(name, icon, color, capacity);
			else
				item = new fb::game::consume::core(name, icon, color, std::max(capacity, uint32_t(1)));
		}
		else if(types == "weapon")
		{
			item = new fb::game::weapon::core(name, icon, look, color);
		}
		else if(types == "armor")
		{
			item = new fb::game::armor::core(name, icon, look, color);
		}
		else if(types == "helmet")
		{
			item = new fb::game::helmet::core(name, icon, look, color);
		}
		else if(types == "shield")
		{
			item = new fb::game::shield::core(name, icon, look, color);
		}
		else if(types == "ring")
		{
			item = new fb::game::ring::core(name, icon, look, color);
		}
		else if(types == "auxiliary")
		{
			item = new fb::game::auxiliary::core(name, icon, look, color);
		}
		else if(types == "arrow")
		{
			item = new fb::game::arrow::core(name, icon, look, color);
		}
		else
		{
			std::cout << "cannot load item : " << name << std::endl;
			continue;
		}

		item->price(price);
		item->limit(limit);
		item->penalty(penalty);
		item->trade(trade);
		item->entrust(entrust);
		item->desc(desc);
		item->active_script(active_script);

		if(item->attr() & item::attrs::ITEM_ATTR_EQUIPMENT)
		{
			fb::game::equipment::core* equipment = static_cast<fb::game::equipment::core*>(item);
			equipment->durability(durability);
			equipment->repairable(repairable);
			equipment->repair_price(repair_price);
			equipment->rename_price(rename_price);
			equipment->hit(hit);
			equipment->damage(damage);
			equipment->strength(strength);
			equipment->intelligence(intelligence);
			equipment->dexteritry(dexteritry);
			equipment->base_hp(base_hp);
			equipment->base_mp(base_mp);
			equipment->hp_percentage(hp_percent);
			equipment->mp_percentage(mp_percent);
			equipment->healing_cycle(healing_cycle);
			equipment->defensive_physical(defensive_physical);
			equipment->defensive_magical(defensive_magical);
		}

		if(item->attr() & item::attrs::ITEM_ATTR_WEAPON)
		{
			weapon::core* weapon = static_cast<fb::game::weapon::core*>(item);
			weapon->damage_small(damage_small_min, damage_small_max);
			weapon->damage_large(damage_large_min, damage_large_max);
			weapon->sound(sound);
			weapon->spell(spell);
		}

		this->_items.insert(std::make_pair(code, item));
	}

	return true;
}

bool fb::game::acceptor::load_npc(const std::string& db_fname)
{
	std::ifstream ifstream;
	ifstream.open(db_fname);
	if(ifstream.is_open() == false)
		return false;

	Json::Value json_npc;
	Json::Reader reader;
	if(reader.parse(ifstream, json_npc) == false)
		return false;
	ifstream.close();


	for(auto i = json_npc.begin(); i != json_npc.end(); i++)
	{
		uint16_t            key = std::stoi(i.key().asString());
		Json::Value         json = *i;

		std::string         name = json["name"].asString();
		uint16_t            look = json["look"].asInt() + 0x7FFF;
		uint8_t             color = json["color"].asInt();

		this->_npcs.insert(std::make_pair(key, new npc::core(name, look, color)));
	}

	return true;
}

bool fb::game::acceptor::load_npc_spawn(const std::string& db_fname)
{
	std::ifstream ifstream;
	ifstream.open(db_fname);
	if(ifstream.is_open() == false)
		return false;

	Json::Value json_npc;
	Json::Reader reader;
	if(reader.parse(ifstream, json_npc) == false)
		return false;
	ifstream.close();


	for(auto i = json_npc.begin(); i != json_npc.end(); i++)
	{
		Json::Value         json = *i;

		std::string         npc_name = json["npc"].asString();
		npc::core*			core = this->name2npc(npc_name);
		if(core == NULL)
		{
			std::cout << "존재하지 않는 NPC입니다. : " << npc_name << std::endl;
			continue;
		}


		std::string         map_name = json["map"].asString();
		map*                map = this->name2map(map_name);
		if(map == NULL)
		{
			std::cout << "존재하지 않는 맵입니다. : " << map_name << std::endl;
			continue;
		}

		std::string         direction_str = json["direction"].asString();
		fb::game::direction direction = fb::game::direction::BOTTOM;
		if(direction_str == "top")
			direction = fb::game::direction::TOP;
		else if(direction_str == "right")
			direction = fb::game::direction::RIGHT;
		else if(direction_str == "bottom")
			direction = fb::game::direction::BOTTOM;
		else if(direction_str == "left")
			direction = fb::game::direction::LEFT;
		else
		{
			std::cout << "NPC의 방향이 올바르지 않습니다. : " << npc_name << std::endl;
			continue;
		}

		point16_t           position(json["position"]["x"].asInt(), json["position"]["y"].asInt());
		std::string         script = json["script"].asString();

		fb::game::npc*      cloned = new npc(core);
		cloned->direction(direction);
		cloned->map(map);
		cloned->position(position);
		cloned->script(script);
	}
	return true;
}

bool fb::game::acceptor::load_mob(const std::string& db_fname)
{
	std::ifstream ifstream;
	ifstream.open(db_fname);
	if(ifstream.is_open() == false)
		return false;

	Json::Value json_npc;
	Json::Reader reader;
	if(reader.parse(ifstream, json_npc) == false)
		return false;
	ifstream.close();


	for(auto i = json_npc.begin(); i != json_npc.end(); i++)
	{
		uint16_t            key = std::stoi(i.key().asString());
		Json::Value         json = *i;

		std::string         name = json["name"].asString();
		uint16_t            look = json["look"].asInt() + 0x7FFF;
		uint8_t             color = json["color"].asInt();
		uint16_t            damage_min = json["damage"]["min"].asInt();
		uint16_t            damage_max = json["damage"]["max"].asInt();
		uint16_t            defensive_physical = json["defensive"]["physical"].asInt();
		uint16_t            defensive_magical = json["defensive"]["magical"].asInt();
		uint32_t            experience = json["experience"].asInt();
		uint32_t            base_hp = json["hp"].asInt();
		uint32_t            speed = json["speed"].asInt();

		std::string			size_str = json["size"].asString();
		mob::sizes			size;
		if(size_str == "small")
			size = mob::sizes::SMALL;
		else if(size_str == "large")
			size = mob::sizes::LARGE;
		else
		{
			std::cout << "invalid mob size : " << key << std::endl;
			continue;
		}

		std::string			offensive = json["offensive"].asString();
		mob::offensive_type	offensive_type;
		if(offensive == "containment")
			offensive_type = mob::offensive_type::CONTAINMENT;
		else if(offensive == "counter")
			offensive_type = mob::offensive_type::COUNTER;
		else if(offensive == "none")
			offensive_type = mob::offensive_type::NONE;
		else if(offensive == "non move")
			offensive_type = mob::offensive_type::NON_MOVE;
		else if(offensive == "run away")
			offensive_type = mob::offensive_type::RUN_AWAY;
		else
		{
			std::cout << "invalid mob offensive type : " << key << std::endl;
			continue;
		}

		std::string script_attack, script_die;
		if(json.isMember("script"))
		{
			if(json["script"].isMember("attack"))
				script_attack = json["script"]["attack"].asString();

			if(json["script"].isMember("die"))
				script_die = json["script"]["die"].asString();
		}

		mob::core* mob = new fb::game::mob::core(name, look, color, base_hp, 0);
		mob->defensive_physical(defensive_physical);
		mob->defensive_magical(defensive_magical);
		mob->experience(experience);
		mob->damage_min(damage_min);
		mob->damage_max(damage_max);
		mob->offensive(offensive_type);
		mob->size(size);
		mob->speed(speed);
		mob->script_attack(script_attack);
		mob->script_die(script_die);

		this->_mobs.insert(std::make_pair(key, mob));
	}

	return true;
}

bool fb::game::acceptor::load_mob_spawn(const std::string& db_fname)
{
	std::ifstream ifstream;
	ifstream.open(db_fname);
	if(ifstream.is_open() == false)
		return false;

	Json::Value db;
	Json::Reader reader;
	if(reader.parse(ifstream, db) == false)
		return false;
	ifstream.close();


	for(auto db_i = db.begin(); db_i != db.end(); db_i++)
	{
		std::string         map_name = db_i.key().asString();
		Json::Value         spawns = *db_i;

		fb::game::map* map = this->name2map(map_name);
		if(map == NULL)
			continue;

		for(auto spawn_i = spawns.begin(); spawn_i != spawns.end(); spawn_i++)
		{
			Json::Value		spawn = *spawn_i;

			mob::core*		core = this->name2mob(spawn["name"].asString());
			if(core == NULL)
				continue;

			uint16_t		x0 = spawn["area"]["x0"].asInt();
			uint16_t		x1 = spawn["area"]["x1"].asInt();
			uint16_t		y0 = spawn["area"]["y0"].asInt();
			uint16_t		y1 = spawn["area"]["y1"].asInt();
			uint16_t		count = spawn["count"].asInt();
			uint32_t		rezen = spawn["rezen time"].asInt();

			for(int i = 0; i < count; i++)
			{
				mob*		mob = static_cast<fb::game::mob*>(core->make());
				mob->spawn_point(x0, y0);
				mob->spawn_size(x1, y1);
				mob->respawn_time(rezen);
				mob->map(map);
			}
		}
	}
	return true;
}

bool fb::game::acceptor::load_class(const std::string& db_fname)
{
	std::ifstream ifstream;
	ifstream.open(db_fname);
	if(ifstream.is_open() == false)
		return false;

	Json::Value db;
	Json::Reader reader;
	if(reader.parse(ifstream, db) == false)
		return false;
	ifstream.close();

	for(auto i1 = db.begin(); i1 != db.end(); i1++)
	{
		uint8_t class_id = i1.key().asInt();
		class_data* cdata = new class_data();
		
		Json::Value levels = (*i1)["levels"];
		for(auto i2 = levels.begin(); i2 != levels.end(); i2++)
		{
			uint32_t key = i2.key().asInt();

			Json::Value ability = *i2;

			std::cout << ability.toStyledString() << std::endl;

			level_ability* allocated = new level_ability(ability["strength"].asInt(),
				ability["intelligence"].asInt(),
				ability["dexteritry"].asInt(),
				ability["hp"].asInt(),
				ability["mp"].asInt(),
				ability["exp"].asInt64());

			cdata->add_level_ability(allocated);
		}

		Json::Value promotions = (*i1)["promotions"];
		for(auto i2 = promotions.begin(); i2 != promotions.end(); i2++)
		{
			cdata->add_promotion((*i2).asString());
		}


		this->_classes.push_back(cdata);
	}

	return true;
}

bool fb::game::acceptor::load_drop_item(const std::string& db_fname)
{
	std::ifstream ifstream(db_fname);
	if(ifstream.is_open() == false)
		return false;

	Json::Value db;
	Json::Reader reader;
	if(reader.parse(ifstream, db) == false)
		return false;

	for(auto i1 = db.begin(); i1 != db.end(); i1++)
	{
		std::string mob_name = i1.key().asString();
		auto mob_core = this->name2mob(mob_name);
		if(mob_core == NULL)
		{
			std::cout << "잘못된 이름입니다. : " << mob_name << std::endl;
			continue;
		}

		if(mob_core->name() == "다람쥐")
			std::cout << std::endl;

		Json::Value items = (*i1);
		for(auto i2 = items.begin(); i2 != items.end(); i2++)
		{
			float percentage = (*i2)["percentage"].asFloat();
			std::string item_name = (*i2)["item"].asString();
			auto item_core = this->name2item(item_name);

			if(item_core == NULL)
			{
				std::cout << "잘못된 아이템 이름입니다. : " << mob_name << std::endl;
				continue;
			}

			mob_core->dropitem_add(item_core, percentage);
		}
	}

	ifstream.close();
	return true;
}

bool fb::game::acceptor::load_warp(const std::string& db_fname)
{
	std::ifstream ifstream(db_fname);
	if(ifstream.is_open() == false)
		return false;

	Json::Value db;
	Json::Reader reader;
	if(reader.parse(ifstream, db) == false)
		return false;

	for(auto i1 = db.begin(); i1 != db.end(); i1++)
	{
		std::string map_name = i1.key().asString();
		fb::game::map* map = this->name2map(map_name);
		if(map == NULL)
			continue;

		Json::Value warps = *i1;
		for(auto i2 = warps.begin(); i2 != warps.end(); i2++)
		{
			std::string next_map_name = (*i2)["map"].asString();
			fb::game::map* next_map = this->name2map(next_map_name);
			if(next_map == NULL)
				continue;

			const point16_t before((*i2)["before"]["x"].asInt(), (*i2)["before"]["y"].asInt());
			const point16_t after((*i2)["after"]["x"].asInt(), (*i2)["after"]["y"].asInt());
			const range8_t limit((*i2)["limit"]["min"].asInt(), (*i2)["limit"]["max"].asInt());

			map->warp_add(next_map, before, after, limit);
		}
	}

	ifstream.close();
	return true;
}

bool fb::game::acceptor::load_itemmix(const std::string& db_fname)
{
	std::ifstream ifstream(db_fname);
	if(ifstream.is_open() == false)
		return false;

	Json::Value db;
	Json::Reader reader;
	if(reader.parse(ifstream, db) == false)
		return false;

	for(auto i1 = db.begin(); i1 != db.end(); i1++)
	{
		
		float percentage = (*i1)["percentage"].asDouble();
		itemmix* itemmix = new fb::game::itemmix(percentage);

		Json::Value require = (*i1)["require"];
		for(auto i2 = require.begin(); i2 != require.end(); i2++)
		{
			item::core* item = this->name2item((*i2)["item"].asString());
			uint32_t count = (*i2)["count"].asInt();
			itemmix->require_add(item, count);
		}

		Json::Value success = (*i1)["success"];
		for(auto i2 = success.begin(); i2 != success.end(); i2++)
		{
			item::core* item = this->name2item((*i2)["item"].asString());
			uint32_t count = (*i2)["count"].asInt();
			itemmix->success_add(item, count);
		}

		Json::Value failed = (*i1)["failed"];
		for(auto i2 = failed.begin(); i2 != failed.end(); i2++)
		{
			item::core* item = this->name2item((*i2)["item"].asString());
			uint32_t count = (*i2)["count"].asInt();
			itemmix->failed_add(item, count);
		}

		this->_itemmixes.push_back(itemmix);
	}

	ifstream.close();
	return true;
}

fb::game::map* fb::game::acceptor::name2map(const std::string& name) const
{
	for(auto i = this->_maps.begin(); i != this->_maps.end(); i++)
	{
		if(i->second->name() == name)
			return i->second;
	}

	return NULL;
}

fb::game::npc::core* fb::game::acceptor::name2npc(const std::string& name) const
{
	for(auto i = this->_npcs.begin(); i != this->_npcs.end(); i++)
	{
		if(i->second->name() == name)
			return i->second;
	}

	return NULL;
}

fb::game::mob::core* fb::game::acceptor::name2mob(const std::string& name) const
{
	for (auto i = this->_mobs.begin(); i != this->_mobs.end(); i++)
	{
		if (i->second->name() == name)
			return i->second;
	}

	return NULL;
}

fb::game::item::core* fb::game::acceptor::name2item(const std::string& name) const
{
	for (auto i = this->_items.begin(); i != this->_items.end(); i++)
	{
		if (i->second->name() == name)
			return i->second;
	}

	return NULL;
}

const std::string* fb::game::acceptor::class2name(uint8_t cls, uint8_t promotion) const
{
	try
	{
		return &this->_classes[cls]->promotions[promotion];
	}
	catch(std::exception& e)
	{
		return NULL;
	}
}

bool fb::game::acceptor::name2class(const std::string& name, uint8_t* class_id, uint8_t* promotion_id) const
{
	for(int i1 = 0; i1 < this->_classes.size(); i1++)
	{
		for(int i2 = 0; i2 < this->_classes[i1]->promotions.size(); i2++)
		{
			if(this->_classes[i1]->promotions[i2] != name)
				continue;

			*class_id = i1;
			*promotion_id = i2;
			return true;
		}
	}

	return false;
}

itemmix* fb::game::acceptor::find_itemmix(const std::vector<item*>& items)
{
	for(auto i = this->_itemmixes.begin(); i != this->_itemmixes.end(); i++)
	{
		itemmix* itemmix = *i;
		if(itemmix->require.size() != items.size())
			continue;

		if(itemmix->matched(items))
			return itemmix;
	}

	return NULL;
}

uint32_t fb::game::acceptor::required_exp(uint8_t class_id, uint8_t level)
{
	try
	{
		return this->_classes[class_id]->level_abilities[level]->exp;
	}
	catch(std::exception& e)
	{
		return 0;
	}
}

bool acceptor::handle_connected(fb::game::session& session)
{
	session.map(this->_maps[315]);
	session.name("채승현");
	session.position(point16_t(6, 8));
	session.look(0x61);
	session.color(0x0A);
	session.money(150);
	session.sex(fb::game::sex::MAN);
	session.legends_add(0x4A, 0x10, "갓승현 ㅋㅋ");
	session.legends_add(0x4A, 0x10, "똥진영 ㅋㅋ");
	session.base_hp(0xFFFFFFFF);
	session.hp(0xFFFFFFFF);
	session.title("갓승현 타이틀");

	session.item_add(static_cast<fb::game::item*>(this->_items[1015]->make())); // 정화의방패
	session.item_add(static_cast<fb::game::item*>(this->_items[243]->make())); // 도씨검
	session.item_add(static_cast<fb::game::item*>(this->_items[698]->make())); // 기모노
	session.item_add(static_cast<fb::game::item*>(this->_items[3014]->make())); // 도토리
	session.item_add(static_cast<fb::game::item*>(this->_items[2200]->make())); // 동동주

	// 착용한 상태로 설정 (내구도 등 변할 수 있는 내용들은 저장해둬야 함)
	session.weapon(static_cast<fb::game::weapon*>(this->_items[15]->make())); // 초심자의 목도
	session.helmet(static_cast<fb::game::helmet*>(this->_items[1340]->make()));
	session.ring(static_cast<fb::game::ring*>(this->_items[1689]->make()));
	session.ring(static_cast<fb::game::ring*>(this->_items[1689]->make()));
	session.auxiliary(static_cast<fb::game::auxiliary*>(this->_items[2135]->make()));
	session.auxiliary(static_cast<fb::game::auxiliary*>(this->_items[2129]->make()));

	return true;
}

bool acceptor::handle_disconnected(fb::game::session& session)
{
	return false;
}

void fb::game::acceptor::handle_timer(uint64_t elapsed_milliseconds)
{
	for(auto i = this->_maps.begin(); i != this->_maps.end(); i++)
		i->second->handle_timer(elapsed_milliseconds);
}

fb::ostream fb::game::acceptor::make_time_stream()
{
	fb::ostream             ostream;
	uint8_t                 hours = 25;
	ostream.write_u8(0x20)      // cmd : 0x20
		   .write_u8(hours%24)  // hours
		   .write_u8(0x00)      // Unknown
		   .write_u8(0x00);     // Unknown

	return ostream;
}

fb::ostream fb::game::acceptor::make_message_stream(const std::string& message, fb::game::message_types types)
{
	fb::ostream             ostream;
	ostream.write_u8(0x0A)
		.write_u8(types)
		.write_u16((uint16_t)message.length())
		.write(message.c_str(), message.length() + 1);

	return ostream;
}

fb::ostream fb::game::acceptor::make_dialog_stream(const std::string& message, bool enabled_prev, bool enabled_next)
{
	fb::ostream             ostream;
	uint16_t                look = 0xC012;
	uint8_t                 color = 0x04;
	uint8_t                 icon_type = look < 0xC000 ? 0x01 : 0x02;

	ostream.write_u8(0x30)
		.write_u8(0x00)
		.write_u8(0x00)
		.write_u8(0x00)
		.write_u8(0x00)
		.write_u16(0x01) // id인듯
		.write_u8(icon_type)
		.write_u8(0x01) // fixed
		.write_u16(look) // icon
		.write_u8(color) // color
		.write_u8(icon_type)
		.write_u16(look) // icon
		.write_u8(color) // color
		.write_u32(0x00000001)
		.write_u8(enabled_prev)
		.write_u8(enabled_next)
		.write_u16((uint16_t)message.length())
		.write(message.c_str(), message.length() + 1);

	return ostream;
}

void fb::game::acceptor::send_stream(object& object, const fb::ostream& stream, acceptor::scope scope, bool exclude_self, bool encrypt)
{
	switch(scope)
	{
	case acceptor::scope::SELF:
		__super::send_stream(static_cast<fb::game::session&>(object), stream, encrypt);
		break;

	case acceptor::scope::PIVOT:
	{
		std::vector<fb::game::session*> sessions = object.looking_sessions();
		if(!exclude_self)
			__super::send_stream(static_cast<fb::game::session&>(object), stream, encrypt);

		for(auto i = sessions.begin(); i != sessions.end(); i++)
			__super::send_stream(**i, stream, encrypt);
	}
		break;
	}
}

bool fb::game::acceptor::handle_move_life(fb::game::life* life, fb::game::direction direction)
{
	if(life->alive() == false)
		return false;

	point16_t					before = life->position();
	fb::ostream					move_stream = life->make_move_stream(direction);
	bool						result = false;

	try
	{
		std::vector<fb::game::session*> shown_sessions, hidden_sessions;

		if(life->move(direction, NULL, NULL, NULL, NULL, NULL, NULL, &shown_sessions, &hidden_sessions) == false)
			throw std::exception();

		for(auto i = shown_sessions.begin(); i != shown_sessions.end(); i++)
			this->send_stream(**i, life->make_show_stream(), scope::SELF);

		for(auto i = hidden_sessions.begin(); i != hidden_sessions.end(); i++)
			this->send_stream(**i, life->make_hide_stream(), scope::SELF);

		std::vector<fb::game::session*>& sessions = life->map()->sessions();
		for(auto i = sessions.begin(); i != sessions.end(); i++)
			this->send_stream(**i, move_stream, scope::SELF);

		result = true;
	}
	catch(std::exception&)
	{
		result = false;
	}

	if(life->direction() != direction)
	{
		life->direction(direction);
		this->send_stream(*life, life->make_direction_stream(), scope::PIVOT, true);
	}

	return result;
}

void fb::game::acceptor::handle_attack_mob(fb::game::session& session, fb::game::mob& mob, uint32_t random_damage)
{
	std::stringstream sstream;

	try
	{
		map* map = mob.map();

		// 몹 체력 깎고 체력게이지 표시
		mob.hp_down(random_damage);
		this->send_stream(mob, mob.make_show_hp_stream(random_damage, true), scope::PIVOT, true);

		

		// 맞고도 살아있으면 더 이상 진행할 거 없음
		if(mob.alive())
		{
			if(mob.offensive() != mob::offensive_type::NONE)
				mob.target(&session);

			return;
		}

		// 몹 체력을 다 깎았으면 죽인다.
		this->send_stream(mob, mob.make_die_stream(), scope::PIVOT, true);
		mob.dead_time(::GetTickCount64());

		// 드롭 아이템 떨구기
		const std::vector<mob::drop>& items = mob.items();
		std::vector<object*> dropped_items;
		for(auto i = items.begin(); i != items.end(); i++)
		{
			if(std::rand() % 100 > (*i).percentage)
				continue;

			item* item = static_cast<fb::game::item*>((*i).item->make());
			item->map(map);
			item->position(mob.position());

			dropped_items.push_back(item);
		}

		if(dropped_items.size() != 0)
			this->send_stream(mob, object::make_show_stream(dropped_items), scope::PIVOT, true);

		// 레벨 5면서 직업 안가졌으면 직업 가지라고 한다.
		if(session.level() >= 5 && session.cls() == 0)
			throw std::runtime_error("직업골라 씨발롬아");

		// 경험치는 최대 3.3%로 제한하여 얻는다.
		uint32_t require = session.max_level() ? 0xFFFFFFFF : this->required_exp(session.cls(), session.level()+1) - this->required_exp(session.cls(), session.level());
#if defined DEBUG | defined _DEBUG
		uint32_t limit_exp = require;
#else
		uint32_t limit_exp = max_level ? mob.experience() : std::min(uint32_t(require / 100.0f*3.3f + 1), mob.experience());
#endif
		session.experience_add(limit_exp);

		uint32_t exp = session.experience();
		uint32_t next_exp = this->required_exp(session.cls(), session.level()+1);

		if(exp >= next_exp && session.level_up())
		{
			session.strength_up(this->_classes[session.cls()]->level_abilities[session.level()]->strength);
			session.intelligence_up(this->_classes[session.cls()]->level_abilities[session.level()]->intelligence);
			session.dexteritry_up(this->_classes[session.cls()]->level_abilities[session.level()]->dexteritry);
			
			session.base_hp_up(this->_classes[session.cls()]->level_abilities[session.level()]->base_hp + std::rand() % 10);
			session.base_mp_up(this->_classes[session.cls()]->level_abilities[session.level()]->base_mp + std::rand() % 10);

			session.hp(session.base_hp());
			session.mp(session.base_mp());

			sstream << "레벨이 올랐습니다.";
			this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
			this->send_stream(session, session.make_effet_stream(0x02), scope::PIVOT);
		}
		else
		{
			float percentage = 0.0f;
			uint32_t prev_exp = this->required_exp(session.cls(), session.level());
			if(exp > prev_exp)
				percentage = ((exp - prev_exp) / float(next_exp - prev_exp)) * 100;

			sstream << "경험치가 " << limit_exp << '(' << int(percentage) << "%) 올랐습니다.";

			this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
		}
	}
	catch(std::exception& e)
	{
		sstream << e.what();
	}

	this->send_stream(session, this->make_message_stream(sstream.str(), fb::game::message_types::MESSAGE_STATE), scope::SELF);
}

bool acceptor::handle_login(fb::game::session& session)
{
	fb::istream&            istream = session.in_stream();
	uint8_t                 enc_key[0x09] = {0,};

	uint8_t cmd = istream.read_u8();
	uint8_t enc_type = istream.read_u8();
	uint8_t key_size = istream.read_u8();
	istream.read(enc_key, key_size);
	session.crt(enc_type, enc_key);

	fb::ostream             ostream;
	ostream.write_u8(0x1E)
		   .write_u8(0x06)
		   .write_u8(0x00);
	this->send_stream(session, ostream, scope::SELF);
	
	this->send_stream(session, this->make_time_stream(), scope::SELF);

	this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);

	this->send_stream(session, this->make_message_stream("0시간 1분만에 바람으로", fb::game::message_types::MESSAGE_STATE), scope::SELF);

	this->send_stream(session, session.make_id_stream(), scope::SELF);

	this->send_stream(session, session.map()->make_config_stream(), scope::SELF);

	this->send_stream(session, session.map()->make_bgm_stream(), scope::SELF);

	this->send_stream(session, session.make_position_stream(), scope::SELF);

	this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);

	this->send_stream(session, session.make_show_objects_stream(), scope::SELF);

	this->send_stream(session, session.make_visual_stream(false), scope::PIVOT);
	this->send_stream(session, session.make_direction_stream(), scope::SELF);
	this->send_stream(session, session.make_option_stream(), scope::SELF);

	std::vector<fb::game::session*> sessions = session.looking_sessions();
	for(auto i = sessions.begin(); i != sessions.end(); i++)
	{
		fb::game::session* other = (*i);
		if(other == &session)
			continue;

		this->send_stream(session, other->make_visual_stream(false), scope::SELF);
	}

	for(int i = 0; i < fb::game::item::MAX_ITEM_SLOT; i++)
		this->send_stream(session, session.make_update_item_slot_stream(i), scope::SELF);

	this->send_stream(session, this->make_dialog_stream("안녕하세요?", false, false), scope::SELF);

	return true;
}

bool fb::game::acceptor::handle_direction(fb::game::session& session)
{
	fb::istream&			istream = session.in_stream();
	uint8_t					cmd = istream.read_u8();
	fb::game::direction		direction = fb::game::direction(istream.read_u8());

	if(session.direction(direction) == false)
		return false;

	this->send_stream(session, session.make_direction_stream(), scope::PIVOT, true);
	return true;
}

bool fb::game::acceptor::handle_move(fb::game::session& session)
{
	fb::istream&            istream = session.in_stream();

	fb::game::map*          map = session.map();
	if(map == NULL)
		return false;

	uint8_t                 cmd = istream.read_u8();
	fb::game::direction     direction = fb::game::direction(istream.read_u8());
	uint8_t                 sequence = istream.read_u8();
	uint16_t                x = istream.read_u16();
	uint16_t                y = istream.read_u16();

	session.direction(direction);

	point16_t               before(x, y);
	if(session.position() != before)
		this->send_stream(session, session.make_position_stream(), scope::SELF);

	std::vector<object*> show_objects, hide_objects;
	std::vector<fb::game::session*> show_sessions, hide_sessions, shown_sessions, hidden_sessions;
	if(session.movable_forward())
	{
		this->send_stream(session, session.make_move_stream(), scope::PIVOT, true);
		session.move_forward(&show_objects, &hide_objects, &show_sessions, &hide_sessions, NULL, NULL, &shown_sessions, &hidden_sessions);
	}
	else
	{
		this->send_stream(session, session.make_position_stream(), scope::SELF);
		return true;
	}


	// 워프 위치라면 워프한다.
	const map::warp*		warp = map->warpable(session.position());
	if(warp != NULL)
	{
		this->handle_session_warp(session, warp);
		return true;
	}
	
	// 오브젝트 갱신
	this->send_stream(session, object::make_show_stream(show_objects), scope::SELF);
	for(auto i = hide_objects.begin(); i != hide_objects.end(); i++)
		this->send_stream(session, (*i)->make_hide_stream(), scope::SELF);

	// 움직인 세션에게 새로 보이는 세션 갱신
	for(auto i = show_sessions.begin(); i != show_sessions.end(); i++)
		this->send_stream(session, (*i)->make_visual_stream(false), scope::SELF);

	// 움직인 세션에게 사라진 세션 갱신
	for(auto i = hide_sessions.begin(); i != hide_sessions.end(); i++)
		this->send_stream(session, (*i)->make_hide_stream(), scope::SELF);

	for(auto i = shown_sessions.begin(); i != shown_sessions.end(); i++)
		this->send_stream(**i, session.make_visual_stream(false), scope::SELF);

	for(auto i = hidden_sessions.begin(); i != hidden_sessions.end(); i++)
		this->send_stream(**i, session.make_hide_stream(), scope::SELF);

	return true;
}

bool fb::game::acceptor::handle_update_move(fb::game::session& session)
{
	if(this->handle_move(session) == false)
		return false;

	fb::istream&            istream = session.in_stream();
	uint16_t                begin_x = istream.read_u16();
	uint16_t                begin_y = istream.read_u16();
	uint8_t                 width = istream.read_u8();
	uint8_t                 height = istream.read_u8();
	uint16_t                crc = istream.read_u16();
	this->send_stream(session, session.map()->make_update_stream(begin_x, begin_y, width, height, crc), scope::SELF);
	return true;
}

bool fb::game::acceptor::handle_attack(fb::game::session& session)
{
	this->send_stream(session, session.make_action_stream(action::ATTACK, duration::DURATION_ATTACK), scope::PIVOT);
	fb::game::weapon*		weapon = session.weapon();
	if(weapon != NULL)
	{
		uint16_t				sound = weapon->sound();
		if(sound != 0)
			this->send_stream(session, session.make_sound_stream(fb::game::action_sounds(sound)), scope::PIVOT);
	}
	else
	{
		// 이거 무슨 사운드인지 확인하도록 하자
		this->send_stream(session, session.make_sound_stream(fb::game::action_sounds(0x015D)), scope::PIVOT);
	}

	object*		front = session.forward_object(object::types::MOB);
	if(front == NULL)
		return true;

	fb::game::mob*			front_mob = static_cast<fb::game::mob*>(front);

#if !defined DEBUG && !defined _DEBUG
	if(std::rand() % 3 == 0)
		return true;
#endif

	bool					critical = false;
	uint32_t				random_damage = session.random_damage(*front_mob, critical);
	
	this->handle_attack_mob(session, *front_mob, random_damage);

	return true;
}

bool fb::game::acceptor::handle_pickup(fb::game::session& session)
{
	fb::istream&            istream = session.in_stream();

	uint8_t                 cmd = istream.read_u8();
	bool                    boost = bool(istream.read_u8());

	fb::game::map*          map = session.map();
	if(map == NULL)
		return false;

	this->send_stream(session, session.make_action_stream(action::PICKUP, duration::DURATION_PICKUP), scope::PIVOT);


	std::string             message;
	std::vector<object*>&   objects = map->objects();

	for(int i = objects.size()-1; i >= 0; i--)
	{
		object*				object = objects[i];
		if(object->position() != session.position())
			continue;

		if(object->type() != object::types::ITEM)
			continue;

		fb::game::item*     below = static_cast<fb::game::item*>(object);
		bool                item_moved = false;
		if(below->attr() & fb::game::item::attrs::ITEM_ATTR_CASH)
		{
			cash*			cash = static_cast<fb::game::cash*>(below);
			uint32_t        remain = session.money_add(cash->chunk());
			cash->chunk(remain); // 먹고 남은 돈으로 설정

			if(remain != 0)
				this->send_stream(session, this->make_message_stream("더 이상 돈을 가질 수 없습니다.", fb::game::message_types::MESSAGE_STATE), scope::SELF);

			this->send_stream(*cash, cash->make_show_stream(), scope::PIVOT, true);

			this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
		}
		else
		{
			uint8_t         index = session.item_add(below);
			if(index == -1)
				break;

			this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);

			item_moved = (session.item(index) == below);
		}

		if(item_moved || below->empty())
		{
			map->object_delete(below);
			this->send_stream(*below, below->make_hide_stream(), scope::PIVOT, true);
		}

		if(below->empty())
			delete below;

		if(boost == false)
			break;
	}

	return true;
}

bool fb::game::acceptor::handle_emotion(fb::game::session& session)
{
	fb::istream&            istream = session.in_stream();

	uint8_t                 cmd = istream.read_u8();
	uint8_t                 emotion = istream.read_u8();

	this->send_stream(session, session.make_action_stream(action(action::EMOTION + emotion), duration::DURATION_EMOTION), scope::SELF);
	return true;
}

bool fb::game::acceptor::handle_update_map(fb::game::session& session)
{
	fb::istream&            istream = session.in_stream();
	
	uint8_t                 cmd = istream.read_u8();
	uint16_t                x = istream.read_u16();
	uint16_t                y = istream.read_u16();
	uint8_t                 width = istream.read_u8();
	uint8_t                 height = istream.read_u8();
	uint16_t                crc = istream.read_u16();

	this->send_stream(session, session.map()->make_update_stream(x, y, width, height, crc), scope::SELF);
	return true;
}

bool fb::game::acceptor::handle_refresh(fb::game::session& session)
{
	this->send_stream(session, session.make_position_stream(), scope::SELF);
	return true;
}

bool fb::game::acceptor::handle_active_item(fb::game::session& session)
{
	fb::istream&            istream = session.in_stream();

	uint8_t                 cmd = istream.read_u8();
	uint8_t                 index = istream.read_u8() - 1;

	fb::game::item*         item = NULL;
	uint8_t                 updated_index = 0;
	std::string             message;
	fb::game::equipment::eq_slots slot;
	if(session.item_active(index, &item, &updated_index, slot, message) == false)
	{
		//this->send_stream(session, this->make_message_stream(message, 0x03), scope::SELF);
		return true;
	}

	if(item->attr() & fb::game::item::attrs::ITEM_ATTR_EQUIPMENT)
	{
		this->send_stream(session, session.make_delete_item_slot_stream(fb::game::item::delete_attr::DELETE_NONE, index), scope::SELF);
		if(updated_index != 0xFF)
			this->send_stream(session, session.make_update_item_slot_stream(updated_index), scope::SELF);
		
		this->send_stream(session, session.make_state_stream(fb::game::state_level::LEVEL_MAX), scope::SELF);
		this->send_stream(session, session.make_update_equipment_stream(slot), scope::SELF);
		this->send_stream(session, session.make_visual_stream(true), scope::PIVOT);
		this->send_stream(session, session.make_sound_stream(action_sounds::SOUND_EQUIPMENT_ON), scope::PIVOT);

		std::stringstream sstream;
		switch(slot)
		{
		case fb::game::equipment::eq_slots::WEAPON_SLOT:
			sstream << "w:무기  :";
			break;

		case fb::game::equipment::eq_slots::ARMOR_SLOT:
			sstream << "a:갑옷  :";
			break;

		case fb::game::equipment::eq_slots::SHIELD_SLOT:
			sstream << "s:방패  :";
			break;

		case fb::game::equipment::eq_slots::HELMET_SLOT:
			sstream << "h:머리  :";
			break;

		case fb::game::equipment::eq_slots::LEFT_HAND_SLOT:
			sstream << "l:왼손  :";
			break;

		case fb::game::equipment::eq_slots::RIGHT_HAND_SLOT:
			sstream << "r:오른손  :";
			break;

		case fb::game::equipment::eq_slots::LEFT_AUX_SLOT:
			sstream << "[:보조1  :";
			break;

		case fb::game::equipment::eq_slots::RIGHT_AUX_SLOT:
			sstream << "]:보조2  :";
			break;
		}
		sstream << item->name();
		this->send_stream(session, this->make_message_stream(sstream.str(), fb::game::message_types::MESSAGE_STATE), scope::SELF);
		
		sstream.str(std::string());
		sstream << "갑옷 강도  " << session.defensive_physical() <<"  " << session.regenerative() << " S  " << session.defensive_magical();
		this->send_stream(session, this->make_message_stream(sstream.str(), fb::game::message_types::MESSAGE_STATE), scope::SELF);

		return true;
	}


	if(item->attr() & fb::game::item::attrs::ITEM_ATTR_CONSUME)
	{	
		this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
		this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);
		this->send_stream(session, session.make_action_stream(action::EAT, duration::DURATION_EAT), scope::SELF);
		this->send_stream(session, session.make_sound_stream(action_sounds::SOUND_EAT), scope::SELF);

		if(item->empty())
		{
			this->send_stream(session, session.make_delete_item_slot_stream(fb::game::item::delete_attr::DELETE_EAT, index), scope::SELF);
			delete item;
		}
	}

	return true;
}

bool fb::game::acceptor::handle_inactive_item(fb::game::session& session)
{
	fb::istream&            istream = session.in_stream();
	
	uint8_t                 cmd = istream.read_u8();
	fb::game::equipment::eq_slots     slot = fb::game::equipment::eq_slots(istream.read_u8());
	item*                   item = NULL;

	// 말, 귀신 상태인지 검사

	std::string message;
	uint8_t                 item_index = session.equipment_off(slot, message);
	if(item_index == -1)
		return true;


	// 마법 딜레이 스트림 필요
	this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
	this->send_stream(session, session.make_equipment_off_stream(slot), scope::SELF);
	this->send_stream(session, session.make_visual_stream(true), scope::PIVOT);
	this->send_stream(session, session.make_sound_stream(action_sounds::SOUND_EQUIPMENT_OFF), scope::PIVOT);
	this->send_stream(session, session.make_update_item_slot_stream(item_index), scope::SELF);

	return true;
}

bool fb::game::acceptor::handle_drop_item(fb::game::session& session)
{
	fb::istream&            istream = session.in_stream();

	uint8_t                 cmd = istream.read_u8();
	uint8_t                 index = istream.read_u8() - 1;
	bool                    drop_all = bool(istream.read_u8());

	fb::game::item*         item = session.item(index);
	if(item == NULL)
		return true;

	fb::game::item*         dropped = item->handle_drop(session, drop_all ? item->count() : 1);

	if(item == dropped)
	{
		this->send_stream(session, session.make_delete_item_slot_stream(fb::game::item::delete_attr::DELETE_DROP, index), scope::SELF);
		session.item_remove(index);
	}
	
	fb::game::map* map = session.map();
	this->send_stream(session, session.make_action_stream(action::PICKUP, duration::DURATION_PICKUP), scope::PIVOT);
	this->send_stream(session, dropped->make_show_stream(), scope::PIVOT);
	this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);

	return true;
}

bool fb::game::acceptor::handle_drop_cash(fb::game::session& session)
{
	fb::istream&            istream = session.in_stream();

	fb::game::map*          map = session.map();
	if(map == NULL)
		return false;

	uint8_t                 cmd = istream.read_u8();
	uint32_t                chunk = std::min(session.money(), istream.read_u32());
	cash*					cash = new fb::game::cash(chunk);
	
	session.money_reduce(chunk);

	cash->map(map);
	cash->position(session.position());
	this->send_stream(session, session.make_action_stream(action::PICKUP, duration::DURATION_PICKUP), scope::PIVOT);
	this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
	this->send_stream(*cash, cash->make_show_stream(), scope::PIVOT, true);
	this->send_stream(session, this->make_message_stream("돈을 버렸습니다.", fb::game::message_types::MESSAGE_STATE), scope::SELF);

	return true;
}

bool fb::game::acceptor::handle_front_info(fb::game::session& session)
{
	fb::game::map* map = session.map();
	if(map == NULL)
		return false;

	std::vector<fb::game::session*> sessions = session.forward_sessions();
	for(auto i = sessions.begin(); i != sessions.end(); i++)
	{
		this->send_stream(session, this->make_message_stream((*i)->name(), message_types::MESSAGE_STATE), scope::SELF);
	}

	std::vector<object*> objects = session.forward_objects(object::types::UNKNOWN);
	for(auto i = objects.begin(); i != objects.end(); i++)
	{
		object* object = *i;
		if(object->type() == object::types::ITEM)
		{
			fb::game::item* item = static_cast<fb::game::item*>(object);
			this->send_stream(session, this->make_message_stream(item->name_styled(), message_types::MESSAGE_STATE), scope::SELF);
		}
		else
		{
			this->send_stream(session, this->make_message_stream(object->name(), message_types::MESSAGE_STATE), scope::SELF);
		}
	}

	return true;
}

bool fb::game::acceptor::handle_self_info(fb::game::session& session)
{
	this->send_stream(session, session.make_internal_info_stream(), scope::SELF);
	return true;
}

bool fb::game::acceptor::handle_option_changed(fb::game::session& session)
{
	fb::istream&            istream = session.in_stream();

	uint8_t                 cmd = istream.read_u8();
	options                 option = options(istream.read_u8());


	bool                    enabled = session.option_toggle(option);
	std::stringstream       sstream;
	switch(option)
	{
	case options::RIDE:
	{
		std::string message;
		try
		{
			if(session.state_assert(message, state(state::GHOST | state::DISGUISE)) == false)
				throw std::runtime_error(message);

			object* horse = NULL;

			if(session.state() == fb::game::state::RIDING)
			{
				message = "말에서 내렸습니다.";
				session.state(fb::game::state::NORMAL);

				
			}
			else
			{
				horse = session.forward_object(object::types::MOB);
				//if(horse == NULL || static_cast<fb::game::mob*>(horse)->)
				//	throw std::runtime_error("탈 것이 없습니다.");

				message = "말에 탔습니다.";
				session.state(fb::game::state::RIDING);
			}

			this->send_stream(session, session.make_visual_stream(true), scope::PIVOT);
			this->send_stream(session, this->make_message_stream(message, message_types::MESSAGE_STATE), scope::SELF);
		}
		catch(std::exception& e)
		{
			this->send_stream(session, this->make_message_stream(e.what(), message_types::MESSAGE_STATE), scope::SELF);
		}
		return true;
	}

	case options::WHISPER:
		sstream << "귓속말듣기  ";
		break;

	case options::GROUP:
		sstream << "그룹허가    ";
		break;

	case options::ROAR:
		sstream << "외치기듣기  ";
		break;

	case options::ROAR_WORLDS:
		sstream << "세계후      ";
		break;

	case options::MAGIC_EFFECT:
		sstream << "마법이펙트  ";
		break;

	case options::WEATHER_EFFECT:
		sstream << "날씨변화    ";
		break;

	case options::FIXED_MOVE:
		sstream << "고정이동     ";
		break;

	case options::TRADE:
		sstream << "교환가능    ";
		break;

	case options::FAST_MOVE:
		sstream << "빠른이동    ";
		break;

	case options::EFFECT_SOUND:
		sstream << "소리듣기    ";
		break;

	case options::PK:
		sstream << "PK보호      ";
		break;

	default:
		return false;
	}

	sstream << ": " << (enabled ? "ON" : "OFF");
	this->send_stream(session, this->make_message_stream(sstream.str(), fb::game::message_types::MESSAGE_STATE), scope::SELF);
	this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
	this->send_stream(session, session.make_option_stream(), scope::SELF);

	return true;
}

bool fb::game::acceptor::handle_click_object(fb::game::session& session)
{
	fb::istream&			istream = session.in_stream();

	uint8_t					cmd = istream.read_u8();
	uint8_t					unknown = istream.read_u8();
	uint32_t				fd = istream.read_u32();

	if(fd == 0xFFFFFFFF) // Press F1
	{
		return true;
	}

	if(fd == 0xFFFFFFFE) // Preff F2
	{
		return true;
	}

	fb::game::session* other = this->session(fd);
	if(other != NULL) // character
	{
		this->send_stream(session, other->make_external_info_stream(), scope::SELF);
		return true;
	}

	object* object = session.map()->object(fd);
	if(object != NULL) // object
	{
		return true;
	}

	return true;
}

bool fb::game::acceptor::handle_item_info(fb::game::session& session)
{
	fb::istream&				istream = session.in_stream();

	uint8_t cmd = istream.read_u8();
	uint16_t position = istream.read_u16();
	uint8_t unknown1 = istream.read_u8();
	uint8_t unknown2 = istream.read_u8();
	uint8_t unknown3 = istream.read_u8();
	uint8_t slot = istream.read_u8() - 1;

	fb::game::item* item = session.item(slot);
	if(item == NULL)
		return false;

	this->send_stream(session, item->make_tip_stream(position), scope::SELF);

	return true;
}

bool fb::game::acceptor::handle_itemmix(fb::game::session& session)
{
	fb::istream&				istream = session.in_stream();

	uint8_t cmd = istream.read_u8();
	uint8_t count = istream.read_u8();
	if(count > item::MAX_ITEM_SLOT - 1)
		return false;

	std::vector<item*> items;
	for(int i = 0; i < count; i++)
	{
		uint8_t index = istream.read_u8() - 1;
		item* item = session.item(index);
		if(item == NULL)
			return true;

		items.push_back(item);
	}
	
	try
	{
		itemmix* itemmix = this->find_itemmix(items);
		if(itemmix == NULL)
			throw std::runtime_error("조합할 수 없습니다");

		uint8_t free_size = session.inventory_free_size();
		if(int(itemmix->success.size()) - int(itemmix->require.size()) > free_size)
			throw std::runtime_error("인벤토리가 부족합니다.");


		for(auto i = itemmix->require.begin(); i != itemmix->require.end(); i++)
		{
			item::core* core = (*i).item;
			uint16_t count = (*i).count;
			uint8_t index = session.item2index(core);
			if(index == 0xFF)
				return true;

			fb::game::item* item = session.item(index);
			item->reduce(count);


			if(item->empty())
			{
				this->send_stream(session, session.make_delete_item_slot_stream(item::delete_attr::DELETE_NONE, index), scope::SELF);
				session.item_remove(index);
				delete item;
			}
			else
			{
				this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);
			}
		}

		bool success = (std::rand() % 100) < itemmix->percentage;
		if(success)
		{
			for(auto i = itemmix->success.begin(); i != itemmix->success.end(); i++)
			{
				item* item = static_cast<fb::game::item*>((*i).item->make());
				item->count((*i).count);
				uint8_t index = session.item_add(item);
				this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);
			}

			throw std::runtime_error("성공하였습니다.");
		}
		else
		{
			for(auto i = itemmix->failed.begin(); i != itemmix->failed.end(); i++)
			{
				item* item = static_cast<fb::game::item*>((*i).item->make());
				item->count((*i).count);
				uint8_t index = session.item_add(item);
				this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);
			}

			throw std::runtime_error("실패하였습니다.");
		}
	}
	catch(std::exception& e)
	{ 
		this->send_stream(session, this->make_message_stream(e.what(), message_types::MESSAGE_STATE), scope::SELF);
		return true;
	}

	

	return true;
}

bool fb::game::acceptor::handle_trade(fb::game::session& session)
{
	fb::istream&				istream = session.in_stream();

	uint8_t						cmd = istream.read_u8();
	uint8_t						action = istream.read_u8();
	uint32_t					fd = istream.read_u32();

	fb::game::session*			partner = this->session(fd);			// 파트너
	trade_system&				ts_mine = session.trade_system();		// 나의 거래시스템
	trade_system&				ts_your = partner->trade_system();		// 상대방의 거래시스템

	if(partner == NULL)
		return true;

	switch(action)
	{
	case 0:
	{
		if(session.id() == partner->id())
		{
			// 자기 자신과 거래를 하려고 시도하는 경우
			break;
		}

		if(session.option(options::TRADE) == false)
		{
			// 내가 교환 거부중
			this->send_stream(session, this->make_message_stream("교환 거부 상태입니다.", message_types::MESSAGE_STATE), scope::SELF);
			break;
		}

		if(partner->option(options::TRADE) == false)
		{
			// 상대방이 교환 거부중
			std::stringstream sstream;
			sstream << partner->name() << "님은 교환 거부 상태입니다.";
			this->send_stream(session, this->make_message_stream(sstream.str(), message_types::MESSAGE_STATE), scope::SELF);
			break;
		}

		if(ts_mine.trading())
		{
			// 내가 이미 교환중
			break;
		}

		if(ts_your.trading())
		{
			// 상대방이 이미 교환중
			std::stringstream sstream;
			sstream << partner->name() << "님은 이미 교환 중입니다.";
			this->send_stream(session, this->make_message_stream(sstream.str(), message_types::MESSAGE_STATE), scope::SELF);
			break;
		}

		if(session.sight(*partner) == false)
		{
			// 상대방이 시야에서 보이지 않음
			this->send_stream(session, this->make_message_stream("대상이 보이지 않습니다.", message_types::MESSAGE_STATE), scope::SELF);
			break;
		}

		if(session.distance_sqrt(*partner) > 16)
		{
			// 상대방과의 거리가 너무 멈
			std::stringstream sstream;
			sstream << partner->name() << "님과 너무 멀리 떨어져 있습니다.";
			this->send_stream(session, this->make_message_stream(sstream.str(), message_types::MESSAGE_STATE), scope::SELF);
			break;
		}

		// 교환 시작
		ts_mine.begin(partner);
		this->send_stream(session, ts_your.make_dialog_stream(), scope::SELF);

		ts_your.begin(&session);
		this->send_stream(*partner, ts_mine.make_dialog_stream(), scope::SELF);
		break;
	}

	case 1: // 아이템 올릴때
	{
		uint8_t index = istream.read_u8() - 1;
		fb::game::item* item = session.item(index);
		if(item == NULL)
			return true;

		// 교환 불가능한 아이템 거래 시도
		if(item->trade() == false)
		{
			this->send_stream(session, this->make_message_stream("교환이 불가능한 아이템입니다.", message_types::MESSAGE_TRADE), scope::SELF);
			break;
		}

		if((item->attr() & fb::game::item::attrs::ITEM_ATTR_BUNDLE) && (item->count() > 1))
		{
			// 묶음 단위의 아이템 형식 거래 시도
			ts_mine.select(item);
			this->send_stream(session, ts_mine.make_bundle_stream(), scope::SELF);
		}
		else
		{
			// 일반 아이템의 거래 시도

			uint8_t trade_index = ts_mine.add(item); // 거래중인 아이템 리스트의 인덱스
			if(trade_index == 0xFF)
				return false;

			// 현재 인벤토리에서 거래중인 아이템 리스트로 아이템 이동
			session.item_remove(index);
			this->send_stream(session, session.make_delete_item_slot_stream(item::delete_attr::DELETE_NONE, index), scope::PIVOT);

			// 나와 상대 둘 다에게 올린 아이템을 표시함
			this->send_stream(session, ts_mine.make_show_stream(true, trade_index), scope::SELF);
			this->send_stream(*partner, ts_mine.make_show_stream(false, trade_index), scope::SELF);
		}
	}
		break;

	case 2: // 아이템 갯수까지 해서 올릴 때
	{
		// 이전에 올리려고 시도한 묶음 단위의 아이템
		fb::game::item* selected = ts_mine.selected();
		if(selected == NULL)
			return false;

		// 올릴 갯수 (클라이언트가 입력한 값)
		uint16_t count = istream.read_u16();
		if(selected->count() < count)
		{
			this->send_stream(session, this->make_message_stream("개수가 올바르지 않습니다.", message_types::MESSAGE_TRADE), scope::SELF);
			break;
		}

		uint8_t index = session.item2index(selected->based<item::core>());
		if(selected->count() == count)
		{
			// 모두 다 올리는 경우, 아이템을 따로 복사하지 않고 있는 그대로 거래리스트에 옮겨버린다.
			uint8_t trade_index = ts_mine.add(selected);
			if(index != 0xFF)
			{
				session.item_remove(index);
				this->send_stream(session, session.make_delete_item_slot_stream(item::delete_attr::DELETE_NONE, index), scope::SELF);

				this->send_stream(session, ts_mine.make_show_stream(true, trade_index), scope::SELF);
				this->send_stream(*partner, ts_mine.make_show_stream(false, trade_index), scope::SELF);
			}
		}
		else
		{
			// 일부만 올리는 경우, 기존의 것에서 갯수를 깎고 새로 복사된 아이템을 거래 리스트로 옮긴다.
			selected->reduce(count);
			item* cloned = selected->clone<fb::game::item>();
			cloned->count(count);

			uint8_t trade_index = ts_mine.add(cloned);

			this->send_stream(session, session.make_update_item_slot_stream(index), scope::SELF);

			this->send_stream(session, ts_mine.make_show_stream(true, trade_index), scope::SELF);
			this->send_stream(*partner, ts_mine.make_show_stream(false, trade_index), scope::SELF);
		}
		break;
	}

	case 3: // 금전 올릴 때
	{
		// 클라이언트가 입력한 금전 양
		uint32_t money = istream.read_u32();

		// 입력한 금전 양을 계속해서 빼면 안된다.
		// 100전 입력한 경우 -1, -10, -100 이렇게 까여버림
		uint32_t total = session.money() + ts_mine.money();
		if(money > total)
			money = total;

		session.money(total - money);
		ts_mine.money(money);

		this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
		this->send_stream(session, ts_mine.make_money_stream(true), scope::SELF);
		this->send_stream(*partner, ts_mine.make_money_stream(false), scope::SELF);
		break;
	}

	case 4: // 취소한 경우
	{
		std::vector<uint8_t> indices;

		// 거래리스트에 올렸던 아이템과 금전을 원상복귀시킨다.
		indices = ts_mine.restore();
		this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);
		for(auto i = indices.begin(); i != indices.end(); i++)
			this->send_stream(session, session.make_update_item_slot_stream(*i), scope::SELF);
		// 메시지 표시하고 거래종료
		this->send_stream(session, ts_mine.make_close_stream("내가 교환을 취소했습니다."), scope::SELF);
		ts_mine.end();

		// 거래리스트에 올렸던 아이템과 금전을 원상복귀시킨다.
		indices = ts_your.restore();
		this->send_stream(*partner, partner->make_state_stream(state_level::LEVEL_MIN), scope::SELF);
		for(auto i = indices.begin(); i != indices.end(); i++)
			this->send_stream(*partner, partner->make_update_item_slot_stream(*i), scope::SELF);
		// 메시지 표시하고 거래종료
		this->send_stream(*partner, ts_your.make_close_stream("상대방이 교환을 취소했습니다."), scope::SELF);
		ts_your.end();
		break;
	}

	case 5:
	{
		this->send_stream(session, ts_mine.make_lock_stream(), scope::SELF);
		ts_mine.lock(true);

		if(ts_your.lock())
		{
			// 상대방이 이미 교환확인 누른 경우
			if(ts_mine.flushable(*partner) == false || ts_your.flushable(session) == false)
			{
				// 나 혹은 상대가 거래리스트에 올라온 아이템이나 금전을 다 받을 수 없는 상황일 때
				// 인벤토리가 가득 차거나, 더 이상 금전을 얻을 수 없는 경우
				this->send_stream(session, ts_mine.make_close_stream("교환에 실패했습니다."), scope::SELF);
				this->send_stream(*partner, ts_your.make_close_stream("교환에 실패했습니다."), scope::SELF);
			}
			else
			{
				std::vector<uint8_t> indices;
				
				// 상대의 거래리스트 물품들을 전부 받고 업데이트
				ts_mine.flush(*partner, indices);
				for(auto i = indices.begin(); i != indices.end(); i++)
					this->send_stream(*partner, partner->make_update_item_slot_stream(*i), scope::SELF);
				this->send_stream(*partner, partner->make_state_stream(state_level::LEVEL_MIN), scope::SELF);

				// 나의 거래리스트 물품들을 전부 주고 업데이트
				ts_your.flush(session, indices);
				for(auto i = indices.begin(); i != indices.end(); i++)
					this->send_stream(session, session.make_update_item_slot_stream(*i), scope::SELF);
				this->send_stream(session, session.make_state_stream(state_level::LEVEL_MIN), scope::SELF);


				// 메시지 표시하고 거래 종료
				this->send_stream(session, ts_mine.make_close_stream("교환에 성공했습니다."), scope::SELF);
				ts_mine.end();

				this->send_stream(*partner, ts_your.make_close_stream("교환에 성공했습니다."), scope::SELF);
				ts_your.end();
			}
		}
		else
		{
			// 상대방이 아직 교환확인을 누르지 않은 경우
			this->send_stream(*partner, this->make_message_stream("상대방이 확인을 눌렀습니다.", message_types::MESSAGE_TRADE), scope::SELF);
		}
		break;
	}
	}

	return true;
}

bool fb::game::acceptor::handle_group(fb::game::session& session)
{
	fb::istream&				istream = session.in_stream();

	uint8_t						cmd = istream.read_u8();
	uint8_t						name_size = istream.read_u8();
	
	char*						buffer = new char[name_size];
	istream.read(buffer, name_size);
	std::string					name(buffer);
	delete[] buffer;


	// 여기서 접속해있는 세션 전체 대상으로 루프를 돌 게 아니라
	// DB에 접근해서 하는 방식이 나을 것 같다는 생각이 드는구만
	
	return true;
}

void fb::game::acceptor::handle_counter_mob_action(fb::game::mob* mob)
{
	fb::game::session* target = mob->target();
	if(target == NULL)
	{
		fb::game::direction before_direction = mob->direction();
		this->handle_move_life(mob, fb::game::direction(std::rand() % 4));
		return;
	}

	// 상하좌우로 타겟이 있는지 검사한다.
	for(int i = 0; i < 4; i++)
	{
		fb::game::direction direction = fb::game::direction(i);
		if(mob->near_session(direction) != target)
			continue;

		if(mob->direction() != direction)
		{
			mob->direction(direction);
			this->send_stream(*mob, mob->make_direction_stream(), scope::PIVOT, true);
		}

		uint32_t random_damage = mob->random_damage(*target);
		target->hp_down(random_damage);


		// 공격하는 패킷 보낸다.
		this->send_stream(*mob, mob->make_action_stream(fb::game::action::ATTACK, fb::game::duration::DURATION_ATTACK), scope::PIVOT, true);
		this->send_stream(*target, target->make_show_hp_stream(random_damage, false), scope::PIVOT);
		this->send_stream(*target, target->make_state_stream(state_level::LEVEL_MIDDLE), scope::SELF);
		return;
	}


	// 타겟 방향으로 이동이 가능하다면 이동한다.
	bool x_axis = bool(std::rand()%2);
	if(x_axis)
	{
		if(mob->x() > target->x() && this->handle_move_life(mob, fb::game::direction::LEFT))   return;
		if(mob->x() < target->x() && this->handle_move_life(mob, fb::game::direction::RIGHT))  return;
		if(mob->y() > target->y() && this->handle_move_life(mob, fb::game::direction::TOP))    return;
		if(mob->y() < target->y() && this->handle_move_life(mob, fb::game::direction::BOTTOM)) return;
	}
	else
	{
		if(mob->y() > target->y() && this->handle_move_life(mob, fb::game::direction::TOP))    return;
		if(mob->y() < target->y() && this->handle_move_life(mob, fb::game::direction::BOTTOM)) return;
		if(mob->x() > target->x() && this->handle_move_life(mob, fb::game::direction::LEFT))   return;
		if(mob->x() < target->x() && this->handle_move_life(mob, fb::game::direction::RIGHT))  return;
	}


	// 이동할 수 있는 방향으로 일단 이동한다.
	uint8_t random_direction = std::rand() % 4;
	for(int i = 0; i < 4; i++)
	{
		if(this->handle_move_life(mob, fb::game::direction((random_direction + i) % 4)))
			return;
	}
}

void fb::game::acceptor::handle_containment_mob_action(fb::game::mob* mob)
{
	fb::game::session* target = mob->target();
	fb::game::direction before_direction = mob->direction();

	try
	{
		if(target == NULL || target->sight(*mob) == false)
			target = mob->autoset_target();

		if(target == NULL)
			throw std::runtime_error("no target");
	}
	catch(std::exception&)
	{ }

	this->handle_counter_mob_action(mob);
}

void fb::game::acceptor::handle_mob_action(uint64_t now)
{
	for(auto map_i = this->_maps.begin(); map_i != this->_maps.end(); map_i++)
	{
		fb::game::map* map = map_i->second;
		const std::vector<object*>& objects = map->objects();

		for(auto obj_i = objects.begin(); obj_i != objects.end(); obj_i++)
		{
			object* object = (*obj_i);
			if(object->type() != object::types::MOB)
				continue;

			fb::game::mob* mob = static_cast<fb::game::mob*>(object);
			if(now < mob->action_time() + mob->speed())
				continue;

			if(mob->alive() == false)
				continue;

			switch(mob->offensive())
			{
			case mob::offensive_type::COUNTER:
				this->handle_counter_mob_action(mob);
				break;

			case mob::offensive_type::CONTAINMENT:
				this->handle_containment_mob_action(mob);
				break;

			default:
				this->handle_counter_mob_action(mob);
				break;
			}

			mob->action_time(now);
		}
	}
}

void fb::game::acceptor::handle_mob_respawn(uint64_t now)
{
	// 리젠된 전체 몹을 저장
	std::vector<object*> spawned_mobs;
	for(auto map_i = this->_maps.begin(); map_i != this->_maps.end(); map_i++)
	{
		fb::game::map* map = map_i->second;
		const std::vector<object*>& objects = map->objects();
		for(auto obj_i = objects.begin(); obj_i != objects.end(); obj_i++)
		{
			if((*obj_i)->type() != object::types::MOB)
				continue;

			fb::game::mob* mob = static_cast<fb::game::mob*>(*obj_i);
			if(mob == NULL)
				continue;

			if(mob->spawn(now) == false)
				continue;

			spawned_mobs.push_back(mob);
		}
	}


	// 화면에 보이는 몹만 갱신
	std::vector<object*> buffer;
	std::vector<fb::game::session*>& sessions = this->sessions();
	for(auto i = sessions.begin(); i != sessions.end(); i++)
	{
		fb::game::session* session = static_cast<fb::game::session*>(*i);

		if(session == NULL)
			continue;

		buffer.clear();
		for(auto obj_i = spawned_mobs.begin(); obj_i != spawned_mobs.end(); obj_i++)
		{
			if(session->sight(**obj_i) == false)
				continue;

			buffer.push_back(*obj_i);
		}

		this->send_stream(*session, object::make_show_stream(buffer), scope::SELF);
	}
}

void fb::game::acceptor::handle_session_warp(fb::game::session& session, const map::warp* warp)
{
	map*					map = session.map();

	// 이전에 보이던 오브젝트들 전부 제거
	auto                    objects = session.shown_objects();
	for(auto i = objects.begin(); i != objects.end(); i++)
		this->send_stream(session, (*i)->make_hide_stream(), scope::SELF);

	auto					sessions = session.shown_sessions();
	for(auto i = sessions.begin(); i != sessions.end(); i++)
		this->send_stream(session, (*i)->make_hide_stream(), scope::SELF);

	session.map(warp->map);
	session.position(warp->after);

	this->send_stream(session, session.make_id_stream(), scope::SELF);
	this->send_stream(session, warp->map->make_config_stream(), scope::SELF);
	this->send_stream(session, warp->map->make_bgm_stream(), scope::SELF);
	this->send_stream(session, session.make_state_stream(state_level::LEVEL_MAX), scope::SELF);
	this->send_stream(session, session.make_position_stream(), scope::SELF);
	this->send_stream(session, session.make_visual_stream(false), scope::SELF);
	this->send_stream(session, session.make_direction_stream(), scope::SELF);

	// 새로 보이는 오브젝트 보여줌
	this->send_stream(session, session.make_show_objects_stream(), scope::SELF);

	// 새로 보이는 세션들 보여줌
	sessions = session.shown_sessions();
	for(auto i = sessions.begin(); i != sessions.end(); i++)
		this->send_stream(session, (*i)->make_visual_stream(false), scope::SELF);
}

fb::game::acceptor* fb::game::acceptor::instance()
{
	if(fb::game::acceptor::_instance == NULL)
		fb::game::acceptor::_instance = new fb::game::acceptor(10021);

	return fb::game::acceptor::_instance;
}

void fb::game::acceptor::release()
{
	if(fb::game::acceptor::_instance != NULL)
		delete fb::game::acceptor::_instance;
}
