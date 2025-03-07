#include <fb/game/context.h>
using namespace fb::game;
using namespace std::chrono_literals;

context::context(boost::asio::io_context& context, uint16_t port) :
    fb::acceptor<character>(context, "GAME", port),
    maps(*this, fb::config<uint32_t>("id"))
{
    auto& ist = fb::lua::context_pool::ist();
    ist.setup(this->threads);

    lua::env<fb::game::context>("context", this);
    lua::build<door, lua::luable>();
    lua::build<clan, lua::luable>();
    lua::build<clan_member, lua::luable>();
    lua::build<trace, lua::luable>();
    lua::build<spell, lua::luable>();
    lua::build<buff, lua::luable>();
    lua::build<fb::model::spell, lua::luable>();
    lua::build<fb::model::map, lua::luable>();
    lua::build<fb::model::trace, lua::luable>();
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

    lua::build("seed", builtin_seed);
    lua::build("sleep", builtin_sleep);
    lua::build("name2mob", builtin_name2mob);
    lua::build("name2spell", builtin_name2spell);
    lua::build("name2item", builtin_name2item);
    lua::build("name2npc", builtin_name2npc);
    lua::build("name2map", builtin_name2map);
    lua::build("name2ch", builtin_name2ch);
    lua::build("broadcast", builtin_broadcast);
    lua::build("assert_alive", builtin_assert_alive);
    lua::build("pursuit_sell", builtin_pursuit_sell);
    lua::build("pursuit_sell_price", builtin_pursuit_sell_price);
    lua::build("pursuit_sell_name", builtin_pursuit_sell_name);
    lua::build("pursuit_buy", builtin_pursuit_buy);
    lua::build("timer", builtin_timer);
    lua::build("weather", builtin_weather);
    lua::build("bright", builtin_bright);
    lua::build("name_with", builtin_name_with);
    lua::build("assert_korean", builtin_assert_korean);
    lua::build("CP949", builtin_cp949);
    lua::build("debug", builtin_debug);
    lua::build("name2class", builtin_name2class);
    lua::build("class2name", builtin_class2name);
    lua::build("save", builtin_save);
    lua::build("mknpc", builtin_mknpc);
    lua::build("maps", builtin_maps);

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
{
    for (int i = 0; i < threads.count(); i++)
    {
        auto thread = this->threads.at(i);
        auto params = thread->template data<thread_params>();
        delete params;
    }
}

async::task<void> context::handle_start()
{
    co_await fb::acceptor<character>::handle_start();

    auto maps_division = std::unordered_map<fb::thread*, std::vector<fb::game::map*>>{};
    for (int i = 0; i < this->threads.count(); i++)
    {
        auto thread = this->threads.at(i);
        maps_division.insert({thread, std::vector<fb::game::map*>{}});
    }

    for (auto& [id, map] : this->maps)
    {
        auto thread = this->threads.modular(id);
        maps_division[thread].push_back(&map);
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

    this->_amqp_thread = std::make_unique<std::thread>(&context::amqp_thread, this);

    this->bind(&context::handle_login);          // 게임서버 접속 핸들러
    this->bind(&context::handle_direction);      // 방향전환 핸들러
    this->bind(&context::handle_logout);         // 접속 종료
    this->bind(&context::handle_update_move);    // 이동과 맵 데이터 업데이트 핸들러
    this->bind(&context::handle_move);           // 이동 핸들러
    this->bind(&context::handle_attack);         // 공격 핸들러
    this->bind(&context::handle_pickup);         // 아이템 줍기 핸들러
    this->bind(&context::handle_emotion);        // 감정표현 핸들러
    this->bind(&context::handle_update_map);     // 맵 데이터 업데이트 핸들러
    this->bind(&context::handle_update_screen);  // 새로고침 핸들러
    this->bind(&context::handle_active_item);    // 아이템 사용 핸들러
    this->bind(&context::handle_inactive_item);  // 아이템 장착 해제 핸들러
    this->bind(&context::handle_drop_item);      // 아이템 버리기 핸들러
    this->bind(&context::handle_drop_money);     // 금전 버리기 핸들러
    this->bind(&context::handle_front_info);     // 앞방향 정보 핸들러
    this->bind(&context::handle_self_info);      // 나 자신의 정보 핸들러
    this->bind(&context::handle_option_changed); // 옵션 설정 핸들러
    this->bind(&context::handle_click_object);   // 오브젝트 클릭 핸들러
    this->bind(&context::handle_item_info);      // 인벤토리 우클릭 핸들러
    this->bind(&context::handle_itemmix);        // 아이템 조합 핸들러
    this->bind(&context::handle_trade);          // 교환 핸들러
    this->bind(&context::handle_group);          // 그룹 핸들러
    this->bind(&context::handle_user_list);      // 유저 리스트 핸들러
    this->bind(&context::handle_chat);           // 유저 채팅 핸들러
    this->bind(&context::handle_board);          // 게시판 섹션 리스트 핸들러
    this->bind(&context::handle_swap);           // 스펠 순서 변경
    this->bind(&context::handle_dialog);         // 다이얼로그
    this->bind(&context::handle_dialog, 0x39);   // 다이얼로그
    this->bind(&context::handle_throw_item);     // 아이템 던지기 핸들러
    this->bind(&context::handle_spell);          // 스펠 핸들러
    this->bind(&context::handle_door);           // 도어 핸들러
    this->bind(&context::handle_whisper);        // 귓속말 핸들러
    this->bind(&context::handle_world);          // 월드맵 핸들러
    this->bind(&context::handle_object_miss);
    this->bind(&context::handle_give_item);
    this->bind(&context::handle_give_money);
    this->bind(&context::handle_post);

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
    this->bind_npc_interaction(&context::npc_interaction_deposit_item);
    this->bind_npc_interaction(&context::npc_interaction_withdraw_item);
    this->bind_npc_interaction(&context::npc_interaction_sell_list);
    this->bind_npc_interaction(&context::npc_interaction_buy_list);
    this->bind_npc_interaction(&context::npc_interaction_sell_price);
    this->bind_npc_interaction(&context::npc_interaction_buy_price);
    this->bind_npc_interaction(&context::npc_interaction_show_deposited_money);
    this->bind_npc_interaction(&context::npc_interaction_rename_weapon);
    this->bind_npc_interaction(&context::npc_interaction_hold_item_list);
    this->bind_npc_interaction(&context::npc_interaction_hold_item_count);
    this->bind_npc_interaction(&context::npc_interaction_revive);
    this->bind_npc_interaction(&context::npc_interaction_appreciate);

    this->bind_amqp(std::format("fb.game.{}", config<uint32_t>("id")), &context::handle_amqp_Pong);
    this->bind_amqp(std::format("fb.game.{}", config<uint32_t>("id")), &context::handle_amqp_KickOut);
    this->bind_amqp(std::format("fb.game.{}", config<uint32_t>("id")), &context::handle_amqp_Whisper);
    this->bind_amqp("fb.global", &context::handle_amqp_Pong);
    this->bind_amqp("fb.global", &context::handle_amqp_Broadcast);
    this->bind_amqp("fb.group", &context::handle_amqp_EnterGroup);
    this->bind_amqp("fb.group", &context::handle_amqp_LeaveGroup);
    this->bind_amqp("fb.clan", &context::handle_amqp_SetClanTitle);
    this->bind_amqp("fb.clan", &context::handle_amqp_JoinClan);
    this->bind_amqp("fb.clan", &context::handle_amqp_LeaveClan);
    this->bind_amqp("fb.clan", &context::handle_amqp_BroadcastClan);
    this->bind_amqp("fb.mail", &context::handle_amqp_WriteMail);
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

async::task<bool> context::handle_connected(fb::socket<character>& socket)
{
    co_return true;
}

async::task<bool> context::handle_disconnected(fb::socket<character>& socket)
{
    auto ch = socket.data();
    if (ch == nullptr)
        co_return false;

    if (ch->trade.trading())
        ch->trade.cancel();

    auto id = ch->id();
    this->_shard[id]->ids.lock([&id](auto& ids) {
        ids.erase(id);
    });

    auto& name = ch->name();
    this->_shard[name]->names.lock([&name](auto& names) {
        names.erase(name);
    });

    fb::logger::info("{}님이 접속을 종료했습니다.", ch->name());

    co_await this->save(*ch);
    try
    {
        std::ignore =
            co_await this->post<internal_reqs::Logout, internal_resp::Logout>("internal",
                                                                              "/in-game/logout",
                                                                              internal_reqs::Logout{ch->name()});
    }
    catch (std::exception& e)
    {
        fb::logger::fatal(e.what());
    }

    co_await this->update_thread(*ch);

    auto& group_lock = ch->group();
    if (group_lock != nullptr)
    {
        group_lock->lock([this, ch](auto& group) {
            group.leave(*ch);
        });
        group_lock.reset();
    }

    auto& clan_lock = ch->clan();
    if (clan_lock != nullptr)
    {
        clan_lock->lock([ch](auto& clan) {
            clan.detach_character(*ch);
        });
        clan_lock.reset();
    }
    ch->init(false);
    co_await ch->destroy();
    socket.data(nullptr);
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

void context::foreach_ch(const std::string&                                  name,
                         const std::function<void(fb::game::character&)>&    fn,
                         const std::function<void(const std::string& name)>& miss)
{
    this->foreach_ch(std::vector<std::string>{name}, fn, miss);
}

void context::foreach_ch(const std::string& name, const std::function<void(fb::game::character&)>& fn)
{
    this->foreach_ch({name}, fn, [](auto&) {
    });
}

void context::foreach_ch(const std::vector<std::string>&                     names,
                         const std::function<void(fb::game::character&)>&    fn,
                         const std::function<void(const std::string& name)>& miss)
{
    auto g = std::unordered_map<uint32_t, std::vector<std::string>>{};
    for (auto& name : names)
    {
        auto mod = this->_shard.mod(name);

        if (!g.contains(mod))
            g.insert({mod, std::vector<std::string>{}});

        g[mod].push_back(name);
    }

    for (auto& [mod, names] : g)
    {
        this->_shard[mod]->names.lock([this, &names, &fn, &miss](auto& ch_names) {
            for (auto& name : names)
            {
                if (!ch_names.contains(name))
                {
                    miss(name);
                    continue;
                }

                auto ch     = ch_names[name];
                auto thread = ch->thread();
                std::ignore = thread->dispatch([this, fn, ch, fd = ch->fd()](auto& thread) -> async::task<void> {
                    co_await this->update_thread(*ch);
                    fn(*ch);
                });
            }
        });
    }
}

void context::foreach_ch(const std::vector<std::string>& names, const std::function<void(fb::game::character&)>& fn)
{
    this->foreach_ch(names, fn, [](auto&) {
    });
}

void context::foreach_ch(const std::function<void(fb::game::character&)>& fn)
{
    for (int i = 0, n = this->threads.size(); i < n; i++)
    {
        auto thread = this->threads.at(i);
        std::ignore = thread->dispatch([=](auto& thread) -> async::task<void> {
            auto params = thread.template data<thread_params>();
            for (auto& [_, ch] : params->characters)
            {
                fn(*ch);
            }
            co_return;
        });
    }
}

async::task<bool> context::init_ch(const internal::Character&           response,
                                   character&                           ch,
                                   std::optional<uint32_t>              group,
                                   std::optional<uint32_t>              clan,
                                   const std::optional<transfer_param>& transfer)
{
    auto map = response.map;
    ch.id(response.id);
    ch.name(response.name);
    ch.pw(response.pw);
    ch.birthday(response.birth);
    ch.updated_date(fb::model::datetime(response.updated_date));
    ch.admin(response.admin);
    ch.cls(static_cast<CLASS>(response.class_type));
    ch.color(response.color);
    ch.direction(DIRECTION(response.direction));
    ch.look(response.look);
    ch.money(response.money);
    ch.deposited_money(response.deposited_money);
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
        this->upsert_group_then(group.value(), [&ch](auto& lock) {
            lock->lock([&ch](auto& group) {
                group.enter(ch);
            });
            ch.group(lock);
        });
    }

    if (clan.has_value())
    {
        this->upsert_clan_then(clan.value(), [&ch](auto& lock) {
            lock->lock([&ch](auto& clan) {
                clan.attach_character(ch);
            });
            ch.clan(lock);
        });
    }

    co_return co_await ch.map(&this->maps[map], fb::model::point16_t(position_x, position_y));
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
        auto item = this->model.item[x.model].make(*this);
        item->owner(&ch);
        item->count(x.count);

        if (x.durability.has_value())
            item->durability(x.durability.value());

        if (x.custom_name.has_value() && item->based<fb::model::item>().attr(ITEM_ATTRIBUTE::WEAPON))
            static_cast<weapon*>(item)->custom_name(x.custom_name.value());

        if (x.deposited != -1)
            ch.deposit_item(*item);
        else if (x.parts == static_cast<uint32_t>(EQUIPMENT_PARTS::UNKNOWN))
            ch.items.add(*item, x.index);
        else
            ch.items.wear((EQUIPMENT_PARTS)x.parts, static_cast<equipment*>(item));
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

void context::init_traces(const std::vector<fb::protocol::internal::Trace>& response, fb::game::character& ch)
{
    for (auto& x : response)
    {
        if (this->model.trace.contains(x.model) == false)
            continue;

        ch.traces.insert({x.model, std::make_unique<fb::game::trace>(this->model.trace[x.model], x.text)});
    }
}

character* context::handle_accepted(fb::socket<character>& socket)
{
    return this->make<character>(socket);
}

async::task<void>
context::send(object& object, const fb::protocol::header& header, context::scope scope, bool exclude_self, bool encrypt)
{
    auto stream = fb::stream();
    auto writer = fb::stream_writer<big_endian>(stream);
    co_await header.serialize(writer);

    switch (scope)
    {
    case context::scope::PIVOT:
    {
        if (!exclude_self)
            object.send(stream, encrypt);

        for (auto& x : object.nears(OBJECT_TYPE::CHARACTER))
        {
            if (x->sight(object) == false)
                continue;

            x->send(stream, encrypt);
        }
    }
    break;

    case context::scope::GROUP:
    {
        if (object.is(OBJECT_TYPE::CHARACTER) == false)
            co_return;

        auto& ch                = static_cast<const character&>(object);
        auto& shared_group_lock = ch.group();
        if (shared_group_lock == nullptr)
            co_return;

        shared_group_lock->lock([&stream, encrypt](auto& group) {
            for (auto ch : group.characters())
            {
                ch->send(stream, encrypt);
            }
        });
    }
    break;

    case context::scope::MAP:
    {
        auto map = object.map();
        if (map == nullptr)
            co_return;

        for (const auto& [seq, obj] : object.map()->objects)
        {
            if (exclude_self && obj == object)
                continue;

            obj.send(stream, encrypt);
        }
    }
    break;

    case context::scope::WORLD:
    {
        this->foreach_ch([stream, encrypt](auto& ch) -> async::task<void> {
            ch.send(stream, encrypt);
            co_return;
        });
    }
    break;
    }
}

async::task<void> context::save(character& ch)
{
    if (ch.inited() == false)
        co_return;

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

    auto& deposited_items = ch.deposited_items();
    for (int i = 0; i < deposited_items.size(); i++)
    {
        auto item          = deposited_items.at(i);
        auto protocol      = item->to_protocol();
        protocol.deposited = i;
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

    auto traces = std::vector<internal::Trace>();
    for (auto& [model, trace] : ch.traces)
    {
        traces.push_back(internal::Trace{ch.id(), model, trace->text});
    }

    auto fd     = ch.fd();
    std::ignore = co_await this->post<internal_reqs::Save, internal_resp::Save>(
        "internal",
        "/user/save",
        internal_reqs::Save{ch.to_protocol(), items, spells, traces});

    co_await this->update_thread(ch);
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

void context::amqp_thread()
{
    auto timeout = timeval{5, 0};
    while (this->running())
    {
        try
        {
            this->_amqp = std::make_unique<fb::amqp::socket>();
            this->_amqp->connect(fb::config<std::string>("amqp:ip"),
                                 fb::config<uint16_t>("amqp:port"),
                                 fb::config<std::string>("amqp:uid"),
                                 fb::config<std::string>("amqp:pwd"),
                                 "/");

            auto& queue1 = this->_amqp->declare_queue();
            queue1.bind("amq.direct", std::format("fb.game.{}", fb::config<uint32_t>("id")));
            this->bind_amqp(queue1);

            auto& queue2 = this->_amqp->declare_queue();
            queue2.bind("amq.direct", "fb.global");
            this->bind_amqp(queue2);

            auto& queue3 = this->_amqp->declare_queue();
            queue3.bind("amq.direct", "fb.group");
            this->bind_amqp(queue3);

            auto& queue4 = this->_amqp->declare_queue();
            queue4.bind("amq.direct", "fb.clan");
            this->bind_amqp(queue4);

            auto& queue5 = this->_amqp->declare_queue();
            queue5.bind("amq.direct", "fb.mail");
            this->bind_amqp(queue5);
        }
        catch (std::exception& e)
        {
            fb::logger::fatal(e.what());
            std::this_thread::sleep_for(1s);
            continue;
        }

        while (this->running())
        {
            try
            {
                if (this->_amqp->select(&timeout) == false)
                    continue;
            }
            catch (std::exception&)
            {
                break;
            }
        }
    }
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

    // ch.dialog.release();

    // auto lua = ch.dialog.new_context();
    auto lua = fb::lua::new_context();
#if defined DEBUG | defined _DEBUG
    lua->load("scripts/npc.lua");
    lua->load(model.script);
#endif
    lua->func(model.click);
    lua->pushobject(ch);
    lua->pushobject(npc.based<fb::model::npc>());
    std::ignore = lua->call(2);
}

async::task<void> context::broadcast(const std::string& message, MESSAGE_TYPE type, BROADCAST_TYPE broadcast_type)
{
    switch (broadcast_type)
    {
    case BROADCAST_TYPE::GLOBAL:
    {
        auto&& resp = co_await this->post<internal_reqs::Broadcast, internal_resp::Broadcast>(
            "internal",
            "/in-game/broadcast",
            internal_reqs::Broadcast{fb::config<uint32_t>("id"), message, static_cast<uint8_t>(type)});
        this->on_broadcast(resp);
    }
    break;

    case BROADCAST_TYPE::WORLD:
    {
        this->foreach_ch([message, type](auto& ch) {
            ch.message(message, type);
        });
    }
    break;
    }
}

void context::on_broadcast(const internal_resp::Broadcast& resp)
{
    std::ignore = this->broadcast(resp.message, static_cast<MESSAGE_TYPE>(resp.type), BROADCAST_TYPE::WORLD);
}