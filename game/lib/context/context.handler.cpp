#include "context.h"

using namespace fb::game;
using namespace std::chrono_literals;

async::task<bool> context::handle_login(fb::socket<character>& socket, const fb_reqs::login& request)
{
    auto ch = socket.data();
    if (ch->inited())
        co_return false;

    // Set crypt data
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
    if (this->sockets.contains(fd) == false)
        co_return false;

    auto&& login_resp = co_await this->post<internal_reqs::Login, internal_resp::Login>(
        "internal",
        "/in-game/login",
        internal_reqs::Login{id, name, fb::config<uint8_t>("id")});
    if (login_resp.error != (uint32_t)ERROR_CODE::NONE)
        co_return false;

    auto&& response = co_await this->get<internal_resp::Init>("internal", std::format("/user/init/{}", id));
    if (this->sockets.contains(fd) == false)
        co_return false;

    this->_shard[name]->characters.lock([&name, ch](auto& characters) {
        if (characters.contains(name) == false)
            characters.insert({name, ch});
    });

    if (co_await this->init_ch(response.character, *ch, transfer) == false)
        co_return false;
    auto thread = ch->thread();
    thread->assert_ptr(ch);

    this->init_items(response.items, *ch);
    this->init_spells(response.spells, *ch);
    this->init_traces(response.traces, *ch);
    this->_shard[name]->characters.lock([&name, ch](auto& characters) {
        characters.insert({name, ch});
    });

    this->init_option(response.option, *ch);
    this->send(*ch, fb_resp::init(), scope::SELF);
    this->send(*ch, fb_resp::time(this->_time.hours()), scope::SELF);
    this->send(*ch, fb_resp::character::state(*ch, STATE_LEVEL::LEVEL_MIN), scope::SELF);
    if (from == internal::Service::Login)
    {
        auto msg = this->elapsed_message(response.character.updated_date);
        if (msg.empty() == false)
            ch->message(msg, MESSAGE_TYPE::STATE);
    }

    this->send(*ch, fb_resp::character::state(*ch, STATE_LEVEL::LEVEL_MAX), scope::SELF);
    this->send(*ch, fb_resp::character::option(*ch), scope::SELF);
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

    // TODO: 실제로 이동하지 않고 이동했을때의 위치를 구해서
    // 해당 위치에서 워프가 가능한지 확인하고
    // 워프가능하면 워프처리, 그렇지 않다면 해당 위치로 이동한다.
    auto forward = ch->position_forward(request.direction);

    // 워프 위치라면 워프한다.
    const auto warp = map->warpable(forward);
    if (warp != nullptr)
    {
        if (ch->condition(warp->condition) == false)
        {
            // 메시지 보냄
            ch->message("감히 접근할 수 없습니다.");
            this->send(*ch, fb_resp::character::position(*ch), scope::SELF);
            co_return true;
        }

        switch (warp->dest.header)
        {
        case DSL::map:
        {
            auto  params = dsl::map(warp->dest.params);
            auto& map    = this->maps[params.id];
            std::ignore  = co_await ch->map(&map, point16_t(params.x, params.y));
        }
        break;

        case DSL::world:
        {
            auto  params = dsl::world(warp->dest.params);
            auto& world  = this->model.world[params.id][params.index];
            ch->send(fb_resp::map::worlds(this->model, params.id, params.index));
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
        this->send(*ch, fb_resp::map::update(*map, request.begin, request.size), scope::SELF);

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

async::task<bool> context::handle_update_map(fb::socket<character>& socket, const fb_reqs::map::update& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto map = ch->map();
    if (map == nullptr)
        co_return true;

    this->send(*ch, fb_resp::map::update(*map, request.position, request.size), scope::SELF);
    co_return true;
}

async::task<bool> context::handle_refresh(fb::socket<character>& socket, const fb_reqs::refresh& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    this->send(*ch, fb_resp::character::position(*ch), scope::SELF);
    co_return true;
}

async::task<bool> context::handle_active_item(fb::socket<character>& socket, const fb_reqs::item::active& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    ch->items.active(request.index);
    co_return true;
}

async::task<bool> context::handle_inactive_item(fb::socket<character>& socket, const fb_reqs::item::inactive& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    ch->items.inactive(request.parts);
    co_return true;
}

async::task<bool> context::handle_drop_item(fb::socket<character>& socket, const fb_reqs::item::drop& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    ch->items.drop(request.index, request.all ? -1 : 1);
    co_return true;
}

async::task<bool> context::handle_drop_cash(fb::socket<character>& socket, const fb_reqs::item::drop_cash& request)
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

    this->send(*ch, fb_resp::character::internal_info(*ch, this->model), scope::SELF);

    for (auto& [id, buff] : ch->buffs)
        this->send(*ch, fb_resp::spell::buff(*buff), scope::SELF);
    co_return true;
}

async::task<bool> context::handle_option_changed(fb::socket<character>& socket, const fb_reqs::change_option& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto option = SETTING(request.option);
    switch (option)
    {
    case SETTING::EXTENSION:
        if (request.ride)
        {
            if (ch->state() == STATE::RIDING)
                ch->unride();
            else
                ch->ride();
        }
        break;

    default:
        auto enabled = ch->option_toggle(option);
        if (option == SETTING::GROUP && !enabled)
        {
            auto&& response = co_await this->post<internal_reqs::LeaveGroup, internal_resp::LeaveGroup>(
                "internal",
                "/in-game/group/leave",
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

    if (request.fd == 1 && ch->dialog.active())
    {
        ch->dialog.pushnil().resume(1);
    }
    else
    {
        auto map = ch->map();
        auto you = map->objects[request.fd];
        if (you == nullptr)
            co_return true;

        switch (you->what())
        {
        case OBJECT_TYPE::CHARACTER:
            this->send(*ch, fb_resp::character::external_info(static_cast<character&>(*you), this->model), scope::SELF);
            break;

        case OBJECT_TYPE::MOB:
            this->handle_click_mob(*ch, static_cast<mob&>(*you));
            break;

        case OBJECT_TYPE::NPC:
            this->handle_click_npc(*ch, static_cast<npc&>(*you));
            break;
        }
    }

    co_return true;
}

// TODO : on_item_detail
async::task<bool> context::handle_item_info(fb::socket<character>& socket, const fb_reqs::item::info& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto item = ch->items[request.slot];
    if (item == nullptr)
        co_return false;

    this->send(*ch, fb_resp::item::tip(request.position, item->tip_message()), scope::SELF);
    co_return true;
}

async::task<bool> context::handle_itemmix(fb::socket<character>& socket, const fb_reqs::item::mix& request)
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

    if (listener != nullptr)
    {
        auto& message = success ? message::mix::SUCCESS : message::mix::FAILED;
        listener->on_notify(*ch, message);
    }

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
        auto item = me->items[request.parameter.index - 1];
        if (item == nullptr)
            break;

        me->trade.up(*item);
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
        me->trade.up(request.parameter.money);
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

async::task<bool> context::handle_world(fb::socket<character>& socket, const fb_reqs::map::world& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto& world  = this->model.world[request.value];
    auto& before = world[request.before];
    auto& after  = world[request.after];

    if (ch->map() == &this->maps[after.map])
    {
        ch->refresh_map();
    }
    else
    {
        std::ignore = co_await ch->map(&this->maps[after.map], after.position);
        co_await this->save(*ch);
    }
    co_return true;
}

async::task<bool> context::handle_group(fb::socket<character>& socket, const fb_reqs::group& request)
{
    auto me = socket.data();
    if (me->inited() == false)
        co_return true;

    auto error = std::string();

    try
    {
        if (me->option(SETTING::GROUP) == false)
            throw std::runtime_error(message::group::DISABLED_MINE);

        auto&& response = co_await this->post<internal_reqs::EnterGroup, internal_resp::EnterGroup>(
            "internal",
            "/in-game/group/create",
            internal_reqs::EnterGroup{me->id(), request.name});

        this->on_enter_group(response);
    }
    catch (std::exception& e)
    {
        error = e.what();
    }

    if (error.empty() == false)
        this->send(*me, fb_resp::message(error, MESSAGE_TYPE::STATE), scope::SELF);

    co_return true;
}

async::task<bool> context::handle_user_list(fb::socket<character>& socket, const fb_reqs::user_list& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    this->send(*ch, fb_resp::user_list(*ch, this->sockets), scope::SELF);
    co_return true;
}

async::task<bool> context::handle_chat(fb::socket<character>& socket, const fb_reqs::chat& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    if (ch->admin() && co_await handle_command(*ch, request.message))
        co_return true;

    ch->chat(request.message, request.shout ? CHAT_TYPE::SHOUT : CHAT_TYPE::NORMAL);

    auto npcs = std::vector<npc*>();
    if (request.shout)
    {
        for (auto& [fd, obj] : ch->map()->objects)
        {
            if (obj.is(OBJECT_TYPE::NPC))
                npcs.push_back(static_cast<npc*>(&obj));
        }
    }
    else
    {
        for (auto npc : ch->showings(OBJECT_TYPE::NPC))
        {
            npcs.push_back(static_cast<fb::game::npc*>(npc));
        }
    }

    ch->inline_interaction(request.message, npcs);

    co_return true;
}

async::task<bool> context::handle_board(fb::socket<character>& socket, const fb_reqs::board::board& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto fd = ch->fd();
    switch (request.action)
    {
    case BOARD_ACTION::SECTIONS:
    {
        this->send(*ch, fb_resp::board::sections(this->model), scope::SELF);
    }
    break;

    case BOARD_ACTION::ARTICLES:
    {
        auto error = std::string();
        try
        {
            if (this->model.board.contains(request.section) == false)
                throw std::runtime_error(message::board::SECTION_NOT_EXIST);

            auto section = &this->model.board[request.section];
            auto offset  = request.offset;

            auto&& response = co_await this->get<internal_resp::GetArticleList>(
                "internal",
                std::format("/board/{}&offset={}", section->id, offset));
            if (this->sockets.contains(fd) == false)
                co_return false;

            auto articles = std::list<board::article>();
            for (auto& summary : response.summary_list)
            {
                auto dt = datetime(summary.created_date);
                articles.push_back(board::article{summary.id,
                                                  section->id,
                                                  summary.user,
                                                  summary.user_name,
                                                  summary.title,
                                                  (uint8_t)dt.month(),
                                                  (uint8_t)dt.day()});
            }

            auto button_flags = BOARD_BUTTON_ENABLE::UP;
            if (ch->condition(section->condition) == false)
                button_flags |= BOARD_BUTTON_ENABLE::WRITE;
            this->send(*ch, fb_resp::board::articles(*section, articles, button_flags), scope::SELF);
        }
        catch (std::exception& e)
        {
            error = e.what();
        }

        if (error.empty() == false)
            this->send(*ch, fb_resp::board::message(error, false, false), scope::SELF);
    }
    break;

    case BOARD_ACTION::ARTICLE:
    {
        auto error = std::string();
        try
        {
            if (this->model.board.contains(request.section) == false)
                throw std::runtime_error(message::board::SECTION_NOT_EXIST);

            auto   section  = &this->model.board[request.section]; // 코루틴땜시 포인터로
            auto&& response = co_await this->get<internal_resp::GetArticle>(
                "internal",
                std::format("/board/{}/{}", section->id, request.article));
            if (this->sockets.contains(fd) == false)
                co_return false;

            if (response.success == false)
            {
                this->send(*ch, fb_resp::board::message(message::board::ARTICLE_NOT_EXIST, false, false), scope::SELF);
                co_return true;
            }

            auto dt           = datetime(response.article.created_date);
            auto button_flags = BOARD_BUTTON_ENABLE::NONE;
            if (response.next)
                button_flags |= BOARD_BUTTON_ENABLE::NEXT;

            if (ch->condition(section->condition) == false)
                button_flags |= BOARD_BUTTON_ENABLE::WRITE;

            auto article = board::article{response.article.id,
                                          section->id,
                                          response.article.user,
                                          response.article.user_name,
                                          response.article.title,
                                          (uint8_t)dt.month(),
                                          (uint8_t)dt.day(),
                                          response.article.contents};
            this->send(*ch, fb_resp::board::article(article, button_flags), scope::SELF);
        }
        catch (std::exception& e)
        {
            error = e.what();
        }

        if (error.empty() == false)
            this->send(*ch, fb_resp::board::message(error, false, false), scope::SELF);
    }
    break;

    case BOARD_ACTION::WRITE:
    {
        auto error = std::string();
        try
        {
            if (this->model.board.contains(request.section) == false)
                throw std::runtime_error(message::board::SECTION_NOT_EXIST);

            auto section = &this->model.board[request.section];
            if (ch->condition(section->condition) == false)
                throw std::runtime_error(message::board::NOT_AUTH);

            if (request.title.length() > 64)
                throw std::runtime_error(message::board::TOO_LONG_TITLE);

            if (request.contents.length() > 256)
                throw std::runtime_error(message::board::TOO_LONG_CONTENTS);

            auto&& response = co_await this->post<internal_reqs::WriteArticle, internal_resp::WriteArticle>(
                "internal",
                "/board/write",
                internal_reqs::WriteArticle{section->id, ch->id(), request.title, request.contents});

            if (this->sockets.contains(fd) == false)
                co_return false;

            if (response.success == false)
                throw std::runtime_error("게시글 작성 실패");

            this->send(*ch, fb_resp::board::message(message::board::WRITE, true, true), scope::SELF);
        }
        catch (std::exception& e)
        {
            error = e.what();
        }
        this->send(*ch, fb_resp::board::message(error, false, false), scope::SELF);
    }
    break;

    case BOARD_ACTION::DELETE:
    {
        auto error = std::string();
        try
        {
            if (this->model.board.contains(request.section) == false)
                throw std::runtime_error(message::board::SECTION_NOT_EXIST);

            auto section = &this->model.board[request.section];
            if (ch->condition(section->condition) == false)
                throw std::runtime_error(message::board::NOT_AUTH);

            auto&& response = co_await this->post<internal_reqs::DeleteArticle, internal_resp::DeleteArticle>(
                "internal",
                "/board/delete",
                internal_reqs::DeleteArticle{request.article, ch->id()});

            if (this->sockets.contains(fd) == false)
                co_return false;

            switch (response.result)
            {
            case -1: // article not found
                throw std::runtime_error(message::board::ARTICLE_NOT_EXIST);

            case -2: // article deleted
                throw std::runtime_error(message::board::ARTICLE_NOT_EXIST);

            case -3: // no authenticate
                throw std::runtime_error(message::board::NOT_AUTH);
            }

            this->send(*ch, fb_resp::board::message(message::board::SUCCESS_DELETE, true, false), scope::SELF);
        }
        catch (std::exception& e)
        {
            error = e.what();
        }

        if (error.empty() == false)
            this->send(*ch, fb_resp::board::message(error, false, false), scope::SELF);
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

async::task<bool> context::handle_throw_item(fb::socket<character>& socket, const fb_reqs::item::throws& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    ch->items.throws(request.index);
    co_return true;
}

async::task<bool> context::handle_spell(fb::socket<character>& socket, const fb_reqs::spell::use& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    if (request.slot > CONTAINER_CAPACITY - 1)
        co_return false;

    auto model = ch->spells[request.slot];
    if (model == nullptr)
        co_return false;

    request.parse(model->type);
    switch (model->type)
    {
    case SPELL_TYPE::INPUT:
        ch->active(*model, request.message);
        break;

    case SPELL_TYPE::TARGET:
        ch->active(*model, request.fd);
        break;

    case SPELL_TYPE::NORMAL:
        ch->active(*model);
        break;
    }

    co_return true;
}

async::task<bool> context::handle_door(fb::socket<character>& socket, const fb_reqs::door& request)
{
    auto ch = socket.data();
    if (ch->inited() == false)
        co_return true;

    auto thread = lua::get();
    if (thread == nullptr)
        co_return true;

    thread->from("scripts/common/door.lua").func("on_door").pushobject(ch).resume(1);
    co_return true;
}

async::task<bool> context::handle_whisper(fb::socket<character>& socket, const fb_reqs::whisper& request)
{
    auto me = socket.data();
    if (me->inited() == false)
        co_return true;

    auto  fd      = me->fd();
    auto& from    = me->name();
    auto  to      = std::string(request.name);
    auto  message = std::string(request.message);
    auto  error   = std::string();
    try
    {
        if (me->option(SETTING::WHISPER) == false)
            throw std::runtime_error("당신은 귓속말 거부 상태입니다.");

        me->message(std::format("{}< {}", to, message), MESSAGE_TYPE::NOTIFY);
        this->broadcast(
            to,
            [this, from, to = me->id(), message](auto& you) {
                auto response    = internal_resp::Whisper{};
                response.from    = from;
                response.to      = to;
                response.message = message;
                response.host    = fb::config<uint32_t>("id");
                if (you.option(SETTING::WHISPER))
                    response.error = static_cast<uint32_t>(ERROR_CODE::NONE);
                else
                    response.error = static_cast<uint32_t>(ERROR_CODE::DISABLED_WHISPER_TARGET);

                this->assert_whisper(response);
                you.message(std::format("{}> {}", from, message), MESSAGE_TYPE::NOTIFY);
            },
            [this, from, message, fd](const auto& to) {
                async::awaitable_then(this->post<internal_reqs::Whisper, internal_resp::Whisper>(
                                          "internal",
                                          "/in-game/whisper",
                                          internal_reqs::Whisper{from, to, message}),
                                      [this, fd](auto result) {
                                          if (this->sockets.contains(fd) == false)
                                              return;

                                          try
                                          {
                                              auto&& resp = result();
                                              this->assert_whisper(resp);
                                          }
                                          catch (std::exception& e)
                                          { }
                                      });
            });
    }
    catch (std::exception& e)
    {
        if (this->sockets.contains(fd) == false)
            co_return false;

        error = e.what();
    }
    if (error.empty() == false)
        me->message(error, MESSAGE_TYPE::NOTIFY);
    co_return true;
}