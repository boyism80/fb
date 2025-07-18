#include <fb/game/context.h>
using namespace fb::game;
using namespace std::chrono_literals;

context::context(boost::asio::io_context& context, uint16_t port) :
    fb::acceptor<character>(context, "GAME", port),
    maps(*this, fb::config<uint32_t>("id")),
    _redis(config<std::string>("redis:ip").c_str(), config<uint16_t>("redis:port"), config<uint32_t>("redis:pool")),
    listener(*this),
    characters(*this),
    clans([](const std::shared_ptr<clan>& clan) -> uint32_t {
        return clan->id();
    }),
    groups([](const std::shared_ptr<group>& group) -> uint32_t {
        return group->id();
    })
{
    auto& ist = fb::lua::context_pool::ist();
    ist.setup(this->threads);

    lua::env<fb::game::context>("context", this);
    lua::build<door, lua::luable>();
    lua::build<clan, lua::luable>();
    lua::build<clan_member, lua::luable>();
    lua::build<achievement, lua::luable>();
    lua::build<spell, lua::luable>();
    lua::build<buff, lua::luable>();
    lua::build<fb::model::spell, lua::luable>();
    lua::build<fb::model::map, lua::luable>();
    lua::build<fb::model::achievement, lua::luable>();
    lua::build<fb::model::object, lua::luable>();
    lua::build<map, fb::thread_switchable>();
    lua::build<group, fb::thread_switchable>();
    lua::build<object, fb::thread_switchable>();
    lua::build<fb::model::life, fb::model::object>();
    lua::build<life, object>();
    lua::build<fb::model::mob, fb::model::life>();
    lua::build<mob, life>();
    lua::build<fb::model::npc, fb::model::object>();
    lua::build<npc, object>();
    lua::build<fb::model::item, fb::model::object>();
    lua::build<fb::model::weapon, fb::model::item>();
    lua::build<item, object>();
    lua::build<equipment, item>();
    lua::build<weapon, equipment>();
    lua::build<character, life>();

    lua::build("log", builtin::builtin_log);
    lua::build("seed", builtin::builtin_seed);
    lua::build("sleep", builtin::builtin_sleep);
    lua::build("baram_time", builtin::builtin_baram_time);
    lua::build("name2mob", builtin::builtin_name2mob);
    lua::build("name2spell", builtin::builtin_name2spell);
    lua::build("name2item", builtin::builtin_name2item);
    lua::build("name2npc", builtin::builtin_name2npc);
    lua::build("name2map", builtin::builtin_name2map);
    lua::build("name2ch", builtin::builtin_name2ch);
    lua::build("broadcast", builtin::builtin_broadcast);
    lua::build("assert_alive", builtin::builtin_assert_alive);
    lua::build("pursuit_sell", builtin::builtin_pursuit_sell);
    lua::build("pursuit_sell_price", builtin::builtin_pursuit_sell_price);
    lua::build("pursuit_sell_name", builtin::builtin_pursuit_sell_name);
    lua::build("pursuit_buy", builtin::builtin_pursuit_buy);
    lua::build("timer", builtin::builtin_timer);
    lua::build("weather", builtin::builtin_weather);
    lua::build("bright", builtin::builtin_bright);
    lua::build("name_with", builtin::builtin_name_with);
    lua::build("assert_korean", builtin::builtin_assert_korean);
    lua::build("CP949", builtin::builtin_cp949);
    lua::build("debug", builtin::builtin_debug);
    lua::build("name2class", builtin::builtin_name2class);
    lua::build("class2name", builtin::builtin_class2name);
    lua::build("save", builtin::builtin_save);
    lua::build("mknpc", builtin::builtin_mknpc);
    lua::build("maps", builtin::builtin_maps);
    lua::build("shutdown", builtin::builtin_shutdown);

    for (auto& [_, root] : ist)
    {
        auto& thread = root->initial_thread();
        std::ignore  = thread.dispatch([root](auto&) -> async::task<void> {
            fb::model::lua::map_enum(*root);
            co_return;
        });
    }
}

context::~context()
{ }

async::task<void> context::handle_start()
{
    this->threads.deletor = [](void* data) {
        auto params = static_cast<thread_params*>(data);
        delete params;
    };

    co_await fb::acceptor<character>::handle_start();

    auto maps_division = std::unordered_map<fb::thread*, std::vector<std::shared_ptr<fb::game::map>>>{};
    for (int i = 0; i < this->threads.count(); i++)
    {
        auto thread = this->threads.at(i);
        maps_division.insert({thread, std::vector<std::shared_ptr<fb::game::map>>{}});
    }

    for (auto& [id, map] : this->maps)
    {
        auto thread = this->threads.modular(id);
        maps_division[thread].push_back(map);
    }

    auto async_tasks = std::vector<async::task<void>>();
    for (auto& [thread, maps] : maps_division)
    {
        async_tasks.push_back(thread->dispatch([this, maps = std::move(maps)](auto& thread) -> async::task<void> {
            auto params = new thread_params();
            for (auto map : maps)
            {
                params->maps.insert({map->model.id, map});
                if (this->model.mob_spawn.contains(map->model.id))
                {
                    for (auto& spawn : this->model.mob_spawn[map->model.id])
                    {
                        params->rezens.push_back(fb::game::rezen(*this, spawn));
                    }
                }
            }
            thread.data(params);
            co_return;
        }));
    }

    for (auto& async_task : async_tasks)
    {
        async::awaitable_get(async_task);
    }

    this->handler.protocol.bind(&context::handle_login);            // 게임서버 접속 핸들러
    this->handler.protocol.bind(&context::handle_direction);        // 방향전환 핸들러
    this->handler.protocol.bind(&context::handle_logout);           // 접속 종료
    this->handler.protocol.bind(&context::handle_update_move);      // 이동과 맵 데이터 업데이트 핸들러
    this->handler.protocol.bind(&context::handle_move, 1s, 6);      // 이동 핸들러
    this->handler.protocol.bind(&context::handle_attack, 500ms, 2); // 공격 핸들러
    this->handler.protocol.bind(&context::handle_loot);             // 아이템 줍기 핸들러
    this->handler.protocol.bind(&context::handle_emotion);          // 감정표현 핸들러
    this->handler.protocol.bind(&context::handle_update_map);       // 맵 데이터 업데이트 핸들러
    this->handler.protocol.bind(&context::handle_update_screen);    // 새로고침 핸들러
    this->handler.protocol.bind(&context::handle_active_item);      // 아이템 사용 핸들러
    this->handler.protocol.bind(&context::handle_inactive_item);    // 아이템 장착 해제 핸들러
    this->handler.protocol.bind(&context::handle_drop_item);        // 아이템 버리기 핸들러
    this->handler.protocol.bind(&context::handle_drop_money);       // 금전 버리기 핸들러
    this->handler.protocol.bind(&context::handle_front_info);       // 앞방향 정보 핸들러
    this->handler.protocol.bind(&context::handle_self_info);        // 나 자신의 정보 핸들러
    this->handler.protocol.bind(&context::handle_option_changed);   // 옵션 설정 핸들러
    this->handler.protocol.bind(&context::handle_click_object);     // 오브젝트 클릭 핸들러
    this->handler.protocol.bind(&context::handle_item_info);        // 인벤토리 우클릭 핸들러
    this->handler.protocol.bind(&context::handle_item_combine);     // 아이템 조합 핸들러
    this->handler.protocol.bind(&context::handle_trade);            // 교환 핸들러
    this->handler.protocol.bind(&context::handle_group);            // 그룹 핸들러
    this->handler.protocol.bind(&context::handle_user_list);        // 유저 리스트 핸들러
    this->handler.protocol.bind(&context::handle_chat);             // 유저 채팅 핸들러
    this->handler.protocol.bind(&context::handle_bulletin);         // 게시판 섹션 리스트 핸들러
    this->handler.protocol.bind(&context::handle_swap);             // 스펠 순서 변경
    this->handler.protocol.bind(&context::handle_dialog);           // 다이얼로그
    this->handler.protocol.bind(&context::handle_dialog, 0x39);     // 다이얼로그
    this->handler.protocol.bind(&context::handle_throw_item);       // 아이템 던지기 핸들러
    this->handler.protocol.bind(&context::handle_spell, 500ms, 3);  // 스펠 핸들러
    this->handler.protocol.bind(&context::handle_door);             // 도어 핸들러
    this->handler.protocol.bind(&context::handle_whisper);          // 귓속말 핸들러
    this->handler.protocol.bind(&context::handle_world);            // 월드맵 핸들러
    this->handler.protocol.bind(&context::handle_object_miss);
    this->handler.protocol.bind(&context::handle_give_item);
    this->handler.protocol.bind(&context::handle_give_money);
    this->handler.protocol.bind(&context::handle_post);

    this->bind_timer(&context::handle_heart_beat, 1s);
    this->bind_timer(&context::handle_announce,
                     std::chrono::seconds(fb::model::const_value::time::ANNOUNCE.total_milliseconds() / 1000));
    this->bind_thread_timer(&context::handle_mob_action, 100ms); // 몹 행동 타이머
    this->bind_thread_timer(&context::handle_mob_respawn, 1s);   // 몹 리젠 타이머
    this->bind_thread_timer(&context::handle_buff_timer, 1s);    // 버프 타이머
    this->bind_thread_timer(&context::handle_gear_timer, 1s);
    this->bind_thread_timer(&context::handle_soliloquy_timer, 1s);
    this->bind_thread_timer(&context::handle_save_timer,
                            std::chrono::seconds(fb::config<uint32_t>("save"))); // DB 저장 타이머

    this->bind_npc_interaction(&context::npc_interaction_sell);
    this->bind_npc_interaction(&context::npc_interaction_buy);
    this->bind_npc_interaction(&context::npc_interaction_repair);
    this->bind_npc_interaction(&context::npc_interaction_deposit_money);
    this->bind_npc_interaction(&context::npc_interaction_withdraw_money);
    this->bind_npc_interaction(&context::npc_interaction_store_item);
    this->bind_npc_interaction(&context::npc_interaction_retrieve_item);
    this->bind_npc_interaction(&context::npc_interaction_sell_list);
    this->bind_npc_interaction(&context::npc_interaction_buy_list);
    this->bind_npc_interaction(&context::npc_interaction_sell_price);
    this->bind_npc_interaction(&context::npc_interaction_buy_price);
    this->bind_npc_interaction(&context::npc_interaction_show_deposited_money);
    this->bind_npc_interaction(&context::npc_interaction_rename_weapon);
    this->bind_npc_interaction(&context::npc_interaction_store_item_list);
    this->bind_npc_interaction(&context::npc_interaction_store_item_count);
    this->bind_npc_interaction(&context::npc_interaction_revive);
    this->bind_npc_interaction(&context::npc_interaction_appreciate);

    this->handler.amqp.bind(std::format("fb.game.{}", config<uint32_t>("id")), &context::handle_amqp_KickOut);
    this->handler.amqp.bind(std::format("fb.game.{}", config<uint32_t>("id")), &context::handle_amqp_Whisper);
    this->handler.amqp.bind("fb.system", &context::handle_amqp_shutdown);
    this->handler.amqp.bind("fb.global", &context::handle_amqp_Broadcast);
    this->handler.amqp.bind("fb.group", &context::handle_amqp_EnterGroup);
    this->handler.amqp.bind("fb.group", &context::handle_amqp_LeaveGroup);
    this->handler.amqp.bind("fb.clan", &context::handle_amqp_SetClanTitle);
    this->handler.amqp.bind("fb.clan", &context::handle_amqp_JoinClan);
    this->handler.amqp.bind("fb.clan", &context::handle_amqp_LeaveClan);
    this->handler.amqp.bind("fb.clan", &context::handle_amqp_BroadcastClan);
    this->handler.amqp.bind("fb.mail", &context::handle_amqp_WriteMail);
}

bool context::decrypt_policy(uint8_t cmd) const
{
    switch (cmd)
    {
    case 0x10:
        return false;

    default:
        return true;
    }
}

bool context::assert_tps(const fb::socket<fb::game::character>& socket) const
{
    auto ch = socket.data();
    if (ch == nullptr)
        return true;

    return ch->role() == ROLE::USER;
}

async::task<bool> context::handle_connected(fb::socket<character>& socket)
{
    co_return true;
}

async::task<bool> context::handle_disconnected(fb::socket<character>& socket)
{
    auto ch = socket.data();
    if (ch == nullptr)
        co_return false;
    auto weak = ch->weak_from_this_as<character>();

    if (ch->trade.trading())
        ch->trade.cancel();

    fb::logger::info("{} has disconnected.", ch->name());

    auto thread = ch->thread();
    try
    {
        co_await this->save(*ch);
        std::ignore = co_await this->http.post("internal", "/in-game/logout", Logout{ch->name()});
    }
    catch (std::exception& e)
    {
        fb::logger::fatal(e.what());
    }

    // Check if character is still valid before thread switching
    auto switched = false;
    try
    {
        co_await this->threads.switching(weak);
        switched = true;
    }
    catch (std::exception& e)
    {
        fb::logger::warn("Thread switching failed during disconnect: {}", e.what());
    }

    if (!switched)
        co_await thread->switching();

    auto shared = weak.lock();
    if (shared != nullptr)
    {
        auto& group_id = shared->group_id();
        if (group_id.has_value())
        {
            this->groups.write(group_id.value(), [weak](auto& group) {
                group->leave(weak);
            });
            shared->group_reset();
        }

        auto& clan_id = shared->clan_id();
        if (clan_id.has_value())
        {
            this->clans.read(clan_id.value(), [weak](auto& clan) {
                clan->detach_character(weak);
            });
            shared->clan_reset();
        }

        this->characters.remove(shared);
        co_await shared->destroy();
        socket.data(nullptr);
    }
    else
    {
        fb::logger::debug("Character expired during cleanup, skipping group/clan operations");
    }

    co_return true;
}

std::string context::elapsed_message(const std::string& dt)
{
    auto elapsed = fb::model::datetime() - fb::model::datetime(dt);
    if (elapsed.total_milliseconds() > 1000 * 60)
    {
        auto sstream = std::stringstream();
        auto days    = elapsed.days();
        if (days > 0)
            sstream << days << "일 ";

        auto hours = elapsed.hours();
        if (hours > 0)
            sstream << hours << "시간 ";

        auto minutes = elapsed.minutes();
        if (minutes > 0)
            sstream << minutes << "분";
        sstream << "만에 바람으로...";

        return sstream.str();
    }
    else
    {
        return std::string();
    }
}

async::task<bool> context::init_ch(const internal::Character&           response,
                                   character&                           ch,
                                   std::optional<uint32_t>              group,
                                   std::optional<uint32_t>              clan,
                                   const std::optional<transfer_param>& transfer)
{
    auto map  = response.map;
    auto weak = ch.weak_from_this_as<character>();
    ch.id(response.id);
    ch.name(response.name);
    ch.pw(response.pw);
    ch.birthday(response.birth);
    ch.updated_date(fb::model::datetime(response.updated_date));
    ch.role(static_cast<ROLE>(response.role));
    ch.cls(static_cast<CLASS>(response.class_type));
    ch.color(response.color);
    ch.direction(DIRECTION(response.direction));
    ch.look(response.look);
    ch.money(response.money);
    ch.items.deposited(response.deposited_money);
    ch.sex(SEX(response.sex));
    ch.base_hp(response.base_hp);
    ch.hp(response.hp);
    ch.base_mp(response.base_mp);
    ch.mp(response.mp);
    ch.level(response.level);
    ch.exp(response.exp);
    ch.state(STATE(response.state));
    ch.title(response.title);

    if (response.armor_color.has_value())
        ch.armor_color(response.armor_color.value());

    if (response.disguise.has_value())
        ch.disguise(response.disguise.value());
    else
        ch.undisguise();

    for (auto& buff : response.buffs)
    {
        auto& model = this->model.spell[buff.model];
        ch.buffs.push_back(model, buff.time);
    }

    if (this->maps.contains(map) == false)
        co_return false;

    auto position_x = response.position.x;
    auto position_y = response.position.y;
    if (transfer != std::nullopt)
    {
        map        = transfer.value().map;
        position_x = uint32_t(transfer.value().position.x);
        position_y = uint32_t(transfer.value().position.y);
    }

    if (group.has_value())
    {
        this->upsert_group_then(group.value(), [&ch](auto& group) {
            group->enter(ch.weak_from_this_as<character>());
            ch.group_id(group->id());
        });
    }

    if (clan.has_value())
    {
        co_await this->upsert_clan_then(clan.value(), [weak](auto& clan) -> async::task<void> {
            auto ch = weak.lock();
            if (ch == nullptr)
                co_return;

            clan->attach_character(weak);
            ch->clan_id(clan->id());
            co_return;
        });
    }

    co_return co_await ch.map(this->maps[map], fb::model::point16_t(position_x, position_y));
}

void context::init_option(const internal::Option& response, fb::game::character& ch)
{
    ch.option(OPTION::WHISPER, response.whisper, false);
    ch.option(OPTION::GROUP, response.group, false);
    ch.option(OPTION::ROAR, response.roar, false);
    ch.option(OPTION::ROAR_WORLDS, response.roar_worlds, false);
    ch.option(OPTION::MAGIC_EFFECT, response.magic_effect, false);
    ch.option(OPTION::WEATHER_EFFECT, response.weather_effect, false);
    ch.option(OPTION::FIXED_MOVE, response.fixed_move, false);
    ch.option(OPTION::TRADE, response.trade, false);
    ch.option(OPTION::FAST_MOVE, response.fast_move, false);
    ch.option(OPTION::EFFECT_SOUND, response.effect_sound, false);
    ch.option(OPTION::PK_PROTECT, response.pk_protect, false);
}

void context::init_items(const std::vector<internal::Item>& response, character& ch)
{
    for (auto& x : response)
    {
        // Use smart pointer for item creation
        auto item = this->model.item[x.model].make(*this);
        item->count(x.count);

        if (x.durability.has_value())
            item->durability(x.durability.value());

        if (x.stored != -1)
            ch.items.store(item); // Use smart pointer version
        else if (x.parts == static_cast<uint32_t>(EQUIPMENT_PARTS::UNKNOWN))
            ch.items.add(item, x.index); // Use smart pointer version
        else
            ch.items.wear((EQUIPMENT_PARTS)x.parts, std::static_pointer_cast<fb::game::equipment>(item));

        if (x.custom_name.has_value() && item->based<fb::model::item>().attr(ITEM_ATTRIBUTE::WEAPON))
            static_cast<weapon*>(item.get())->custom_name(x.custom_name.value());
    }
}

void context::init_spells(const std::vector<internal::Spell>& response, character& ch)
{
    for (auto& x : response)
    {
        if (this->model.spell.contains(x.model) == false)
            continue;

        auto& model = this->model.spell[x.model];
        auto  delay = fb::model::datetime(x.next) - fb::model::datetime();
        auto  sec   = delay.seconds();
        if (sec >= 0)
            sec += (delay.milliseconds() > 0 ? 1 : 0);
        else
            sec = 0;
        ch.spells.add(model, x.slot, sec);
    }
}

void context::init_achievements(const std::vector<fb::protocol::internal::Achievement>& response,
                                fb::game::character&                                    ch)
{
    for (auto& achievement : response)
    {
        if (this->model.achievement.contains(achievement.model) == false)
            continue;

        auto ptr = std::make_unique<fb::game::achievement>(this->model.achievement[achievement.model],
                                                           achievement.text,
                                                           achievement.icon,
                                                           achievement.color);
        ch.achievements.insert({achievement.model, std::move(ptr)});
    }
}

std::shared_ptr<fb::game::character> context::handle_accepted(fb::socket<character>& socket)
{
    return this->make<character>(socket);
}

async::task<void> context::send(object&                     object,
                                const fb::protocol::header& header,
                                fb::game::scope             scope,
                                bool                        exclude_self,
                                bool                        encrypt)
{
    auto stream = fb::stream();
    auto writer = fb::stream_writer<big_endian>(stream);
    co_await header.serialize(writer);

    switch (scope)
    {
    case fb::game::scope::PIVOT:
    {
        if (!exclude_self)
            object.send(stream, encrypt);

        for (auto& x : object.nears(OBJECT_TYPE::CHARACTER, true))
        {
            if (x->sight(object) == false)
                continue;

            if (object.hidden(*x))
                continue;

            x->send(stream, encrypt);
        }
    }
    break;

    case fb::game::scope::GROUP:
    {
        if (object.is(OBJECT_TYPE::CHARACTER) == false)
            co_return;

        auto& ch       = static_cast<const character&>(object);
        auto& group_id = ch.group_id();
        if (group_id.has_value() == false)
            co_return;

        this->groups.read(group_id.value(), [&stream, encrypt](auto& group) {
            for (auto& shared_ptr : group->characters())
            {
                shared_ptr->send(stream, encrypt);
            }
        });
    }
    break;

    case fb::game::scope::MAP:
    {
        auto map = object.map();
        if (map == nullptr)
            co_return;

        for (const auto& [seq, obj] : map->objects)
        {
            if (exclude_self && obj->oid() == object.oid())
                continue;

            obj->send(stream, encrypt);
        }
    }
    break;

    case fb::game::scope::WORLD:
    {
        co_await this->characters.foreach ([stream, encrypt](auto& ch) {
            ch->send(stream, encrypt);
        });
    }
    break;
    }
}

async::task<void> context::save(character& ch)
{
    if (ch.inited() == false)
        co_return;

    auto weak  = ch.weak_from_this();
    auto items = std::vector<internal::Item>();
    for (auto i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto item = ch.items[i];
        if (item == nullptr)
            continue;

        auto protocol  = item->to_protocol();
        protocol.index = i;
        items.push_back(protocol);
    }

    for (auto& [parts, equipment] : ch.items.equipments())
    {
        if (equipment == nullptr)
            continue;

        items.push_back(equipment->to_protocol(parts));
    }

    auto& stored_items = ch.items.stored();
    for (int i = 0; i < stored_items.size(); i++)
    {
        auto item       = stored_items.at(i);
        auto protocol   = item->to_protocol();
        protocol.stored = i;
        items.push_back(protocol);
    }

    auto spells = std::vector<internal::Spell>();
    for (uint8_t i = 0; i < CONTAINER_CAPACITY; i++)
    {
        auto spell = ch.spells[i];
        if (spell == nullptr)
            continue;

        spells.push_back(internal::Spell{ch.id(), i, spell->model.id, spell->next().to_string()});
    }

    auto achievements = std::vector<internal::Achievement>();
    for (auto& [model, achievement] : ch.achievements)
    {
        achievements.push_back(
            internal::Achievement{ch.id(), model, achievement->text, achievement->icon, achievement->color});
    }

    std::ignore =
        co_await this->http.post("internal", "/user/save", Save{ch.to_protocol(), items, spells, achievements});

    co_await this->threads.switching(weak);
    ch.send(fb_resp::save());
}

uint32_t context::thread_id(const fb::socket<character>& socket) const
{
    auto character = socket.data();
    if (character == nullptr)
        throw std::runtime_error("character is empty");

    auto map = character->map();
    if (map == nullptr)
        return 0;

    return map->model.id;
}

fb::thread* context::thread(const map& map)
{
    auto count = this->threads.count();
    if (count == 0)
        return nullptr;

    return this->threads.at(map.model.id % count);
}

void context::handle_init_amqp(fb::amqp::socket& amqp)
{
    this->handler.amqp.declare_queue("amq.direct", "fb.system");
    this->handler.amqp.declare_queue("amq.direct", std::format("fb.game.{}", fb::config<uint32_t>("id")));
    this->handler.amqp.declare_queue("amq.direct", "fb.global");
    this->handler.amqp.declare_queue("amq.direct", "fb.group");
    this->handler.amqp.declare_queue("amq.direct", "fb.clan");
    this->handler.amqp.declare_queue("amq.direct", "fb.mail");
}

// TODO : 클릭도 인터페이스로
void context::handle_click_mob(character& ch, mob& mob)
{
    ch.send(fb_resp::message(mob.name(), MESSAGE_TYPE::STATE));
}

void context::handle_click_npc(character& ch, npc& npc)
{
    auto& model = npc.based<fb::model::npc>();
    if (model.script.empty())
        return;

    auto lua = fb::lua::new_context();
    if (lua == nullptr)
        return;
#if defined DEBUG | defined _DEBUG
    lua->load("scripts/npc.lua");
    lua->load(model.script);
#endif
    lua->func(model.click);
    lua->pushobject(ch);
    lua->pushobject(npc);
    std::ignore = lua->call(2);
}

async::task<void> context::broadcast(const std::string& message, MESSAGE_TYPE type, BROADCAST_TYPE broadcast_type)
{
    switch (broadcast_type)
    {
    case BROADCAST_TYPE::GLOBAL:
    {
        auto&& resp =
            co_await this->http.post("internal",
                                     "/in-game/broadcast",
                                     Broadcast{fb::config<uint32_t>("id"), message, static_cast<uint8_t>(type)});
        this->on_broadcast(resp);
    }
    break;

    case BROADCAST_TYPE::WORLD:
    {
        co_await this->characters.foreach ([message, type](auto& ch) {
            ch->message(message, type);
        });
    }
    break;
    }
}

void context::on_broadcast(const internal_resp::Broadcast& resp)
{
    std::ignore = this->broadcast(resp.message, static_cast<MESSAGE_TYPE>(resp.type), BROADCAST_TYPE::WORLD);
}

void context::rezen_force()
{
    for (int i = 0; i < this->threads.count(); i++)
    {
        auto thread = this->threads.at(i);
        std::ignore = thread->dispatch([](auto& thread) -> async::task<void> {
            auto params = thread.template data<thread_params>();
            for (auto& rezen : params->rezens)
            {
                rezen.force_spawn(thread.id());
            }
            co_return;
        });
    }
}

void context::rezen_force(const fb::game::map& map)
{
    auto thread = map.thread();
    std::ignore = thread->dispatch([map_id = map.model.id](auto& thread) -> async::task<void> {
        auto params = thread.template data<thread_params>();
        for (auto& rezen : params->rezens)
        {
            if (rezen.model.parent == map_id)
                rezen.force_spawn(thread.id());
        }
        co_return;
    });
}