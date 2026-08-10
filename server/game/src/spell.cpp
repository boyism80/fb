#include <fb/game/server.h>
#include <fb/game/life.h>
#include <fb/game/spell.h>
#include <fb/game/character.h>
#include <fb/model/model.h>
#include <fb/encoding.h>
#include <json/json.h>

using namespace fb::game;

spell::spell(const fb::game::server& server, const life& owner, const fb::model::spell& model, uint16_t delay) :
    server(server),
    owner(owner),
    _model_id(model.id),
    _next(server.now() + std::chrono::seconds(delay))
{ }

spell::~spell()
{ }

const fb::model::spell& spell::model() const
{
    return fb::model::table::spell[this->_model_id];
}

void spell::delay(uint16_t value)
{
    this->_next = this->server.now() + std::chrono::seconds(value);
}

uint16_t spell::delay() const
{
    auto diff = this->_next - this->server.now();
    auto sec  = diff.seconds();
    if (sec < 0)
        return 0;

    return sec + (diff.milliseconds() > 0 ? 1 : 0);
}

const fb::model::datetime& fb::game::spell::next() const
{
    return this->_next;
}

spells::spells()
{ }

spells::~spells()
{ }

void spells::owner(std::shared_ptr<fb::game::life> owner)
{
    this->_owner = owner->weak_from_this_as<fb::game::life>();
}

std::shared_ptr<fb::game::life> spells::owner() const
{
    return this->_owner.lock();
}

std::shared_ptr<fb::game::spell> fb::game::spells::find(std::string_view name) const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    owner->assert_thread();

    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto spell = this->at(i);
        if (spell == nullptr)
            continue;

        if (spell->model().name == name)
            return spell;
    }

    return nullptr;
}

std::shared_ptr<fb::game::spell> fb::game::spells::find(const fb::model::spell& model) const
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return nullptr;

    owner->assert_thread();

    for (int i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto spell = this->at(i);
        if (spell == nullptr)
            continue;

        if (spell->model().id == model.id)
            return spell;
    }

    return nullptr;
}

uint8_t spells::add(std::shared_ptr<spell> element)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return 0xFF;

    owner->assert_thread();

    auto index = super::add(element);
    if (index != 0xFF)
    {
        owner->listener.on_spell_update(*owner, index);

        // Log spell add event (only for characters)
        if (owner->is(OBJECT_TYPE::CHARACTER))
        {
            auto& ch                   = static_cast<character&>(*owner);
            auto  log_data             = Json::Value();
            log_data["character_id"]   = static_cast<Json::Int64>(ch.id);
            log_data["character_name"] = UTF8(ch.name(), PLATFORM::WINDOWS);
            log_data["spell_id"]       = static_cast<Json::Int64>(element->model().id);
            log_data["spell_name"]     = UTF8(element->model().name, PLATFORM::WINDOWS);
            log_data["slot"]           = index;
            ch.server.log.write("spell_add", log_data);
        }
    }

    return index;
}

uint8_t spells::add(std::shared_ptr<spell> element, uint8_t index)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return 0xFF;

    owner->assert_thread();

    if (super::add(element, index) != 0xFF)
    {
        owner->listener.on_spell_update(*owner, index);

        // Log spell add event (only for characters)
        if (owner->is(OBJECT_TYPE::CHARACTER))
        {
            auto& ch                   = static_cast<character&>(*owner);
            auto  log_data             = Json::Value();
            log_data["character_id"]   = static_cast<Json::Int64>(ch.id);
            log_data["character_name"] = UTF8(ch.name(), PLATFORM::WINDOWS);
            log_data["spell_id"]       = static_cast<Json::Int64>(element->model().id);
            log_data["spell_name"]     = UTF8(element->model().name, PLATFORM::WINDOWS);
            log_data["slot"]           = index;
            ch.server.log.write("spell_add", log_data);
        }
    }

    return index;
}

uint8_t spells::add(const fb::model::spell& model, uint8_t slot, uint16_t delay)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return 0xFF;

    owner->assert_thread();

    auto& server  = owner->server;
    auto  created = server.make<spell>(*owner, model, delay);
    return this->add(created, slot);
}

uint8_t spells::add(const fb::model::spell& model)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return 0xFF;

    owner->assert_thread();

    auto& server  = owner->server;
    auto  created = server.make<spell>(*owner, model, 0);
    return this->add(created);
}

bool spells::remove(uint8_t index)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    owner->assert_thread();

    auto success = super::remove(index);

    if (success)
    {
        owner->listener.on_spell_remove(*owner, index);

        // Log spell remove event (only for characters)
        if (owner->is(OBJECT_TYPE::CHARACTER))
        {
            auto& ch = static_cast<character&>(*owner);
            // Get spell info before removal (we need to get it from the element that was removed)
            // Note: element is already removed, so we can't get model info here
            // We'll log what we can
            auto log_data              = Json::Value();
            log_data["character_id"]   = static_cast<Json::Int64>(ch.id);
            log_data["character_name"] = UTF8(ch.name(), PLATFORM::WINDOWS);
            log_data["slot"]           = index;
            ch.server.log.write("spell_remove", log_data);
        }
    }

    return success;
}

bool spells::swap(uint8_t src, uint8_t dst)
{
    auto owner = this->_owner.lock();
    if (owner == nullptr)
        return false;

    owner->assert_thread();

    if (super::swap(src, dst) == false)
        return false;

    const auto right = this->at(src);
    if (right != nullptr)
        owner->listener.on_spell_update(*owner, src);
    else
        owner->listener.on_spell_remove(*owner, src);

    const auto left = this->at(dst);
    if (left != nullptr)
        owner->listener.on_spell_update(*owner, dst);
    else
        owner->listener.on_spell_remove(*owner, dst);

    return true;
}

buff::buff(const fb::game::server& server, const fb::model::spell& model, const object* caster, uint32_t seconds) :
    server(server),
    caster(caster),
    start(server.now()),
    _model_id(model.id),
    _duration(std::chrono::seconds(seconds))
{ }

buff::~buff()
{ }

const fb::model::spell& buff::model() const
{
    return fb::model::table::spell[this->_model_id];
}

const fb::model::timespan& buff::duration() const
{
    return this->_duration;
}

void buff::duration(const fb::model::timespan& value)
{
    this->_duration = value;
}

fb::model::timespan buff::remaining() const
{
    auto elapsed = this->server.now() - this->start;
    return this->_duration - elapsed;
}

void buff::remaining(const fb::model::timespan& value)
{
    auto elapsed    = this->server.now() - this->start;
    this->_duration = elapsed + value;
}

buffs::buffs(object& owner) :
    _owner(owner)
{ }

buffs::buffs(const buffs& other) :
    _owner(other._owner)
{
    for (auto& [id, buff] : other)
    {
        super::insert({id, buff});
    }
}

buffs::~buffs()
{ }

bool buffs::contains(const fb::model::spell& model) const
{
    this->_owner.assert_thread();

    return this->contains(model.id);
}

bool buffs::push_back(const std::shared_ptr<buff>& buff)
{
    this->_owner.assert_thread();

    auto& model = buff->model();
    if (this->contains(model.id))
        return false;

    this->insert({model.id, buff});

    // Execute buff script
    auto path = std::format("scripts/spell/{}.lua", model.id);
    auto func = "on_buff";

    auto lua = this->_owner.server.lua.open(path, func);
    if (lua)
    {
        lua->pushobject(this->_owner);
        lua->pushobject(buff->model());
        std::ignore = lua->call(2);
    }

    // Call listener for packet response
    this->_owner.listener.on_buff(this->_owner, *buff);

    return true;
}

async::task<std::shared_ptr<buff>> buffs::push_back(const fb::model::spell&                  model,
                                                    uint32_t                                 seconds,
                                                    const std::shared_ptr<fb::game::object>& caster)
{
    this->_owner.assert_thread();

    if (this->contains(model.id))
    {
        auto& buff = this->at(model.id);
        buff->remaining(std::chrono::seconds(seconds));
        co_return buff;
    }

    auto& server  = this->_owner.server;
    auto  created = server.make<buff>(model, caster.get(), seconds);
    if (created == nullptr)
    {
        fb::logger::warn("Failed to create buff for {}", model.name);
        co_return nullptr;
    }
    else if (this->push_back(created) == false)
    {
        co_await server.destroy(*created);
        co_return nullptr;
    }
    else
    {
        co_return created;
    }
}

async::task<bool> buffs::remove(uint32_t id)
{
    this->_owner.assert_thread();

    auto buff = this->operator[] (id);
    if (buff == nullptr)
        co_return false;

    auto& model = buff->model();

    // Notify client before the script runs. on_unbuff may warp the owner
    // (e.g. spell 72), which changes the active thread and would make
    // send/message assert if they ran afterward.
    this->_owner.listener.on_unbuff(this->_owner, *buff);

    if (this->_owner.is(OBJECT_TYPE::CHARACTER))
    {
        auto& ch = static_cast<character&>(this->_owner);
        ch.message(std::format(_TEXT(MESSAGE_SPELL_UNBUFF), model.name));
    }

    this->erase(id);

    auto path = std::format("scripts/spell/{}.lua", model.id);
    auto func = "on_unbuff";
    auto lua  = this->_owner.server.lua.open(path, func);
    if (lua)
    {
        lua->pushobject(this->_owner);
        lua->pushobject(model);
        std::ignore = co_await lua->call(2);
    }

    co_await this->_owner.server.destroy(*buff);
    co_return true;
}

async::task<bool> buffs::remove(const fb::model::spell& spell)
{
    this->_owner.assert_thread();

    co_return co_await this->remove(spell.id);
}

std::shared_ptr<buff> buffs::operator[] (uint32_t id) const
{
    this->_owner.assert_thread();

    if (this->contains(id) == false)
        return nullptr;

    return super::at(id);
}
