#include "session.h"
#include "mob.h"
#include "map.h"
#include <sysinfoapi.h>
#include <iostream>

fb::game::mob::mob(const std::string& name, uint16_t look, uint8_t color) : 
	life(0xFFFFFFFF, name, look, color),
	_damage(0, 0),
	_offensive_type(offensive_type::NONE),
	_size(sizes::SMALL),
	_speed(1000),
	_action_time(0),
	_dead_time(0),
	_respawn_time(0),
	_target(NULL)
{}

fb::game::mob::mob(const mob& right) : 
	life(right),
	_damage(right._damage),
	_offensive_type(right._offensive_type),
	_size(right._size),
	_speed(right._speed),
	_script_attack(right._script_attack),
	_script_die(right._script_die),
	_spawn_point(right._spawn_point),
	_spawn_size(right._spawn_size),
	_action_time(right._action_time),
	_dead_time(right._dead_time),
	_respawn_time(right._respawn_time),
	_target(right._target),
	_items(right._items.begin(), right._items.end())
{}

fb::game::mob::~mob()
{}

uint16_t fb::game::mob::damage_min() const
{
	return this->_damage.min;
}

void fb::game::mob::damage_min(uint16_t value)
{
	this->_damage.min = value;
}

uint16_t fb::game::mob::damage_max() const
{
	return this->_damage.max;
}

void fb::game::mob::damage_max(uint16_t value)
{
	this->_damage.max = value;
}

fb::game::mob::sizes fb::game::mob::size() const
{
	return this->_size;
}

void fb::game::mob::size(mob::sizes value)
{
	this->_size = value;
}

uint32_t fb::game::mob::speed() const
{
	return this->_speed;
}

void fb::game::mob::speed(uint32_t value)
{
	this->_speed = value;
}

const std::string& fb::game::mob::script_attack() const
{
	return this->_script_attack;
}

void fb::game::mob::script_attack(const std::string& value)
{
	this->_script_attack = value;
}

const std::string& fb::game::mob::script_die() const
{
	return this->_script_die;
}

void fb::game::mob::script_die(const std::string& value)
{
	this->_script_die = value;
}

fb::game::mob::offensive_type fb::game::mob::offensive() const
{
	return this->_offensive_type;
}

void fb::game::mob::offensive(offensive_type value)
{
	this->_offensive_type = value;
}

fb::game::mob* fb::game::mob::make() const
{
	return new fb::game::mob(*this);
}

uint32_t fb::game::mob::random_damage(fb::game::life& life) const
{
	uint32_t difference = std::abs(this->_damage.max - this->_damage.min);
	uint32_t random_damage = this->_damage.min + (std::rand() % difference);

	return __super::random_damage(random_damage, life);
}

const fb::game::point16_t& fb::game::mob::spawn_point() const
{
	return this->_spawn_point;
}

void fb::game::mob::spawn_point(uint16_t x, uint16_t y)
{
	this->_spawn_point.x = x;
	this->_spawn_point.y = y;
}

void fb::game::mob::spawn_point(const fb::game::point16_t point)
{
	this->_spawn_point = point;
}

const fb::game::size16_t& fb::game::mob::spawn_size() const
{
	return this->_spawn_size;
}

void fb::game::mob::spawn_size(uint16_t width, uint16_t height)
{
	this->_spawn_size.width = width;
	this->_spawn_size.height = height;
}

void fb::game::mob::spawn_size(const size16_t size)
{
	this->_spawn_size = size;
}

uint64_t fb::game::mob::action_time() const
{
	return this->_action_time;
}

void fb::game::mob::action_time(uint64_t ms)
{
	this->_action_time = ms;
}

uint64_t fb::game::mob::dead_time() const
{
	return this->_dead_time;
}

void fb::game::mob::dead_time(uint64_t ms)
{
	this->_dead_time = ms;
}

uint32_t fb::game::mob::respawn_time() const
{
	return this->_respawn_time;
}

void fb::game::mob::respawn_time(uint64_t ms)
{
	this->_respawn_time = ms;
}

bool fb::game::mob::spawn(uint64_t now)
{
	if(this->alive())
		return false;

	if((now - this->_dead_time) / 1000 < this->_respawn_time)
		return false;

	this->direction(fb::game::direction(std::rand() % 4));
	this->hp(this->base_hp());

	while(true)
	{
		point16_t position(this->_spawn_point.x + (std::rand() % this->_spawn_size.width), this->_spawn_point.y + (std::rand() % this->_spawn_size.height));

		if(position.x > this->_map->width()-1 || position.y > this->_map->height()-1)
			continue;

		if(this->_map->blocked(position.x, position.y))
			continue;

		this->position(position);
		break;
	}

	this->action_time(now);
	std::cout << "spawn mob : " << this->id() << std::endl;
}

fb::game::session* fb::game::mob::target() const
{
	return this->_target;
}

void fb::game::mob::target(fb::game::session* value)
{
	this->_target = value;
}

fb::game::session* fb::game::mob::autoset_target()
{
	fb::game::map* map = this->_map;
	if(map == NULL)
		return NULL;

	uint32_t min_distance_sqrt = 0xFFFFFFFF;
	fb::game::session* near_session = NULL;
	std::vector<fb::game::session*>& sessions = map->sessions();
	for(auto i = sessions.begin(); i != sessions.end(); i++)
	{
		fb::game::session* session = *i;
		if(session->sight(*this) == false)
			continue;

		uint32_t distance_sqrt = std::abs(session->x() - this->x()) * std::abs(session->y() - this->y());
		if(distance_sqrt > min_distance_sqrt)
			continue;

		this->_target = session;
	}

	return this->_target;
}

void fb::game::mob::dropitem_add(const mob::drop& drop)
{
	this->_items.push_back(drop);
}

void fb::game::mob::dropitem_add(const fb::game::item* item, float percentage)
{
	this->_items.push_back(drop(item, percentage));
}

const std::vector<fb::game::mob::drop>& fb::game::mob::items() const
{
	return this->_items;
}
