#include "object.h"
#include "map.h"
#include "session.h"
#include <iostream>

// fb::game::object
fb::game::object::object(uint32_t id, const std::string& name, uint16_t look, uint8_t color, const point16_t position, fb::game::direction direction, fb::game::map* map) : 
	_id(id),
	_name(name),
	_look(look),
	_color(color),
	_position(position),
	_direction(direction),
	_map(map)
{
}

fb::game::object::object(const object& right) :
	object(right._id, right._name, right._look, right._color, right._position, right._direction, right._map)
{
}

fb::game::object::~object()
{}

uint32_t fb::game::object::id() const
{
	return this->_id;
}

void fb::game::object::id(uint32_t value)
{
	this->_id = value;
}

const std::string& fb::game::object::name() const
{
	return this->_name;
}

void fb::game::object::name(const std::string& value)
{
	this->_name = value;
}

uint16_t fb::game::object::look() const
{
	return this->_look;
}

void fb::game::object::look(uint16_t value)
{
	this->_look = value;
}

uint8_t fb::game::object::color() const
{
	return this->_color;
}

void fb::game::object::color(uint8_t value)
{
	this->_color = value;
}

fb::game::object::types fb::game::object::type() const
{
	return object::types::UNKNOWN;
}

fb::game::point16_t fb::game::object::position() const
{
	return this->_position;
}

bool fb::game::object::position(uint16_t x, uint16_t y)
{
	if(this->_map == NULL)
		return false;

	this->_position.x = std::max(0, std::min(this->_map->width() - 1, int32_t(x)));
	this->_position.y = std::max(0, std::min(this->_map->height() - 1, int32_t(y)));
	return true;
}

bool fb::game::object::position(const point16_t position)
{
	if(this->_map == NULL)
		return false;

	this->_position.x = std::max(0, std::min(this->_map->width() - 1, int32_t(position.x)));
	this->_position.y = std::max(0, std::min(this->_map->height() - 1, int32_t(position.y)));
	return true;
}

uint16_t fb::game::object::x() const
{
	return this->_position.x;
}

bool fb::game::object::x(uint16_t value)
{
	if(this->_map == NULL)
		return false;

	this->_position.x = std::max(uint16_t(0), std::min(uint16_t(this->_map->width() - 1), value));
	return true;
}

uint16_t fb::game::object::y() const
{
	return this->_position.y;
}

bool fb::game::object::y(uint16_t value)
{
	if(this->_map == NULL)
		return false;

	this->_position.y = std::max(uint16_t(0), std::min(uint16_t(this->_map->height() - 1), value));
	return true;
}

bool fb::game::object::movable(fb::game::direction direction) const
{
	if(this->_map == NULL)
		return false;

	return this->_map->movable(*this, direction);
}

bool fb::game::object::movable_forward() const
{
	if(this->_map == NULL)
		return false;

	return this->_map->movable_forward(*this);
}

bool fb::game::object::move(fb::game::direction direction, std::vector<object*>* show_objects, std::vector<object*>* hide_objects, std::vector<session*>* show_sessions, std::vector<session*>* hide_sessions, std::vector<object*>* shown_objects, std::vector<object*>* hidden_objects, std::vector<session*>* shown_sessions, std::vector<session*>* hidden_sessions)
{
	if(this->movable(direction) == false)
		return false;

	point16_t before = this->_position;

	switch(direction)
	{
	case fb::game::direction::TOP:
		this->_position.y--;
		break;

	case fb::game::direction::BOTTOM:
		this->_position.y++;
		break;

	case fb::game::direction::LEFT:
		this->_position.x--;
		break;

	case fb::game::direction::RIGHT:
		this->_position.x++;
		break;
	}

	point16_t& after = this->_position;

	// 내 기준으로 한 오브젝트
	if(show_objects != NULL || hide_objects != NULL)
	{
		fb::game::map* map = this->_map;
		std::vector<object*>& objects = map->objects();
		for(auto i = objects.begin(); i != objects.end(); i++)
		{
			fb::game::object*   object = *i;
			if(object == this)
				continue;

			bool                before_sight = fb::game::object::sight(before, object->position(), map);
			bool                after_sight  = this->sight(*object);

			bool                show = (!before_sight && after_sight);
			bool                hide = (before_sight && !after_sight);

			if(show && show_objects != NULL)
				show_objects->push_back(object);
			else if(hide && hide_objects != NULL)
				hide_objects->push_back(object);
		}
	}

	// 내 기준으로 한 세션
	if(show_sessions != NULL || hide_sessions != NULL)
	{
		fb::game::map* map = this->_map;
		std::vector<session*>& sessions = map->sessions();
		for(auto i = sessions.begin(); i != sessions.end(); i++)
		{
			fb::game::session*   session = *i;
			if(session == this)
				continue;

			bool                before_sight = fb::game::object::sight(before, session->position(), map);
			bool                after_sight  = this->sight(*session);

			bool                show = (!before_sight && after_sight);
			bool                hide = (before_sight && !after_sight);

			if(show && show_sessions != NULL)
				show_sessions->push_back(session);
			else if(hide && hide_sessions != NULL)
				hide_sessions->push_back(session);
		}
	}

	// 상대 기준으로 한 오브젝트
	if(shown_objects != NULL || hidden_objects)
	{
		fb::game::map* map = this->_map;
		std::vector<object*>& objects = map->objects();
		for(auto i = objects.begin(); i != objects.end(); i++)
		{
			fb::game::object*   object = *i;
			if(object == this)
				continue;

			bool                before_sight = fb::game::object::sight(object->position(), before, map);
			bool                after_sight  = object->sight(*this);

			bool                show = (!before_sight && after_sight);
			bool                hide = (before_sight && !after_sight);

			if(show && shown_objects != NULL)
				shown_objects->push_back(object);
			else if(hide && hidden_objects != NULL)
				hidden_objects->push_back(object);
		}
	}

	// 상대 기준으로 한 세션
	if(shown_sessions != NULL || hidden_sessions)
	{
		fb::game::map* map = this->_map;
		std::vector<session*>& sessions = map->sessions();
		for(auto i = sessions.begin(); i != sessions.end(); i++)
		{
			fb::game::session*   session = *i;
			if(session == this)
				continue;

			bool                before_sight = fb::game::object::sight(session->position(), before, map);
			bool                after_sight  = session->sight(*this);

			bool                show = (!before_sight && after_sight);
			bool                hide = (before_sight && !after_sight);

			if(show && shown_sessions != NULL)
				shown_sessions->push_back(session);
			else if(hide && hidden_sessions != NULL)
				hidden_sessions->push_back(session);
		}
	}

	return true;
}

bool fb::game::object::move_forward(std::vector<object*>* show_objects, std::vector<object*>* hide_objects, std::vector<session*>* show_sessions, std::vector<session*>* hide_sessions, std::vector<object*>* shown_objects, std::vector<object*>* hidden_objects, std::vector<session*>* shown_sessions, std::vector<session*>* hidden_sessions)
{
	return this->move(this->_direction, show_objects, hide_objects, show_sessions, hide_sessions, shown_objects, hidden_objects, shown_sessions, hidden_sessions);
}

fb::game::direction fb::game::object::direction() const
{
	return this->_direction;
}

bool fb::game::object::direction(fb::game::direction value)
{
	if(value != fb::game::direction::LEFT && value != fb::game::direction::TOP && value != fb::game::direction::RIGHT && value != fb::game::direction::BOTTOM)
		return false;

	this->_direction = value;
	return true;
}

fb::game::map* fb::game::object::map() const
{
	return this->_map;
}

uint16_t fb::game::object::map(fb::game::map* map)
{
	if(this->_map != NULL)
		this->_map->object_delete(this);

	this->_map = map;
	return this->_map->object_add(this);
}

bool fb::game::object::sight(const point16_t& position) const
{
	return fb::game::object::sight(this->_position, position, this->_map);
}

bool fb::game::object::sight(const fb::game::object& object) const
{
	if(this->_map == NULL)
		return false;

	if(this->_map != object.map())
		return false;


	if(object.alive() == false)
		return false;

	return this->sight(object.position());
}

bool fb::game::object::sight(const point16_t me, const point16_t you, const fb::game::map* map)
{
	point16_t begin, end;

	if(me.x <= fb::game::map::HALF_SCREEN_WIDTH) // 최좌측
	{
		begin.x = 0;
		end.x   = fb::game::map::MAX_SCREEN_WIDTH;
	}
	else if(me.x >= map->width() - fb::game::map::HALF_SCREEN_WIDTH) // 최우측
	{
		begin.x = std::max(int32_t(0), int32_t(map->width() - fb::game::map::MAX_SCREEN_WIDTH - 1));
		end.x   = std::max(int32_t(0), int32_t(map->width() - 1));
	}
	else
	{
		begin.x = std::max(int32_t(0), int32_t(me.x - fb::game::map::HALF_SCREEN_WIDTH - 1));
		end.x   = std::max(int32_t(0), int32_t(me.x + fb::game::map::HALF_SCREEN_WIDTH + 1));
	}


	if(me.y <= fb::game::map::HALF_SCREEN_HEIGHT) // 최상단
	{
		begin.y = 0;
		end.y   = fb::game::map::MAX_SCREEN_HEIGHT;
	}
	else if(me.y >= map->height() - fb::game::map::HALF_SCREEN_HEIGHT) // 최하단
	{
		begin.y = std::max(int32_t(0), int32_t(map->height() - fb::game::map::MAX_SCREEN_HEIGHT - 1));
		end.y   = std::max(int32_t(0), map->height() - 1);
	}
	else
	{
		begin.y = std::max(int32_t(0), int32_t(me.y - fb::game::map::HALF_SCREEN_HEIGHT - 1));
		end.y   = std::max(int32_t(0), int32_t(me.y + fb::game::map::HALF_SCREEN_HEIGHT + 1));
	}

	return begin.x <= you.x && end.x >= you.x &&
		begin.y <= you.y && end.y >= you.y;
}

fb::game::session* fb::game::object::near_session(fb::game::direction direction) const
{
	fb::game::map* map = this->_map;
	if(map == NULL)
		return NULL;


	point16_t front = this->position();
	switch(direction)
	{
	case fb::game::direction::TOP:
		front.y--;
		break;

	case fb::game::direction::BOTTOM:
		front.y++;
		break;

	case fb::game::direction::LEFT:
		front.x--;
		break;

	case fb::game::direction::RIGHT:
		front.x++;
		break;
	}

	if(map->existable(front) == false)
		return NULL;

	std::vector<fb::game::session*>& sessions = map->sessions();
	for(auto i = sessions.begin(); i != sessions.end(); i++)
	{
		if((*i)->position() == front)
			return *i;
	}

	return NULL;
}

std::vector<fb::game::session*> fb::game::object::near_sessions(fb::game::direction direction) const
{
	std::vector<fb::game::session*> list;

	fb::game::map* map = this->_map;
	if(map == NULL)
		return list;


	point16_t front = this->position();
	switch(direction)
	{
	case fb::game::direction::TOP:
		front.y--;
		break;

	case fb::game::direction::BOTTOM:
		front.y++;
		break;

	case fb::game::direction::LEFT:
		front.x--;
		break;

	case fb::game::direction::RIGHT:
		front.x++;
		break;
	}

	if(map->existable(front) == false)
		return list;

	std::vector<fb::game::session*>& sessions = map->sessions();
	for(auto i = sessions.begin(); i != sessions.end(); i++)
	{
		if((*i)->position() == front)
			list.push_back(*i);
	}

	return list;
}

fb::game::session* fb::game::object::forward_session() const
{
	return this->near_session(this->_direction);
}

std::vector<fb::game::session*> fb::game::object::forward_sessions() const
{
	return this->near_sessions(this->_direction);
}

fb::game::object* fb::game::object::near_object(fb::game::direction direction, fb::game::object::types type) const
{
	fb::game::map* map = this->_map;
	if(map == NULL)
		return NULL;


	point16_t front = this->position();
	switch(direction)
	{
	case fb::game::direction::TOP:
		front.y--;
		break;

	case fb::game::direction::BOTTOM:
		front.y++;
		break;

	case fb::game::direction::LEFT:
		front.x--;
		break;

	case fb::game::direction::RIGHT:
		front.x++;
		break;
	}

	if(map->existable(front) == false)
		return NULL;

	std::vector<fb::game::object*>& objects = map->objects();
	for(auto i = objects.begin(); i != objects.end(); i++)
	{
		if(type != fb::game::object::types::UNKNOWN && (*i)->type() != type)
			continue;

		if((*i)->alive() == false)
			continue;

		if((*i)->position() == front)
			return *i;
	}

	return NULL;
}

std::vector<fb::game::object*> fb::game::object::near_objects(fb::game::direction direction, fb::game::object::types type) const
{
	std::vector<fb::game::object*> list;

	fb::game::map* map = this->_map;
	if(map == NULL)
		return list;


	point16_t front = this->position();
	switch(direction)
	{
	case fb::game::direction::TOP:
		front.y--;
		break;

	case fb::game::direction::BOTTOM:
		front.y++;
		break;

	case fb::game::direction::LEFT:
		front.x--;
		break;

	case fb::game::direction::RIGHT:
		front.x++;
		break;
	}

	if(map->existable(front) == false)
		return list;

	std::vector<fb::game::object*>& objects = map->objects();
	for(auto i = objects.begin(); i != objects.end(); i++)
	{
		if(type != fb::game::object::types::UNKNOWN && (*i)->type() != type)
			continue;

		if((*i)->alive() == false)
			continue;

		if((*i)->position() == front)
			list.push_back(*i);
	}

	return list;
}

fb::game::object* fb::game::object::forward_object(fb::game::object::types type) const
{
	return this->near_object(this->_direction, type);
}

std::vector<fb::game::object*> fb::game::object::forward_objects(fb::game::object::types type) const
{
	return this->near_objects(this->_direction, type);
}

std::vector<fb::game::object*> fb::game::object::shown_objects() const
{
	std::vector<object*> list;

	fb::game::map*          map = this->map();
	if(map == NULL)
		return list;

	auto                    objects = map->objects();
	for(auto i = objects.begin(); i != objects.end(); i++)
	{
		fb::game::object*   object = *i;
		if(object == this)
			continue;

		if(object->alive() == false)
			continue;

		if(this->sight(*object) == false)
			continue;

		list.push_back(object);
	}

	return list;
}

std::vector<fb::game::session*> fb::game::object::shown_sessions() const
{
	std::vector<session*> list;

	fb::game::map*          map = this->map();
	if(map == NULL)
		return list;

	auto                    sessions = map->sessions();
	for(auto i = sessions.begin(); i != sessions.end(); i++)
	{
		fb::game::session*   session = *i;
		if(session == this)
			continue;

		if(this->sight(*session) == false)
			continue;

		list.push_back(session);
	}

	return list;
}

std::vector<fb::game::session*> fb::game::object::looking_sessions() const
{
	std::vector<session*> list;

	fb::game::map*          map = this->map();
	if(map == NULL)
		return list;

	auto                    sessions = map->sessions();
	for(auto i = sessions.begin(); i != sessions.end(); i++)
	{
		fb::game::session*   session = *i;
		if(session == this)
			continue;

		if(session->sight(*this) == false)
			continue;

		list.push_back(session);
	}

	return list;
}

bool fb::game::object::alive() const
{
	return true;
}

fb::ostream fb::game::object::make_show_stream() const
{
	fb::ostream             ostream;

	ostream.write_u8(0x07)
		.write_u16(0x0001) // count
		.write_u16(this->x()) // object x
		.write_u16(this->y()) // object y
		.write_u32(this->id()) // object id
		.write_u16(this->look()) // npc icon code
		.write_u8(this->color())  // color
		.write_u8(this->direction()); // side

	std::cout << "show mob : " << this->id() << std::endl;

	return ostream;
}

fb::ostream fb::game::object::make_hide_stream() const
{
	fb::ostream             ostream;
	ostream.write_u8(0x0E)
		.write_u32(this->id())
		.write_u8(0x00);

	std::cout << "hide mob : " << this->id() << std::endl;

	return ostream;
}

fb::ostream fb::game::object::make_show_stream(const std::vector<fb::game::object*>& objects)
{
	fb::ostream             ostream;

	if(objects.size() == 0)
		return ostream;

	ostream.write_u8(0x07)
		.write_u16(objects.size());

	for(auto i = objects.begin(); i != objects.end(); i++)
	{
		fb::game::object*   object = *i;
		ostream.write_u16(object->x()) // object x
			.write_u16(object->y()) // object y
			.write_u32(object->id()) // object id
			.write_u16(object->look()) // npc icon code
			.write_u8(object->color())  // color
			.write_u8(object->direction()); // side

		std::cout << "show mob : " << object->id() << std::endl;
	}

	return ostream;
}

fb::ostream fb::game::object::make_sound_stream(fb::game::action_sounds sound) const
{
	// 0x019A : 장비 해제 사운드
	// 0x019B : 장비 장착 사운드
	// 0x0006 : 소비아이템 사용 사운드

	fb::ostream             ostream;
	ostream.write_u8(0x19)
		.write_u8(0x00)
		.write_u8(0x03)
		.write_u16(sound) // sound
		.write_u8(100)
		.write_u16(0x0004)
		.write_u32(this->id())
		.write_u32(0x0100)
		.write_u16(0x0202)
		.write_u16(0x0004)
		.write_u8(0x00);

	return ostream;
}

fb::game::life::life() : 
	_hp(0), _base_hp(0),
	_mp(0), _base_mp(0),
	_experience(0),
	_condition(fb::game::condition::NONE)
{
}

fb::game::life::life(uint32_t id, const std::string& name, uint16_t look, uint8_t color, uint32_t hp, uint32_t mp, uint32_t exp) : 
	object(id, name, look, color),
	_hp(hp), _base_hp(hp),
	_mp(mp), _base_mp(mp),
	_experience(exp),
	_condition(fb::game::condition::NONE)
{
}

fb::game::life::life(const fb::game::object& object, uint32_t hp, uint32_t mp, uint32_t exp) : 
	object(object),
	_hp(hp), _base_hp(hp),
	_mp(mp), _base_mp(mp),
	_experience(exp),
	_condition(fb::game::condition::NONE)
{
}


fb::game::life::~life()
{
}

uint32_t fb::game::life::random_damage(uint32_t value, const fb::game::life& life) const
{
	uint32_t Xrate = life.direction() == this->direction() ? 2 : 1;
	uint32_t n = (100 - life.defensive_physical()) / 10;
	float defensive_percent = -125 + (n * (2*14.75f - (n-1)/2.0f))/2.0f;
	uint32_t random_damage = value - (defensive_percent * (value/100.0f));

	return random_damage * Xrate;
}

void fb::game::life::hp_increase(uint32_t value)
{
	uint32_t capacity = 0xFFFFFFFF - this->_base_hp;
	this->_hp += std::min(capacity, value);
}

void fb::game::life::hp_decrease(uint32_t value)
{
	this->_hp -= std::min(value, this->_hp);
}

uint32_t fb::game::life::hp() const
{
	return this->_hp;
}

void fb::game::life::hp(uint32_t value)
{
	this->_hp = value;
}

uint32_t fb::game::life::mp() const
{
	return this->_mp;
}

void fb::game::life::mp(uint32_t value)
{
	this->_mp = value;
}

uint32_t fb::game::life::base_hp() const
{
	return this->_base_hp;
}

void fb::game::life::base_hp(uint32_t value)
{
	this->_base_hp = std::max(uint32_t(1), value);
}

uint32_t fb::game::life::base_mp() const
{
	return this->_base_mp;
}

void fb::game::life::base_mp(uint32_t value)
{
	this->_base_mp = std::max(uint32_t(1), value);
}

uint32_t fb::game::life::experience() const
{
	return this->_experience;
}

void fb::game::life::experience(uint32_t value)
{
	this->_experience = value;
}

uint32_t fb::game::life::defensive_physical() const
{
	return this->_defensive.physical;
}

void fb::game::life::defensive_physical(uint8_t value)
{
	this->_defensive.physical = value;
}

uint32_t fb::game::life::defensive_magical() const
{
	return this->_defensive.magical;
}

void fb::game::life::defensive_magical(uint8_t value)
{
	this->_defensive.magical = value;
}

fb::game::condition fb::game::life::condition() const
{
	return this->_condition;
}

fb::game::condition fb::game::life::condition_add(fb::game::condition value)
{
	this->_condition = fb::game::condition(this->_condition | value);
	return this->_condition;
}

fb::game::condition fb::game::life::condition_remove(fb::game::condition value)
{
	this->_condition = fb::game::condition(this->_condition & ~value);
	return this->_condition;
}

bool fb::game::life::condition_contains(fb::game::condition value) const
{
	return uint32_t(this->_condition) & uint32_t(value);
}

bool fb::game::life::alive() const
{
	return this->_hp != 0;
}

fb::ostream fb::game::life::make_move_stream() const
{
	fb::ostream				ostream;

	ostream.write_u8(0x0C)
		.write_u32(this->_id)
		.write_u16(this->_position.x)
		.write_u16(this->_position.y)
		.write_u8(this->_direction)
		.write_u8(0x00);

	return ostream;
}

fb::ostream fb::game::life::make_move_stream(fb::game::direction direction) const
{
	fb::ostream				ostream;

	ostream.write_u8(0x0C)
		.write_u32(this->_id)
		.write_u16(this->_position.x)
		.write_u16(this->_position.y)
		.write_u8(direction)
		.write_u8(0x00);

	return ostream;
}

fb::ostream fb::game::life::make_direction_stream() const
{
	fb::ostream             ostream;
	ostream.write_u8(0x11)
		.write_u32(this->_id)
		.write_u8(this->_direction)
		.write_u8(0x00);

	return ostream;
}

fb::ostream fb::game::life::make_action_stream(fb::game::action action, fb::game::duration duration) const
{
	uint8_t sound = 0x00;

	fb::ostream             ostream;
	ostream.write_u8(0x1A)
		.write_u32(this->_id)
		.write_u8(action) // type
		.write_u16(duration) // duration
		.write_u8(sound); // sound

	return ostream;
}

fb::ostream fb::game::life::make_show_hp_stream(uint32_t random_damage, bool critical) const
{
	fb::ostream				ostream;
	uint8_t					percentage = uint8_t((this->_hp / float(this->_base_hp)) * 100);

	ostream.write_u8(0x13)
		.write_u32(this->_id)
		.write_u8(critical)
		.write_u8(percentage)
		.write_u32(random_damage)
		.write_u8(0x00);

	return ostream;
}

fb::ostream fb::game::life::make_die_stream() const
{
	fb::ostream				ostream;

	ostream.write_u8(0x5F)
		.write_u32(this->_id)
		.write_u8(0x00);

	return ostream;
}