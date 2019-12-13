#ifndef __MAP_H__
#define	__MAP_H__

#include <stdint.h>
#include <string>
#include <stdexcept>
#include <sstream>
#include "stream.h"
#include "zlib/zlib.h"
#include "object.h"

namespace fb { namespace game {

class session;

class map
{
public:
	enum options : uint8_t {NO_OPTION = 0x00, BUILD_IN = 0x01, DISABLE_TALK = 0x02, DISABLE_WHISPER = 0x04, DISABLE_MAGIC = 0x08, HUNTING_GROUND = 0x10, ENABLE_PK = 0x20, DISABLE_DIE_PENALTY = 0x30};
	enum effects : uint8_t {NO_EFFECT = 0x00, HOT_WAVE = 0x01, SHIPWRECT = 0x02};

	static const uint32_t MAX_SCREEN_WIDTH   = 17;
	static const uint32_t HALF_SCREEN_WIDTH  = uint32_t(MAX_SCREEN_WIDTH / 2);
	static const uint32_t MAX_SCREEN_HEIGHT  = 15;
	static const uint32_t HALF_SCREEN_HEIGHT = uint32_t(MAX_SCREEN_HEIGHT / 2);

public:
	typedef struct _tile
	{
		uint16_t id;
		uint16_t object;
		bool blocked;
	} tile;

	typedef struct _warp
	{
	public:
		map* map;
		const point16_t before, after;
		const range8_t limit;

	public:
		_warp(fb::game::map* map, const point16_t& before, const point16_t& after, const range8_t limit) : 
			map(map), before(before), after(after), limit(limit) {}
		_warp(const _warp& right) : 
			map(right.map), before(right.before), after(right.after), limit(right.limit)
		{}
		~_warp() {}
	} warp;

private:
	uint16_t				_id;
	uint16_t				_parent;
	size16_t				_size;
	tile*					_tiles;
	std::string				_name;
	options					_option;
	effects					_effect;
	uint8_t					_bgm;
	uint16_t				_sequence;
	std::vector<object*>	_objects;
	std::vector<fb::game::session*> _sessions;
	std::vector<warp*>		_warps;

public:
	map(uint16_t id, uint16_t parent, uint8_t bgm, const std::string& name, options option, effects effect, const void* data, uint32_t size);
	~map();

private:
	uint16_t				empty_seq();

public:
	uint16_t				id() const;
	uint16_t				parent() const;
	const std::string&		name() const;
	bool					blocked(uint16_t x, uint16_t y) const;
	bool					block(uint16_t x, uint16_t y, bool option);
	effects					effect() const;
	options					option() const;
	uint16_t				width() const;
	uint16_t				height() const;
	size16_t				size() const;
	uint8_t					bgm() const;

	std::vector<object*>&	objects();
	fb::game::object*		object(uint16_t id);
	uint16_t				object_add(fb::game::object* object);
	bool					object_delete(fb::game::object* object);
	uint16_t				object_add(fb::game::object* object, const point16_t position);
	fb::game::object*		object_exists(point16_t position) const;

	std::vector<fb::game::object*>	all() const;

	std::vector<fb::game::session*>& sessions();
	bool					session_add(fb::game::session* session);
	bool					session_delete(fb::game::session* session);

	bool					existable(const point16_t position) const;
	bool					movable(const point16_t position) const;
	bool					movable(const fb::game::object& object, fb::game::direction direction) const;
	bool					movable_forward(const fb::game::object& object, uint16_t step = 1) const;

	void					warp_add(map* map, const point16_t& before, const point16_t& after, const range8_t& limit);
	const map::warp*		warpable(const point16_t& position) const;

public:
	void					handle_timer(uint64_t elapsed_milliseconds);

public:
	tile*					operator () (uint16_t x, uint16_t y) const;

public:
	fb::ostream				make_config_stream() const;
	fb::ostream				make_bgm_stream() const;
	fb::ostream				make_update_stream(uint16_t begin_x, uint16_t begin_y, uint8_t width, uint8_t height, uint16_t crc) const;
};

} }

#endif // !__MAP_H__
