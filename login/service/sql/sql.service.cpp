#include "sql.service.h"

std::string fb::login::service::sql::auth::insert(const std::string& name, const std::string& pw)
{
	auto& config = fb::config::get();

	std::srand(std::time(nullptr));
	auto hp = config["init"]["hp"]["base"].asInt() + std::rand() % config["init"]["hp"]["range"].asInt();
	auto mp = config["init"]["mp"]["base"].asInt() + std::rand() % config["init"]["mp"]["range"].asInt();


    auto parameters = std::vector<std::string>
	{
		"'" + name + "'",
		"'" + pw + "'",
		std::to_string(hp),
		std::to_string(hp),
		std::to_string(mp),
		std::to_string(mp),
		std::to_string(config["init"]["map"].asInt()),
		std::to_string(config["init"]["position"]["x"].asInt()),
		std::to_string(config["init"]["position"]["y"].asInt()),
	};

	std::stringstream sstream;
	sstream << "INSERT INTO user (name, pw, hp, base_hp, mp, base_mp, map, position_x, position_y) VALUES "
		<< fb::service::sql::vec_to_insert(parameters);

	return sstream.str();
}

std::string fb::login::service::sql::auth::update(const std::string& name, uint8_t hair, uint8_t sex, uint8_t nation, uint8_t creature)
{
	auto dict = std::map<std::string, std::string>
	{
		{ "look", std::to_string(hair) },
		{ "sex", std::to_string(sex) },
		{ "nation", std::to_string(nation) },
		{ "creature", std::to_string(creature) }
	};

	std::stringstream sstream;
	sstream << "UPDATE user SET "
		<< fb::service::sql::dict_to_update(dict)
		<< " WHERE name="
		<< '\'' << name << '\''
		<< " LIMIT 1";
	
	return sstream.str();
}
