#include <fb/game/character.h>
#include <fb/game/context.h>
#include <fb/model/model.h>
#include <fb/game/object.h>

using namespace fb::game;

object::object(fb::game::context& context, const fb::model::object& model, const initial_params& params) :
    luable(params.id),
    context(context),
    _listener(&context),
    _sequence(params.id),
    _model(model),
    _position(params.position),
    _direction(params.direction),
    _map(params.map),
    buffs(*this)
{
    if (this->_listener != nullptr)
    {
        this->_listener->on_create(*this);
    }
}

object::object(const object& right) :
    object(right.context,
           right._model,
           initial_params{.id        = right._sequence,
                          .position  = right._position,
                          .direction = right._direction,
                          .map       = right._map})
{ }

object::~object()
{
    if (this->_listener != nullptr)
    {
        this->_listener->on_destroy(*this);
    }
}

const fb::model::object& object::based() const
{
    this->assert_thread();

    return this->_model;
}

bool object::is(OBJECT_TYPE type) const
{
    this->assert_thread();

    auto mine = this->what();
    return (type & mine) == mine;
}

const std::string& object::name() const
{
    this->assert_thread();

    return this->_model.name;
}

uint16_t object::look() const
{
    this->assert_thread();

    return this->_model.look;
}

uint8_t object::color() const
{
    this->assert_thread();

    return this->_model.color;
}

OBJECT_TYPE object::what() const
{
    this->assert_thread();

    return this->_model.what();
}

void object::update_external(bool light)
{
    this->assert_thread();

    if (this->_listener != nullptr)
        this->_listener->on_update_external(*this, light);
}

void object::update_external(object& to, bool light)
{
    this->assert_thread();

    if (this->_listener != nullptr)
        this->_listener->on_update_external(*this, to, light);
}

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

uint32_t object::sequence() const
{
    this->assert_thread();

    return this->_sequence;
}

void object::sequence(uint32_t value)
{
    this->assert_thread();

    this->_sequence = value;
}

void object::chat(const std::string& message, CHAT_TYPE chat_type)
{
    this->assert_thread();

    if (this->_listener != nullptr)
        this->_listener->on_chat(*this, message, chat_type);
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

    this->_map->update(*this);

    auto nears_before = this->_map->nears(before);
    auto nears_after  = this->_map->nears(this->_position);

    // leave
    {
        // 내 이전 위치에서 내 시야에 보이는 오브젝트들
        auto befores = this->showings(nears_before, before);
        std::sort(befores.begin(), befores.end());

        // 내 현재 위치에서 내 시야에 보이는 오브젝트들
        auto afters = this->showings(nears_after, this->_position);
        std::sort(afters.begin(), afters.end());

        // 내가 이동한 뒤 내 시야에서 사라진 오브젝트들
        auto hides = std::vector<object*>();
        std::set_difference(befores.begin(),
                            befores.end(),
                            afters.begin(),
                            afters.end(),
                            std::inserter(hides, hides.begin()));
        for (auto x : hides)
            x->hide(*this, DESTROY_TYPE::DEFAULT);

        // 내가 이동한 뒤 내 시야에서 나타난 오브젝트들
        auto shows = std::vector<object*>();
        std::set_difference(afters.begin(),
                            afters.end(),
                            befores.begin(),
                            befores.end(),
                            std::inserter(shows, shows.begin()));

        for (auto x : shows)
        {
            x->update_external(*this, false);
        }

        if (refresh)
        {
            // 내가 이동한 뒤 내 시야에 여전히 남은 오브젝트들
            auto stay = std::vector<object*>();
            std::set_difference(afters.begin(),
                                afters.end(),
                                shows.begin(),
                                shows.end(),
                                std::inserter(stay, stay.begin()));
            for (auto x : stay)
            {
                x->update_external(*this, false);
            }
        }
    }

    // enter
    {
        // 내 이전 위치에서 내가 포함된 시야를 가진 오브젝트들
        auto befores = this->showns(nears_before, before);
        std::sort(befores.begin(), befores.end());

        // 내 현재 위치에서 내가 포함된 시야를 가진 오브젝트들
        auto afters = this->showns(nears_after, this->_position);
        std::sort(afters.begin(), afters.end());

        // 내가 이동한 뒤 자기 시야에서 내가 사라진 오브젝트들
        auto hides = std::vector<object*>();
        std::set_difference(befores.begin(),
                            befores.end(),
                            afters.begin(),
                            afters.end(),
                            std::inserter(hides, hides.begin()));
        for (auto x : hides)
            this->hide(*x, DESTROY_TYPE::DEFAULT);

        // 내가 이동한 뒤 자기 시야에서 내가 나타난 오브젝트들
        auto shows = std::vector<object*>();
        std::set_difference(afters.begin(),
                            afters.end(),
                            befores.begin(),
                            befores.end(),
                            std::inserter(shows, shows.begin()));

        for (auto x : shows)
        {
            this->update_external(*x, false);
        }

        if (refresh)
        {
            // 내가 이동한 뒤 자기 시야에 여전히 내가 포함된 시야를 가진 오브젝트들
            auto stay = std::vector<object*>();
            std::set_difference(afters.begin(),
                                afters.end(),
                                shows.begin(),
                                shows.end(),
                                std::inserter(stay, stay.begin()));
            for (auto x : stay)
            {
                this->update_external(*x, false);
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

    auto after = this->_position;
    switch (direction)
    {
    case DIRECTION::TOP:
        after.y--;
        break;

    case DIRECTION::BOTTOM:
        after.y++;
        break;

    case DIRECTION::LEFT:
        after.x--;
        break;

    case DIRECTION::RIGHT:
        after.x++;
        break;
    }

    if (this->_map->movable(after) == false)
        return false;

    if (this->direction(direction) == false)
        return false;

    auto before = this->_position;
    this->position(after);
    if (this->_listener != nullptr)
        this->_listener->on_move(*this, before);

    return true;
}

const fb::model::point16_t object::position_forward() const
{
    this->assert_thread();

    return this->position_forward(this->_direction);
}

const fb::model::point16_t object::position_forward(DIRECTION direction) const
{
    this->assert_thread();

    auto current = fb::model::point16_t(this->_position);
    auto forward = fb::model::point16_t(current);
    forward.forward(direction);
    if (this->_map->movable(forward))
        return forward;

    auto left = fb::model::point16_t(current);
    left.left(this->_direction);
    if (this->_map->movable(left))
        return left;

    auto right = fb::model::point16_t(current);
    right.right(this->_direction);
    if (this->_map->movable(right))
        return right;

    auto backward = fb::model::point16_t(current);
    backward.backward(this->_direction);
    if (this->_map->movable(backward))
        return backward;

    return current;
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
    if (this->_listener != nullptr)
        this->_listener->on_direction(*this);

    return true;
}

map* object::map() const
{
    this->assert_thread();

    return this->_map;
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

    if (object.visible() == false)
        return false;

    return this->sight(object._position);
}

bool object::sector(fb::game::sector* sector)
{
    this->assert_thread();

    if (this->_sector == sector)
        return false;

    if (this->_sector != nullptr)
        this->_sector->erase(*this);

    this->_sector = sector;
    if (sector != nullptr)
        sector->push(*this);
    return true;
}

sector* object::sector()
{
    this->assert_thread();

    return this->_sector;
}

bool object::sight(const fb::model::point16_t me, const fb::model::point16_t you, const fb::game::map* map)
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

async::task<bool> object::map(fb::game::map* map, const fb::model::point16_t& position, DESTROY_TYPE destroy_type)
{
    this->assert_thread();

    try
    {
        if (this->_map_lock)
            co_return false;

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
            auto thread = this->_map->thread();
            thread->pop_ptr(this);
            if (this->is(OBJECT_TYPE::CHARACTER))
            {
                auto params = thread->template data<thread_params>();
                params->characters.erase(static_cast<character*>(this)->id());
            }

            // broadcast near characters
            if (this->_listener != nullptr)
            {
                for (auto x : this->_map->nears(this->_position))
                {
                    if (x != this)
                        x->hide(*this, destroy_type);
                }
            }

            // erase cache of map
            this->_map->objects.pop(*this);

            // reset default map and position
            this->sector(nullptr);

            auto before_map = this->_map;
            this->_map      = nullptr;
            this->_position = fb::model::point16_t(1, 1);
            if (this->_listener != nullptr)
                this->_listener->on_map_changed(*this, before_map, this->_map);

            co_return true;
        }

        if (map->active == false)
            co_return false;

        // here the character is on some map.
        // set map to null.
        auto before_map      = this->_map;
        auto before_position = position;
        std::ignore          = co_await this->map(nullptr);
        this->_map_lock      = true;

        // switch thread of destination map
        // and insert character into thread cache.
        auto thread = map->thread();
        if (thread != nullptr)
        {
            if (thread != this->context.threads.current())
                co_await thread->switching();
            thread->push_ptr(this);

            if (this->is(OBJECT_TYPE::CHARACTER))
            {
                auto params = thread->template data<thread_params>();
                auto ch     = static_cast<character*>(this);
                params->characters.insert({ch->id(), ch});
            }
        }

        // update destination map and position
        this->_map = map;
        this->assert_thread();
        this->_position = before_position;

        // update section
        this->_map->update(*this);

        // insert character into map cache
        this->_map->objects.push(*this);

        // broadcast near characters
        if (this->_listener != nullptr)
            this->_listener->on_map_changed(*this, before_map, map);

        this->update_id();
        this->update_map(*map);
        this->update_bgm(map->model.bgm, 100);
        this->update_position();
        this->update_external(false);

        for (auto obj : map->nears(this->_position))
        {
            if (obj == this)
                continue;

            obj->update_external(*this, false);
        }

        this->_map_lock = false;
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

object* object::side(DIRECTION direction, OBJECT_TYPE type) const
{
    this->assert_thread();

    auto map = this->_map;
    if (map == nullptr)
        return nullptr;

    fb::model::point16_t front = this->position();
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

    if (map->existable(front) == false)
        return nullptr;

    auto nears = map->nears(this->_position, type);
    auto found = std::find_if(nears.begin(), nears.end(), [&front](auto x) {
        return x->position() == front;
    });

    return found != nears.end() ? *found : nullptr;
}

std::vector<object*> object::sides(DIRECTION direction, OBJECT_TYPE type) const
{
    this->assert_thread();

    auto result = std::vector<object*>();
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

        if (map->existable(front) == false)
            throw std::exception();

        auto nears = map->nears(this->_position, type);
        std::copy_if(nears.begin(), nears.end(), std::back_inserter(result), [&front](auto x) {
            return x->position() == front;
        });
    }
    catch (std::exception&)
    { }

    return std::move(result);
}

object* object::forward(OBJECT_TYPE type) const
{
    this->assert_thread();

    return this->side(this->_direction, type);
}

std::vector<object*> object::forwards(OBJECT_TYPE type) const
{
    this->assert_thread();

    return this->sides(this->_direction, type);
}

std::vector<object*> object::showns(OBJECT_TYPE type) const
{
    this->assert_thread();

    if (this->_map == nullptr)
        return std::vector<object*>{};

    return this->showns(this->_map->nears(this->_position), this->_position, type);
}

std::vector<object*> object::showns(const std::vector<object*>& source,
                                    const fb::model::point16_t& position,
                                    OBJECT_TYPE                 type) const
{
    this->assert_thread();

    auto objects = std::vector<object*>();
    if (this->_map == nullptr)
        return objects;

    std::copy_if(source.begin(), source.end(), std::back_inserter(objects), [&](auto x) {
        if (this == x)
            return false;

        if (x->visible() == false)
            return false;

        if (type != OBJECT_TYPE::UNKNOWN && x->is(type) == false)
            return false;

        return sight(position, x->_position, this->_map);
    });

    return std::move(objects);
}

std::vector<object*> object::showings(OBJECT_TYPE type) const
{
    this->assert_thread();

    if (this->_map == nullptr)
        return std::vector<object*>{};
    else
        return this->showings(this->_map->nears(this->_position), this->_position, type);
}

std::vector<object*> object::showings(const std::vector<object*>& source,
                                      const fb::model::point16_t& position,
                                      OBJECT_TYPE                 type) const
{
    this->assert_thread();

    auto objects = std::vector<object*>();
    if (this->_map == nullptr)
        return objects;

    std::copy_if(source.begin(), source.end(), std::back_inserter(objects), [&](auto x) {
        if (x == this)
            return false;

        if (x->visible() == false)
            return false;

        if (type != OBJECT_TYPE::UNKNOWN && x->is(type) == false)
            return false;

        return x->sight(position);
    });

    return std::move(objects);
}

bool object::visible() const
{
    this->assert_thread();

    return this->_visible;
}

void object::visible(bool value)
{
    this->assert_thread();

    if (this->_visible == value)
        return;

    this->_visible = value;
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
    if (this->_listener != nullptr)
        this->_listener->on_hide(*this, destroy_type);
}

void object::hide(object& to, DESTROY_TYPE destroy_type)
{
    if (this->_listener != nullptr)
        this->_listener->on_hide(*this, to, destroy_type);
}

fb::thread* object::thread() const
{
    if (this->_map == nullptr)
        return this->context.threads.modular(this->_sequence);
    else
        return this->context.threads.modular(this->_map->model.id);
}

void object::assert_thread() const
{
    if (this->_map == nullptr)
        return;

    fb::thread_switchable::assert_thread();
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
    auto listener = this->get_listener<object>();
    if (listener != nullptr)
        listener->on_sound(*this, sound);
}

void object::effect(uint8_t value)
{
    auto listener = this->get_listener<object>();
    if (listener != nullptr)
        listener->on_effect(*this, value);
}

bool object::operator== (const object& right) const
{
    this->assert_thread();

    return this->_map == right._map && this->sequence() == right.sequence();
}

bool object::operator!= (const object& right) const
{
    this->assert_thread();

    return !((*this) == right);
}