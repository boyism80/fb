#include <fb/game/context.h>

using namespace fb::game;
using namespace std::chrono_literals;

async::task<bool> context::handle_login(fb::socket<character>& socket, const fb_reqs::login& request)
{
    auto ch = socket.data();
    if (ch->inited())
        co_return false;

    socket.crt(request.enc_type, request.enc_key);

    ch->name(request.name);
    fb::logger::info("{}님이 접속했습니다.", request.name);

    auto fd       = socket.fd();
    auto id       = request.id;
    auto name     = std::string(request.name);
    auto from     = request.from;
    auto transfer = request.transfer;
    auto delay    = fb::config<uint32_t>("delay");
    co_await this->sleep(std::chrono::seconds(delay));
    co_await this->update_thread(*ch);

    auto&& login_resp = co_await this->post<internal_reqs::Login, internal_resp::Login>(
        "internal",
        "/in-game/login",
        internal_reqs::Login{id, name, fb::config<uint8_t>("id")});
    if (login_resp.error != (uint32_t)ERROR_CODE::NONE)
        co_return false;

    auto&& response = co_await this->get<internal_resp::Init>("internal", std::format("/user/init/{}", id));
    co_await this->update_thread(*ch);

    if (co_await this->init_ch(response.character, *ch, response.group, response.clan, transfer) == false)
        co_return false;

    this->update_thread(*ch);
    ch->unread_mail(response.mail);

    this->init_items(response.items, *ch);
    this->init_spells(response.spells, *ch);
    this->init_traces(response.traces, *ch);
    this->_shard[name]->names.lock([&name, ch](auto& names) {
        names.insert({name, ch});
    });
    this->_shard[id]->ids.lock([id, ch](auto& ids) {
        ids.insert({id, ch});
    });

    this->init_option(response.option, *ch);
    ch->init();
    ch->update_time(this->_time.hours());
    if (from == internal::Service::Login)
    {
        auto msg = this->elapsed_message(response.character.updated_date);
        if (msg.empty() == false)
            ch->message(msg, MESSAGE_TYPE::STATE);
    }

    ch->update(STATE_LEVEL::LEVEL_MAX);
    ch->update_option();
    ch->init(true);
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
            auto  params = fb::model::dsl::map(warp->dest.params);
            auto& map    = this->maps[params.id];
            std::ignore  = co_await ch->map(&map, fb::model::point16_t(params.x, params.y));
        }
        break;

        case DSL::world:
        {
            auto  params = fb::model::dsl::world(warp->dest.params);
            auto& world  = this->model.world[params.id][params.index];
            co_await ch->map(nullptr);
            co_await this->update_thread(*ch);
            ch->show_world_map(params.id, params.index);
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

    ch->attack();
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

async::task<bool> context::handle_drop_cash(fb::socket<character>& socket, const fb_reqs::item_drop_cash& request)
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
        auto message = object->is(OBJECT_TYPE::ITEM) ? static_cast<item*>(*i)->inven_name() : (*i)->name();

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
            auto&& response = co_await this->post<internal_reqs::LeaveGroup, internal_resp::LeaveGroup>(
                "internal",
                "/group/leave",
                internal_reqs::LeaveGroup{ch->name()});

            this->on_leave_group(response);
        }

        auto&& response = co_await this->post<internal_reqs::SetOption, internal_resp::SetOption>(
            "internal",
            "/user/option",
            internal_reqs::SetOption{ch->id(), static_cast<uint8_t>(option), enabled});

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
        throw std::runtime_error("no match exception");

    if (found->success.size() - found->source.size() > ch->items.free_size())
        throw std::runtime_error("full inven exception");

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
            auto deleted = ch->items.remove(*item, count);
            if (deleted != nullptr)
                co_await deleted->destroy();

            deleted_count += count;
        }
    }

    auto  listener = ch->get_listener<character>();
    auto  success  = (std::rand() % 100) < found->percent;
    auto& result   = success ? found->success : found->failed;
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

    if (ch->map() == &this->maps[after.map])
    {
        ch->update_map();
    }
    else
    {
        std::ignore = co_await ch->map(&this->maps[after.map], after.position);
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

    auto obj = map->objects[request.sequence];
    if (obj == nullptr)
        co_return true;

    fb::logger::info("{} 오브젝트 미스", obj->sequence());

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

    auto map = ch->map();
    if (map == nullptr)
        co_return true;

    if (ch->admin() == false && ENUM_IN(map->model.option, MAP_OPTION::DISABLE_TALK))
        co_return true;

#if defined DEBUG | defined _DEBUG
    fb::lua::load("scripts/interaction.lua");
    fb::lua::load("scripts/command.lua");
#endif

    auto lua = fb::lua::new_context();
    lua->func("on_chat");
    lua->pushobject(ch);
    lua->pushstring(request.message);
    lua->pushboolean(request.shout);
    lua->resume(3, false);
    switch (lua->state())
    {
    case LUA_OK:
        auto stop = lua->toboolean(1);
        lua->release();
        if (stop)
            co_return true;
    }

    auto message = std::string{request.message};
    auto type    = request.shout ? CHAT_TYPE::SHOUT : CHAT_TYPE::NORMAL;
    switch (type)
    {
    case CHAT_TYPE::NORMAL:
        message = std::format("{}: {}", ch->name(), message);
        break;

    case CHAT_TYPE::SHOUT:
        message = std::format("{}! {}", ch->name(), message);
        break;
    }

    ch->chat(message, type);

    auto npcs = std::vector<npc*>();
    if (type == CHAT_TYPE::SHOUT)
    {
        for (auto& [fd, obj] : ch->map()->objects)
        {
            if (obj.is(OBJECT_TYPE::NPC))
                npcs.push_back(static_cast<npc*>(&obj));
        }
    }
    else
    {
        for (auto npc : ch->sight_in(OBJECT_TYPE::NPC))
        {
            npcs.push_back(static_cast<fb::game::npc*>(npc));
        }
    }

    this->npc_interaction(*ch, request.message, npcs);

    co_return true;
}

async::task<bool> context::handle_board(fb::socket<character>& socket, const fb_reqs::board& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto fd = ch->fd();
    switch (request.action)
    {
    case BOARD_ACTION::SECTIONS:
    {
        ch->show_board();
    }
    break;

    case BOARD_ACTION::ARTICLES:
    {
        auto mail = request.section == 0xFFFF;
        try
        {
            if (mail)
            {
                auto&& resp = co_await this->mail_list(*ch, request.offset, 20);
                co_await this->update_thread(*ch);

                ch->show_mail_box(resp.summary_list, MAIL_BUTTON_ENABLE::NEW);
            }
            else
            {
                auto   section  = request.section;
                auto&& articles = co_await this->board_list(request.section, request.offset);
                co_await this->update_thread(*ch);

                auto& model = this->model.board[section];
                auto  flag  = BOARD_BUTTON_ENABLE::UP;
                if (ch->condition(model.condition))
                    flag |= BOARD_BUTTON_ENABLE::WRITE;

                ch->show_board(model, articles, flag);
            }
        }
        catch (std::exception& e)
        {
            if (this->alive(*ch))
                ch->show_board_message(e.what(), false, mail);
        }
    }
    break;

    case BOARD_ACTION::ARTICLE:
    {
        auto mail = request.section == 0xFFFF;
        try
        {
            if (mail)
            {
                auto&& resp = co_await this->read_mail(*ch, request.article);
                auto   flag = MAIL_BUTTON_ENABLE::NEW;
                ch->show_mail_box(resp.mail, flag);
            }
            else
            {
                auto&& article = co_await this->read_board(request.section, request.article);
                co_await this->update_thread(*ch);

                auto flag = BOARD_BUTTON_ENABLE::NONE;
                if (article.next)
                    flag |= BOARD_BUTTON_ENABLE::NEXT;

                if (ch->condition(this->model.board[article.section].condition) == false)
                    flag |= BOARD_BUTTON_ENABLE::WRITE;

                ch->show_board(article, flag);
            }
        }
        catch (std::exception& e)
        {
            if (this->alive(*ch))
                ch->show_board_message(e.what(), false, mail);
        }
    }
    break;

    case BOARD_ACTION::WRITE:
    {
        try
        {
            co_await this->write_board(*ch, request.section, request.title, request.contents);
            co_await this->update_thread(*ch);

            ch->show_board_message(_TEXT(MESSAGE_BOARD_WRITE), true, true);
        }
        catch (std::exception& e)
        {
            if (this->alive(*ch))
                ch->show_board_message(e.what(), false, false);
        }
    }
    break;

    case BOARD_ACTION::DELETE:
    {
        auto mail = request.section == 0xFFFF;
        try
        {
            if (mail)
            {
                auto&& resp = co_await this->delete_mail(*ch, request.article);
                ch->show_board_message(_TEXT(MESSAGE_BOARD_SUCCESS_DELETE), true, true);
            }
            else
            {
                co_await this->delete_board(*ch, request.section, request.article);
                co_await this->update_thread(*ch);

                ch->show_board_message(_TEXT(MESSAGE_BOARD_SUCCESS_DELETE), true, false);
            }
        }
        catch (std::exception& e)
        {
            if (this->alive(*ch))
                ch->show_board_message(e.what(), false, mail);
        }
    }
    break;

    case BOARD_ACTION::MAIL:
    {
        try
        {
            auto&& resp = co_await this->mail_list(*ch, 0xFFFF, 20); // TODO: 20 -> const
            co_await this->update_thread(*ch);

            this->assert_mail(resp.error);
            ch->show_mail_box(resp.summary_list, MAIL_BUTTON_ENABLE::NEW);
        }
        catch (std::exception& e)
        {
            if (this->alive(*ch))
                ch->show_board_message(e.what(), false, true);
        }
    }
    break;

    case BOARD_ACTION::SEND_MAIL:
    {
        try
        {
            auto&& resp = co_await this->send_mail(*ch, request.user, request.title, request.contents);
            ch->show_board_message("우편을 보냈습니다.", true, true);
        }
        catch (std::exception& e)
        {
            if (this->alive(*ch))
                ch->show_board_message(e.what(), false, true);
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

    if (ch->dialog.active() == false)
        co_return true;

    switch (request.interaction)
    {
    case dialog::interaction::NORMAL: // 일반 다이얼로그
    {
        ch->dialog.pushinteger(request.action).resume(1);
        break;
    }

    case dialog::interaction::INPUT:
    {
        ch->dialog.pushstring(request.message).resume(1);
        break;
    }

    case dialog::interaction::INPUT_EX:
    {
        if (request.action == 0x02) // OK button
            ch->dialog.pushstring(request.message);
        else
            ch->dialog.pushinteger(request.action);

        ch->dialog.resume(1);
        break;
    }

    case dialog::interaction::MENU:
    {
        ch->dialog.pushinteger(request.index).resume(1);
        break;
    }

    case dialog::interaction::LIST:
    {
        if (request.button == DIALOG_RESULT::NEXT)
            ch->dialog.pushinteger(request.index);
        else
            ch->dialog.pushnil();

        ch->dialog.pushinteger(static_cast<uint32_t>(request.button));
        ch->dialog.resume(2);
        break;
    }

    case dialog::interaction::SLOT:
    {
        ch->dialog.pushinteger(request.index).resume(1);
        break;
    }

    case dialog::interaction::ITEM:
    {
        ch->dialog.pushstring(request.name).resume(1);
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
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return true;

    if (ch->admin() == false && ENUM_IN(map->model.option, MAP_OPTION::DISABLE_SPELL))
    {
        ch->message("마력이 미치지 않습니다.");
        co_return true;
    }

    if (request.slot > CONTAINER_CAPACITY - 1)
        co_return false;

    auto spell = ch->spells[request.slot];
    if (spell == nullptr)
        co_return false;

    if (ch->admin() == false && spell->update_lock() == false)
        co_return true;

    auto delay = spell->delay();
    if (delay > 0)
    {
        ch->message(std::format("{}초 후에 사용할 수 있습니다.", delay));
        co_return true;
    }

#if defined DEBUG | defined _DEBUG
    fb::lua::load("scripts/spell.lua");
#endif

    const_cast<fb_reqs::spell_cast&>(request).parse(spell->model.type);
    switch (spell->model.type)
    {
    case SPELL_TYPE::INPUT:
        ch->active(*spell, request.message);
        break;

    case SPELL_TYPE::TARGET:
        ch->active(*spell, request.fd);
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

    auto thread = lua::new_context();
    if (thread == nullptr)
        co_return true;

#if defined DEBUG | defined _DEBUG
    thread->from("scripts/interaction.lua");
#endif

    thread->func("on_door");
    thread->pushobject(ch);
    thread->resume(1);
    co_return true;
}

async::task<bool> context::handle_whisper(fb::socket<character>& socket, const fb_reqs::whisper& request)
{
    auto me = socket.data();
    if (me->inited() == false)
        co_return true;

    auto map = me->map();
    if (map == nullptr)
        co_return true;

    if (me->admin() == false && ENUM_IN(map->model.option, MAP_OPTION::DISABLE_WHISPER))
    {
        me->message("귓속말을 할 수 없는 지역입니다.");
        co_return true;
    }

    auto fd = me->fd();
    try
    {
        co_await this->whisper(*me, request.name, request.message);
    }
    catch (std::exception& e)
    {
        if (this->alive(*me))
            me->message(e.what(), MESSAGE_TYPE::NOTIFY);
    }
    co_return true;
}