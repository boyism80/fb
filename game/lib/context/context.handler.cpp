#include <fb/game/context.h>

using namespace fb::game;
using namespace std::chrono_literals;

async::task<bool> context::handle_login(fb::socket<character>& socket, const fb_reqs::login& request)
{
    auto ch = socket.data();
    if (ch == nullptr)
        co_return false;

    auto weak = ch->weak_from_this();
    socket.crt(request.enc_type, request.enc_key);

    ch->name(request.name);
    fb::logger::info("{} has connected.", request.name);

    auto delay = fb::config<uint32_t>("delay");
    co_await this->sleep(std::chrono::seconds(delay));

    auto&& login_resp = co_await this->http.post("internal",
                                                 "/in-game/login",
                                                 Login{request.id, request.name, fb::config<uint8_t>("id")});
    if (login_resp.error != (uint32_t)ERROR_CODE::NONE)
        co_return false;

    auto&& response =
        co_await this->http.get<internal_resp::Init>("internal", std::format("/user/init/{}", request.id));
    auto map = request.transfer.has_value() ? request.transfer->map : response.character.map;
    ch->thread(this->maps[map]->thread());
    co_await this->threads.switching(weak);

    if (co_await this->init_ch(response.character, *ch, response.group, response.clan, request.transfer) == false)
        co_return false;
    co_await this->threads.switching(weak);

    ch->unread_mail(response.mail);

    this->init_items(response.items, *ch);
    this->init_spells(response.spells, *ch);
    this->init_achievements(response.achievements, *ch);
    this->init_option(response.option, *ch);
    ch->init();
    ch->update_time(this->_time.hours());
    if (request.from == internal::Service::Login)
    {
        auto msg = this->elapsed_message(response.character.updated_date);
        if (msg.empty() == false)
            ch->message(msg, MESSAGE_TYPE::STATE);

        auto lua = fb::lua::new_context();
        if (lua != nullptr)
        {
#if defined DEBUG | defined _DEBUG
            lua->load("scripts/interaction.lua");
#endif
            lua->func("on_login");
            lua->pushobject(ch);
            std::ignore = lua->call(1);
        }
    }

    ch->update(STATE_LEVEL::LEVEL_MAX);
    ch->update_option();
    this->characters.insert(ch);
    co_return true;
}

async::task<bool> context::handle_direction(fb::socket<character>& socket, const fb_reqs::direction& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    if (ch->direction(request.value) == false)
        co_return false;

    co_return true;
}

async::task<bool> context::handle_logout(fb::socket<character>& socket, const fb_reqs::exit& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    std::ignore = this->transfer(socket,
                                 fb::config<std::string>("login:ip"),
                                 fb::config<uint16_t>("login:port"),
                                 internal::Service::Game);
    co_return true;
}

async::task<bool> context::handle_move(fb::socket<character>& socket, const fb_reqs::move& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return true;

    if (ch->paralysis() || ch->cover())
    {
        ch->update_position();
        co_return true;
    }

    // TODO: 실제로 이동하지 않고 이동했을때의 위치를 구해서
    // 해당 위치에서 워프가 가능한지 확인하고
    // 워프가능하면 워프처리, 그렇지 않다면 해당 위치로 이동한다.
    auto forward = ch->side_position(request.direction);

    // 워프 위치라면 워프한다.
    const auto warp = map->warpable(forward);
    if (warp != nullptr)
    {
        if (ch->condition(warp->condition) == false)
        {
            ch->message("감히 접근할 수 없습니다.");
            ch->update_position();
            co_return true;
        }

        switch (warp->dest.header)
        {
        case DSL::map:
        {
            auto params = fb::model::dsl::map(warp->dest.params);
            auto map    = this->maps[params.id];
            std::ignore = co_await ch->map(map, fb::model::point16_t(params.x, params.y));
        }
        break;

        case DSL::world:
        {
            auto  params = fb::model::dsl::world(warp->dest.params);
            auto& world  = this->model.world[params.id][params.index];
            ch->show_world_map(params.id, params.index);
        }
        break;

        case DSL::script:
        {
            ch->move(request.direction, request.position);

            auto params = fb::model::dsl::script(warp->dest.params);
            auto lua    = fb::lua::new_context();
            if (lua != nullptr)
            {
#if defined DEBUG | defined _DEBUG
                lua->load(params.path);
#endif
                lua->func(params.function);
                lua->pushobject(ch);
                std::ignore = lua->call(1, false);
            }
        }
        break;

        default:
            throw std::runtime_error("invalid dsl header");
        }
    }
    else
    {
        ch->move(request.direction, request.position);
    }
    co_return true;
}

async::task<bool> context::handle_update_move(fb::socket<character>& socket, const fb_reqs::update_move& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return true;

    if (co_await this->handle_move(socket, request))
        ch->update_map(*map, request.begin, request.size);

    co_return true;
}

async::task<bool> context::handle_attack(fb::socket<character>& socket, const fb_reqs::attack& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    co_await ch->attack();
    co_return true;
}

async::task<bool> context::handle_pickup(fb::socket<character>& socket, const fb_reqs::pick_up& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    ch->items.pickup(request.boost);
    co_return true;
}

async::task<bool> context::handle_emotion(fb::socket<character>& socket, const fb_reqs::emotion& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    ch->action(ACTION(static_cast<int>(ACTION::EMOTION) + request.value), DURATION::EMOTION);
    co_return true;
}

async::task<bool> context::handle_update_map(fb::socket<character>& socket, const fb_reqs::map_update& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return true;

    ch->update_map(*map, request.position, request.size);
    co_return true;
}

async::task<bool> context::handle_update_screen(fb::socket<character>& socket, const fb_reqs::update_screen& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    ch->update_position();
    co_return true;
}

async::task<bool> context::handle_active_item(fb::socket<character>& socket, const fb_reqs::item_active& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    ch->items.active(request.index);
    co_return true;
}

async::task<bool> context::handle_inactive_item(fb::socket<character>& socket, const fb_reqs::item_inactive& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    if (ch->items.free_size() == 0)
    {
        ch->message(_TEXT(MESSAGE_EXCEPTION_INVENTORY_OVERFLOW));
        co_return true;
    }

    ch->items.inactive(request.parts);
    co_return true;
}

async::task<bool> context::handle_drop_item(fb::socket<character>& socket, const fb_reqs::item_drop& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    ch->items.drop(request.index, request.all ? -1 : 1);
    co_return true;
}

async::task<bool> context::handle_drop_money(fb::socket<character>& socket, const fb_reqs::item_drop_money& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return false;

    auto chunk = std::min(ch->money(), request.chunk);

    ch->money_drop(chunk);
    co_return true;
}

async::task<bool> context::handle_front_info(fb::socket<character>& socket, const fb_reqs::front_info& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return false;

    auto forwards = ch->forwards();
    for (auto i = forwards.begin(); i != forwards.end(); i++)
    {
        auto object  = *i;
        auto message = object->is(OBJECT_TYPE::ITEM) ? std::static_pointer_cast<fb::game::item>(object)->inven_name()
                                                     : object->name();

        ch->message(message, MESSAGE_TYPE::STATE);
    }

    co_return true;
}

async::task<bool> context::handle_self_info(fb::socket<character>& socket, const fb_reqs::self_info& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    ch->update_internal();
    ch->update_buff();
    co_return true;
}

async::task<bool> context::handle_option_changed(fb::socket<character>& socket, const fb_reqs::update_option& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto weak   = ch->weak_from_this();
    auto option = OPTION(request.option);
    switch (option)
    {
    case OPTION::EXTENSION:
        if (request.ride)
        {
            if (ch->state() == STATE::RIDING)
                ch->unride();
            else
                ch->ride();
        }
        else
        {
            ch->update_option();
        }
        break;

    default:
        auto enabled = ch->option_toggle(option);
        if (option == OPTION::GROUP && !enabled)
        {
            auto&& response = co_await this->http.post("internal", "/group/leave", LeaveGroup{ch->name()});
            co_await this->threads.switching(weak);

            co_await this->on_leave_group(response);
        }

        auto&& response = co_await this->http.post("internal",
                                                   "/user/option",
                                                   SetOption{ch->id(), static_cast<uint8_t>(option), enabled});
        co_await this->threads.switching(weak);

        if (response.success == false)
            ch->message("설정을 변경하지 못했습니다.");
        break;
    }
    co_return true;
}

async::task<bool> context::handle_click_object(fb::socket<character>& socket, const fb_reqs::click& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    if (request.fd == 0xFFFFFFFF) // Press F1
        co_return true;

    if (request.fd == 0xFFFFFFFE) // Preff F2
        co_return true;

    auto map = ch->map();
    auto you = map->objects[request.fd];
    if (you == nullptr)
        co_return true;

    switch (you->what())
    {
    case OBJECT_TYPE::CHARACTER:
        ch->browse_ch(static_cast<character&>(*you));
        break;

    case OBJECT_TYPE::MOB:
        this->handle_click_mob(*ch, static_cast<mob&>(*you));
        break;

    case OBJECT_TYPE::NPC:
        this->handle_click_npc(*ch, static_cast<npc&>(*you));
        break;
    }

    co_return true;
}

// TODO : on_item_detail
async::task<bool> context::handle_item_info(fb::socket<character>& socket, const fb_reqs::item_info& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto item = ch->items[request.slot];
    if (item == nullptr)
        co_return false;

    ch->item_tooltip(*item, request.position);
    co_return true;
}

async::task<bool> context::handle_itemmix(fb::socket<character>& socket, const fb_reqs::item_mix& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    if (request.indices.size() > CONTAINER_CAPACITY - 1)
        co_return false;

    auto dsl = std::vector<fb::model::dsl::item>();
    for (auto index : request.indices)
    {
        auto item = ch->items[index];
        if (item == nullptr)
            continue;

        auto& model = item->based<fb::model::item>();
        dsl.push_back(fb::model::dsl::item(model.id, item->count(), 0.0));
    }

    auto found = this->model.recipe.find(dsl);
    if (found == nullptr)
        co_return true;

    if (found->success.size() - found->source.size() > ch->items.free_size())
        co_return true;

    for (auto& x : found->source)
    {
        auto params        = fb::model::dsl::item(x.params);
        auto deleted_count = uint32_t(0);
        while (deleted_count <= params.count)
        {
            auto item  = ch->items.find(this->model.item[params.id]);
            auto index = ch->items.index(this->model.item[params.id]);
            if (item == nullptr)
                throw std::runtime_error("no match exception");

            auto count   = item->count();
            auto deleted = ch->items.remove(item, count);
            if (deleted != nullptr)
                std::ignore = deleted->destroy();

            deleted_count += count;
        }
    }

    auto  success = (std::rand() % 100) < found->percent;
    auto& result  = success ? found->success : found->failed;
    for (auto& dsl : result)
    {
        auto  params = fb::model::dsl::item(dsl.params);
        auto& model  = this->model.item[params.id];
        auto  remain = params.count;
        while (remain > 0)
        {
            auto item  = this->make<fb::game::item>(this->model.item[params.id]);
            auto count = std::min<uint16_t>(model.capacity, remain);
            item->count(count);
            ch->items.add(item);
            remain -= count;
        }
    }

    auto& message = success ? _TEXT(MESSAGE_MIX_SUCCESS) : _TEXT(MESSAGE_MIX_FAILED);
    ch->message(message);
    co_return true;
}

async::task<bool> context::handle_trade(fb::socket<character>& socket, const fb_reqs::trade& request)
{
    auto me = socket.data();
    if (me->inited() == false)
        co_return true;

    auto map = me->map();
    if (map == nullptr)
        co_return true;

    auto you = static_cast<character*>(map->objects[request.fd]); // 파트너
    if (you == nullptr)
        co_return true;

    switch (static_cast<trade::state>(request.action))
    {
    case trade::state::REQUEST:
    {
        me->trade.begin(*you);
        break;
    }

    case trade::state::UP_ITEM: // 아이템 올릴때
    {
        auto index = request.parameter.index - 1;
        me->trade.up_item(index);
        break;
    }

    case trade::state::ITEM_COUNT: // 아이템 갯수까지 해서 올릴 때
    {
        me->trade.count(request.parameter.count);
        break;
    }

    case trade::state::UP_MONEY: // 금전 올릴 때
    {
        // 클라이언트가 입력한 금전 양
        me->trade.up_money(request.parameter.money);
        break;
    }

    case trade::state::CANCEL: // 취소한 경우
    {
        me->trade.cancel();
        break;
    }

    case trade::state::LOCK:
    {
        me->trade.lock();
        break;
    }
    }

    co_return true;
}

async::task<bool> context::handle_world(fb::socket<character>& socket, const fb_reqs::map_world& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto& world  = this->model.world[request.value];
    auto& before = world[request.before];
    auto& after  = world[request.after];

    if (ch->map() == this->maps[after.map])
    {
        ch->update_map();
        ch->update_external(true);
    }
    else
    {
        std::ignore = co_await ch->map(this->maps[after.map], after.position);
    }
    co_return true;
}

async::task<bool> context::handle_object_miss(fb::socket<character>& socket, const fb_reqs::miss& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return true;

    auto obj = map->objects[request.oid];
    if (obj == nullptr)
        co_return true;

    obj->update_external(*ch, false);
    fb::logger::info("Object miss for {}", obj->oid());

    co_return true;
}

async::task<bool> context::handle_give_item(fb::socket<character>& socket, const fb_reqs::give_item& request)
{
    auto me = socket.data();
    if (me->inited() == false)
        co_return true;

    if (me->map() == nullptr)
        co_return true;

    auto forward = me->forward();
    if (forward == nullptr)
        co_return true;

    auto item = me->items[request.slot - 1];
    if (item == nullptr)
        co_return true;

    auto  count = request.all ? item->count() : 1;
    auto& model = item->based<fb::model::item>();

    try
    {
        if (model.trade == false)
            throw std::runtime_error("줄 수 없습니다.");

        switch (forward->what())
        {
        case OBJECT_TYPE::CHARACTER:
        {
            auto you = std::static_pointer_cast<fb::game::character>(forward);
            if (model.attr(ITEM_ATTRIBUTE::BUNDLE) && you->items.index(model) != 0xFF)
            {
                auto exists = you->items.find(model);
                count       = std::min(model.capacity - exists->count(), count);
                if (count == 0)
                    throw std::runtime_error("상대방의 인벤토리가 가득 찼습니다.");
            }
            else
            {
                if (you->items.free() == false)
                    throw std::runtime_error("상대방의 인벤토리가 가득 찼습니다.");
            }

            item = me->items.remove(item, count, ITEM_DELETE_TYPE::GIVE);
            if (count == 1)
                you->message(std::format("{}님이 {} 주었습니다.", me->name(), name_with(item->name())));
            else
                you->message(std::format("{}님이 {} {}개 주었습니다.", me->name(), name_with(item->name()), count));
            you->items.add(item);
        }
        break;

        case OBJECT_TYPE::MOB:
        {
            auto mob = std::static_pointer_cast<fb::game::mob>(forward);
            if (mob->items().size() >= CONTAINER_CAPACITY)
                throw std::runtime_error("더 이상 줄 수 없습니다.");

            item = me->items.remove(item, count, ITEM_DELETE_TYPE::GIVE);
            mob->push_item(*item);
        }
        break;

        default:
            co_return true;
        }
    }
    catch (std::exception& e)
    {
        me->message(e.what());
    }

    co_return true;
}

async::task<bool> context::handle_give_money(fb::socket<character>& socket, const fb_reqs::give_money& request)
{
    auto me = socket.data();
    if (me->inited() == false)
        co_return true;

    if (me->map() == nullptr)
        co_return true;

    auto forward = me->forward();
    if (forward == nullptr)
        co_return true;

    try
    {
        auto money = std::min(request.money, me->money());
        switch (forward->what())
        {
        case OBJECT_TYPE::CHARACTER:
        {
            auto you      = std::static_pointer_cast<fb::game::character>(forward);
            auto capacity = 0xFFFFFFFF - you->money();
            money         = std::min(capacity, money);
            if (money == 0)
                throw std::runtime_error("상대방이 돈을 받을 수 없습니다.");

            you->money_add(money);
            you->message(std::format("{}님이 {}전을 주었습니다.", me->name(), money));
        }
        break;

        case OBJECT_TYPE::MOB:
        {
            auto mob = std::static_pointer_cast<fb::game::mob>(forward);
            if (mob->items().size() >= CONTAINER_CAPACITY)
                throw std::runtime_error("더 이상 줄 수 없습니다.");

            auto item = this->make<fb::game::cash>(money);
            mob->push_item(*item);
        }
        break;

        default:
            co_return true;
        }

        me->money_reduce(money);
    }
    catch (std::exception& e)
    {
        me->message(e.what());
    }
    co_return true;
}

async::task<bool> context::handle_post(fb::socket<character>& socket, const fb_reqs::post& request)
{
    co_return true;
}

async::task<bool> context::handle_group(fb::socket<character>& socket, const fb_reqs::group& request)
{
    auto me = socket.data();
    if (me->inited() == false)
        co_return true;

    std::ignore = co_await this->create_group(*me, request.name);
    co_return true;
}

async::task<bool> context::handle_user_list(fb::socket<character>& socket, const fb_reqs::user_list& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    ch->show_user_list();
    co_return true;
}

async::task<bool> context::handle_chat(fb::socket<character>& socket, const fb_reqs::chat& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto weak = ch->weak_from_this_as<character>();
    auto map  = ch->map();
    if (map == nullptr)
        co_return true;

    if (ch->role() == ROLE::USER && ENUM_IN(map->model.option, MAP_OPTION::DISABLE_TALK))
        co_return true;

    auto stop = false;
    auto lua  = fb::lua::new_context();
    if (lua != nullptr)
    {
#if defined DEBUG | defined _DEBUG
        lua->load("scripts/interaction.lua");
        lua->load("scripts/command.lua");
#endif
        lua->func("on_chat");
        lua->pushobject(ch);
        lua->pushstring(request.message);
        lua->pushboolean(request.shout);
        std::ignore = co_await lua->call(3, false);
        if (weak.expired())
        {
            lua->release();
            co_return true;
        }
        co_await ch->thread()->switching();

        stop = lua->toboolean(1);
        lua->release();
    }
    if (stop)
        co_return true;

    auto message = std::string{request.message};
    auto type    = request.shout ? CHAT_TYPE::SHOUT : CHAT_TYPE::NORMAL;
    ch->chat(message, type, true);

    auto npcs = std::vector<std::shared_ptr<fb::game::npc>>();
    if (type == CHAT_TYPE::SHOUT)
    {
        for (auto& [fd, obj] : ch->map()->objects)
        {
            if (obj->is(OBJECT_TYPE::NPC))
            {
                npcs.push_back(std::static_pointer_cast<fb::game::npc>(obj));
            }
        }
    }
    else
    {
        for (auto npc : ch->sight_in(OBJECT_TYPE::NPC))
        {
            npcs.push_back(std::static_pointer_cast<fb::game::npc>(npc));
        }
    }

    this->npc_interaction(*ch, request.message, npcs);

    co_return true;
}

async::task<bool> context::handle_bulletin(fb::socket<character>& socket, const fb_reqs::bulletin& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto weak = ch->weak_from_this();
    switch (request.action)
    {
    case BULLETIN_ACTION::SECTIONS:
    {
        ch->show_bulletin();
    }
    break;

    case BULLETIN_ACTION::ARTICLES:
    {
        auto mail = request.section == 0xFFFF;
        try
        {
            if (mail)
            {
                auto&& resp = co_await this->mail_list(*ch, request.offset, 20);
                co_await this->threads.switching(weak);

                ch->show_mail_box(resp.summary_list, MAIL_BUTTON_ENABLE::NEW);
            }
            else
            {
                auto   section  = request.section;
                auto&& articles = co_await this->bulletin_list(request.section, request.offset);
                co_await this->threads.switching(weak);

                auto& model = this->model.bulletin[section];
                auto  flag  = BULLETIN_BUTTON_ENABLE::UP;
                if (ch->condition(model.condition))
                    flag |= BULLETIN_BUTTON_ENABLE::WRITE;

                ch->show_bulletin(model, articles, flag);
            }
        }
        catch (std::exception& e)
        {
            if (weak.expired() == false)
                ch->show_bulletin_message(e.what(), false, mail);
        }
    }
    break;

    case BULLETIN_ACTION::ARTICLE:
    {
        auto mail = request.section == 0xFFFF;
        try
        {
            if (mail)
            {
                auto&& resp = co_await this->read_mail(*ch, request.article);
                co_await this->threads.switching(weak);
                auto flag = MAIL_BUTTON_ENABLE::NEW;
                ch->show_mail_box(resp.mail, flag);
            }
            else
            {
                auto&& article = co_await this->read_bulletin(request.section, request.article);
                co_await this->threads.switching(weak);

                auto flag = BULLETIN_BUTTON_ENABLE::NONE;
                if (article.next)
                    flag |= BULLETIN_BUTTON_ENABLE::NEXT;

                if (ch->condition(this->model.bulletin[article.section].condition) == false)
                    flag |= BULLETIN_BUTTON_ENABLE::WRITE;

                ch->show_bulletin(article, flag);
            }
        }
        catch (std::exception& e)
        {
            if (weak.expired() == false)
                ch->show_bulletin_message(e.what(), false, mail);
        }
    }
    break;

    case BULLETIN_ACTION::WRITE:
    {
        try
        {
            co_await this->write_bulletin(*ch, request.section, request.title, request.contents);
            co_await this->threads.switching(weak);

            ch->show_bulletin_message(_TEXT(MESSAGE_BULLETIN_WRITE), true, false);
        }
        catch (std::exception& e)
        {
            if (weak.expired() == false)
                ch->show_bulletin_message(e.what(), false, false);
        }
    }
    break;

    case BULLETIN_ACTION::DELETE:
    {
        auto mail = request.section == 0xFFFF;
        try
        {
            if (mail)
            {
                auto&& resp = co_await this->delete_mail(*ch, request.article);
                co_await this->threads.switching(weak);
                ch->show_bulletin_message(_TEXT(MESSAGE_BULLETIN_SUCCESS_DELETE), true, true);
            }
            else
            {
                co_await this->delete_bulletin(*ch, request.section, request.article);
                co_await this->threads.switching(weak);

                ch->show_bulletin_message(_TEXT(MESSAGE_BULLETIN_SUCCESS_DELETE), true, false);
            }
        }
        catch (std::exception& e)
        {
            if (weak.expired() == false)
                ch->show_bulletin_message(e.what(), false, mail);
        }
    }
    break;

    case BULLETIN_ACTION::MAIL:
    {
        try
        {
            auto&& resp = co_await this->mail_list(*ch, 0xFFFF, 20); // TODO: 20 -> const
            co_await this->threads.switching(weak);

            this->assert_mail(resp.error);
            ch->show_mail_box(resp.summary_list, MAIL_BUTTON_ENABLE::NEW);
        }
        catch (std::exception& e)
        {
            if (weak.expired() == false)
                ch->show_bulletin_message(e.what(), false, true);
        }
    }
    break;

    case BULLETIN_ACTION::SEND_MAIL:
    {
        try
        {
            auto&& resp = co_await this->send_mail(*ch, request.user, request.title, request.contents);
            co_await this->threads.switching(weak);
            ch->show_bulletin_message("우편을 보냈습니다.", true, true);
        }
        catch (std::exception& e)
        {
            if (weak.expired() == false)
                ch->show_bulletin_message(e.what(), false, true);
        }
    }
    break;

    default:
        co_return false;
    }

    co_return true;
}

async::task<bool> context::handle_swap(fb::socket<character>& socket, const fb_reqs::swap& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    switch (request.type)
    {
    case SWAP_TYPE::SPELL:
    {
        ch->spells.swap(request.src - 1, request.dst - 1);
        break;
    }

    case SWAP_TYPE::ITEM:
    {
        ch->items.swap(request.src - 1, request.dst - 1);
        break;
    }

    default:
        co_return false;
    }

    co_return true;
}

async::task<bool> context::handle_dialog(fb::socket<character>& socket, const fb_reqs::dialog& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    if (ch->dialog == nullptr)
        co_return true;

    auto lua   = ch->dialog;
    ch->dialog = nullptr;
    switch (request.interaction)
    {
    case dialog::interaction::NORMAL: // 일반 다이얼로그
    {
        lua->pushinteger(request.action);
        lua->resume(1);
        break;
    }

    case dialog::interaction::INPUT:
    {
        lua->pushstring(request.message);
        lua->resume(1);
        break;
    }

    case dialog::interaction::INPUT_EX:
    {
        if (request.action == 0x02) // OK button
            lua->pushstring(request.message);
        else
            lua->pushinteger(request.action);

        lua->resume(1);
        break;
    }

    case dialog::interaction::MENU:
    {
        lua->pushinteger(request.index);
        lua->resume(1);
        break;
    }

    case dialog::interaction::LIST:
    {
        if (request.button == DIALOG_RESULT::NEXT)
            lua->pushinteger(request.index);
        else
            lua->pushnil();

        lua->pushinteger(static_cast<uint32_t>(request.button));
        lua->resume(2);
        break;
    }

    case dialog::interaction::SLOT:
    {
        lua->pushinteger(request.index);
        lua->resume(1);
        break;
    }

    case dialog::interaction::ITEM:
    {
        lua->pushstring(request.name);
        lua->resume(1);
        break;
    }

    default:
    {
        break;
    }
    }

    co_return true;
}

async::task<bool> context::handle_throw_item(fb::socket<character>& socket, const fb_reqs::item_throws& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    ch->items.throws(request.index);
    co_return true;
}

async::task<bool> context::handle_spell(fb::socket<character>& socket, const fb_reqs::spell_cast& request)
{
    if (request.slot == 0)
        co_return false;

    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return true;

    if (ch->role() == ROLE::USER && ENUM_IN(map->model.option, MAP_OPTION::DISABLE_SPELL))
    {
        ch->message("마력이 미치지 않습니다.");
        co_return true;
    }

    if (request.slot > CONTAINER_CAPACITY - 1)
        co_return false;

    auto spell = ch->spells[request.slot - 1];
    if (spell == nullptr)
        co_return false;

    auto delay = spell->delay();
    if (delay > 0)
    {
        ch->message(std::format("{}초 후에 사용할 수 있습니다.", delay));
        co_return true;
    }

    const_cast<fb_reqs::spell_cast&>(request).parse(spell->model.type);
    switch (spell->model.type)
    {
    case SPELL_TYPE::INPUT:
        ch->active(*spell, request.message);
        break;

    case SPELL_TYPE::TARGET:
        ch->active(*spell, request.oid);
        break;

    case SPELL_TYPE::NORMAL:
        ch->active(*spell);
        break;
    }

    co_return true;
}

async::task<bool> context::handle_door(fb::socket<character>& socket, const fb_reqs::door& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto lua = fb::lua::new_context();
    if (lua == nullptr)
        co_return true;

#if defined DEBUG | defined _DEBUG
    lua->load("scripts/interaction.lua");
#endif

    lua->func("on_door");
    lua->pushobject(ch);
    std::ignore = co_await lua->call(1);
    co_return true;
}

async::task<bool> context::handle_whisper(fb::socket<character>& socket, const fb_reqs::whisper& request)
{
    auto me = socket.data();
    if (me->inited() == false)
        co_return true;

    auto weak = me->weak_from_this();
    auto map  = me->map();
    if (map == nullptr)
        co_return true;

    if (me->role() == ROLE::USER && ENUM_IN(map->model.option, MAP_OPTION::DISABLE_WHISPER))
    {
        me->message("귓속말을 할 수 없는 지역입니다.");
        co_return true;
    }

    try
    {
        co_await this->whisper(*me, request.name, request.message);
        co_await this->threads.switching(weak);
    }
    catch (std::exception& e)
    {
        if (weak.expired() == false)
            me->message(e.what(), MESSAGE_TYPE::NOTIFY);
    }
    co_return true;
}