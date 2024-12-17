#include <character.h>
#include <context.h>
#include <fb/model/model.h>
#include <object.h>

using namespace fb::model::enum_value;

fb::game::object::object(fb::game::context&       context,
                         const fb::model::object& model,
                         const initial_params&    initial_params) :
    luable(initial_params.id),
    context(context),
    _listener(&context),
    _sequence(initial_params.id),
    _model(model),
    _position(initial_params.position),
    _direction(initial_params.direction),
    _map(initial_params.map),
    buffs(*this)
{
    if (this->_listener != nullptr)
    {
        this->_listener->on_create(*this);
    }
}

fb::game::object::object(const object& right) :
    object(right.context,
           right._model,
           initial_params{.id        = right._sequence,
                          .position  = right._position,
                          .direction = right._direction,
                          .map       = right._map})
{ }

fb::game::object::~object()
{
    if (this->_listener != nullptr)
    {
        this->_listener->on_destroy(*this);
    }
}

const fb::model::object& fb::game::object::based() const
{
    this->assert_thread();

    return this->_model;
}

bool fb::game::object::is(OBJECT_TYPE type) const
{
    this->assert_thread();

    auto mine = this->what();
    return (type & mine) == mine;
}

const std::string& fb::game::object::name() const
{
    this->assert_thread();

    return this->_model.name;
}

uint16_t fb::game::object::look() const
{
    this->assert_thread();

    return this->_model.look;
}

uint8_t fb::game::object::color() const
{
    this->assert_thread();

    return this->_model.color;
}

OBJECT_TYPE fb::game::object::what() const
{
    this->assert_thread();

    return this->_model.what();
}

async::task<void> fb::game::object::destroy(DESTROY_TYPE destroy_type)
{
    this->assert_thread();

    co_await this->context.destroy(*this, destroy_type);
}

async::task<size_t> fb::game::object::send(const fb::stream& stream, bool encrypt, bool wrap)
{
    this->assert_thread();
    co_return 0;
}

async::task<size_t> fb::game::object::send(const fb::protocol::base::header& response, bool encrypt, bool wrap)
{
    this->assert_thread();
    co_return 0;
}

uint32_t fb::game::object::sequence() const
{
    this->assert_thread();

    return this->_sequence;
}

void fb::game::object::sequence(uint32_t value)
{
    this->assert_thread();

    this->_sequence = value;
}

void fb::game::object::chat(const std::string& message, bool shout)
{
    this->assert_thread();

    if (this->_listener != nullptr)
        this->_listener->on_chat(*this, message, shout);
}

const point16_t& fb::game::object::position() const
{
    this->assert_thread();

    return this->_position;
}

bool fb::game::object::position(uint16_t x, uint16_t y, bool refresh)
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
        this->on_hold();

    this->_map->update(*this);

    auto nears_before = this->_map->nears(before);
    auto nears_after  = this->_map->nears(this->_position);

    if (this->_listener != nullptr)
    {
        // 내 이전 위치에서 내 시야에 보이는 오브젝트들
        auto befores = this->showings(nears_before, before);
        std::sort(befores.begin(), befores.end());

        // 내 현재 위치에서 내 시야에 보이는 오브젝트들
        auto afters = this->showings(nears_after, this->_position);
        std::sort(afters.begin(), afters.end());

        // 내가 이동한 뒤 내 시야에서 사라진 오브젝트들
        auto hides = std::vector<fb::game::object*>();
        std::set_difference(befores.begin(),
                            befores.end(),
                            afters.begin(),
                            afters.end(),
                            std::inserter(hides, hides.begin()));
        for (auto x : hides)
            this->_listener->on_hide(*x, *this, DESTROY_TYPE::DEFAULT);

        // 내가 이동한 뒤 내 시야에서 나타난 오브젝트들
        auto shows = std::vector<fb::game::object*>();
        std::set_difference(afters.begin(),
                            afters.end(),
                            befores.begin(),
                            befores.end(),
                            std::inserter(shows, shows.begin()));
        for (auto x : shows)
            this->_listener->on_show(*x, *this, false);

        if (refresh)
        {
            // 내가 이동한 뒤 내 시야에 여전히 남은 오브젝트들
            auto stay = std::vector<fb::game::object*>();
            std::set_difference(afters.begin(),
                                afters.end(),
                                shows.begin(),
                                shows.end(),
                                std::inserter(stay, stay.begin()));
            for (auto x : stay)
                this->_listener->on_show(*x, *this, false);
        }
    }

    if (this->_listener != nullptr)
    {
        // 내 이전 위치에서 내가 포함된 시야를 가진 오브젝트들
        // auto befores = fb::game::object::showns(nears_before, *this, OBJECT_TYPE::UNKNOWN/* , true, false */);
        auto befores = this->showns(nears_before, before);
        std::sort(befores.begin(), befores.end());

        // 내 현재 위치에서 내가 포함된 시야를 가진 오브젝트들
        // auto afters = fb::game::object::showns(nears_after, *this, OBJECT_TYPE::UNKNOWN/* , false, false */);
        auto afters = this->showns(nears_after, this->_position);
        std::sort(afters.begin(), afters.end());

        // 내가 이동한 뒤 자기 시야에서 내가 사라진 오브젝트들
        auto hides = std::vector<fb::game::object*>();
        std::set_difference(befores.begin(),
                            befores.end(),
                            afters.begin(),
                            afters.end(),
                            std::inserter(hides, hides.begin()));
        for (auto x : hides)
            this->_listener->on_hide(*this, *x, DESTROY_TYPE::DEFAULT);

        // 내가 이동한 뒤 자기 시야에서 내가 나타난 오브젝트들
        auto shows = std::vector<fb::game::object*>();
        std::set_difference(afters.begin(),
                            afters.end(),
                            befores.begin(),
                            befores.end(),
                            std::inserter(shows, shows.begin()));
        for (auto x : shows)
            this->_listener->on_show(*this, *x, false);

        if (refresh)
        {
            // 내가 이동한 뒤 자기 시야에 여전히 내가 포함된 시야를 가진 오브젝트들
            auto stay = std::vector<fb::game::object*>();
            std::set_difference(afters.begin(),
                                afters.end(),
                                shows.begin(),
                                shows.end(),
                                std::inserter(stay, stay.begin()));
            for (auto x : stay)
                this->_listener->on_show(*this, *x, false);
        }
    }

    return true;
}

bool fb::game::object::position(const point16_t position, bool refresh)
{
    this->assert_thread();

    return this->position(position.x, position.y, refresh);
}

bool fb::game::object::move()
{
    this->assert_thread();

    return this->move(this->_direction);
}

bool fb::game::object::move(DIRECTION direction)
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

const point16_t fb::game::object::position_forward() const
{
    this->assert_thread();

    return this->position_forward(this->_direction);
}

const point16_t fb::game::object::position_forward(DIRECTION direction) const
{
    this->assert_thread();

    auto current = point16_t(this->_position);
    auto forward = point16_t(current);
    forward.forward(direction);
    if (this->_map->movable(forward))
        return forward;

    auto left = point16_t(current);
    left.left(this->_direction);
    if (this->_map->movable(left))
        return left;

    auto right = point16_t(current);
    right.right(this->_direction);
    if (this->_map->movable(right))
        return right;

    auto backward = point16_t(current);
    backward.backward(this->_direction);
    if (this->_map->movable(backward))
        return backward;

    return current;
}

uint16_t fb::game::object::x() const
{
    this->assert_thread();

    return this->_position.x;
}

bool fb::game::object::x(uint16_t value)
{
    this->assert_thread();

    return this->position(value, this->_position.y);
}

uint16_t fb::game::object::y() const
{
    this->assert_thread();

    return this->_position.y;
}

bool fb::game::object::y(uint16_t value)
{
    this->assert_thread();

    return this->position(this->_position.x, value);
}

DIRECTION fb::game::object::direction() const
{
    this->assert_thread();

    return this->_direction;
}

bool fb::game::object::direction(DIRECTION value)
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

fb::game::map* fb::game::object::map() const
{
    this->assert_thread();

    return this->_map;
}

bool fb::game::object::sight(const point16_t& position) const
{
    this->assert_thread();

    return fb::game::object::sight(this->_position, position, this->_map);
}

bool fb::game::object::sight(const fb::game::object& object) const
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

bool fb::game::object::sector(fb::game::sector* sector)
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

fb::game::sector* fb::game::object::sector()
{
    this->assert_thread();

    return this->_sector;
}

bool fb::game::object::sight(const point16_t me, const point16_t you, const fb::game::map* map)
{
    point16_t begin, end;

    if (me.x <= fb::game::map::HALF_SCREEN_WIDTH) // 최좌측
    {
        begin.x = 0;
        end.x   = fb::game::map::MAX_SCREEN_WIDTH;
    }
    else if (me.x >= map->width() - fb::game::map::HALF_SCREEN_WIDTH) // 최우측
    {
        begin.x = std::max(int32_t(0), int32_t(map->width() - fb::game::map::MAX_SCREEN_WIDTH - 1));
        end.x   = std::max(int32_t(0), int32_t(map->width() - 1));
    }
    else
    {
        begin.x = std::max(int32_t(0), int32_t(me.x - fb::game::map::HALF_SCREEN_WIDTH - 1));
        end.x   = std::max(int32_t(0), int32_t(me.x + fb::game::map::HALF_SCREEN_WIDTH + 1));
    }

    if (me.y <= fb::game::map::HALF_SCREEN_HEIGHT) // 최상단
    {
        begin.y = 0;
        end.y   = fb::game::map::MAX_SCREEN_HEIGHT;
    }
    else if (me.y >= map->height() - fb::game::map::HALF_SCREEN_HEIGHT) // 최하단
    {
        begin.y = std::max(int32_t(0), int32_t(map->height() - fb::game::map::MAX_SCREEN_HEIGHT - 1));
        end.y   = std::max(int32_t(0), map->height() - 1);
    }
    else
    {
        begin.y = std::max(int32_t(0), int32_t(me.y - fb::game::map::HALF_SCREEN_HEIGHT - 1));
        end.y   = std::max(int32_t(0), int32_t(me.y + fb::game::map::HALF_SCREEN_HEIGHT + 1));
    }

    return begin.x <= you.x && end.x >= you.x && begin.y <= you.y && end.y >= you.y;
}

async::task<bool> fb::game::object::map(fb::game::map* map, const point16_t& position, DESTROY_TYPE destroy_type)
{
    if (this->_map != nullptr)
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
                        this->_listener->on_hide(*x, *this, destroy_type);
                }
            }
            this->on_map_changed(this->_map);

            // erase cache of map
            this->_map->objects.pop(*this);

            // reset default map and position
            this->sector(nullptr);
            this->_map      = nullptr;
            this->_position = point16_t(1, 1);
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
        this->on_map_changed(this->_map);

        // update section
        this->_map->update(*this);

        // insert character into map cache
        this->_map->objects.push(*this);

        // broadcast near characters
        if (this->_listener != nullptr)
            this->_listener->on_map_changed(*this, before_map, map);

        for (auto x : map->nears(this->_position))
        {
            if (x == this)
                continue;

            if (this->_listener != nullptr)
            {
                this->_listener->on_show(*this, *x, false);
                this->_listener->on_show(*x, *this, false);
            }
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

async::task<bool> fb::game::object::map(fb::game::map* map, DESTROY_TYPE destroy_type)
{
    this->assert_thread();

    co_return co_await this->map(map, point16_t(0, 0), destroy_type);
}

fb::game::object* fb::game::object::side(DIRECTION direction, OBJECT_TYPE type) const
{
    this->assert_thread();

    auto map = this->_map;
    if (map == nullptr)
        return nullptr;

    point16_t front = this->position();
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

std::vector<fb::game::object*> fb::game::object::sides(DIRECTION direction, OBJECT_TYPE type) const
{
    this->assert_thread();

    auto result = std::vector<fb::game::object*>();
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

fb::game::object* fb::game::object::forward(OBJECT_TYPE type) const
{
    this->assert_thread();

    return this->side(this->_direction, type);
}

std::vector<fb::game::object*> fb::game::object::forwards(OBJECT_TYPE type) const
{
    this->assert_thread();

    return this->sides(this->_direction, type);
}

std::vector<fb::game::object*> fb::game::object::showns(OBJECT_TYPE type) const
{
    this->assert_thread();

    if (this->_map == nullptr)
        return std::vector<fb::game::object*>{};

    return this->showns(this->_map->nears(this->_position), this->_position, type);
}

std::vector<fb::game::object*> fb::game::object::showns(const std::vector<object*>& source,
                                                        const point16_t&            position,
                                                        OBJECT_TYPE                 type) const
{
    this->assert_thread();

    auto objects = std::vector<fb::game::object*>();
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

std::vector<fb::game::object*> fb::game::object::showings(OBJECT_TYPE type) const
{
    this->assert_thread();

    if (this->_map == nullptr)
        return std::vector<object*>{};
    else
        return this->showings(this->_map->nears(this->_position), this->_position, type);
}

std::vector<fb::game::object*> fb::game::object::showings(const std::vector<object*>& source,
                                                          const point16_t&            position,
                                                          OBJECT_TYPE                 type) const
{
    this->assert_thread();

    auto objects = std::vector<fb::game::object*>();
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

bool fb::game::object::visible() const
{
    this->assert_thread();

    return this->_visible;
}

void fb::game::object::visible(bool value)
{
    this->assert_thread();

    if (this->_visible == value)
        return;

    this->_visible = value;
}

double fb::game::object::distance(const object& right) const
{
    this->assert_thread();

    return std::sqrt(this->distance_sqrt(right));
}

uint32_t fb::game::object::distance_sqrt(const object& right) const
{
    this->assert_thread();

    return (uint32_t)std::pow(this->_position.x - right._position.x, 2) +
           (uint32_t)std::pow(this->_position.y - right._position.y, 2);
}

bool fb::game::object::condition(const std::vector<fb::model::dsl>& conditions) const
{
    this->assert_thread();

    return true;
}

bool fb::game::object::available() const
{
    this->assert_thread();

    return true;
}

fb::thread* fb::game::object::thread() const
{
    if (this->_map == nullptr)
        return this->context.threads.modular(this->_sequence);
    else
        return this->context.threads.modular(this->_map->model.id);
}

void fb::game::object::assert_thread() const
{
    if (this->_map == nullptr)
        return;

    fb::thread_switchable::assert_thread();
}

void fb::game::object::on_timer(uint64_t elapsed_milliseconds)
{
    this->assert_thread();
}

void fb::game::object::on_kill(fb::game::life& you)
{
    this->assert_thread();
}

void fb::game::object::on_hold()
{
    this->assert_thread();
}

void fb::game::object::on_map_changed(fb::game::map*)
{
    this->assert_thread();
}

bool fb::game::object::operator== (const object& right) const
{
    this->assert_thread();

    return this->_map == right._map && this->sequence() == right.sequence();
}

bool fb::game::object::operator!= (const object& right) const
{
    this->assert_thread();

    return !((*this) == right);
}