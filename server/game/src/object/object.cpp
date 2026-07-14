#include <fb/game/character.h>
#include <fb/game/map.h>
#include <fb/game/server.h>
#include <fb/model/model.h>
#include <fb/game/object.h>
#include <format>

using namespace fb::game;

object::object(fb::game::server& server, const fb::model::object& model, const initial_params& params) :
    fb::thread_switchable(params.id),
    server(server),
    listener(server.listener),
    _oid(params.id),
    _model(model),
    _position(params.position),
    _direction(params.direction),
    _map(params.map),
    buffs(*this)
{
    this->listener.on_create(*this);
}

object::object(const object& right) :
    object(
        right.server,
        right._model,
        initial_params{.id = right._oid, .position = right._position, .direction = right._direction, .map = right._map})
{ }

object::~object()
{
    this->listener.on_destroy(*this);
}

const fb::model::object& object::based() const
{
    return this->_model;
}

bool object::is(OBJECT_TYPE type) const
{
    auto mine = this->what();
    return ENUM_IN(mine, type);
}

const std::string& object::name() const
{
    return this->_model.name;
}

uint16_t object::look() const
{
    return this->_model.look;
}

uint8_t object::color() const
{
    return this->_model.color;
}

OBJECT_TYPE object::what() const
{
    return this->_model.what();
}

void object::update_external(bool detailed)
{
    this->assert_thread();
    this->listener.on_update_external(*this, detailed);
}

void object::update_external(object& to, bool detailed)
{
    this->assert_thread();
    this->listener.on_update_external(*this, to, detailed);
}

bool object::super_hide() const
{
    return false;
}

bool object::hidden(const object& target) const
{
    return false;
}

void object::on_init()
{ }

async::task<void> object::destroy(DESTROY_TYPE destroy_type)
{
    this->assert_thread();

    co_await this->server.destroy(*this, destroy_type);
}

async::task<size_t> object::send(const fb::stream& stream, bool encrypt, bool wrap)
{
    this->assert_thread();
    co_return 0;
}

async::task<size_t> object::send(const fb::protocol::header& response, bool encrypt, bool wrap)
{
    this->assert_thread();
    co_return 0;
}

uint32_t object::oid() const
{
    this->assert_thread();

    if (this->_map == nullptr)
        return 0xFFFFFFFD;

    return this->_oid;
}

void object::oid(uint32_t value)
{
    this->assert_thread();

    this->_oid = value;
    this->update_id();
}

void object::chat(std::string_view message, CHAT_TYPE chat_type, bool decorate)
{
    this->assert_thread();

    if (decorate)
    {
        auto decorated = std::string{};
        switch (chat_type)
        {
        case CHAT_TYPE::SHOUT:
            decorated = std::format("{}! {}", this->name(), message);
            break;

        default:
            decorated = std::format("{}: {}", this->name(), message);
            break;
        }

        this->listener.on_chat(*this, decorated, chat_type);
    }
    else
    {
        this->listener.on_chat(*this, message, chat_type);
    }
}

const fb::model::point16_t& object::position() const
{
    this->assert_thread();

    return this->_position;
}

bool object::position(uint16_t x, uint16_t y, bool refresh)
{
    this->assert_thread();

    if (this->_map == nullptr)
        return false;

    if (this->_position.x == x && this->_position.y == y)
        return true;

    auto before       = this->_position;
    this->_position.x = std::max(0, std::min(this->_map->width() - 1, int32_t(x)));
    this->_position.y = std::max(0, std::min(this->_map->height() - 1, int32_t(y)));

    if (refresh)
        this->update_position();

    if (sight(before, this->_position, this->_map) == false)
        this->update_external(*this, true);

    this->update_sector();

    for (auto& obj : this->_map->nears(before))
    {
        if (this == obj.get())
            continue;

        // I left the other object's sight
        if (obj->sight(before) && !obj->sight(*this))
        {
            this->hide(*obj);
        }

        // Other object left my sight
        if (sight(before, obj->_position, this->_map) && !this->sight(*obj))
        {
            obj->hide(*this);
        }
    }

    for (auto& obj : this->_map->nears(this->_position))
    {
        if (this == obj.get())
            continue;

        if (this->hidden(*obj) == false)
        {
            auto before_sight = obj->sight(before);
            auto after_sight  = obj->sight(*this);

            if (!before_sight && after_sight) // I entered the other object's sight
            {
                this->update_external(*obj, true);
            }
            else if (refresh && before_sight && after_sight) // Force refresh while already in sight
            {
                this->update_external(*obj, true);
            }
            else
            {
            }
        }

        // Other object entered my sight
        if (obj->hidden(*this) == false)
        {
            if (!sight(before, obj->_position, this->_map) && this->sight(*obj))
            {
                obj->update_external(*this, true);
            }
        }
    }

    return true;
}

bool object::position(const fb::model::point16_t position, bool refresh)
{
    this->assert_thread();

    return this->position(position.x, position.y, refresh);
}

bool object::move()
{
    this->assert_thread();

    return this->move(this->_direction);
}

bool object::move(DIRECTION direction)
{
    this->assert_thread();

    if (this->_map == nullptr)
        return false;

    auto after = this->side_position(direction);
    if (this->_map->movable(*this, after) == false)
        return false;

    if (this->direction(direction) == false)
        return false;

    auto before = this->_position;
    this->position(after);

    {
        auto lua = this->server.lua.open("scripts/interaction.lua", "on_move");
        if (lua)
        {
            lua->pushobject(*this);
            std::ignore = lua->call(1);
        }
    }

    {
        auto& map_model = this->_map->model;
        auto  path      = std::format("scripts/map/{}.lua", map_model.id);
        auto  func      = std::format("ON_MAP_MOVE_{}", map_model.id);

        auto map_lua = this->server.lua.open(path, func);
        if (map_lua)
        {
            map_lua->pushobject(*this);
            std::ignore = map_lua->call(1);
        }
    }

    this->listener.on_move(*this, before);

    return true;
}

uint16_t object::x() const
{
    this->assert_thread();

    return this->_position.x;
}

bool object::x(uint16_t value)
{
    this->assert_thread();

    return this->position(value, this->_position.y);
}

uint16_t object::y() const
{
    this->assert_thread();

    return this->_position.y;
}

bool object::y(uint16_t value)
{
    this->assert_thread();

    return this->position(this->_position.x, value);
}

DIRECTION object::direction() const
{
    this->assert_thread();

    return this->_direction;
}

bool object::direction(DIRECTION value)
{
    this->assert_thread();

    if (value != DIRECTION::LEFT && value != DIRECTION::TOP && value != DIRECTION::RIGHT && value != DIRECTION::BOTTOM)
        return false;

    this->_direction = value;

    auto lua = this->server.lua.open("scripts/interaction.lua", "on_direction");
    if (lua)
    {
        lua->pushobject(*this);
        std::ignore = lua->call(1);
    }

    this->listener.on_direction(*this);

    return true;
}

std::shared_ptr<fb::game::map> object::map() const
{
    this->assert_thread();
    auto _ = std::shared_lock(this->_map_lock);
    return this->_map;
}

async::task<void> object::invoke_map_character_hook(const fb::model::map& map_model, std::string_view hook)
{
    if (this->is(OBJECT_TYPE::CHARACTER) == false)
        co_return;

    auto path = std::format("scripts/map/{}.lua", map_model.id);
    auto func = std::format("{}{}", hook, map_model.id);

    auto lua = this->server.lua.open(path, func);
    if (!lua)
        co_return;

    lua->pushobject(*this);
    std::ignore = co_await lua->call(1);
}

void object::update_sector()
{
    this->assert_thread();

    auto before = this->_sector;
    auto after  = this->_map->sectors.at(this->_position);
    if (before == after)
        return;

    if (before)
        before->erase(this->shared_from_this_as<object>());

    this->_sector = nullptr;
    if (this->_map == nullptr)
        return;

    this->_sector = after;
    if (after)
        after->push(this->shared_from_this_as<object>());
}

bool object::active_sector() const
{
    this->assert_thread();
    return this->_sector != nullptr && this->_map->sectors.activated(*this->_sector);
}

bool object::sight(const fb::model::point16_t& position) const
{
    this->assert_thread();

    return object::sight(this->_position, position, this->_map);
}

bool object::sight(const object& object) const
{
    this->assert_thread();

    if (this->_map == nullptr)
        return false;

    if (this->_map != object.map())
        return false;

    return this->sight(object._position);
}

bool object::sight(const fb::model::point16_t me, const fb::model::point16_t you, const map_ptr& map)
{
    return map != nullptr && object::sight_area(me, map).contains(you);
}

fb::model::area<uint16_t> object::sight_area(const fb::model::point16_t& position, const map_ptr& map)
{
    if (map == nullptr)
        return fb::model::area<uint16_t>(0, 0, 0, 0);

    fb::model::point16_t begin, end;

    if (position.x <= map::HALF_SCREEN_WIDTH)
    {
        begin.x = 0;
        end.x   = map::MAX_SCREEN_WIDTH;
    }
    else if (position.x >= map->width() - map::HALF_SCREEN_WIDTH)
    {
        begin.x = std::max(int32_t(0), int32_t(map->width() - map::MAX_SCREEN_WIDTH - 1));
        end.x   = std::max(int32_t(0), int32_t(map->width() - 1));
    }
    else
    {
        begin.x = std::max(int32_t(0), int32_t(position.x - map::HALF_SCREEN_WIDTH - 1));
        end.x   = std::max(int32_t(0), int32_t(position.x + map::HALF_SCREEN_WIDTH + 1));
    }

    if (position.y <= map::HALF_SCREEN_HEIGHT)
    {
        begin.y = 0;
        end.y   = map::MAX_SCREEN_HEIGHT;
    }
    else if (position.y >= map->height() - map::HALF_SCREEN_HEIGHT)
    {
        begin.y = std::max(int32_t(0), int32_t(map->height() - map::MAX_SCREEN_HEIGHT - 1));
        end.y   = std::max(int32_t(0), map->height() - 1);
    }
    else
    {
        begin.y = std::max(int32_t(0), int32_t(position.y - map::HALF_SCREEN_HEIGHT - 1));
        end.y   = std::max(int32_t(0), int32_t(position.y + map::HALF_SCREEN_HEIGHT + 1));
    }

    return fb::model::area<uint16_t>(static_cast<uint16_t>(begin.x),
                                     static_cast<uint16_t>(begin.y),
                                     static_cast<uint16_t>(end.x) + 1,
                                     static_cast<uint16_t>(end.y) + 1);
}

fb::model::area<uint16_t> object::sight_area() const
{
    this->assert_thread();
    return object::sight_area(this->_position, this->_map);
}

async::task<bool> object::map(map_ptr map, std::optional<fb::model::point16_t> position, map_options options)
{
    this->assert_thread();

    if (map != nullptr && position.has_value() == false)
        position = map->model.spawn_position();

    auto  weak         = this->weak_from_this_as<object>();
    auto& context      = this->server;
    auto  resolved     = position.value_or(fb::model::point16_t{0, 0});
    auto  destroy_type = options.destroy_type;
    auto  notify       = options.notify;
    try
    {
        if (this->_map == map)
        {
            this->position(resolved, true);
            co_return true;
        }

        // if destination is null, erase cache of map
        // and broadcast all near characters
        // and set default map(id = 0) and position(1, 1)
        if (map == nullptr)
        {
            if (this->_map != nullptr)
                co_await this->invoke_map_character_hook(this->_map->model, "ON_MAP_LEAVE_");

            // broadcast near characters
            for (const auto& x : this->_map->nears(this->_position))
            {
                if (x.get() != this)
                    this->hide(*x, destroy_type);
            }

            // erase cache of map
            this->_map->objects.pop(*this);

            // don't call 'update_sector'
            // when object's map has changed, active thread is changed too.
            if (this->_sector != nullptr)
            {
                this->_sector->erase(this->shared_from_this_as<object>());
                this->_sector = nullptr;
            }

            // Call listener for packet response
            this->listener.on_map_leave(*this, *this->_map);

            // Handle thread management for characters
            if (this->is(OBJECT_TYPE::CHARACTER))
            {
                auto& ch     = static_cast<character&>(*this);
                auto  thread = this->_map->thread();
                if (thread != nullptr)
                {
                    auto params = thread->template data<thread_params>();
                    params->characters.remove(ch.shared_from_this_as<character>());
                }
                this->_map->on_character_leave();
            }

            {
                auto _     = std::unique_lock(this->_map_lock);
                this->_map = nullptr;
            }
            co_await this->server.threads.switching(weak);
            this->_position = fb::model::point16_t(1, 1);
            co_return true;
        }

        if (map->active == false)
            co_return false;

        if (map->closing())
            co_return false;

        if (this->server.maps.ensure_loaded(map) == false)
            co_return false;

        // here the character is on some map.
        // set map to null.
        auto before_position = fb::model::point16_t{resolved};
        if (this->_map != nullptr)
        {
            before_position.x = std::min<uint16_t>(resolved.x, map->width() - 1);
            before_position.y = std::min<uint16_t>(resolved.y, map->height() - 1);
        }

        if (this->_map != nullptr)
        {
            map_options leave_options;
            leave_options.destroy_type = destroy_type;
            std::ignore                = co_await this->map(nullptr, std::nullopt, leave_options);
        }

        {
            auto _ = std::unique_lock(this->_map_lock);

            this->_map    = map;
            this->_thread = map->thread();
        }

        co_await this->server.threads.switching(weak);

        // Call listener for packet response
        this->listener.on_map_enter(*this, *map);

        // Handle thread management for characters
        if (this->is(OBJECT_TYPE::CHARACTER))
        {
            auto& ch     = static_cast<character&>(*this);
            auto  thread = map->thread();
            if (thread != nullptr)
            {
                auto params = thread->template data<thread_params>();
                params->characters.insert(ch.shared_from_this_as<character>());
            }
        }

        this->_position = before_position;

        this->update_sector();

        // insert character into map cache
        this->_map->objects.push(*this);
        if (this->is(OBJECT_TYPE::CHARACTER))
            map->on_character_enter();
        this->update_map(*map);
        this->update_position();
        if (notify)
            this->update_external(true);
        this->update_bgm(map->model.bgm, 100);

        if (notify)
        {
            for (auto& obj : map->nears(this->_position))
            {
                if (obj.get() == this)
                    continue;

                obj->update_external(*this, true);
            }
        }

        co_await this->invoke_map_character_hook(map->model, "ON_MAP_ENTER_");

        co_return true;
    }
    catch (std::exception& e)
    {
        co_return false;
    }
    catch (...)
    {
        co_return false;
    }
}

fb::model::point16_t object::side_position(DIRECTION direction, int step) const
{
    auto front = this->position();
    switch (direction)
    {
    case DIRECTION::TOP:
        front.y -= step;
        break;

    case DIRECTION::BOTTOM:
        front.y += step;
        break;

    case DIRECTION::LEFT:
        front.x -= step;
        break;

    case DIRECTION::RIGHT:
        front.x += step;
        break;
    }

    return front;
}

fb::model::point16_t object::front_position(int step) const
{
    return this->side_position(this->_direction, step);
}

std::shared_ptr<fb::game::object> object::side(DIRECTION direction, OBJECT_TYPE type) const
{
    this->assert_thread();

    auto map = this->_map;
    if (map == nullptr)
        return nullptr;

    auto front = side_position(direction);
    if (map->in_ground(front) == false)
        return nullptr;

    auto nears = map->nears(this->_position, type);
    auto found = std::find_if(nears.begin(), nears.end(), [this, &front](auto x) {
        if (x->hidden(*this))
            return false;

        return x->position() == front;
    });

    return found != nears.end() ? *found : nullptr;
}

std::vector<std::shared_ptr<fb::game::object>> object::sides(DIRECTION direction, OBJECT_TYPE type) const
{
    this->assert_thread();

    auto result = std::vector<std::shared_ptr<fb::game::object>>();
    try
    {
        auto map = this->_map;
        if (map == nullptr)
            throw std::exception();

        auto front = this->position();
        switch (direction)
        {
        case DIRECTION::TOP:
            front.y--;
            break;

        case DIRECTION::BOTTOM:
            front.y++;
            break;

        case DIRECTION::LEFT:
            front.x--;
            break;

        case DIRECTION::RIGHT:
            front.x++;
            break;
        }

        if (map->in_ground(front) == false)
            throw std::exception();

        auto nears = map->nears(this->_position, type);
        std::copy_if(nears.begin(), nears.end(), std::back_inserter(result), [this, &front](auto x) {
            if (x->hidden(*this))
                return false;

            return x->position() == front;
        });
    }
    catch (std::exception&)
    { }

    return std::move(result);
}

std::shared_ptr<fb::game::object> object::forward(OBJECT_TYPE type) const
{
    this->assert_thread();

    return this->side(this->_direction, type);
}

std::vector<std::shared_ptr<fb::game::object>> object::forwards(OBJECT_TYPE type) const
{
    this->assert_thread();

    return this->sides(this->_direction, type);
}

std::vector<std::shared_ptr<fb::game::object>> object::sight_in(OBJECT_TYPE type) const
{
    this->assert_thread();
    auto result = std::vector<std::shared_ptr<fb::game::object>>{};
    for (auto& obj : this->nears())
    {
        if (obj->is(type) == false)
            continue;

        if (obj->hidden(*this))
            continue;

        if (this->sight(*obj) == false && obj->sight(*this))
            continue;

        result.push_back(std::static_pointer_cast<fb::game::object>(obj));
    }

    return std::move(result);
}

std::vector<std::shared_ptr<fb::game::object>> object::nears(OBJECT_TYPE type, bool contains_super_hide) const
{
    this->assert_thread();
    if (this->_map == nullptr)
        return {};

    auto result = std::vector<std::shared_ptr<fb::game::object>>{};
    for (const auto& obj : this->_map->nears(this->_position))
    {
        if (this == obj.get())
            continue;

        if (!contains_super_hide && obj->hidden(*this))
            continue;

        if (obj->is(type) == false)
            continue;

        result.push_back(std::static_pointer_cast<fb::game::object>(obj));
    }

    return std::move(result);
}

double object::distance(const object& right) const
{
    this->assert_thread();

    return std::sqrt(this->distance_sqrt(right));
}

uint32_t object::distance_sqrt(const object& right) const
{
    this->assert_thread();

    return (uint32_t)std::pow(this->_position.x - right._position.x, 2) +
           (uint32_t)std::pow(this->_position.y - right._position.y, 2);
}

bool object::condition(const std::vector<fb::model::dsl>& conditions) const
{
    this->assert_thread();

    return true;
}

bool object::available() const
{
    this->assert_thread();

    return true;
}

void object::hide(DESTROY_TYPE destroy_type)
{
    this->listener.on_hide(*this, destroy_type);
}

void object::hide(object& to, DESTROY_TYPE destroy_type)
{
    this->listener.on_hide(*this, to, destroy_type);
}

void object::thread(fb::thread* value)
{
    this->assert_thread();
    auto _ = std::unique_lock(this->_map_lock);

    this->_thread = value;
}

fb::thread* object::thread() const
{
    auto _ = std::shared_lock(this->_map_lock);

    if (this->_thread != nullptr)
        return this->_thread;
    else
        return this->server.threads.modular(this->_model.id);
}

void object::update_id()
{ }

void object::update_map(const fb::game::map& map)
{ }

void object::update_bgm(uint16_t bgm, uint8_t volume)
{ }

void object::update_position()
{ }

void object::sound(SOUND sound)
{
    this->listener.on_sound(*this, sound);
}

void object::effect(uint8_t value)
{
    this->listener.on_effect(*this, value);
}

bool object::operator== (const object& right) const
{
    this->assert_thread();

    return this->_map == right._map && this->oid() == right.oid();
}

bool object::operator!= (const object& right) const
{
    this->assert_thread();

    return !((*this) == right);
}