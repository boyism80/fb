#include <object.h>
#include <character.h>
#include <context.h>
#include <model.h>

using namespace fb::model::enum_value;

// fb::game::object
fb::game::object::object(fb::game::context& context, const fb::model::object& model, const fb::game::object::config& c) : 
    luable(c.id),
    context(context),
    _listener(&context),
    _sequence(c.id),
    _model(model),
    _position(c.position),
    _direction(c.direction),
    _map(c.map),
    buffs(*this)
{
    if(this->_listener != nullptr)
        this->_listener->on_create(*this);
}

fb::game::object::object(const object& right) :
    object(right.context, right._model, 
        fb::game::object::config 
        {
            .id = right._sequence,
            .position = right._position,
            .direction = right._direction,
            .map = right._map
        })
{ }

fb::game::object::~object()
{
    if(this->_listener != nullptr)
        this->_listener->on_destroy(*this);
}

const fb::model::object& fb::game::object::based() const
{
    return this->_model;
}

bool fb::game::object::is(OBJECT_TYPE type) const
{
    auto mine = this->what();
    return (type & mine) == mine;
}

const std::string& fb::game::object::name() const
{
    return this->_model.name;
}

uint16_t fb::game::object::look() const
{
    return this->_model.look;
}

uint8_t fb::game::object::color() const
{
    return this->_model.color;
}

OBJECT_TYPE fb::game::object::what() const
{
    return this->_model.what();
}

async::task<void> fb::game::object::destroy(DESTROY_TYPE destroy_type)
{
    co_await this->context.destroy(*this, destroy_type);
}

void fb::game::object::chat(const std::string& message, bool shout)
{
    if(this->_listener != nullptr)
        if(this->_listener != nullptr)
            this->_listener->on_chat(*this, message, shout);
}

const point16_t& fb::game::object::position() const
{
    return this->_position;
}

bool fb::game::object::position(uint16_t x, uint16_t y, bool refresh)
{
    if(this->_map == nullptr)
        return false;

    if(this->_position.x == x && this->_position.y == y)
        return true;

    auto before = this->_position;
    this->_position.x = std::max(0, std::min(this->_map->width() - 1, int32_t(x)));
    this->_position.y = std::max(0, std::min(this->_map->height() - 1, int32_t(y)));

    if(refresh)
        this->on_hold();

    this->_map->update(*this);
    
    auto nears_before = this->_map->nears(before);
    auto nears_after = this->_map->nears(this->_position);

    if(this->_listener != nullptr)
    {
        // 내 이전 위치에서 내 시야에 보이는 오브젝트들
        auto befores = this->showings(nears_before, before);
        std::sort(befores.begin(), befores.end());

        // 내 현재 위치에서 내 시야에 보이는 오브젝트들
        auto afters = this->showings(nears_after, this->_position);
        std::sort(afters.begin(), afters.end());

        // 내가 이동한 뒤 내 시야에서 사라진 오브젝트들
        auto hides = std::vector<fb::game::object*>();
        std::set_difference(befores.begin(), befores.end(), afters.begin(), afters.end(), std::inserter(hides, hides.begin()));
        for(auto x : hides)
            this->_listener->on_hide(*x, *this);

        // 내가 이동한 뒤 내 시야에서 나타난 오브젝트들
        auto shows = std::vector<fb::game::object*>();
        std::set_difference(afters.begin(), afters.end(), befores.begin(), befores.end(), std::inserter(shows, shows.begin()));
        for(auto x : shows)
            this->_listener->on_show(*x, *this, false);

        if(refresh)
        {
            // 내가 이동한 뒤 내 시야에 여전히 남은 오브젝트들
            auto stay = std::vector<fb::game::object*>();
            std::set_difference(afters.begin(), afters.end(), shows.begin(), shows.end(), std::inserter(stay, stay.begin()));
            for(auto x : stay)
                this->_listener->on_show(*x, *this, false);
        }
    }

    if(this->_listener != nullptr)
    {
        // 내 이전 위치에서 내가 포함된 시야를 가진 오브젝트들
        //auto befores = fb::game::object::showns(nears_before, *this, OBJECT_TYPE::UNKNOWN/* , true, false */);
        auto befores = this->showns(nears_before, before);
        std::sort(befores.begin(), befores.end());

        // 내 현재 위치에서 내가 포함된 시야를 가진 오브젝트들
        //auto afters = fb::game::object::showns(nears_after, *this, OBJECT_TYPE::UNKNOWN/* , false, false */);
        auto afters = this->showns(nears_after, this->_position);
        std::sort(afters.begin(), afters.end());

        // 내가 이동한 뒤 자기 시야에서 내가 사라진 오브젝트들
        auto hides = std::vector<fb::game::object*>();
        std::set_difference(befores.begin(), befores.end(), afters.begin(), afters.end(), std::inserter(hides, hides.begin()));
        for(auto x : hides)
            this->_listener->on_hide(*this, *x);

        // 내가 이동한 뒤 자기 시야에서 내가 나타난 오브젝트들
        auto shows = std::vector<fb::game::object*>();
        std::set_difference(afters.begin(), afters.end(), befores.begin(), befores.end(), std::inserter(shows, shows.begin()));
        for(auto x : shows)
            this->_listener->on_show(*this, *x, false);

        if(refresh)
        {
            // 내가 이동한 뒤 자기 시야에 여전히 내가 포함된 시야를 가진 오브젝트들
            auto stay = std::vector<fb::game::object*>();
            std::set_difference(afters.begin(), afters.end(), shows.begin(), shows.end(), std::inserter(stay, stay.begin()));
            for(auto x : stay)
                this->_listener->on_show(*this, *x, false);
        }
    }

    return true;
}

bool fb::game::object::position(const point16_t position, bool refresh)
{
    return this->position(position.x, position.y, refresh);
}

bool fb::game::object::move()
{
    return this->move(this->_direction);
}

bool fb::game::object::move(DIRECTION direction)
{
    if(this->_map == nullptr)
        return false;

    auto after = this->_position;
    switch(direction)
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

    if(this->_map->movable(after) == false)
        return false;

    if(this->direction(direction) == false)
        return false;

    auto before = this->_position;
    this->position(after);
    if(this->_listener != nullptr)
        this->_listener->on_move(*this, before);

    return true;
}

const point16_t fb::game::object::position_forward() const
{
    return this->position_forward(this->_direction);
}

const point16_t fb::game::object::position_forward(DIRECTION direction) const
{
    auto current = point16_t(this->_position);
    auto forward = point16_t(current);
    forward.forward(direction);
    if(this->_map->movable(forward))
        return forward;


    auto left = point16_t(current);
    left.left(this->_direction);
    if(this->_map->movable(left))
        return left;


    auto right = point16_t(current);
    right.right(this->_direction);
    if(this->_map->movable(right))
        return right;

    auto backward = point16_t(current);
    backward.backward(this->_direction);
    if(this->_map->movable(backward))
        return backward;

    return current;
}

uint16_t fb::game::object::x() const
{
    return this->_position.x;
}

bool fb::game::object::x(uint16_t value)
{
    return this->position(value, this->_position.y);
}

uint16_t fb::game::object::y() const
{
    return this->_position.y;
}

bool fb::game::object::y(uint16_t value)
{
    return this->position(this->_position.x, value);
}

DIRECTION fb::game::object::direction() const
{
    return this->_direction;
}

bool fb::game::object::direction(DIRECTION value)
{
    if(value != DIRECTION::LEFT && 
       value != DIRECTION::TOP && 
       value != DIRECTION::RIGHT && 
       value != DIRECTION::BOTTOM)
        return false;

    if(this->_direction == value)
        return true;

    this->_direction = value;
    if(this->_listener != nullptr)
        this->_listener->on_direction(*this);

    return true;
}

fb::game::map* fb::game::object::map() const
{
    return this->_map;
}

bool fb::game::object::sight(const point16_t& position) const
{
    return fb::game::object::sight(this->_position, position, this->_map);
}

bool fb::game::object::sight(const fb::game::object& object) const
{
    if(this->_map == nullptr)
        return false;

    if(this->_map != object.map())
        return false;

    if(object.visible() == false)
        return false;

    return this->sight(object._position);
}

bool fb::game::object::sector(fb::game::sector* sector)
{
    if(this->_sector == sector)
        return false;

    if(this->_sector != nullptr)
        this->_sector->erase(*this);

    this->_sector = sector;
    if(sector != nullptr)
        sector->push(*this);
    return true;
}

fb::game::sector* fb::game::object::sector()
{
    return this->_sector;
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

async::task<bool> fb::game::object::map(fb::game::map* map, const point16_t& position, DESTROY_TYPE destroy_type)
{
    try
    {
        if(this->_map_lock)
            co_return false;

        if(map == nullptr)
        {
            if(this->_map != nullptr)
            {
                this->_map->objects.pop(*this);
                if(this->_listener != nullptr)
                {
                    for (auto x : this->_map->nears(this->_position))
                    {
                        if (x != this)
                            this->_listener->on_hide(*x, *this, destroy_type);
                    }
                }

                this->_map = nullptr;
                this->on_map_changed(this->_map);
            }
            this->_position = point16_t(1, 1); // 가상계 위치
            this->sector(nullptr);
            co_return true;
        }

        if(this->_map == map)
        {
            this->position(position, true);
            co_return true;
        }

        if(map->active == false)
        {
            co_return false;
        }

        auto before = this->_map;
        auto position_x = position;
        if(this->_map != nullptr)
        {
            co_await this->map(nullptr);
        }
        this->_map_lock = true;

        auto thread = this->context.thread(map);
        if(thread != nullptr && thread != this->context.current_thread())
            co_await thread->dispatch(uint32_t(-1));

        this->_map = map;
        this->_position = position_x;
        this->on_map_changed(this->_map);
        this->_map->update(*this);
        this->_map->objects.push(*this);
        if(this->_listener != nullptr)
            this->_listener->on_map_changed(*this, before, map);

        for(auto x : map->nears(this->_position))
        {
            if(x == this)
                continue;

            if(this->_listener != nullptr)
            {
                this->_listener->on_show(*this, *x, false);
                this->_listener->on_show(*x, *this, false);
            }
        }

        this->_map_lock = false;
        co_return true;
    }
    catch(std::exception& e)
    {
        co_return false;
    }
    catch(...)
    {
        co_return false;
    }
}

async::task<bool> fb::game::object::map(fb::game::map* map, DESTROY_TYPE destroy_type)
{
    co_return co_await this->map(map, point16_t(0, 0), destroy_type);
}

fb::game::object* fb::game::object::side(DIRECTION direction, OBJECT_TYPE type) const
{
    auto map = this->_map;
    if(map == nullptr)
        return nullptr;


    point16_t front = this->position();
    switch(direction)
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

    if(map->existable(front) == false)
        return nullptr;

    auto nears = map->nears(this->_position, type);
    auto found = std::find_if
    (
        nears.begin(), nears.end(), 
        [&front] (auto x)
        {
            return x->position() == front;
        }
    );

    return found != nears.end() ?
        *found : nullptr;
}

std::vector<fb::game::object*> fb::game::object::sides(DIRECTION direction, OBJECT_TYPE type) const
{
    auto result = std::vector<fb::game::object*>();
    try
    {
        auto map = this->_map;
        if(map == nullptr)
            throw std::exception();

        auto front = this->position();
        switch(direction)
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

        if(map->existable(front) == false)
            throw std::exception();

        auto nears = map->nears(this->_position, type);
        std::copy_if
        (
            nears.begin(), nears.end(), std::back_inserter(result),
            [&front] (auto x)
            {
                return x->position() == front;
            }
        );
    }
    catch(std::exception&)
    { }

    return std::move(result);
}

fb::game::object* fb::game::object::forward(OBJECT_TYPE type) const
{
    return this->side(this->_direction, type);
}

std::vector<fb::game::object*> fb::game::object::forwards(OBJECT_TYPE type) const
{
    return this->sides(this->_direction, type);
}

std::vector<fb::game::object*> fb::game::object::showns(OBJECT_TYPE type) const
{
    if(this->_map == nullptr)
        return std::vector<fb::game::object*> { };

    return this->showns(this->_map->nears(this->_position), this->_position, type);
}

std::vector<fb::game::object*> fb::game::object::showns(const std::vector<object*>& source, const point16_t& position, OBJECT_TYPE type) const
{
    auto                    objects = std::vector<fb::game::object*>();
    if(this->_map == nullptr)
        return objects;

    std::copy_if
    (
        source.begin(), source.end(), std::back_inserter(objects), 
        [&](auto x)
        {
            if(this == x)
                return false;

            if(x->visible() == false)
                return false;

            if(type != OBJECT_TYPE::UNKNOWN && x->is(type) == false)
                return false;

            return sight(position, x->_position, this->_map);
        }
    );

    return std::move(objects);
}

std::vector<fb::game::object*> fb::game::object::showings(OBJECT_TYPE type) const
{
    if(this->_map == nullptr)
        return std::vector<object*> { };
    else
        return this->showings(this->_map->nears(this->_position), this->_position, type);
}

std::vector<fb::game::object*> fb::game::object::showings(const std::vector<object*>& source, const point16_t& position, OBJECT_TYPE type) const
{
    auto                    objects = std::vector<fb::game::object*>();
    if(this->_map == nullptr)
        return objects;

    std::copy_if
    (
        source.begin(), source.end(), std::back_inserter(objects), 
        [&](auto x)
        {
            if(x == this)
                return false;

            if(x->visible() == false)
                return false;

            if(type != OBJECT_TYPE::UNKNOWN && x->is(type) == false)
                return false;

            return x->sight(position);
        }
    );

    return std::move(objects);
}

bool fb::game::object::visible() const
{
    return this->_visible;
}

void fb::game::object::visible(bool value)
{
    if(this->_visible == value)
        return;

    this->_visible = value;
}

double fb::game::object::distance(const object& right) const
{
    return std::sqrt(this->distance_sqrt(right));
}

uint32_t fb::game::object::distance_sqrt(const object& right) const
{
    return (uint32_t)std::pow(this->_position.x - right._position.x, 2) + 
        (uint32_t)std::pow(this->_position.y - right._position.y, 2);
}

bool fb::game::object::condition(const std::vector<fb::model::dsl>& conditions) const
{
    return true;
}

bool fb::game::object::operator==(const object& right) const
{
    return this->_map == right._map &&
        this->sequence() == right.sequence();
}

bool fb::game::object::operator!=(const object& right) const
{
    return !((*this) == right);
}

int fb::game::object::builtin_model(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr)
        return 0;
    
    auto& model = object->based();
    thread->pushobject(model);
    return 1;
}

int fb::game::object::builtin_id(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr)
        return 0;
    
    thread->pushinteger(object->sequence());
    return 1;
}

int fb::game::object::builtin_eq(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto me = thread->touserdata<fb::game::object>(1);
    if(me == nullptr)
        return 0;
    
    auto you = thread->touserdata<fb::game::object>(2);
    if(you == nullptr)
        return 0;

    thread->pushboolean(me->sequence() == you->sequence());
    return 1;
}

int fb::game::object::builtin_tostring(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto me = thread->touserdata<fb::game::object>(1);
    if(me == nullptr)
        return 0;
    
    thread->pushstring(me->name());
    return 1;
}

int fb::game::object::builtin_name(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr)
        return 0;
    
    thread->pushstring(object->name());
    return 1;
}

int fb::game::object::builtin_sound(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr)
        return 0;
    
    auto sound = thread->tointeger(2);

    context->send(*object, fb::protocol::game::response::object::sound(*object, SOUND(sound)), context::scope::PIVOT);
    thread->pushinteger(-1);
    return 1;
}

int fb::game::object::builtin_position(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr)
        return 0;
    

    if(argc == 1)
    {
        thread->pushinteger(object->_position.x);
        thread->pushinteger(object->_position.y);
        return 2;
    }
    

    uint16_t x, y;
    if(thread->is_table(2))
    {
        thread->rawgeti(2, 1);
        x = (uint16_t)thread->tointeger(-1);
        thread->remove(-1);

        thread->rawgeti(2, 2);
        y = (uint16_t)thread->tointeger(-1);
        thread->remove(-1);
    }
    else
    {
        x = (uint16_t)thread->tointeger(2);
        y = (uint16_t)thread->tointeger(3);
    }

    std::vector<fb::game::object*> shows, hides, showings, hiddens;
    object->position(x, y, true);

    if(object->is(OBJECT_TYPE::CHARACTER))
    {
        context->send
        (
            *object, 
            [object](const auto& to)
            {
                return std::unique_ptr<fb::protocol::base::header>(new fb::protocol::game::response::session::show(static_cast<fb::game::character&>(*object), to));
            }, 
            context::scope::PIVOT
        );
    }
    else
    {
        context->send(*object, fb::protocol::game::response::object::show(*object), context::scope::PIVOT);
    }

    if(object->is(OBJECT_TYPE::CHARACTER))
    {
        auto session = static_cast<fb::game::character*>(object);
        context->send(*object, fb::protocol::game::response::session::position(*session), context::scope::SELF);
    }

    return 0;
}

int fb::game::object::builtin_direction(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr)
        return 0;
    

    if(argc == 1)
    {
        thread->pushinteger(object->_direction);
        return 1;
    }
    else
    {
        auto direction = DIRECTION(thread->tointeger(2));
        object->direction(direction);

        auto context = thread->env<fb::game::context>("context");
        context->send(*object, fb::protocol::game::response::object::direction(*object), context::scope::PIVOT);
        return 0;
    }
}

int fb::game::object::builtin_chat(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr)
        return 0;
    
    auto message = thread->tostring(2);
    auto type = argc < 3 ? CHAT_TYPE::NORMAL : CHAT_TYPE(thread->tointeger(3));
    auto decorate = argc < 4 ? true : thread->toboolean(4);

    std::stringstream sstream;
    if(decorate)
    {
        if(type == CHAT_TYPE::SHOUT)
            sstream << object->name() << "! " << message;
        else
            sstream << object->name() << ": " << message;
    }
    else
    {
        sstream << message;
    }

    context->send(*object, fb::protocol::game::response::object::chat(*object, type, sstream.str()), context::scope::PIVOT);
    return 0;
}

int fb::game::object::builtin_message(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr)
        return 0;
    
    auto message = thread->tostring(2);
    auto type = argc < 3 ? static_cast<int>(MESSAGE_TYPE::STATE) : thread->tointeger(3);

    if(object->is(OBJECT_TYPE::CHARACTER))
        context->send(*object, fb::protocol::game::response::message(message, MESSAGE_TYPE(type)), context::scope::SELF);

    return 0;
}

int fb::game::object::builtin_buff(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr)
        return 0;
    
    auto spell = thread->touserdata<fb::model::spell>(2);
    if(spell == nullptr)
        return 0;

    auto seconds = (uint32_t)thread->tointeger(3);
    auto buff = async::awaitable_get(object->buffs.push_back(*spell, seconds));
    if(buff == nullptr)
        thread->pushnil();
    else
        context->send(*object, fb::protocol::game::response::spell::buff(*buff), context::scope::SELF);

    return 1;
}

int fb::game::object::builtin_unbuff(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr)
        return 0;
    

    if(thread->is_str(2))
    {
        auto name = thread->tostring(2);
        auto model = context->model.spell.name2spell(name);
        if (model == nullptr)
            thread->pushboolean(false);
        else
            thread->pushboolean(object->buffs.remove(*model));
    }
    else if(thread->is_obj(2))
    {
        auto buff = thread->touserdata<fb::model::spell>(2);
        if(buff == nullptr)
            return 0;
        
        thread->pushboolean(object->buffs.remove(*buff));
    }
    else
    {
        thread->pushboolean(false);
    }

    return 1;
}

int fb::game::object::builtin_isbuff(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr)
        return 0;
    
    if(thread->is_str(2))
    {
        auto name = thread->tostring(2);
        auto model = context->model.spell.name2spell(name);
        if(model == nullptr)
            thread->pushboolean(false);
        else
            thread->pushboolean(object->buffs.contains(*model));
    }
    else if(thread->is_obj(2))
    {
        auto buff = thread->touserdata<fb::model::spell>(2);
        if(buff == nullptr)
            return 0;
        
        thread->pushboolean(object->buffs.contains(*buff));
    }
    else
    {
        thread->pushboolean(false);
    }
    return 1;
}

int fb::game::object::builtin_effect(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr)
        return 0;
    
    auto effect = (uint8_t)thread->tointeger(2);

    if(object->is(OBJECT_TYPE::ITEM) == false)
        context->send(*object, fb::protocol::game::response::object::effect(*object, effect), context::scope::PIVOT);
    return 0;
}

int fb::game::object::builtin_map(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    try
    {
        auto argc = thread->argc();
        auto object = thread->touserdata<fb::game::object>(1);
        if(object == nullptr)
            return 0;
        

        if(argc == 1)
        {
            auto map = object->map();
            if(map == nullptr)
                thread->pushnil();
            else
                thread->pushobject(map);
            return 1;
        }

        fb::game::map* map = nullptr;
        if(thread->is_obj(2))
        {
            map = thread->touserdata<fb::game::map>(2);
            if(map == nullptr)
                return 0;
            
            if(map == nullptr)
                throw std::exception();
        }
        else if(thread->is_str(2))
        {
            map = context->maps.name2map(thread->tostring(2));
            if(map == nullptr)
                throw std::exception();
        }
        else
        {
            throw std::exception();
        }

        point16_t position;
        if(thread->is_table(3))
        {
            thread->rawgeti(3, 1);
            position.x = (uint16_t)thread->tointeger(-1);
            thread->remove(-1);

            thread->rawgeti(3, 2);
            position.y = (uint16_t)thread->tointeger(-1);
            thread->remove(-1);
        }
        else if(thread->is_num(3) && thread->is_num(4))
        {
            position.x = (uint16_t)thread->tointeger(3);
            position.y = (uint16_t)thread->tointeger(4);
        }
        else
        {
            throw std::exception();
        }

        async::awaitable_get(object->map(map, position));
    }
    catch(...)
    { }

    return 0;
}

int fb::game::object::builtin_mkitem(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr)
        return 0;
    
    auto name = thread->tostring(2);

    auto model = context->model.item.name2item(name);
    if(model == nullptr)
    {
        thread->pushnil();
    }
    else
    {
        auto context = thread->env<fb::game::context>("context");
        auto item = model->make(*context);
        async::awaitable_get(item->map(object->_map, object->_position));
        thread->pushobject(item);
        
        context->send(*item, fb::protocol::game::response::object::show(*item), context::scope::PIVOT);
    }

    return 1;
}

int fb::game::object::builtin_showings(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr)
        return 0;
    
    auto filter = argc < 2 ? OBJECT_TYPE::UNKNOWN : OBJECT_TYPE(thread->tointeger(2));

    thread->new_table();
    const auto& objects = object->showns(filter);

    for(int i = 0; i < objects.size(); i++)
    {
        thread->pushobject(objects[i]);
        thread->rawseti(-2, uint64_t(i+1));
    }

    return 1;
}

int fb::game::object::builtin_showns(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr)
        return 0;
    
    auto filter = argc < 2 ? OBJECT_TYPE::UNKNOWN : OBJECT_TYPE(thread->tointeger(2));

    thread->new_table();
    const auto& objects = object->showings(filter);

    for(int i = 0; i < objects.size(); i++)
    {
        thread->pushobject(objects[i]);
        thread->rawseti(-2, uint64_t(i+1));
    }

    return 1;
}

int fb::game::object::builtin_front(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;

    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr)
        return 0;
    
    auto filter = argc < 2 ? OBJECT_TYPE::UNKNOWN : OBJECT_TYPE(thread->tointeger(2));
    auto front = object->forward(filter);
    if(front == nullptr)
        thread->pushnil();
    else
        thread->pushobject(front);

    return 1;
}

int fb::game::object::builtin_is(lua_State* lua)
{
    auto thread = fb::game::lua::get(lua);
    if(thread == nullptr)
        return 0;
    
    auto context = thread->env<fb::game::context>("context");
    auto argc = thread->argc();
    auto object = thread->touserdata<fb::game::object>(1);
    if(object == nullptr)
        return 0;
    
    auto type = thread->tointeger(2);
    thread->pushboolean(object->is(OBJECT_TYPE(type)));
    return 1;
}