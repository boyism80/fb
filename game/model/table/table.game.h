#ifndef __FB_GAME_MASTER_H__
#define __FB_GAME_MASTER_H__

#include <map>
#include "model/map/map.h"
#include "model/item/item.h"
#include "model/npc/npc.h"
#include "model/spell/spell.h"
#include "model/mob/mob.h"
#include "model/board/board.h"
#include "model/clan/clan.h"
#include "module/config/config.h"
#include "module/string/string.h"
#include "module/table/table.h"
#include "module/encoding/encoding.h"

namespace fb { namespace game {

class table;

namespace container {

class map : private std::map<uint16_t, fb::game::map*>
{
private:
    friend class fb::game::table;

public:
    using std::map<uint16_t, fb::game::map*>::begin;
    using std::map<uint16_t, fb::game::map*>::end;
    using std::map<uint16_t, fb::game::map*>::cbegin;
    using std::map<uint16_t, fb::game::map*>::cend;
    using std::map<uint16_t, fb::game::map*>::size;

private:
    static fb::game::map::effects           to_effect(const std::string& effect);
    static fb::game::map::options           to_option(const Json::Value& data);
    static bool                             load_data(uint16_t id, std::vector<char>& buffer);
    static bool                             load_blocks(uint16_t id, Json::Value& buffer);

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);
    bool                                    load_warps(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    fb::game::map*                          name2map(const std::string& name);

public:
    fb::game::map*                          operator [] (uint16_t id) const;
};

class item : private std::map<uint16_t, fb::game::item::master*>
{
private:
    friend class fb::game::table;

public:
    using std::map<uint16_t, fb::game::item::master*>::begin;
    using std::map<uint16_t, fb::game::item::master*>::end;
    using std::map<uint16_t, fb::game::item::master*>::cbegin;
    using std::map<uint16_t, fb::game::item::master*>::cend;
    using std::map<uint16_t, fb::game::item::master*>::size;
    using std::map<uint16_t, fb::game::item::master*>::operator[];

private:
    static fb::game::item::master*          create(uint32_t id, const Json::Value& data);
    static fb::game::item::item_limit       to_limit(const Json::Value& data);
    static fb::game::item::penalties        to_penalty(const std::string& penalty);

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    fb::game::item::master*                 name2item(const std::string& name);
};

class npc : private std::map<uint16_t, fb::game::npc::master*>
{
private:
    friend class fb::game::table;

public:
    using std::map<uint16_t, fb::game::npc::master*>::begin;
    using std::map<uint16_t, fb::game::npc::master*>::end;
    using std::map<uint16_t, fb::game::npc::master*>::cbegin;
    using std::map<uint16_t, fb::game::npc::master*>::cend;
    using std::map<uint16_t, fb::game::npc::master*>::size;
    using std::map<uint16_t, fb::game::npc::master*>::operator[];

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);
    bool                                    load_spawn(const std::string& path, fb::game::listener* listener, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    fb::game::npc::master*                  name2npc(const std::string& name);
};

class mob : private std::map<uint16_t, fb::game::mob::master*>
{
private:
    friend class fb::game::table;

public:
    using std::map<uint16_t, fb::game::mob::master*>::begin;
    using std::map<uint16_t, fb::game::mob::master*>::end;
    using std::map<uint16_t, fb::game::mob::master*>::cbegin;
    using std::map<uint16_t, fb::game::mob::master*>::cend;
    using std::map<uint16_t, fb::game::mob::master*>::size;
    using std::map<uint16_t, fb::game::mob::master*>::operator[];

private:
    static fb::game::mob::sizes             to_size(const std::string& size);
    static fb::game::mob::offensive_type    to_offensive(const std::string& offensive);

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);
    bool                                    load_drops(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);
    bool                                    load_spawn(const std::string& path, fb::game::listener* listener, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    fb::game::mob::master*                  name2mob(const std::string& name);
};

class spell : private std::map<uint16_t, fb::game::spell*>
{
private:
    friend class fb::game::table;

public:
    using std::map<uint16_t, fb::game::spell*>::begin;
    using std::map<uint16_t, fb::game::spell*>::end;
    using std::map<uint16_t, fb::game::spell*>::cbegin;
    using std::map<uint16_t, fb::game::spell*>::cend;
    using std::map<uint16_t, fb::game::spell*>::size;
    using std::map<uint16_t, fb::game::spell*>::operator[];

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    fb::game::spell*                        name2spell(const std::string& name);
};

class cls : private std::vector<fb::game::class_data*>
{
private:
    friend class fb::game::table;

public:
    using std::vector<fb::game::class_data*>::begin;
    using std::vector<fb::game::class_data*>::end;
    using std::vector<fb::game::class_data*>::cbegin;
    using std::vector<fb::game::class_data*>::cend;
    using std::vector<fb::game::class_data*>::size;
    using std::vector<fb::game::class_data*>::operator[];

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    const std::string*                      class2name(uint8_t cls, uint8_t promotion);
    bool                                    name2class(const std::string& name, uint8_t* class_id, uint8_t* promotion_id);
    uint32_t                                exp(uint8_t class_id, uint8_t level);
};

class mix : private std::vector<fb::game::itemmix*>
{
private:
    friend class fb::game::table;

public:
    using std::vector<fb::game::itemmix*>::begin;
    using std::vector<fb::game::itemmix*>::end;
    using std::vector<fb::game::itemmix*>::cbegin;
    using std::vector<fb::game::itemmix*>::cend;
    using std::vector<fb::game::itemmix*>::size;
    using std::vector<fb::game::itemmix*>::operator[];

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);

public:
    fb::game::itemmix*                      find(const std::vector<fb::game::item*>& items);
};

class door : private std::vector<fb::game::door::master*>
{
private:
    friend class fb::game::table;

public:
    using std::vector<fb::game::door::master*>::begin;
    using std::vector<fb::game::door::master*>::end;
    using std::vector<fb::game::door::master*>::cbegin;
    using std::vector<fb::game::door::master*>::cend;
    using std::vector<fb::game::door::master*>::size;
    using std::vector<fb::game::door::master*>::operator[];

public:
    bool                                    load(const std::string& path, fb::table::handle_callback callback, fb::table::handle_error error, fb::table::handle_complete complete);
};

}

class session;

class table
{
public:
    static std::vector<clan*>               clans;
    static fb::game::board                  board;
    static fb::game::container::map         maps;
    static fb::game::container::item        items;
    static fb::game::container::npc         npcs;
    static fb::game::container::mob         mobs;
    static fb::game::container::spell       spells;
    static fb::game::container::cls         classes;
    static fb::game::container::mix         mixes;
    static fb::game::container::door        doors;

private:
    table();
    ~table();
};

} }

#endif // !__FB_GAME_MASTER_H__
