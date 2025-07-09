#include <fb/game/character.h>
#include <fb/game/context.h>
#include <fb/model/model.h>
#include <fb/game/object.h>

using namespace fb::game;

object::object(fb::game::context& context, const fb::model::object& model, const initial_params& params) :
    fb::thread_switchable(params.id),
    context(context),
    listener(context.listener),
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
        right.context,
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
    return (type & mine) == mine;
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

    co_await this->context.destroy(*this, destroy_type);
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
}

void object::chat(const std::string& message, CHAT_TYPE chat_type, bool decorate)
{
    this->assert_thread();

    if (decorate)
    {
        auto decorated = std::string{message};
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

        // 상대 시야에서 내가 사라짐
        if (obj->sight(before) && !obj->sight(*this))
        {
            this->hide(*obj);
        }

        // 내 시야에서 상대방이 사라짐
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

            if (!before_sight && after_sight) // 상대 시야에 내가 추가됨
            {
                this->update_external(*obj, true);
            }
            else if (refresh && before_sight && after_sight) // 상대 시야에 원래 있었는데 위치 강제이동
            {
                this->update_external(*obj, true);
            }
            else
            {
            }
        }

        // 내 시야에 상대가 추가됨
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

    // Execute move script
    auto lua = fb::lua::new_context();
    if (lua != nullptr)
    {
#if defined DEBUG | defined _DEBUG
        lua->load("scripts/interaction.lua");
#endif
        lua->func("on_move");
        lua->pushobject(*this);
        std::ignore = lua->call(1);
    }

    // Call listener for packet response
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

    if (this->_direction == value)
        return true;

    this->_direction = value;

    // Execute direction change script
    auto lua = fb::lua::new_context();
    if (lua != nullptr)
    {
#if defined DEBUG | defined _DEBUG
        lua->load("scripts/interaction.lua");
#endif
        lua->func("on_direction");
        lua->pushobject(*this);
        std::ignore = lua->call(1);
    }

    // Call listener for packet response
    this->listener.on_direction(*this);

    return true;
}

std::shared_ptr<fb::game::map> object::map() const
{
    this->assert_thread();

    return this->_map;
}

void object::update_sector()
{
    this->assert_thread();

    auto before = this->_sector;
    auto after  = this->_map->sector_at(this->_position);
    if (before == after)
        return;

    if (before)
        before->erase(std::static_pointer_cast<fb::game::object>(this->shared_from_this()));

    this->_sector.reset();
    if (this->_map == nullptr)
        return;

    this->_sector = after;
    if (after)
        after->push(std::static_pointer_cast<fb::game::object>(this->shared_from_this()));
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

bool object::sight(const fb::model::point16_t            me,
                   const fb::model::point16_t            you,
                   const std::shared_ptr<fb::game::map>& map)
{
    fb::model::point16_t begin, end;

    if (me.x <= map::HALF_SCREEN_WIDTH) // 최좌측
    {
        begin.x = 0;
        end.x   = map::MAX_SCREEN_WIDTH;
    }
    else if (me.x >= map->width() - map::HALF_SCREEN_WIDTH) // 최우측
    {
        begin.x = std::max(int32_t(0), int32_t(map->width() - map::MAX_SCREEN_WIDTH - 1));
        end.x   = std::max(int32_t(0), int32_t(map->width() - 1));
    }
    else
    {
        begin.x = std::max(int32_t(0), int32_t(me.x - map::HALF_SCREEN_WIDTH - 1));
        end.x   = std::max(int32_t(0), int32_t(me.x + map::HALF_SCREEN_WIDTH + 1));
    }

    if (me.y <= map::HALF_SCREEN_HEIGHT) // 최상단
    {
        begin.y = 0;
        end.y   = map::MAX_SCREEN_HEIGHT;
    }
    else if (me.y >= map->height() - map::HALF_SCREEN_HEIGHT) // 최하단
    {
        begin.y = std::max(int32_t(0), int32_t(map->height() - map::MAX_SCREEN_HEIGHT - 1));
        end.y   = std::max(int32_t(0), map->height() - 1);
    }
    else
    {
        begin.y = std::max(int32_t(0), int32_t(me.y - map::HALF_SCREEN_HEIGHT - 1));
        end.y   = std::max(int32_t(0), int32_t(me.y + map::HALF_SCREEN_HEIGHT + 1));
    }

    return begin.x <= you.x && end.x >= you.x && begin.y <= you.y && end.y >= you.y;
}

async::task<bool> object::map(std::shared_ptr<fb::game::map> map, DESTROY_TYPE destroy_type)
{
    this->assert_thread();

    if (map == nullptr)
    {
        co_return co_await this->map(map, fb::model::point16_t{0, 0}, destroy_type);
    }
    else
    {
        if (map->model.teleport.size() == 0)
        {
            co_return co_await this->map(map, fb::model::point16_t{0, 0}, destroy_type);
        }

        auto& dsl = map->model.teleport.at(random<uint32_t>(0, map->model.teleport.size() - 1));
        switch (dsl.header)
        {
        case DSL::area:
        {
            auto params = fb::model::dsl::area(dsl.params);
            auto x      = random<uint16_t>(params.left, params.right);
            auto y      = random<uint16_t>(params.top, params.bottom);
            co_return co_await this->map(map, fb::model::point16_t{x, y});
        }
        break;

        case DSL::point:
        {
            auto params = fb::model::dsl::point(dsl.params);
            co_return co_await this->map(map, fb::model::point16_t{params.x, params.y});
        }
        break;
        }
    }
}

async::task<bool> object::map(std::shared_ptr<fb::game::map> map,
                              const fb::model::point16_t&    position,
                              DESTROY_TYPE                   destroy_type,
                              bool                           notify)
{
    this->assert_thread();

    auto  weak    = this->weak_from_this_as<object>();
    auto& context = this->context;
    try
    {
        if (this->_map == map)
        {
            this->position(position, true);
            co_return true;
        }

        // if destination is null, erase cache of map
        // and broadcast all near characters
        // and set default map(id = 0) and position(1, 1)
        if (map == nullptr)
        {
            // broadcast near characters
            for (auto x : this->_map->nears(this->_position))
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
                this->_sector.reset();
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
                    params->characters.erase(ch.id());
                }
            }

            this->_map = nullptr;
            co_await this->context.threads.switching(weak);
            this->_position = fb::model::point16_t(1, 1);
            co_return true;
        }

        if (map->active == false)
            co_return false;

        // here the character is on some map.
        // set map to null.
        auto before_position = fb::model::point16_t{position};
        if (this->_map != nullptr)
        {
            before_position.x = std::min<uint16_t>(position.x, map->width() - 1);
            before_position.y = std::min<uint16_t>(position.y, map->height() - 1);
        }

        if (this->_map != nullptr)
            std::ignore = co_await this->map(nullptr);

        this->_map = map;
        if (this->is(OBJECT_TYPE::CHARACTER))
            static_cast<character*>(this)->thread(map->thread());

        co_await this->context.threads.switching(weak);

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
                params->characters.insert({ch.id(), ch.shared_from_this_as<character>()});
            }
        }

        this->_position = before_position;

        this->update_sector();

        // insert character into map cache
        this->_map->objects.push(*this);
        this->update_id();
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
    auto result = std::vector<std::shared_ptr<fb::game::object>>{};
    for (auto obj : this->nears())
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
    if (this->_map == nullptr)
        return {};

    auto result = std::vector<std::shared_ptr<fb::game::object>>{};
    for (auto obj : this->_map->nears(this->_position))
    {
        if (this == obj.get())
            continue;

        if (!contains_super_hide && obj->hidden(*this))
            continue;

        if (type != OBJECT_TYPE::UNKNOWN && obj->is(type) == false)
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

fb::thread* object::thread() const
{
    if (this->_map == nullptr)
        return this->context.threads.modular(this->_oid);
    else
        return this->context.threads.modular(this->_map->model.id);
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