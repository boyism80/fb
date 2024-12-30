#include <context.h>
using namespace fb::game;
using namespace std::chrono_literals;

context::context(boost::asio::io_context& context, uint16_t port) :
    fb::acceptor<character>(context, "GAME", port),
    maps(*this, fb::config<uint32_t>("id"))
{ }

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

    lua::env<context>("context", this);
    lua::build<map, lua::luable>();
    lua::build<door, lua::luable>();
    lua::build<group, lua::luable>();
    lua::build<clan, lua::luable>();
    lua::build<clan_member, lua::luable>();
    lua::build<trace, lua::luable>();
    lua::build<fb::model::spell, lua::luable>();
    lua::build<fb::model::map, lua::luable>();
    lua::build<fb::model::trace, lua::luable>();
    lua::build<fb::model::object, lua::luable>();
    lua::build<object, lua::luable>();
    lua::build<fb::model::life, fb::model::object>();
    lua::build<life, object>();
    lua::build<fb::model::mob, fb::model::life>();
    lua::build<mob, life>();
    lua::build<fb::model::npc, fb::model::object>();
    lua::build<npc, object>();
    lua::build<fb::model::item, fb::model::object>();
    lua::build<item, object>();
    lua::build<character, life>();

    lua::build("seed", builtin_seed);
    lua::build("sleep", builtin_sleep);
    lua::build("name2mob", builtin_name2mob);
    lua::build("name2item", builtin_name2item);
    lua::build("name2npc", builtin_name2npc);
    lua::build("name2map", builtin_name2map);
    lua::build("broadcast", builtin_broadcast);
    lua::build("pursuit_sell", builtin_pursuit_sell);
    lua::build("pursuit_buy", builtin_pursuit_buy);
    lua::build("sell_price", builtin_sell_price);
    lua::build("buy_price", builtin_buy_price);
    lua::build("timer", builtin_timer);
    lua::build("weather", builtin_weather);
    lua::build("name_with", builtin_name_with);
    lua::build("assert_korean", builtin_assert_korean);
    lua::build("CP949", builtin_cp949);

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
            auto& ist  = lua::container::ist();
            auto& main = ist.get();
            fb::model::lua::map_enum(main);
            main.load_file("scripts/script.lua");
            main.load_file("scripts/common/npc.lua");
            main.load_file("scripts/common/door.lua");
            main.load_file("scripts/common/pickup.lua");
            main.load_file("scripts/common/attack.lua");

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
    this->bind(&context::handle_refresh);        // 새로고침 핸들러
    this->bind(&context::handle_active_item);    // 아이템 사용 핸들러
    this->bind(&context::handle_inactive_item);  // 아이템 장착 해제 핸들러
    this->bind(&context::handle_drop_item);      // 아이템 버리기 핸들러
    this->bind(&context::handle_drop_cash);      // 금전 버리기 핸들러
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

    this->bind_timer(&context::handle_heart_beat, 1s);
    this->bind_timer(&context::handle_time, 1min); // 세계 시간 타이머

    this->bind_thread_timer(&context::handle_mob_action, 100ms); // 몹 행동 타이머
    this->bind_thread_timer(&context::handle_mob_respawn, 1s);   // 몹 리젠 타이머
    this->bind_thread_timer(&context::handle_buff_timer, 1s);    // 버프 타이머
    this->bind_thread_timer(&context::handle_save_timer,
                            std::chrono::seconds(fb::config<uint32_t>("save"))); // DB 저장 타이머

    this->command("맵이동", &context::handle_command_map, true);
    this->command("사운드", &context::handle_command_sound, true);
    this->command("액션", &context::handle_command_action, true);
    this->command("날씨", &context::handle_command_weather, true);
    this->command("밝기", &context::handle_command_bright, true);
    this->command("타이머", &context::handle_command_timer, true);
    this->command("이펙트", &context::handle_command_effect, true);
    this->command("변신", &context::handle_command_disguise, true);
    this->command("변신해제", &context::handle_command_undisguise, true);
    this->command("마법배우기", &context::handle_command_spell, true);
    this->command("몬스터생성", &context::handle_command_mob, true);
    this->command("직업바꾸기", &context::handle_command_class, true);
    this->command("레벨바꾸기", &context::handle_command_level, true);
    this->command("아이템생성", &context::handle_command_item, true);
    this->command("월드맵", &context::handle_command_world, true);
    this->command("스크립트", &context::handle_command_script, true);
    this->command("머리바꾸기", &context::handle_command_hair, true);
    this->command("머리염색", &context::handle_command_hair_color, true);
    this->command("갑옷염색", &context::handle_command_armor_color, true);
    this->command("서버종료", &context::handle_command_exit, true);
    this->command("타일", &context::handle_command_tile, true);
    this->command("서버저장", &context::handle_command_save, true);
    this->command("맵오브젝트", &context::handle_command_mapobj, true);
    this->command("랜덤이동", &context::handle_command_randmap, false);
    this->command("엔피씨생성", &context::handle_command_npc, true);
    this->command("내구도", &context::handle_command_durability, true);
    this->command("동시성테스트", &context::handle_command_concurrency, true);
    this->command("sleep", &context::handle_command_sleep, true);
    this->command("맵타일", &context::handle_map_tile, true);
    this->command("메일", &context::handle_mail, true);

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

    auto& name = ch->name();
    this->_shard[name]->characters.lock([&name](auto& characters) {
        characters.erase(name);
    });

    fb::logger::info("{}님이 접속을 종료했습니다.", ch->name());

    this->save(*ch);
    std::ignore = co_await this->post<internal_reqs::Logout, internal_resp::Logout>("internal",
                                                                                    "/in-game/logout",
                                                                                    internal_reqs::Logout{ch->name()});

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

async::task<void> context::handle_timer(uint64_t elapsed_milliseconds)
{
    for (auto& [key, map] : this->maps)
        co_await map.on_timer(elapsed_milliseconds);
}

std::string context::elapsed_message(const std::string& dt)
{
    auto elapsed = datetime() - datetime(dt);
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
        this->_shard[mod]->characters.lock([this, &names, &fn, &miss](auto& characters) {
            for (auto& name : names)
            {
                if (!characters.contains(name))
                {
                    miss(name);
                    continue;
                }

                auto ch     = characters[name];
                auto thread = ch->thread();
                std::ignore = thread->dispatch([this, fn, ch, fd = ch->fd()](auto& thread) -> async::task<void> {
                    if (this->assert_socket(fd) == false)
                        co_return;

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
    ch.updated_date(datetime(response.updated_date));
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

    if (response.armor_color.has_value())
        ch.armor_color(response.armor_color.value());

    if (response.disguise.has_value())
        ch.disguise(response.disguise.value());
    else
        ch.undisguise();

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

    if (co_await ch.map(&this->maps[map], point16_t(position_x, position_y)) == false)
        co_return false;

    if (group.has_value())
    {
        auto gid = group.value();
        this->upsert_group_then(gid, [&ch](auto& group_lock_ptr) {
            group_lock_ptr->lock([&ch](auto& group) {
                group.enter(ch);
            });
            ch.group(group_lock_ptr);
        });
    }

    if (clan.has_value())
    {
        auto id = clan.value();
        this->upsert_clan_then(id, [&ch](auto& clan_lock_ptr) {
            clan_lock_ptr->lock([&ch](auto& clan) {
                clan.attach_character(ch);
            });
            ch.clan(clan_lock_ptr);
        });
    }

    co_return true;
}

void context::init_option(const internal::Option& response, fb::game::character& ch)
{
    ch.option(SETTING::WHISPER, response.whisper, false);
    ch.option(SETTING::GROUP, response.group, false);
    ch.option(SETTING::ROAR, response.roar, false);
    ch.option(SETTING::ROAR_WORLDS, response.roar_worlds, false);
    ch.option(SETTING::MAGIC_EFFECT, response.magic_effect, false);
    ch.option(SETTING::WEATHER_EFFECT, response.weather_effect, false);
    ch.option(SETTING::FIXED_MOVE, response.fixed_move, false);
    ch.option(SETTING::TRADE, response.trade, false);
    ch.option(SETTING::FAST_MOVE, response.fast_move, false);
    ch.option(SETTING::EFFECT_SOUND, response.effect_sound, false);
    ch.option(SETTING::PK_PROTECT, response.pk_protect, false);
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
        ch.spells.add(model, x.slot);
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

void context::assert_whisper(const internal_resp::Whisper& response) const
{
    switch (static_cast<ERROR_CODE>(response.error))
    {
    case ERROR_CODE::NONE:
        return;

    case ERROR_CODE::OFFLINE:
        throw std::runtime_error(std::format("{}님은 바람의나라에 없습니다.", response.to));

    case ERROR_CODE::DISABLED_WHISPER_TARGET:
        throw std::runtime_error(std::format("{}님은 귓속말 거부 상태입니다.", response.to));

    default:
        throw std::runtime_error(std::format("알 수 없는 에러가 발생했습니다. (에러코드 : {})", response.error));
    }
}

character* context::handle_accepted(fb::socket<character>& socket)
{
    return this->make<character>(socket);
}

void context::send(object&                           object,
                   const fb::protocol::base::header& header,
                   context::scope                    scope,
                   bool                              exclude_self,
                   bool                              encrypt)
{
    switch (scope)
    {
    case context::scope::SELF:
        object.send(header, encrypt);
        break;

    case context::scope::PIVOT:
    {
        auto nears = object.showings(OBJECT_TYPE::CHARACTER);
        if (!exclude_self)
            object.send(header, encrypt);

        for (auto& x : nears)
            x->send(header, encrypt);
    }
    break;

    case context::scope::GROUP:
    {
        if (object.is(OBJECT_TYPE::CHARACTER) == false)
            return;

        auto& ch                = static_cast<character&>(object);
        auto& shared_group_lock = ch.group();
        if (shared_group_lock == nullptr)
            return;

        shared_group_lock->lock([&header, encrypt](auto& group) {
            for (auto ch : group.characters())
            {
                ch->send(header, encrypt);
            }
        });
    }
    break;

    case context::scope::MAP:
    {
        auto map = object.map();
        if (map == nullptr)
            return;

        for (const auto& [seq, obj] : object.map()->objects)
        {
            if (exclude_self && obj == object)
                continue;

            obj.send(header, encrypt);
        }
    }
    break;

    case context::scope::WORLD:
    {
        this->send(header, encrypt);
    }
    break;
    }
}

void context::send(object& object, const protocol_generator& fn, context::scope scope, bool exclude_self, bool encrypt)
{
    switch (scope)
    {
    case context::scope::SELF:
        object.send(*fn(object).get(), encrypt);
        break;

    case context::scope::PIVOT:
    {
        auto nears = object.showings(OBJECT_TYPE::CHARACTER);
        if (!exclude_self)
            object.send(*fn(object).get(), encrypt);

        for (auto& x : nears)
            x->send(*fn(*x).get(), encrypt);
    }
    break;

    case context::scope::GROUP:
    {
        if (object.is(OBJECT_TYPE::CHARACTER) == false)
            return;

        auto& ch                = static_cast<character&>(object);
        auto& shared_group_lock = ch.group();
        if (shared_group_lock == nullptr)
            return;

        shared_group_lock->lock([&fn, encrypt](auto& group) {
            for (auto ch : group.characters())
            {
                ch->send(*fn(*ch).get(), encrypt);
            }
        });
    }
    break;

    case context::scope::MAP:
    {
        for (const auto& [seq, obj] : object.map()->objects)
        {
            if (exclude_self && obj == object)
                continue;

            obj.send(*fn(obj).get(), encrypt);
        }
    }
    break;

    case context::scope::WORLD:
    {
        for (int i = 0, n = this->threads.size(); i < n; i++)
        {
            auto thread = this->threads.at(i);
            auto params = thread->data<thread_params>();
            for (auto& [_, ch] : params->characters)
            {
                ch->send(*fn(*ch).get(), encrypt); // TODO: check thread switch required
            }
        }
    }
    break;
    }
}

void context::send(const fb::protocol::base::header& response, const map& map, bool encrypt)
{
    auto thread = map.thread();
    auto params = thread->data<thread_params>();
    for (auto& [_, ch] : params->characters)
    {
        ch->send(response, encrypt);
    }
}

void context::send(const fb::protocol::base::header& response, bool encrypt)
{
    for (int i = 0, n = this->threads.size(); i < n; i++)
    {
        auto thread = this->threads.at(i);
        auto params = thread->data<thread_params>();
        for (auto& [_, ch] : params->characters)
        {
            ch->send(response, encrypt); // TODO: check thread switch required
        }
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

        spells.push_back(internal::Spell{ch.id(), i, spell->id});
    }

    auto traces = std::vector<internal::Trace>();
    for (auto& [model, trace] : ch.traces)
    {
        traces.push_back(internal::Trace{ch.id(), model, trace->text});
    }

    std::ignore = co_await this->post<internal_reqs::Save, internal_resp::Save>(
        "internal",
        "/user/save",
        internal_reqs::Save{ch.to_protocol(), items, spells, traces});
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
            queue1.handler<internal_resp::Pong>([](auto& response) -> async::task<void> {
                co_return;
            });
            queue1.handler<internal_resp::KickOut>([this](auto& response) -> async::task<void> {
                auto ch = this->_shard[response.name]->characters.template lock<character*>(
                    [&name = response.name](shard_params::character_container& container) -> character* {
                        if (!container.contains(name))
                            return nullptr;

                        return container.at(name);
                    });

                if (ch == nullptr)
                    co_return;

                auto& socket = static_cast<fb::socket<character>&>(*ch);
                socket.close();
            });

            queue1.handler<internal_resp::Whisper>([this](auto& response) -> async::task<void> {
                if (response.host == fb::config<uint16_t>("id"))
                    co_return;

                try
                {
                    this->assert_whisper(response);
                    this->foreach_ch(response.to, [&response](auto& you) {
                        you.message(std::format("{}> {}", response.from, response.message), MESSAGE_TYPE::NOTIFY);
                    });
                }
                catch (std::exception& e)
                {
                    this->foreach_ch(response.from, [&response, error = e.what()](auto& me) {
                        me.message(error, MESSAGE_TYPE::NOTIFY);
                    });
                }
            });

            auto& queue2 = this->_amqp->declare_queue();
            queue2.bind("amq.direct", "fb.global");
            queue2.handler<internal_resp::Pong>([](auto& response) -> async::task<void> {
                co_return;
            });

            auto& queue3 = this->_amqp->declare_queue();
            queue3.bind("amq.direct", "fb.group");
            queue3.handler<internal_resp::EnterGroup>([this](internal_resp::EnterGroup& response) -> async::task<void> {
                if (response.host == fb::config<uint32_t>("id"))
                    co_return;

                this->on_enter_group(response);
            });

            queue3.handler<internal_resp::LeaveGroup>([this](internal_resp::LeaveGroup& response) -> async::task<void> {
                if (response.host == fb::config<uint32_t>("id"))
                    co_return;

                this->on_leave_group(response);
            });

            auto& queue4 = this->_amqp->declare_queue();
            queue4.bind("amq.direct", "fb.clan");
            queue4.handler<internal_resp::SetClanTitle>(
                [this](internal_resp::SetClanTitle& response) -> async::task<void> {
                    if (response.host == fb::config<uint32_t>("id"))
                        co_return;

                    this->on_clan_title_changed(response);
                });

            queue4.handler<internal_resp::JoinClan>([this](internal_resp::JoinClan& response) -> async::task<void> {
                if (response.host == fb::config<uint32_t>("id"))
                    co_return;

                this->on_clan_join_member(response);
            });

            queue4.handler<internal_resp::LeaveClan>([this](internal_resp::LeaveClan& response) -> async::task<void> {
                if (response.host == fb::config<uint32_t>("id"))
                    co_return;

                this->on_clan_leave_member(response);
            });

            queue4.handler<internal_resp::BroadcastClan>(
                [this](internal_resp::BroadcastClan& response) -> async::task<void> {
                    if (response.host == fb::config<uint32_t>("id"))
                        co_return;

                    this->on_clan_broadcast(response);
                });
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
    this->send(ch, fb_resp::character::message(mob.name(), MESSAGE_TYPE::STATE), scope::SELF);
}

void context::handle_click_npc(character& ch, npc& npc)
{
    auto& model = npc.based<fb::model::npc>();
    if (model.script.empty())
        return;

    ch.dialog.release();
    ch.dialog.from(model.script.c_str())
        .func("on_interact")
        .pushobject(ch)
        .pushobject(npc.based<fb::model::npc>())
        .resume(2);
}

async::task<bool> context::handle_command(character& ch, const std::string& message)
{
    if (message.starts_with('/') == false)
        co_return false;

    std::vector<std::string> splitted;
    std::istringstream       sstream(message.substr(1));
    std::string              unit;
    while (std::getline(sstream, unit, ' '))
    {
        splitted.push_back(unit);
    }

    if (splitted.empty())
        co_return false;

    auto found = this->_commands.find(splitted[0]);
    if (found == this->_commands.end())
        co_return false;

    if (found->second.admin && ch.admin() == false)
        co_return false;

    Json::Value parameters;
    for (auto i = splitted.begin() + 1; i != splitted.end(); i++)
    {
        auto digit = std::all_of((*i).begin(), (*i).end(), [](uint8_t c) {
            return std::isdigit(c);
        });
        if (digit)
            parameters.append(std::stoi(*i));
        else
            parameters.append(*i);
    }

    co_return co_await found->second.fn(ch, parameters);
}
