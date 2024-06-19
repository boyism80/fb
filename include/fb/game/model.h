#ifndef __MODEL_H__
#define __MODEL_H__

#include <string>
#include <map>
#include <vector>
#include <queue>
#include <any>
#include <optional>
#include <chrono>
#include <jsoncpp/json/json.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <unordered_map>
#include <fb/game/lua.h>
#include <fb/game/model.preprocessor.h>

#ifdef ROOT_PREPROCESSOR
ROOT_PREPROCESSOR
#endif

namespace fb { namespace model {

static inline std::function<std::string(const std::string&)> string_encoding_func;

#pragma region base type
class date_range
{
public:
    const std::optional<boost::posix_time::ptime> begin, end;

public:
    date_range(const std::optional<boost::posix_time::ptime>& begin, const std::optional<boost::posix_time::ptime>& end) : 
        begin(begin),
        end(end)
    {}
    ~date_range() = default;
};
#pragma endregion

#pragma region enum
namespace enum_value {

template <typename T>
inline T enum_parse(const std::string k)
{
    throw std::runtime_error("cannot parse enum value");
}

enum class BUNDLE_TYPE
{
    NONE = 0, 
    BUNDLE = 1, 
    PACKAGE = 2
};

template <>
inline BUNDLE_TYPE enum_parse<BUNDLE_TYPE>(const std::string k)
{
    static const std::unordered_map<std::string, BUNDLE_TYPE> enums
    {
        { "NONE", BUNDLE_TYPE::NONE }, 
        { "BUNDLE", BUNDLE_TYPE::BUNDLE }, 
        { "PACKAGE", BUNDLE_TYPE::PACKAGE }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class CLASS
{
    NONE = 0, 
    WARRIOR = 1, 
    THIEF = 2, 
    MAGICION = 3, 
    ASCETIC = 4
};

template <>
inline CLASS enum_parse<CLASS>(const std::string k)
{
    static const std::unordered_map<std::string, CLASS> enums
    {
        { "NONE", CLASS::NONE }, 
        { "WARRIOR", CLASS::WARRIOR }, 
        { "THIEF", CLASS::THIEF }, 
        { "MAGICION", CLASS::MAGICION }, 
        { "ASCETIC", CLASS::ASCETIC }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class DEATH_PENALTY
{
    NONE = 0, 
    DROP = 1, 
    DESTROY = 2
};

template <>
inline DEATH_PENALTY enum_parse<DEATH_PENALTY>(const std::string k)
{
    static const std::unordered_map<std::string, DEATH_PENALTY> enums
    {
        { "NONE", DEATH_PENALTY::NONE }, 
        { "DROP", DEATH_PENALTY::DROP }, 
        { "DESTROY", DEATH_PENALTY::DESTROY }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class DIRECTION
{
    LEFT = 1, 
    TOP = 2, 
    RIGHT = 3, 
    BOTTOM = 4
};

template <>
inline DIRECTION enum_parse<DIRECTION>(const std::string k)
{
    static const std::unordered_map<std::string, DIRECTION> enums
    {
        { "LEFT", DIRECTION::LEFT }, 
        { "TOP", DIRECTION::TOP }, 
        { "RIGHT", DIRECTION::RIGHT }, 
        { "BOTTOM", DIRECTION::BOTTOM }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class DSL
{
    item = 0, 
    level = 1, 
    sex = 2, 
    strength = 3, 
    intelligence = 4, 
    dexteritry = 5, 
    promotion = 6
};

template <>
inline DSL enum_parse<DSL>(const std::string k)
{
    static const std::unordered_map<std::string, DSL> enums
    {
        { "item", DSL::item }, 
        { "level", DSL::level }, 
        { "sex", DSL::sex }, 
        { "strength", DSL::strength }, 
        { "intelligence", DSL::intelligence }, 
        { "dexteritry", DSL::dexteritry }, 
        { "promotion", DSL::promotion }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class ITEM_ATTRIBUTE
{
    NONE = 0x00000000, 
    CONSUME = 0x00000001, 
    BUNDLE = 0x00000002, 
    SCRIPT = 0x00000003, 
    CASH = 0x00000004, 
    EQUIPMENT = 0x10000000, 
    PACK = CONSUME | 0x00000100, 
    WEAPON = EQUIPMENT | 0x00000100, 
    ARMOR = EQUIPMENT | 0x00000200, 
    SHIELD = EQUIPMENT | 0x00000400, 
    HELMET = EQUIPMENT | 0x00000800, 
    RING = EQUIPMENT | 0x00001000, 
    AUXILIARY = EQUIPMENT | 0x00002000, 
    ARROW = EQUIPMENT | 0x00004000
};

template <>
inline ITEM_ATTRIBUTE enum_parse<ITEM_ATTRIBUTE>(const std::string k)
{
    static const std::unordered_map<std::string, ITEM_ATTRIBUTE> enums
    {
        { "NONE", ITEM_ATTRIBUTE::NONE }, 
        { "CONSUME", ITEM_ATTRIBUTE::CONSUME }, 
        { "BUNDLE", ITEM_ATTRIBUTE::BUNDLE }, 
        { "SCRIPT", ITEM_ATTRIBUTE::SCRIPT }, 
        { "CASH", ITEM_ATTRIBUTE::CASH }, 
        { "EQUIPMENT", ITEM_ATTRIBUTE::EQUIPMENT }, 
        { "PACK", ITEM_ATTRIBUTE::PACK }, 
        { "WEAPON", ITEM_ATTRIBUTE::WEAPON }, 
        { "ARMOR", ITEM_ATTRIBUTE::ARMOR }, 
        { "SHIELD", ITEM_ATTRIBUTE::SHIELD }, 
        { "HELMET", ITEM_ATTRIBUTE::HELMET }, 
        { "RING", ITEM_ATTRIBUTE::RING }, 
        { "AUXILIARY", ITEM_ATTRIBUTE::AUXILIARY }, 
        { "ARROW", ITEM_ATTRIBUTE::ARROW }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class ITEM_TYPE
{
    STUFF = 1, 
    CONSUME = 2, 
    WEAPON = 3, 
    ARMOR = 4, 
    HELMET = 5, 
    RING = 6, 
    SHIELD = 7, 
    AUXILIARY = 8, 
    BOW = 9, 
    PACKAGE = 10
};

template <>
inline ITEM_TYPE enum_parse<ITEM_TYPE>(const std::string k)
{
    static const std::unordered_map<std::string, ITEM_TYPE> enums
    {
        { "STUFF", ITEM_TYPE::STUFF }, 
        { "CONSUME", ITEM_TYPE::CONSUME }, 
        { "WEAPON", ITEM_TYPE::WEAPON }, 
        { "ARMOR", ITEM_TYPE::ARMOR }, 
        { "HELMET", ITEM_TYPE::HELMET }, 
        { "RING", ITEM_TYPE::RING }, 
        { "SHIELD", ITEM_TYPE::SHIELD }, 
        { "AUXILIARY", ITEM_TYPE::AUXILIARY }, 
        { "BOW", ITEM_TYPE::BOW }, 
        { "PACKAGE", ITEM_TYPE::PACKAGE }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class MOB_ATTACK_TYPE
{
    NONE = 0, 
    COUNTER = 1, 
    CONTAINMENT = 2, 
    RUN_AWAY = 3, 
    NO_MOVE = 4
};

template <>
inline MOB_ATTACK_TYPE enum_parse<MOB_ATTACK_TYPE>(const std::string k)
{
    static const std::unordered_map<std::string, MOB_ATTACK_TYPE> enums
    {
        { "NONE", MOB_ATTACK_TYPE::NONE }, 
        { "COUNTER", MOB_ATTACK_TYPE::COUNTER }, 
        { "CONTAINMENT", MOB_ATTACK_TYPE::CONTAINMENT }, 
        { "RUN_AWAY", MOB_ATTACK_TYPE::RUN_AWAY }, 
        { "NO_MOVE", MOB_ATTACK_TYPE::NO_MOVE }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class MOB_SIZE
{
    SMALL = 1, 
    LARGE = 2, 
    ALL = SMALL | LARGE
};

template <>
inline MOB_SIZE enum_parse<MOB_SIZE>(const std::string k)
{
    static const std::unordered_map<std::string, MOB_SIZE> enums
    {
        { "SMALL", MOB_SIZE::SMALL }, 
        { "LARGE", MOB_SIZE::LARGE }, 
        { "ALL", MOB_SIZE::ALL }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class OBJECT_TYPE
{
    UNKNOWN = 0x00, 
    ITEM = 0x01, 
    NPC = 0x02, 
    MOB = 0x04, 
    SESSION = 0x08, 
    OBJECT = ITEM | NPC | MOB, 
    LIFE = MOB | SESSION
};

template <>
inline OBJECT_TYPE enum_parse<OBJECT_TYPE>(const std::string k)
{
    static const std::unordered_map<std::string, OBJECT_TYPE> enums
    {
        { "UNKNOWN", OBJECT_TYPE::UNKNOWN }, 
        { "ITEM", OBJECT_TYPE::ITEM }, 
        { "NPC", OBJECT_TYPE::NPC }, 
        { "MOB", OBJECT_TYPE::MOB }, 
        { "SESSION", OBJECT_TYPE::SESSION }, 
        { "OBJECT", OBJECT_TYPE::OBJECT }, 
        { "LIFE", OBJECT_TYPE::LIFE }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class REGEX
{
    SELL = 0, 
    BUY = 1, 
    REPAIR = 2, 
    DEPOSIT_MONEY = 3, 
    WITHDRAW_MONEY = 4, 
    DEPOSIT_ITEM = 5, 
    WITHDRAW_ITEM = 6, 
    SELL_LIST = 7, 
    BUY_LIST = 8, 
    SELL_PRICE = 9, 
    BUY_PRICE = 10, 
    DEPOSITED_MONEY = 11, 
    RENAME_WEAPON = 12, 
    HOLD_ITEM_LIST = 13, 
    HOLD_ITEM_COUNT = 14
};

template <>
inline REGEX enum_parse<REGEX>(const std::string k)
{
    static const std::unordered_map<std::string, REGEX> enums
    {
        { "SELL", REGEX::SELL }, 
        { "BUY", REGEX::BUY }, 
        { "REPAIR", REGEX::REPAIR }, 
        { "DEPOSIT_MONEY", REGEX::DEPOSIT_MONEY }, 
        { "WITHDRAW_MONEY", REGEX::WITHDRAW_MONEY }, 
        { "DEPOSIT_ITEM", REGEX::DEPOSIT_ITEM }, 
        { "WITHDRAW_ITEM", REGEX::WITHDRAW_ITEM }, 
        { "SELL_LIST", REGEX::SELL_LIST }, 
        { "BUY_LIST", REGEX::BUY_LIST }, 
        { "SELL_PRICE", REGEX::SELL_PRICE }, 
        { "BUY_PRICE", REGEX::BUY_PRICE }, 
        { "DEPOSITED_MONEY", REGEX::DEPOSITED_MONEY }, 
        { "RENAME_WEAPON", REGEX::RENAME_WEAPON }, 
        { "HOLD_ITEM_LIST", REGEX::HOLD_ITEM_LIST }, 
        { "HOLD_ITEM_COUNT", REGEX::HOLD_ITEM_COUNT }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class SEX
{
    MAN = 1, 
    WOMAN = 2, 
    ALL = MAN | WOMAN
};

template <>
inline SEX enum_parse<SEX>(const std::string k)
{
    static const std::unordered_map<std::string, SEX> enums
    {
        { "MAN", SEX::MAN }, 
        { "WOMAN", SEX::WOMAN }, 
        { "ALL", SEX::ALL }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

enum class WEAPON_TYPE
{
    NORMAL = 0, 
    SPEAR = 1, 
    BOW = 2, 
    FAN = 3, 
    UNKNOWN = 4
};

template <>
inline WEAPON_TYPE enum_parse<WEAPON_TYPE>(const std::string k)
{
    static const std::unordered_map<std::string, WEAPON_TYPE> enums
    {
        { "NORMAL", WEAPON_TYPE::NORMAL }, 
        { "SPEAR", WEAPON_TYPE::SPEAR }, 
        { "BOW", WEAPON_TYPE::BOW }, 
        { "FAN", WEAPON_TYPE::FAN }, 
        { "UNKNOWN", WEAPON_TYPE::UNKNOWN }
    };

    auto i = enums.find(k);
    if (i == enums.end())
        throw std::runtime_error("no enum value");

    return i->second;
}

}
#pragma endregion

#pragma region const
namespace const_value {

class regex
{
public:
    inline static constexpr const char* SELL = "(?P<name>\\S+)\\s+(?:(?:(?:(?P<count>\\d+)개)|(?P<all>다|전부))\\s+)?(?:판다|팜|팔게)";
    inline static constexpr const char* BUY = "(?P<name>\\S+)\\s+(?:(?:(?:(?P<count>\\d+)개))\\s+)?(?:산다|줘|주세요)";
    inline static constexpr const char* REPAIR = "(((?P<all>전부|모두|다)|(?P<name>\\S+))\\s+?(?:고쳐|수리\\s*해))\\s*줘";
    inline static constexpr const char* DEPOSIT_MONEY = "(?:돈|금전)\\s+(?:(?P<money>\\d+)(?:원|전)|(?P<all>(?:전부)?(?:\\s*다)?))\\s+맡아\\s*(?:줘|놔|주세요)";
    inline static constexpr const char* WITHDRAW_MONEY = "(?:돈|금전)\\s+(?:(?P<money>\\d+)(?:원|전)|(?P<all>(?:전부)?(?:\\s*다)?))\\s+돌려\\s*(?:줘|놔|주세요)";
    inline static constexpr const char* DEPOSIT_ITEM = "(?P<name>\\S+)\\s+(?:(?:(?P<count>\\d+)(?:개)|(?P<all>(?:전부)?(?:\\s*다)?))\\s+)?맡아\\s*(?:줘|놔|주세요)";
    inline static constexpr const char* WITHDRAW_ITEM = "(?P<name>\\S+)\\s+(?:(?:(?P<count>\\d+)(?:개)|(?P<all>(?:전부)?(?:\\s*다)?))\\s+)?돌려\\s*(?:줘|놔|주세요)";
    inline static constexpr const char* SELL_LIST = "(?:뭐|뭘|무엇을|무얼)\\s*(?:파니|파냐|팔고\\s*(?:있니|있냐))";
    inline static constexpr const char* BUY_LIST = "(?:뭐|뭘|무엇을|무얼)\\s*(?:사니|사냐|사고\\s*(?:있니|있냐))";
    inline static constexpr const char* SELL_PRICE = "(?P<name>\\S+)\\s+(?:(?:얼마(?:니|야|임|냐|에\\s*파(?:니|냐)))|(?:파(?:니|냐|)))";
    inline static constexpr const char* BUY_PRICE = "(?P<name>\\S+)\\s+(?:얼마에\\s?)?사(?:니|냐)";
    inline static constexpr const char* DEPOSITED_MONEY = "(?:돈|금전)\\s*얼마(?:나)?\\s*맡(?:아두)?고\\s*있(?:니|냐)";
    inline static constexpr const char* RENAME_WEAPON = "(?P<weapon>\\S+?)?(?:의|$)?\\s+이름을\\s+(?P<name>\\S+?)?(?:으|$)?로\\s+명명";
    inline static constexpr const char* HOLD_ITEM_LIST = "(?:뭐|뭘|무엇을|무얼)\\s*맡고\\s*(?:있니|있냐)";
    inline static constexpr const char* HOLD_ITEM_COUNT = "(?P<name>\\S+)\\s+(?:몇\\s*개|얼마나)\\s*맡고\\s*있(?:니|냐)";

private:
    regex() = default;
    ~regex() = default;
};
class string
{
public:
    inline static constexpr const char* ACCOUNT_INVALID_NAME = "이름이 길거나 적합하지 않습니다.";
    inline static constexpr const char* ACCOUNT_ALREADY_LOGIN = "이미 접속중입니다.";
    inline static constexpr const char* ACCOUNT_NOT_FOUND_NAME = "존재하지 않는 이름입니다.";
    inline static constexpr const char* ACCOUNT_PASSWORD_SIZE = "암호는 4자 이상 8자 이하";
    inline static constexpr const char* ACCOUNT_INVALID_PASSWORD = "비밀번호가 올바르지 않습니다.";
    inline static constexpr const char* ACCOUNT_SIMPLE_PASSWORD = "암호가 단순합니다.";
    inline static constexpr const char* ACCOUNT_SUCCESS_REGISTER_ACCOUNT = "등록완료, 이어하기를 선택하세요.";
    inline static constexpr const char* ACCOUNT_SUCCESS_CHANGE_PASSWORD = "변경됐다리";
    inline static constexpr const char* ACCOUNT_INVALID_BIRTHDAY = "생년월일이 올바르지 않습니다.";
    inline static constexpr const char* ACCOUNT_NEW_PW_EQUALIZATION = "기존 암호화 동일합니다.";
    inline static constexpr const char* ACCOUNT_ALREADY_EXISTS = "이미 존재하는 이름입니다.";
    inline static constexpr const char* MESSAGE_ASSET_MAP_LOADED = "* [%0.2lf%%] 맵 정보를 읽었습니다. (%s)";
    inline static constexpr const char* MESSAGE_ASSET_REGEX_LOADED = "* [%0.2lf%%] 정규표현식 정보를 읽었습니다. (%s)";
    inline static constexpr const char* MESSAGE_ASSET_WORLD_MAP_LOADED = "* [%0.2lf%%] 월드맵 정보를 읽었습니다. (%s)";
    inline static constexpr const char* MESSAGE_ASSET_DOOR_LOADED = "* [%0.2lf%%] 도어 정보를 읽었습니다. (%s)";
    inline static constexpr const char* MESSAGE_ASSET_SPELL_LOADED = "* [%0.2lf%%] 스펠 정보를 읽었습니다. (%s)";
    inline static constexpr const char* MESSAGE_ASSET_WARP_LOADED = "* [%0.2lf%%] 워프 정보를 읽었습니다. (%s)";
    inline static constexpr const char* MESSAGE_ASSET_ITEM_LOADED = "* [%0.2lf%%] 아이템 정보를 읽었습니다. (%s)";
    inline static constexpr const char* MESSAGE_ASSET_ITEM_MIX_LOADED = "* [%0.2lf%%] 조합 정보를 읽었습니다. (%s)";
    inline static constexpr const char* MESSAGE_ASSET_SELL_LOADED = "* [%0.2lf%%] 판매 정보를 읽었습니다. (%s)";
    inline static constexpr const char* MESSAGE_ASSET_BUY_LOADED = "* [%0.2lf%%] 구매 정보를 읽었습니다. (%s)";
    inline static constexpr const char* MESSAGE_ASSET_NPC_LOADED = "* [%0.2lf%%] NPC 정보를 읽었습니다. (%s)";
    inline static constexpr const char* MESSAGE_ASSET_MOB_LOADED = "* [%0.2lf%%] 몹 정보를 읽었습니다. (%s)";
    inline static constexpr const char* MESSAGE_ASSET_DROP_LOADED = "* [%0.2lf%%] 드롭 정보를 읽었습니다. (%s)";
    inline static constexpr const char* MESSAGE_ASSET_NPC_SPAWN_LOADED = "* [%0.2lf%%] NPC 스폰 정보를 읽었습니다. (%s)";
    inline static constexpr const char* MESSAGE_ASSET_MOB_SPAWN_LOADED = "* [%0.2lf%%] 몹 스폰 정보를 읽었습니다. (%s)";
    inline static constexpr const char* MESSAGE_ASSET_CLASS_LOADED = "* [%0.2lf%%] 클래스 정보를 읽었습니다. (%s)";
    inline static constexpr const char* MESSAGE_ASSET_BOARD_LOADED = "* [%0.2lf%%] 게시판 정보를 읽었습니다. (%s)";
    inline static constexpr const char* MESSAGE_ASSET_MAP_ALL_LOADED = "* [100%%] 총 %d개의 맵 정보를 읽었습니다.";
    inline static constexpr const char* MESSAGE_ASSET_REGEX_ALL_LOADED = "* [100%%] 총 %d개의 정규표현식 정보를 읽었습니다.";
    inline static constexpr const char* MESSAGE_ASSET_WORLD_MAP_ALL_LOADED = "* [100%%] 총 %d개의 월드맵 정보를 읽었습니다.";
    inline static constexpr const char* MESSAGE_ASSET_DOOR_ALL_LOADED = "* [100%%] 총 %d개의 도어 정보를 읽었습니다.";
    inline static constexpr const char* MESSAGE_ASSET_SPELL_ALL_LOADED = "* [100%%] 총 %d개의 스펠 정보를 읽었습니다.";
    inline static constexpr const char* MESSAGE_ASSET_WARP_ALL_LOADED = "* [100%%] 총 %d개의 워프 정보를 읽었습니다.";
    inline static constexpr const char* MESSAGE_ASSET_ITEM_ALL_LOADED = "* [100%%] 총 %d개의 아이템 정보를 읽었습니다.";
    inline static constexpr const char* MESSAGE_ASSET_ITEM_MIX_ALL_LOADED = "* [100%%] 총 %d개의 조합 정보를 읽었습니다.";
    inline static constexpr const char* MESSAGE_ASSET_SELL_ALL_LOADED = "* [100%%] 총 %d개의 판매 정보를 읽었습니다.";
    inline static constexpr const char* MESSAGE_ASSET_BUY_ALL_LOADED = "* [100%%] 총 %d개의 구매 정보를 읽었습니다.";
    inline static constexpr const char* MESSAGE_ASSET_NPC_ALL_LOADED = "* [100%%] 총 %d개의 NPC 정보를 읽었습니다.";
    inline static constexpr const char* MESSAGE_ASSET_MOB_ALL_LOADED = "* [100%%] 총 %d개의 몹 정보를 읽었습니다.";
    inline static constexpr const char* MESSAGE_ASSET_DROP_ALL_LOADED = "* [100%%] 총 %d개의 드롭 정보를 읽었습니다.";
    inline static constexpr const char* MESSAGE_ASSET_NPC_SPAWN_ALL_LOADED = "* [100%%] 총 %d개의 NPC 스폰 정보를 읽었습니다.";
    inline static constexpr const char* MESSAGE_ASSET_MOB_SPAWN_ALL_LOADED = "* [100%%] 총 %d개의 몹 스폰 정보를 읽었습니다.";
    inline static constexpr const char* MESSAGE_ASSET_CLASS_ALL_LOADED = "* [100%%] 총 %d개의 클래스 정보를 읽었습니다.";
    inline static constexpr const char* MESSAGE_ASSET_BOARD_ALL_LOADED = "* [100%%] 총 %d개의 게시판 정보를 읽었습니다.";
    inline static constexpr const char* MESSAGE_ASSET_INVALID_SEX = "성별을 확인할 수 없습니다.";
    inline static constexpr const char* MESSAGE_ASSET_INVALID_DEATH_PENALTY = "아이템 데스 패널티를 확인할 수 없습니다.";
    inline static constexpr const char* MESSAGE_ASSET_INVALID_MOB_SIZE = "몹 크기 형식을 확인할 수 없습니다.";
    inline static constexpr const char* MESSAGE_ASSET_INVALID_MOB_OFFENSIVE = "몹 공격 타입을 확인할 수 없습니다.";
    inline static constexpr const char* MESSAGE_ASSET_CANNOT_LOAD_MAP_DATA = "맵 데이터를 읽을 수 없습니다.";
    inline static constexpr const char* MESSAGE_ASSET_CANNOT_LOAD_MAP_BLOCK = "맵 블록 데이터를 읽을 수 없습니다.";
    inline static constexpr const char* MESSAGE_ASSET_INVALID_NPC_NAME = "존재하지 않는 NPC입니다.";
    inline static constexpr const char* MESSAGE_ASSET_INVALID_MAP_NAME = "존재하지 않는 맵입니다.";
    inline static constexpr const char* MESSAGE_ASSET_INVALID_NPC_DIRECTION = "NPC의 방향이 올바르지 않습니다.";
    inline static constexpr const char* MESSAGE_ASSET_INVALID_NPC_POSITION = "NPC의 위치가 올바르지 않습니다.";
    inline static constexpr const char* MESSAGE_ASSET_INVALID_MOB_NAME = "올바르지 않은 몹 이름입니다.";
    inline static constexpr const char* MESSAGE_ASSET_INVALID_ITEM_NAME = "올바르지 않은 아이템 이름입니다.";
    inline static constexpr const char* MESSAGE_ITEM_CANNOT_PICKUP_ANYMORE = "더 이상 가질 수 없습니다.";
    inline static constexpr const char* MESSAGE_GROUP_CANNOT_FIND_TARGET = "대상을 찾을 수 없습니다.";
    inline static constexpr const char* MESSAGE_GROUP_DISABLED_MINE = "그룹 거부 상태입니다.";
    inline static constexpr const char* MESSAGE_GROUP_DISABLED_TARGET = "상대방이 그룹 거부 상태입니다.";
    inline static constexpr const char* MESSAGE_GROUP_NOT_OWNER = "그룹장만 할 수 있습니다.";
    inline static constexpr const char* MESSAGE_GROUP_FULL_MEMBER = "자리가 없습니다.";
    inline static constexpr const char* MESSAGE_GROUP_ALREADY_JOINED = "님은 이미 그룹 참여 중입니다.";
    inline static constexpr const char* MESSAGE_GROUP_JOINED = "님 그룹에 참여";
    inline static constexpr const char* MESSAGE_GROUP_LEFT = "님 그룹 탈퇴";
    inline static constexpr const char* MESSAGE_EXCEPTION_RIDDING = "말을 타고는 할 수 없습니다.";
    inline static constexpr const char* MESSAGE_EXCEPTION_GHOST = "귀신은 할 수 없습니다.";
    inline static constexpr const char* MESSAGE_EXCEPTION_REQUIRE_CLASS = "직업을 선택해야 합니다.";
    inline static constexpr const char* MESSAGE_EXCEPTION_NO_CONVEYANCE = "탈 것이 없습니다.";
    inline static constexpr const char* MESSAGE_EXCEPTION_DISGUISE = "변신 중에는 할 수 없습니다.";
    inline static constexpr const char* MESSAGE_EXCEPTION_CANNOT_DROP_ITEM = "버릴 수 없는 물건입니다.";
    inline static constexpr const char* MESSAGE_EXCEPTION_CANNOT_THROW_ITEM = "던질 수 없는 물건입니다.";
    inline static constexpr const char* MESSAGE_EXCEPTION_INVENTORY_OVERFLOW = "소지품이 꽉 찼습니다.";
    inline static constexpr const char* MESSAGE_EXCEPTION_INVALID_TARGET = "대상이 올바르지 않습니다.";
    inline static constexpr const char* MESSAGE_TRADE_NOT_TRADING = "교환중이 아닙니다.";
    inline static constexpr const char* MESSAGE_TRADE_NOT_SELECTED = "선택된 아이템이 없습니다.";
    inline static constexpr const char* MESSAGE_TRADE_SUCCESS = "교환에 성공했습니다.";
    inline static constexpr const char* MESSAGE_TRADE_FAILED = "교환에 실패했습니다.";
    inline static constexpr const char* MESSAGE_TRADE_NOTIFY_LOCK_TO_PARTNER = "상대방이 확인을 눌렀습니다.";
    inline static constexpr const char* MESSAGE_TRADE_CANCELLED_BY_ME = "내가 교환을 취소했습니다.";
    inline static constexpr const char* MESSAGE_TRADE_CANCELLED_BY_PARTNER = "상대방이 교환을 취소했습니다.";
    inline static constexpr const char* MESSAGE_TRADE_INVALID_COUNT = "갯수가 올바르지 않습니다.";
    inline static constexpr const char* MESSAGE_TRADE_NOT_ALLOWED_TO_TRADE = "교환이 불가능한 아이템입니다.";
    inline static constexpr const char* MESSAGE_TRADE_REFUSED_BY_ME = "교환 거부 상태입니다.";
    inline static constexpr const char* MESSAGE_TRADE_REFUSED_BY_PARTNER = "님은 교환 거부 상태입니다.";
    inline static constexpr const char* MESSAGE_TRADE_PARTNER_ALREADY_TRADING = "님은 이미 교환 중입니다.";
    inline static constexpr const char* MESSAGE_TRADE_PARTNER_INVISIBLE = "대상이 보이지 않습니다.";
    inline static constexpr const char* MESSAGE_TRADE_PARTNER_TOO_FAR = "님과 너무 멀리 떨어져 있습니다.";
    inline static constexpr const char* MESSAGE_MIX_SUCCESS = "성공하였습니다.";
    inline static constexpr const char* MESSAGE_MIX_FAILED = "실패하였습니다.";
    inline static constexpr const char* MESSAGE_RIDE_UNRIDE = "말에 타고 있지 않습니다.";
    inline static constexpr const char* MESSAGE_RIDE_ALREADY_RIDE = "이미 타고 있습니다.";
    inline static constexpr const char* MESSAGE_RIDE_ON = "말에 탔습니다.";
    inline static constexpr const char* MESSAGE_RIDE_OFF = "말에서 내렸습니다.";
    inline static constexpr const char* MESSAGE_MONEY_DROP = "돈을 버렸습니다.";
    inline static constexpr const char* MESSAGE_MONEY_FULL = "더 이상 돈을 가질 수 없습니다.";
    inline static constexpr const char* MESSAGE_LEVEL_UP = "레벨이 올랐습니다";
    inline static constexpr const char* MESSAGE_BOARD_WRITE = "글을 작성하였습니다";
    inline static constexpr const char* MESSAGE_BOARD_ARTICLE_NOT_EXIST = "게시글이 존재하지 않습니다.";
    inline static constexpr const char* MESSAGE_BOARD_SECTION_NOT_EXIST = "섹션이 존재하지 않습니다.";
    inline static constexpr const char* MESSAGE_BOARD_NOT_AUTH = "권한이 없습니다.";
    inline static constexpr const char* MESSAGE_BOARD_SUCCESS_DELETE = "글이 삭제되었습니다.";
    inline static constexpr const char* MESSAGE_BOARD_TOO_LONG_TITLE = "게시글 제목이 너무 깁니다.";
    inline static constexpr const char* MESSAGE_BOARD_TOO_LONG_CONTENTS = "게시글 내용이 너무 깁니다.";
    inline static constexpr const char* MESSAGE_DOOR_OPEN = "문을 열었습니다.";
    inline static constexpr const char* MESSAGE_DOOR_CLOSE = "문을 닫았습니다.";
    inline static constexpr const char* MESSAGE_DOOR_LOCK = "문을 잠궜습니다.";
    inline static constexpr const char* MESSAGE_DOOR_UNLOCK = "문을 열었습니다.";
    inline static constexpr const char* MESSAGE_DOOR_LOCKED = "문이 잠겼습니다.";
    inline static constexpr const char* MESSAGE_ERROR_UNKNOWN = "올바르지 않은 명령입니다.";

private:
    string() = default;
    ~string() = default;
};

}
#pragma endregion

template<class T>
struct is_optional : std::false_type {};

template<class T>
struct is_optional<std::optional<T>> : std::true_type {};

template<class T>
struct is_unique : std::false_type {};

template<class T>
struct is_unique<std::unique_ptr<T>> : std::true_type {};

template<class T>
struct is_vector : std::false_type {};

template<class T>
struct is_vector<std::vector<T>> : std::true_type {};

template <typename T, typename Enabler = void>
struct is_map : std::false_type {};

template <typename T>
struct is_map<T, std::void_t<typename T::mapped_type>> : std::true_type{};

template <typename T>
struct is_default
{
    static constexpr bool value =
        !std::is_enum<T>::value &&
        !std::is_pointer<T>::value &&
        !fb::model::is_vector<T>::value &&
        !fb::model::is_map<T>::value &&
        !fb::model::is_optional<T>::value &&
        !fb::model::is_unique<T>::value;
};

class dsl;

template <typename T> static typename std::enable_if<fb::model::is_default<T>::value, T>::type build(const Json::Value& json);
template <typename T> static typename std::enable_if<std::is_enum<T>::value, T>::type build(const Json::Value& json);
template <typename T> static typename std::enable_if<std::is_pointer<T>::value, T>::type build(const Json::Value& json);
template <typename T> static typename std::enable_if<fb::model::is_vector<T>::value, T>::type build(const Json::Value& json);
template <typename T> static typename std::enable_if<fb::model::is_map<T>::value, T>::type build(const Json::Value& json);
template <typename T> static typename std::enable_if<fb::model::is_optional<T>::value, T>::type build(const Json::Value& json);
template <typename T> static typename std::enable_if<fb::model::is_unique<T>::value, T>::type build(const Json::Value& json);
template <> static int8_t build<int8_t>(const Json::Value& json);
template <> static uint8_t build<uint8_t>(const Json::Value& json);
template <> static int16_t build<int16_t>(const Json::Value& json);
template <> static uint16_t build<uint16_t>(const Json::Value& json);
template <> static int build<int>(const Json::Value& json);
template <> static uint32_t build<uint32_t>(const Json::Value& json);
template <> static int64_t build<int64_t>(const Json::Value& json);
template <> static uint64_t build<uint64_t>(const Json::Value& json);
template <> static std::string build<std::string>(const Json::Value& json);
template <> static float build<float>(const Json::Value& json);
template <> static double build<double>(const Json::Value& json);
template <> static bool build<bool>(const Json::Value& json);
template <> static boost::posix_time::ptime build<boost::posix_time::ptime>(const Json::Value& json);
template <> static std::chrono::milliseconds build<std::chrono::milliseconds>(const Json::Value& json);
template <> static fb::model::date_range build<fb::model::date_range>(const Json::Value& json);
template <> static fb::model::dsl build<fb::model::dsl>(const Json::Value& json);

#pragma region dsl
class dsl
{
public:
    class dexteritry;
    class intelligence;
    class item;
    class level;
    class promotion;
    class sex;
    class strength;

public:
    fb::model::enum_value::DSL header;
    const std::vector<std::any> params;

private:
    static std::vector<std::any> parse_params(const Json::Value& json);

public:
    dsl(fb::model::enum_value::DSL header, const std::vector<std::any>& params) : header(header), params(params)
    { }
    dsl(const Json::Value& json) : header(build<fb::model::enum_value::DSL>(json["Type"])), params(parse_params(json))
    { }
    ~dsl()
    { }
};

class fb::model::dsl::dexteritry
{
public:
    const uint8_t value;

public:
    dexteritry(uint8_t value) : 
        value(value)
    { }
    dexteritry(const Json::Value& json) : 
        value(fb::model::build<uint8_t>(json[0]))
    { }
    dexteritry(const std::vector<std::any>& parameters) : 
        value(any_cast<uint8_t>(parameters[0]))
    { }

public:
    fb::model::dsl to_dsl()
    {
        return fb::model::dsl(fb::model::enum_value::DSL::dexteritry, {value});
    }
};


class fb::model::dsl::intelligence
{
public:
    const uint8_t value;

public:
    intelligence(uint8_t value) : 
        value(value)
    { }
    intelligence(const Json::Value& json) : 
        value(fb::model::build<uint8_t>(json[0]))
    { }
    intelligence(const std::vector<std::any>& parameters) : 
        value(any_cast<uint8_t>(parameters[0]))
    { }

public:
    fb::model::dsl to_dsl()
    {
        return fb::model::dsl(fb::model::enum_value::DSL::intelligence, {value});
    }
};


class fb::model::dsl::item
{
public:
    const uint32_t id;
    const uint32_t count;
    const double percent;

public:
    item(uint32_t id, uint32_t count, double percent) : 
        id(id),
        count(count),
        percent(percent)
    { }
    item(const Json::Value& json) : 
        id(fb::model::build<uint32_t>(json[0])),
        count(fb::model::build<uint32_t>(json[1])),
        percent(fb::model::build<double>(json[2]))
    { }
    item(const std::vector<std::any>& parameters) : 
        id(any_cast<uint32_t>(parameters[0])),
        count(any_cast<uint32_t>(parameters[1])),
        percent(any_cast<double>(parameters[2]))
    { }

public:
    fb::model::dsl to_dsl()
    {
        return fb::model::dsl(fb::model::enum_value::DSL::item, {id, count, percent});
    }
};


class fb::model::dsl::level
{
public:
    const std::optional<uint8_t> min;
    const std::optional<uint8_t> max;

public:
    level(std::optional<uint8_t> min, std::optional<uint8_t> max) : 
        min(min),
        max(max)
    { }
    level(const Json::Value& json) : 
        min(fb::model::build<std::optional<uint8_t>>(json[0])),
        max(fb::model::build<std::optional<uint8_t>>(json[1]))
    { }
    level(const std::vector<std::any>& parameters) : 
        min(any_cast<const std::optional<uint8_t>&>(parameters[0])),
        max(any_cast<const std::optional<uint8_t>&>(parameters[1]))
    { }

public:
    fb::model::dsl to_dsl()
    {
        return fb::model::dsl(fb::model::enum_value::DSL::level, {min, max});
    }
};


class fb::model::dsl::promotion
{
public:
    const uint8_t value;

public:
    promotion(uint8_t value) : 
        value(value)
    { }
    promotion(const Json::Value& json) : 
        value(fb::model::build<uint8_t>(json[0]))
    { }
    promotion(const std::vector<std::any>& parameters) : 
        value(any_cast<uint8_t>(parameters[0]))
    { }

public:
    fb::model::dsl to_dsl()
    {
        return fb::model::dsl(fb::model::enum_value::DSL::promotion, {value});
    }
};


class fb::model::dsl::sex
{
public:
    const fb::model::enum_value::SEX value;

public:
    sex(fb::model::enum_value::SEX value) : 
        value(value)
    { }
    sex(const Json::Value& json) : 
        value(fb::model::build<fb::model::enum_value::SEX>(json[0]))
    { }
    sex(const std::vector<std::any>& parameters) : 
        value(any_cast<fb::model::enum_value::SEX>(parameters[0]))
    { }

public:
    fb::model::dsl to_dsl()
    {
        return fb::model::dsl(fb::model::enum_value::DSL::sex, {value});
    }
};


class fb::model::dsl::strength
{
public:
    const uint8_t value;

public:
    strength(uint8_t value) : 
        value(value)
    { }
    strength(const Json::Value& json) : 
        value(fb::model::build<uint8_t>(json[0]))
    { }
    strength(const std::vector<std::any>& parameters) : 
        value(any_cast<uint8_t>(parameters[0]))
    { }

public:
    fb::model::dsl to_dsl()
    {
        return fb::model::dsl(fb::model::enum_value::DSL::strength, {value});
    }
};


inline std::vector<std::any> fb::model::dsl::parse_params(const Json::Value& json)
{
    static auto data = std::unordered_map<fb::model::enum_value::DSL, std::function<std::vector<std::any>(const Json::Value&)>>
    {
        { fb::model::enum_value::DSL::dexteritry, [](const Json::Value& json) { return fb::model::dsl::dexteritry(json).to_dsl().params; }},
        { fb::model::enum_value::DSL::intelligence, [](const Json::Value& json) { return fb::model::dsl::intelligence(json).to_dsl().params; }},
        { fb::model::enum_value::DSL::item, [](const Json::Value& json) { return fb::model::dsl::item(json).to_dsl().params; }},
        { fb::model::enum_value::DSL::level, [](const Json::Value& json) { return fb::model::dsl::level(json).to_dsl().params; }},
        { fb::model::enum_value::DSL::promotion, [](const Json::Value& json) { return fb::model::dsl::promotion(json).to_dsl().params; }},
        { fb::model::enum_value::DSL::sex, [](const Json::Value& json) { return fb::model::dsl::sex(json).to_dsl().params; }},
        { fb::model::enum_value::DSL::strength, [](const Json::Value& json) { return fb::model::dsl::strength(json).to_dsl().params; }}
    };

    auto header = build<fb::model::enum_value::DSL>(json["Type"]);
    auto i = data.find(header);
    if(i == data.end())
        throw std::runtime_error("invalid dsl header");

    return i->second(json["Parameters"]);
}
#pragma endregion

#pragma region class model
class ability
#ifdef DECLARE_ABILITY_INHERIT
DECLARE_ABILITY_INHERIT
#endif
{
public:
    const fb::model::enum_value::CLASS parent;
    const uint8_t level;
    const uint8_t dexteritry;
    const uint8_t intelligence;
    const uint8_t strength;
    const uint32_t exp;
    const uint32_t hp;
    const uint32_t mp;

public:
    ability(const Json::Value& json) : 
#ifdef DECLARE_ABILITY_CONSTRUCTOR
DECLARE_ABILITY_CONSTRUCTOR
#endif
        parent(fb::model::build<fb::model::enum_value::CLASS>(json["parent"])),
        level(fb::model::build<uint8_t>(json["level"])),
        dexteritry(fb::model::build<uint8_t>(json["dexteritry"])),
        intelligence(fb::model::build<uint8_t>(json["intelligence"])),
        strength(fb::model::build<uint8_t>(json["strength"])),
        exp(fb::model::build<uint32_t>(json["exp"])),
        hp(fb::model::build<uint32_t>(json["hp"])),
        mp(fb::model::build<uint32_t>(json["mp"]))
    { }
    virtual ~ability()
    { }
#ifdef DECLARE_ABILITY_EXTENSION
DECLARE_ABILITY_EXTENSION
#endif
};
class ability_attribute
#ifdef DECLARE_ABILITY_ATTRIBUTE_INHERIT
DECLARE_ABILITY_ATTRIBUTE_INHERIT
#endif
{
public:
    const fb::model::enum_value::CLASS id;

public:
    ability_attribute(const Json::Value& json) : 
#ifdef DECLARE_ABILITY_ATTRIBUTE_CONSTRUCTOR
DECLARE_ABILITY_ATTRIBUTE_CONSTRUCTOR
#endif
        id(fb::model::build<fb::model::enum_value::CLASS>(json["id"]))
    { }
    virtual ~ability_attribute()
    { }
#ifdef DECLARE_ABILITY_ATTRIBUTE_EXTENSION
DECLARE_ABILITY_ATTRIBUTE_EXTENSION
#endif
};
class board
#ifdef DECLARE_BOARD_INHERIT
DECLARE_BOARD_INHERIT
#endif
{
public:
    const uint32_t id;
    const std::string name;
    const uint8_t min_level;
    const uint8_t max_level;
    const bool admin;

public:
    board(const Json::Value& json) : 
#ifdef DECLARE_BOARD_CONSTRUCTOR
DECLARE_BOARD_CONSTRUCTOR
#endif
        id(fb::model::build<uint32_t>(json["id"])),
        name(fb::model::build<std::string>(json["name"])),
        min_level(fb::model::build<uint8_t>(json["min_level"])),
        max_level(fb::model::build<uint8_t>(json["max_level"])),
        admin(fb::model::build<bool>(json["admin"]))
    { }
    virtual ~board()
    { }
#ifdef DECLARE_BOARD_EXTENSION
DECLARE_BOARD_EXTENSION
#endif
};
class buy
#ifdef DECLARE_BUY_INHERIT
DECLARE_BUY_INHERIT
#endif
{
public:
    const uint32_t parent;
    const uint32_t item;
    const std::optional<uint32_t> price;

public:
    buy(const Json::Value& json) : 
#ifdef DECLARE_BUY_CONSTRUCTOR
DECLARE_BUY_CONSTRUCTOR
#endif
        parent(fb::model::build<uint32_t>(json["parent"])),
        item(fb::model::build<uint32_t>(json["item"])),
        price(fb::model::build<std::optional<uint32_t>>(json["price"]))
    { }
    virtual ~buy()
    { }
#ifdef DECLARE_BUY_EXTENSION
DECLARE_BUY_EXTENSION
#endif
};
class buy_attribute
#ifdef DECLARE_BUY_ATTRIBUTE_INHERIT
DECLARE_BUY_ATTRIBUTE_INHERIT
#endif
{
public:
    const uint32_t id;

public:
    buy_attribute(const Json::Value& json) : 
#ifdef DECLARE_BUY_ATTRIBUTE_CONSTRUCTOR
DECLARE_BUY_ATTRIBUTE_CONSTRUCTOR
#endif
        id(fb::model::build<uint32_t>(json["id"]))
    { }
    virtual ~buy_attribute()
    { }
#ifdef DECLARE_BUY_ATTRIBUTE_EXTENSION
DECLARE_BUY_ATTRIBUTE_EXTENSION
#endif
};
class combine
#ifdef DECLARE_COMBINE_INHERIT
DECLARE_COMBINE_INHERIT
#endif
{
public:
    const std::vector<fb::model::dsl> source;
    const std::vector<fb::model::dsl> success;
    const std::vector<fb::model::dsl> failed;
    const double percent;

public:
    combine(const Json::Value& json) : 
#ifdef DECLARE_COMBINE_CONSTRUCTOR
DECLARE_COMBINE_CONSTRUCTOR
#endif
        source(fb::model::build<std::vector<fb::model::dsl>>(json["source"])),
        success(fb::model::build<std::vector<fb::model::dsl>>(json["success"])),
        failed(fb::model::build<std::vector<fb::model::dsl>>(json["failed"])),
        percent(fb::model::build<double>(json["percent"]))
    { }
    virtual ~combine()
    { }
#ifdef DECLARE_COMBINE_EXTENSION
DECLARE_COMBINE_EXTENSION
#endif
};
class door
#ifdef DECLARE_DOOR_INHERIT
DECLARE_DOOR_INHERIT
#endif
{
public:
    const uint32_t id;
    const std::vector<uint32_t> pairs;

public:
    door(const Json::Value& json) : 
#ifdef DECLARE_DOOR_CONSTRUCTOR
DECLARE_DOOR_CONSTRUCTOR
#endif
        id(fb::model::build<uint32_t>(json["id"])),
        pairs(fb::model::build<std::vector<uint32_t>>(json["pairs"]))
    { }
    virtual ~door()
    { }
#ifdef DECLARE_DOOR_EXTENSION
DECLARE_DOOR_EXTENSION
#endif
};
class door_pair
#ifdef DECLARE_DOOR_PAIR_INHERIT
DECLARE_DOOR_PAIR_INHERIT
#endif
{
public:
    const uint32_t id;
    const uint32_t open;
    const uint32_t close;

public:
    door_pair(const Json::Value& json) : 
#ifdef DECLARE_DOOR_PAIR_CONSTRUCTOR
DECLARE_DOOR_PAIR_CONSTRUCTOR
#endif
        id(fb::model::build<uint32_t>(json["id"])),
        open(fb::model::build<uint32_t>(json["open"])),
        close(fb::model::build<uint32_t>(json["close"]))
    { }
    virtual ~door_pair()
    { }
#ifdef DECLARE_DOOR_PAIR_EXTENSION
DECLARE_DOOR_PAIR_EXTENSION
#endif
};
class drop
#ifdef DECLARE_DROP_INHERIT
DECLARE_DROP_INHERIT
#endif
{
public:
    const std::string id;
    const std::vector<fb::model::dsl> dsl;

public:
    drop(const Json::Value& json) : 
#ifdef DECLARE_DROP_CONSTRUCTOR
DECLARE_DROP_CONSTRUCTOR
#endif
        id(fb::model::build<std::string>(json["id"])),
        dsl(fb::model::build<std::vector<fb::model::dsl>>(json["dsl"]))
    { }
    virtual ~drop()
    { }
#ifdef DECLARE_DROP_EXTENSION
DECLARE_DROP_EXTENSION
#endif
};
class map
#ifdef DECLARE_MAP_INHERIT
DECLARE_MAP_INHERIT
#endif
{
public:
    const uint32_t id;
    const std::string name;
    const uint32_t root;
    const uint16_t bgm;
    const uint8_t host;
    const bool hunting_ground;
    const bool enable_die_penalty;
    const bool enable_talk;
    const bool building;
    const bool enable_whisper;
    const bool enable_pk;
    const bool enable_spell;

public:
    map(const Json::Value& json) : 
#ifdef DECLARE_MAP_CONSTRUCTOR
DECLARE_MAP_CONSTRUCTOR
#endif
        id(fb::model::build<uint32_t>(json["id"])),
        name(fb::model::build<std::string>(json["name"])),
        root(fb::model::build<uint32_t>(json["root"])),
        bgm(fb::model::build<uint16_t>(json["bgm"])),
        host(fb::model::build<uint8_t>(json["host"])),
        hunting_ground(fb::model::build<bool>(json["hunting_ground"])),
        enable_die_penalty(fb::model::build<bool>(json["enable_die_penalty"])),
        enable_talk(fb::model::build<bool>(json["enable_talk"])),
        building(fb::model::build<bool>(json["building"])),
        enable_whisper(fb::model::build<bool>(json["enable_whisper"])),
        enable_pk(fb::model::build<bool>(json["enable_pk"])),
        enable_spell(fb::model::build<bool>(json["enable_spell"]))
    { }
    virtual ~map()
    { }
#ifdef DECLARE_MAP_EXTENSION
DECLARE_MAP_EXTENSION
#endif
};
class mob_spawn
#ifdef DECLARE_MOB_SPAWN_INHERIT
DECLARE_MOB_SPAWN_INHERIT
#endif
{
public:
    const uint32_t parent;
    const uint32_t x0;
    const uint32_t x1;
    const uint32_t y0;
    const uint32_t y1;
    const uint32_t count;
    const uint32_t mob;
    const std::chrono::milliseconds rezen;

public:
    mob_spawn(const Json::Value& json) : 
#ifdef DECLARE_MOB_SPAWN_CONSTRUCTOR
DECLARE_MOB_SPAWN_CONSTRUCTOR
#endif
        parent(fb::model::build<uint32_t>(json["parent"])),
        x0(fb::model::build<uint32_t>(json["x0"])),
        x1(fb::model::build<uint32_t>(json["x1"])),
        y0(fb::model::build<uint32_t>(json["y0"])),
        y1(fb::model::build<uint32_t>(json["y1"])),
        count(fb::model::build<uint32_t>(json["count"])),
        mob(fb::model::build<uint32_t>(json["mob"])),
        rezen(fb::model::build<std::chrono::milliseconds>(json["rezen"]))
    { }
    virtual ~mob_spawn()
    { }
#ifdef DECLARE_MOB_SPAWN_EXTENSION
DECLARE_MOB_SPAWN_EXTENSION
#endif
};
class mob_spawn_attribute
#ifdef DECLARE_MOB_SPAWN_ATTRIBUTE_INHERIT
DECLARE_MOB_SPAWN_ATTRIBUTE_INHERIT
#endif
{
public:
    const uint32_t id;

public:
    mob_spawn_attribute(const Json::Value& json) : 
#ifdef DECLARE_MOB_SPAWN_ATTRIBUTE_CONSTRUCTOR
DECLARE_MOB_SPAWN_ATTRIBUTE_CONSTRUCTOR
#endif
        id(fb::model::build<uint32_t>(json["id"]))
    { }
    virtual ~mob_spawn_attribute()
    { }
#ifdef DECLARE_MOB_SPAWN_ATTRIBUTE_EXTENSION
DECLARE_MOB_SPAWN_ATTRIBUTE_EXTENSION
#endif
};
class npc_spawn
#ifdef DECLARE_NPC_SPAWN_INHERIT
DECLARE_NPC_SPAWN_INHERIT
#endif
{
public:
    const uint32_t parent;
    const uint32_t npc;
    const uint32_t x;
    const uint32_t y;
    const fb::model::enum_value::DIRECTION direction;

public:
    npc_spawn(const Json::Value& json) : 
#ifdef DECLARE_NPC_SPAWN_CONSTRUCTOR
DECLARE_NPC_SPAWN_CONSTRUCTOR
#endif
        parent(fb::model::build<uint32_t>(json["parent"])),
        npc(fb::model::build<uint32_t>(json["npc"])),
        x(fb::model::build<uint32_t>(json["x"])),
        y(fb::model::build<uint32_t>(json["y"])),
        direction(fb::model::build<fb::model::enum_value::DIRECTION>(json["direction"]))
    { }
    virtual ~npc_spawn()
    { }
#ifdef DECLARE_NPC_SPAWN_EXTENSION
DECLARE_NPC_SPAWN_EXTENSION
#endif
};
class npc_spawn_attribute
#ifdef DECLARE_NPC_SPAWN_ATTRIBUTE_INHERIT
DECLARE_NPC_SPAWN_ATTRIBUTE_INHERIT
#endif
{
public:
    const uint32_t map;

public:
    npc_spawn_attribute(const Json::Value& json) : 
#ifdef DECLARE_NPC_SPAWN_ATTRIBUTE_CONSTRUCTOR
DECLARE_NPC_SPAWN_ATTRIBUTE_CONSTRUCTOR
#endif
        map(fb::model::build<uint32_t>(json["map"]))
    { }
    virtual ~npc_spawn_attribute()
    { }
#ifdef DECLARE_NPC_SPAWN_ATTRIBUTE_EXTENSION
DECLARE_NPC_SPAWN_ATTRIBUTE_EXTENSION
#endif
};
class object
#ifdef DECLARE_OBJECT_INHERIT
DECLARE_OBJECT_INHERIT
#endif
{
public:
    const std::string name;
    const uint16_t look;
    const uint8_t color;

public:
    object(const Json::Value& json) : 
#ifdef DECLARE_OBJECT_CONSTRUCTOR
DECLARE_OBJECT_CONSTRUCTOR
#endif
        name(fb::model::build<std::string>(json["name"])),
        look(fb::model::build<uint16_t>(json["look"])),
        color(fb::model::build<uint8_t>(json["color"]))
    { }
    virtual ~object()
    { }
#ifdef DECLARE_OBJECT_EXTENSION
DECLARE_OBJECT_EXTENSION
#endif
};
class promotion
#ifdef DECLARE_PROMOTION_INHERIT
DECLARE_PROMOTION_INHERIT
#endif
{
public:
    const fb::model::enum_value::CLASS parent;
    const uint8_t step;

public:
    promotion(const Json::Value& json) : 
#ifdef DECLARE_PROMOTION_CONSTRUCTOR
DECLARE_PROMOTION_CONSTRUCTOR
#endif
        parent(fb::model::build<fb::model::enum_value::CLASS>(json["parent"])),
        step(fb::model::build<uint8_t>(json["step"]))
    { }
    virtual ~promotion()
    { }
#ifdef DECLARE_PROMOTION_EXTENSION
DECLARE_PROMOTION_EXTENSION
#endif
};
class promotion_attribute
#ifdef DECLARE_PROMOTION_ATTRIBUTE_INHERIT
DECLARE_PROMOTION_ATTRIBUTE_INHERIT
#endif
{
public:
    const fb::model::enum_value::CLASS id;

public:
    promotion_attribute(const Json::Value& json) : 
#ifdef DECLARE_PROMOTION_ATTRIBUTE_CONSTRUCTOR
DECLARE_PROMOTION_ATTRIBUTE_CONSTRUCTOR
#endif
        id(fb::model::build<fb::model::enum_value::CLASS>(json["id"]))
    { }
    virtual ~promotion_attribute()
    { }
#ifdef DECLARE_PROMOTION_ATTRIBUTE_EXTENSION
DECLARE_PROMOTION_ATTRIBUTE_EXTENSION
#endif
};
class reward
#ifdef DECLARE_REWARD_INHERIT
DECLARE_REWARD_INHERIT
#endif
{
public:
    const std::string id;
    const std::vector<fb::model::dsl> dsl;

public:
    reward(const Json::Value& json) : 
#ifdef DECLARE_REWARD_CONSTRUCTOR
DECLARE_REWARD_CONSTRUCTOR
#endif
        id(fb::model::build<std::string>(json["id"])),
        dsl(fb::model::build<std::vector<fb::model::dsl>>(json["dsl"]))
    { }
    virtual ~reward()
    { }
#ifdef DECLARE_REWARD_EXTENSION
DECLARE_REWARD_EXTENSION
#endif
};
class sell
#ifdef DECLARE_SELL_INHERIT
DECLARE_SELL_INHERIT
#endif
{
public:
    const uint32_t parent;
    const uint32_t item;
    const std::optional<uint32_t> price;

public:
    sell(const Json::Value& json) : 
#ifdef DECLARE_SELL_CONSTRUCTOR
DECLARE_SELL_CONSTRUCTOR
#endif
        parent(fb::model::build<uint32_t>(json["parent"])),
        item(fb::model::build<uint32_t>(json["item"])),
        price(fb::model::build<std::optional<uint32_t>>(json["price"]))
    { }
    virtual ~sell()
    { }
#ifdef DECLARE_SELL_EXTENSION
DECLARE_SELL_EXTENSION
#endif
};
class sell_attribute
#ifdef DECLARE_SELL_ATTRIBUTE_INHERIT
DECLARE_SELL_ATTRIBUTE_INHERIT
#endif
{
public:
    const uint32_t id;
    const std::string group;

public:
    sell_attribute(const Json::Value& json) : 
#ifdef DECLARE_SELL_ATTRIBUTE_CONSTRUCTOR
DECLARE_SELL_ATTRIBUTE_CONSTRUCTOR
#endif
        id(fb::model::build<uint32_t>(json["id"])),
        group(fb::model::build<std::string>(json["group"]))
    { }
    virtual ~sell_attribute()
    { }
#ifdef DECLARE_SELL_ATTRIBUTE_EXTENSION
DECLARE_SELL_ATTRIBUTE_EXTENSION
#endif
};
class spell
#ifdef DECLARE_SPELL_INHERIT
DECLARE_SPELL_INHERIT
#endif
{
public:
    const uint32_t id;
    const std::string name;
    const uint8_t type;
    const std::string cast;
    const std::string uncast;
    const std::string concast;
    const std::string message;

public:
    spell(const Json::Value& json) : 
#ifdef DECLARE_SPELL_CONSTRUCTOR
DECLARE_SPELL_CONSTRUCTOR
#endif
        id(fb::model::build<uint32_t>(json["id"])),
        name(fb::model::build<std::string>(json["name"])),
        type(fb::model::build<uint8_t>(json["type"])),
        cast(fb::model::build<std::string>(json["cast"])),
        uncast(fb::model::build<std::string>(json["uncast"])),
        concast(fb::model::build<std::string>(json["concast"])),
        message(fb::model::build<std::string>(json["message"]))
    { }
    virtual ~spell()
    { }
#ifdef DECLARE_SPELL_EXTENSION
DECLARE_SPELL_EXTENSION
#endif
};
class warp
#ifdef DECLARE_WARP_INHERIT
DECLARE_WARP_INHERIT
#endif
{
public:
    const uint32_t parent;
    const uint32_t x;
    const uint32_t y;
    const std::string world;
    const std::optional<uint32_t> next;
    const uint32_t next_x;
    const uint32_t next_y;
    const std::vector<fb::model::dsl> condition;

public:
    warp(const Json::Value& json) : 
#ifdef DECLARE_WARP_CONSTRUCTOR
DECLARE_WARP_CONSTRUCTOR
#endif
        parent(fb::model::build<uint32_t>(json["parent"])),
        x(fb::model::build<uint32_t>(json["x"])),
        y(fb::model::build<uint32_t>(json["y"])),
        world(fb::model::build<std::string>(json["world"])),
        next(fb::model::build<std::optional<uint32_t>>(json["next"])),
        next_x(fb::model::build<uint32_t>(json["next_x"])),
        next_y(fb::model::build<uint32_t>(json["next_y"])),
        condition(fb::model::build<std::vector<fb::model::dsl>>(json["condition"]))
    { }
    virtual ~warp()
    { }
#ifdef DECLARE_WARP_EXTENSION
DECLARE_WARP_EXTENSION
#endif
};
class warp_attribute
#ifdef DECLARE_WARP_ATTRIBUTE_INHERIT
DECLARE_WARP_ATTRIBUTE_INHERIT
#endif
{
public:
    const uint32_t map;

public:
    warp_attribute(const Json::Value& json) : 
#ifdef DECLARE_WARP_ATTRIBUTE_CONSTRUCTOR
DECLARE_WARP_ATTRIBUTE_CONSTRUCTOR
#endif
        map(fb::model::build<uint32_t>(json["map"]))
    { }
    virtual ~warp_attribute()
    { }
#ifdef DECLARE_WARP_ATTRIBUTE_EXTENSION
DECLARE_WARP_ATTRIBUTE_EXTENSION
#endif
};
class world
#ifdef DECLARE_WORLD_INHERIT
DECLARE_WORLD_INHERIT
#endif
{
public:
    const uint16_t parent;
    const std::string group;

public:
    world(const Json::Value& json) : 
#ifdef DECLARE_WORLD_CONSTRUCTOR
DECLARE_WORLD_CONSTRUCTOR
#endif
        parent(fb::model::build<uint16_t>(json["parent"])),
        group(fb::model::build<std::string>(json["group"]))
    { }
    virtual ~world()
    { }
#ifdef DECLARE_WORLD_EXTENSION
DECLARE_WORLD_EXTENSION
#endif
};
class world_attribute
#ifdef DECLARE_WORLD_ATTRIBUTE_INHERIT
DECLARE_WORLD_ATTRIBUTE_INHERIT
#endif
{
public:
    const uint16_t id;
    const std::string name;

public:
    world_attribute(const Json::Value& json) : 
#ifdef DECLARE_WORLD_ATTRIBUTE_CONSTRUCTOR
DECLARE_WORLD_ATTRIBUTE_CONSTRUCTOR
#endif
        id(fb::model::build<uint16_t>(json["id"])),
        name(fb::model::build<std::string>(json["name"]))
    { }
    virtual ~world_attribute()
    { }
#ifdef DECLARE_WORLD_ATTRIBUTE_EXTENSION
DECLARE_WORLD_ATTRIBUTE_EXTENSION
#endif
};
class world_group
#ifdef DECLARE_WORLD_GROUP_INHERIT
DECLARE_WORLD_GROUP_INHERIT
#endif
{
public:
    const std::string parent;
    const std::string id;
    const std::string name;
    const uint32_t map;
    const uint32_t x;
    const uint32_t y;
    const uint16_t world_x;
    const uint16_t world_y;

public:
    world_group(const Json::Value& json) : 
#ifdef DECLARE_WORLD_GROUP_CONSTRUCTOR
DECLARE_WORLD_GROUP_CONSTRUCTOR
#endif
        parent(fb::model::build<std::string>(json["parent"])),
        id(fb::model::build<std::string>(json["id"])),
        name(fb::model::build<std::string>(json["name"])),
        map(fb::model::build<uint32_t>(json["map"])),
        x(fb::model::build<uint32_t>(json["x"])),
        y(fb::model::build<uint32_t>(json["y"])),
        world_x(fb::model::build<uint16_t>(json["world_x"])),
        world_y(fb::model::build<uint16_t>(json["world_y"]))
    { }
    virtual ~world_group()
    { }
#ifdef DECLARE_WORLD_GROUP_EXTENSION
DECLARE_WORLD_GROUP_EXTENSION
#endif
};
class world_group_attribute
#ifdef DECLARE_WORLD_GROUP_ATTRIBUTE_INHERIT
DECLARE_WORLD_GROUP_ATTRIBUTE_INHERIT
#endif
{
public:
    const std::string id;

public:
    world_group_attribute(const Json::Value& json) : 
#ifdef DECLARE_WORLD_GROUP_ATTRIBUTE_CONSTRUCTOR
DECLARE_WORLD_GROUP_ATTRIBUTE_CONSTRUCTOR
#endif
        id(fb::model::build<std::string>(json["id"]))
    { }
    virtual ~world_group_attribute()
    { }
#ifdef DECLARE_WORLD_GROUP_ATTRIBUTE_EXTENSION
DECLARE_WORLD_GROUP_ATTRIBUTE_EXTENSION
#endif
};
class item : public fb::model::object
{
public:
    const uint32_t id;
    const uint32_t price;
    const std::optional<uint32_t> deposit_price;
    const bool trade;
    const fb::model::enum_value::ITEM_TYPE type;
    const std::string desc;
    const fb::model::enum_value::DEATH_PENALTY death_penalty;
    const uint16_t capacity;
    const std::string script_active;
    const std::vector<fb::model::dsl> condition;

public:
    item(const Json::Value& json) : fb::model::object(json),
        id(fb::model::build<uint32_t>(json["id"])),
        price(fb::model::build<uint32_t>(json["price"])),
        deposit_price(fb::model::build<std::optional<uint32_t>>(json["deposit_price"])),
        trade(fb::model::build<bool>(json["trade"])),
        type(fb::model::build<fb::model::enum_value::ITEM_TYPE>(json["type"])),
        desc(fb::model::build<std::string>(json["desc"])),
        death_penalty(fb::model::build<fb::model::enum_value::DEATH_PENALTY>(json["death_penalty"])),
        capacity(fb::model::build<uint16_t>(json["capacity"])),
        script_active(fb::model::build<std::string>(json["script_active"])),
        condition(fb::model::build<std::vector<fb::model::dsl>>(json["condition"]))
    { }
    virtual ~item()
    { }
#ifdef DECLARE_ITEM_EXTENSION
DECLARE_ITEM_EXTENSION
#endif
};
class life : public fb::model::object
{
public:
    const uint32_t hp;
    const uint32_t mp;
    const uint32_t exp;
    const int defensive_physical;
    const int defensive_magical;

public:
    life(const Json::Value& json) : fb::model::object(json),
        hp(fb::model::build<uint32_t>(json["hp"])),
        mp(fb::model::build<uint32_t>(json["mp"])),
        exp(fb::model::build<uint32_t>(json["exp"])),
        defensive_physical(fb::model::build<int>(json["defensive_physical"])),
        defensive_magical(fb::model::build<int>(json["defensive_magical"]))
    { }
    virtual ~life()
    { }
#ifdef DECLARE_LIFE_EXTENSION
DECLARE_LIFE_EXTENSION
#endif
};
class npc : public fb::model::object
{
public:
    const uint32_t id;
    const std::string script;
    const std::vector<uint32_t> sell;
    const std::vector<uint32_t> buy;
    const bool repair;
    const bool hold_money;
    const bool hold_item;
    const bool rename;

public:
    npc(const Json::Value& json) : fb::model::object(json),
        id(fb::model::build<uint32_t>(json["id"])),
        script(fb::model::build<std::string>(json["script"])),
        sell(fb::model::build<std::vector<uint32_t>>(json["sell"])),
        buy(fb::model::build<std::vector<uint32_t>>(json["buy"])),
        repair(fb::model::build<bool>(json["repair"])),
        hold_money(fb::model::build<bool>(json["hold_money"])),
        hold_item(fb::model::build<bool>(json["hold_item"])),
        rename(fb::model::build<bool>(json["rename"]))
    { }
    virtual ~npc()
    { }
#ifdef DECLARE_NPC_EXTENSION
DECLARE_NPC_EXTENSION
#endif
};
class cash : public fb::model::item
{
public:
    cash(const Json::Value& json) : fb::model::item(json)
    { }
    virtual ~cash()
    { }
#ifdef DECLARE_CASH_EXTENSION
DECLARE_CASH_EXTENSION
#endif
};
class consume : public fb::model::item
{
public:
    consume(const Json::Value& json) : fb::model::item(json)
    { }
    virtual ~consume()
    { }
#ifdef DECLARE_CONSUME_EXTENSION
DECLARE_CONSUME_EXTENSION
#endif
};
class equipment : public fb::model::item
{
public:
    const uint16_t dress;
    const uint16_t durability;
    const std::optional<double> repair;
    const std::string dress_script;
    const std::string undress_script;
    const uint8_t hit;
    const uint8_t damage;
    const uint8_t strength;
    const uint8_t intelligence;
    const uint8_t dexteritry;
    const uint32_t base_hp;
    const uint32_t base_mp;
    const float hp_percentage;
    const float mp_percentage;
    const uint8_t healing_cycle;
    const int16_t defensive_physical;
    const int16_t defensive_magical;

public:
    equipment(const Json::Value& json) : fb::model::item(json),
        dress(fb::model::build<uint16_t>(json["dress"])),
        durability(fb::model::build<uint16_t>(json["durability"])),
        repair(fb::model::build<std::optional<double>>(json["repair"])),
        dress_script(fb::model::build<std::string>(json["dress_script"])),
        undress_script(fb::model::build<std::string>(json["undress_script"])),
        hit(fb::model::build<uint8_t>(json["hit"])),
        damage(fb::model::build<uint8_t>(json["damage"])),
        strength(fb::model::build<uint8_t>(json["strength"])),
        intelligence(fb::model::build<uint8_t>(json["intelligence"])),
        dexteritry(fb::model::build<uint8_t>(json["dexteritry"])),
        base_hp(fb::model::build<uint32_t>(json["base_hp"])),
        base_mp(fb::model::build<uint32_t>(json["base_mp"])),
        hp_percentage(fb::model::build<float>(json["hp_percentage"])),
        mp_percentage(fb::model::build<float>(json["mp_percentage"])),
        healing_cycle(fb::model::build<uint8_t>(json["healing_cycle"])),
        defensive_physical(fb::model::build<int16_t>(json["defensive_physical"])),
        defensive_magical(fb::model::build<int16_t>(json["defensive_magical"]))
    { }
    virtual ~equipment()
    { }
#ifdef DECLARE_EQUIPMENT_EXTENSION
DECLARE_EQUIPMENT_EXTENSION
#endif
};
class mob : public fb::model::life
{
public:
    const uint32_t id;
    const fb::model::enum_value::MOB_SIZE size;
    const fb::model::enum_value::MOB_ATTACK_TYPE attack_type;
    const uint32_t damage_min;
    const uint32_t damage_max;
    const uint32_t speed;
    const std::string drop;
    const std::string attack_script;
    const std::string die_script;

public:
    mob(const Json::Value& json) : fb::model::life(json),
        id(fb::model::build<uint32_t>(json["id"])),
        size(fb::model::build<fb::model::enum_value::MOB_SIZE>(json["size"])),
        attack_type(fb::model::build<fb::model::enum_value::MOB_ATTACK_TYPE>(json["attack_type"])),
        damage_min(fb::model::build<uint32_t>(json["damage_min"])),
        damage_max(fb::model::build<uint32_t>(json["damage_max"])),
        speed(fb::model::build<uint32_t>(json["speed"])),
        drop(fb::model::build<std::string>(json["drop"])),
        attack_script(fb::model::build<std::string>(json["attack_script"])),
        die_script(fb::model::build<std::string>(json["die_script"]))
    { }
    virtual ~mob()
    { }
#ifdef DECLARE_MOB_EXTENSION
DECLARE_MOB_EXTENSION
#endif
};
class pack : public fb::model::item
{
public:
    const uint16_t durability;

public:
    pack(const Json::Value& json) : fb::model::item(json),
        durability(fb::model::build<uint16_t>(json["durability"]))
    { }
    virtual ~pack()
    { }
#ifdef DECLARE_PACK_EXTENSION
DECLARE_PACK_EXTENSION
#endif
};
class armor : public fb::model::equipment
{
public:
    armor(const Json::Value& json) : fb::model::equipment(json)
    { }
    virtual ~armor()
    { }
#ifdef DECLARE_ARMOR_EXTENSION
DECLARE_ARMOR_EXTENSION
#endif
};
class auxiliary : public fb::model::equipment
{
public:
    auxiliary(const Json::Value& json) : fb::model::equipment(json)
    { }
    virtual ~auxiliary()
    { }
#ifdef DECLARE_AUXILIARY_EXTENSION
DECLARE_AUXILIARY_EXTENSION
#endif
};
class bow : public fb::model::equipment
{
public:
    bow(const Json::Value& json) : fb::model::equipment(json)
    { }
    virtual ~bow()
    { }
#ifdef DECLARE_BOW_EXTENSION
DECLARE_BOW_EXTENSION
#endif
};
class helmet : public fb::model::equipment
{
public:
    helmet(const Json::Value& json) : fb::model::equipment(json)
    { }
    virtual ~helmet()
    { }
#ifdef DECLARE_HELMET_EXTENSION
DECLARE_HELMET_EXTENSION
#endif
};
class ring : public fb::model::equipment
{
public:
    ring(const Json::Value& json) : fb::model::equipment(json)
    { }
    virtual ~ring()
    { }
#ifdef DECLARE_RING_EXTENSION
DECLARE_RING_EXTENSION
#endif
};
class shield : public fb::model::equipment
{
public:
    shield(const Json::Value& json) : fb::model::equipment(json)
    { }
    virtual ~shield()
    { }
#ifdef DECLARE_SHIELD_EXTENSION
DECLARE_SHIELD_EXTENSION
#endif
};
class weapon : public fb::model::equipment
{
public:
    const uint32_t damage_s_min;
    const uint32_t damage_s_max;
    const uint32_t damage_l_min;
    const uint32_t damage_l_max;
    const uint16_t sound;
    const std::optional<uint32_t> spell;
    const std::optional<uint32_t> rename;

public:
    weapon(const Json::Value& json) : fb::model::equipment(json),
        damage_s_min(fb::model::build<uint32_t>(json["damage_s_min"])),
        damage_s_max(fb::model::build<uint32_t>(json["damage_s_max"])),
        damage_l_min(fb::model::build<uint32_t>(json["damage_l_min"])),
        damage_l_max(fb::model::build<uint32_t>(json["damage_l_max"])),
        sound(fb::model::build<uint16_t>(json["sound"])),
        spell(fb::model::build<std::optional<uint32_t>>(json["spell"])),
        rename(fb::model::build<std::optional<uint32_t>>(json["rename"]))
    { }
    virtual ~weapon()
    { }
#ifdef DECLARE_WEAPON_EXTENSION
DECLARE_WEAPON_EXTENSION
#endif
};

#pragma endregion

#pragma region container
template <typename T>
class hook_funcs
{
public:
    std::function<T*(const Json::Value&)> build;
};

template <typename K, typename V>
class kv_container : private std::unordered_map<K, std::unique_ptr<V>>
{
public:
    class iterator;

private:
    const std::string _fname;

public:
    hook_funcs<V> hook;

public:
    using std::unordered_map<K,std::unique_ptr<V>>::size;

protected:
    kv_container(const std::string& fname) : _fname(fname)
    { }

public:
    kv_container(const Json::Value& json)
    {
        this->load(json);
    }
    ~kv_container() = default;

private:
    void load(const std::string& fname)
    {
        Json::Value             json;
        std::ifstream           ifstream;
        ifstream.open(fname);
        if (ifstream.is_open() == false)
            throw std::runtime_error("cannot find json file");

        Json::Reader            reader;
        if (reader.parse(ifstream, json) == false)
        {
            ifstream.close();
            throw std::runtime_error("cannot parse json file");
        }

        ifstream.close();
        this->load(json);
    }
    void load(const Json::Value& json)
    {
        for (auto i = json.begin(); i != json.end(); i++)
        {
            auto ptr = std::unique_ptr<V>();
            if (this->hook.build != nullptr)
                ptr.reset(this->hook.build(*i));
            if (ptr == nullptr)
                ptr = fb::model::build<std::unique_ptr<V>>(*i);

            std::unordered_map<K, std::unique_ptr<V>>::insert({ fb::model::build<K>(i.key()), std::move(ptr)});
        }
    }

public:
    void load()
    {
        this->load(this->_fname);
    }

    const V* find(const K& k) const
    {
        auto i = std::unordered_map<K, std::unique_ptr<V>>::find(k);
        if (i == std::unordered_map<K, std::unique_ptr<V>>::cend())
            return nullptr;

        return &i->second;
    }

    const V& operator [] (const K& k) const 
    {
        auto found = this->find(k);
        if(found == nullptr)
            throw std::runtime_error("does not exists");

        return *found;
    }

    iterator begin();
    iterator end();
    const iterator cbegin() const;
    const iterator cend() const;
};

template <typename K, typename V>
class kv_container<K,V>::iterator : public std::unordered_map<K, std::unique_ptr<V>>::iterator
{
private:
    std::unique_ptr<std::pair<K, V&>> _pair;

public:
    iterator(const typename std::unordered_map<K, std::unique_ptr<V>>::iterator& i, const std::unordered_map<K, std::unique_ptr<V>>* container) : std::unordered_map<K, std::unique_ptr<V>>::iterator(i)
    {
        if(i != container->end())
            this->_pair = std::make_unique<std::pair<K,V&>>(i->first, *i->second);
    }
    ~iterator() = default;

public:
    std::pair<K,V&>& operator * ()
    {
        return *this->_pair;
    }
    const std::pair<K,V&>& operator * () const
    {
        return *this->_pair;
    }
};

template <typename K, typename V>
kv_container<K,V>::iterator kv_container<K,V>::begin()
{
    return kv_container<K,V>::iterator(std::unordered_map<K, std::unique_ptr<V>>::begin(), this);
}

template <typename K, typename V>
kv_container<K,V>::iterator kv_container<K,V>::end()
{
    return kv_container<K,V>::iterator(std::unordered_map<K, std::unique_ptr<V>>::end(), this);
}

template <typename K, typename V>
const typename kv_container<K,V>::iterator kv_container<K,V>::cbegin() const
{
    return kv_container<K,V>::iterator(std::unordered_map<K, std::unique_ptr<V>>::cbegin(), this);
}

template <typename K, typename V>
const typename kv_container<K,V>::iterator kv_container<K,V>::cend() const
{
    return kv_container<K,V>::iterator(std::unordered_map<K, std::unique_ptr<V>>::cend(), this);
}

template <typename T>
class array_container : private std::vector<std::unique_ptr<T>>
{
public:
    class iterator;

private:
    const std::string _fname;

public:
    hook_funcs<T> hook;

public:
    using std::vector<std::unique_ptr<T>>::size;

protected:
    array_container(const std::string& fname) : _fname(fname)
    { }

public:
    array_container(const Json::Value& json)
    {
        this->load(json);
    }
    ~array_container() = default;

private:
    void load(const std::string& fname)
    {
        Json::Value             json;
        std::ifstream           ifstream;
        ifstream.open(fname);
        if (ifstream.is_open() == false)
            throw std::runtime_error("cannot find json file");

        Json::Reader            reader;
        if (reader.parse(ifstream, json) == false)
        {
            ifstream.close();
            throw std::runtime_error("cannot parse json file");
        }

        ifstream.close();
        this->load(json);
    }
    void load(const Json::Value& json)
    {
        for (auto i = json.begin(); i != json.end(); i++)
        {
            auto ptr = std::unique_ptr<T>();
            if (this->hook.build != nullptr)
                ptr.reset(this->hook.build(*i));
            if (ptr == nullptr)
                ptr = fb::model::build<std::unique_ptr<T>>(*i);

            std::vector<std::unique_ptr<T>>::push_back(std::move(ptr));
        }
    }

public:
    void load()
    {
        this->load(this->_fname);
    }

    const T* find(uint32_t i) const
    {
        if (i > std::vector<std::unique_ptr<T>>::size() - 1)
            return nullptr;

        return &std::vector<std::unique_ptr<T>>::at(i);
    }

    const T& operator [] (uint32_t i) const
    {
        auto found = this->find(i);
        if (found == nullptr)
            throw std::out_of_range("out of range exception");

        return *found;
    }

public:
    iterator begin();
    iterator end();
    const iterator cbegin() const;
    const iterator cend() const;
};

template <typename T>
class array_container<T>::iterator : public std::vector<std::unique_ptr<std::unique_ptr<T>>>
{
public:
    iterator(const typename std::vector<std::unique_ptr<std::unique_ptr<T>>>::iterator& i) : std::vector<std::unique_ptr<std::unique_ptr<T>>>::iterator(i)
    {}
    ~iterator() = default;

public:
    T& operator * ()
    {
        return std::vector<std::unique_ptr<T>>::iterator::operator*();
    }

    const T& operator * () const
    {
        return std::vector<std::unique_ptr<T>>::iterator::operator*();
    }
};

template <typename T>
array_container<T>::iterator array_container<T>::begin()
{
    return array_container<T>::iterator(std::vector<std::unique_ptr<std::unique_ptr<T>>>::begin());
}

template <typename T>
array_container<T>::iterator array_container<T>::end()
{
    return array_container<T>::iterator(std::vector<std::unique_ptr<std::unique_ptr<T>>>::end());
}

template <typename T>
const typename array_container<T>::iterator array_container<T>::cbegin() const
{
    return array_container<T>::iterator(std::vector<std::unique_ptr<std::unique_ptr<T>>>::cbegin());
}

template <typename T>
const typename array_container<T>::iterator array_container<T>::cend() const
{
    return array_container<T>::iterator(std::vector<std::unique_ptr<std::unique_ptr<T>>>::cend());
}

class __ability : public fb::model::kv_container<fb::model::enum_value::CLASS, fb::model::kv_container<uint8_t, fb::model::ability>>
{
public:
    __ability() : fb::model::kv_container<fb::model::enum_value::CLASS, fb::model::kv_container<uint8_t, fb::model::ability>>(std::string("json/ability.json"))
    { }
    ~__ability()
    { }
};

class __ability_attribute : public fb::model::kv_container<fb::model::enum_value::CLASS, fb::model::ability_attribute>
{
public:
    __ability_attribute() : fb::model::kv_container<fb::model::enum_value::CLASS, fb::model::ability_attribute>(std::string("json/ability_attribute.json"))
    { }
    ~__ability_attribute()
    { }
};

class __armor : public fb::model::kv_container<uint32_t, fb::model::armor>
{
public:
    __armor() : fb::model::kv_container<uint32_t, fb::model::armor>(std::string("json/armor.json"))
    { }
    ~__armor()
    { }
};

class __auxiliary : public fb::model::kv_container<uint32_t, fb::model::auxiliary>
{
public:
    __auxiliary() : fb::model::kv_container<uint32_t, fb::model::auxiliary>(std::string("json/auxiliary.json"))
    { }
    ~__auxiliary()
    { }
};

class __board : public fb::model::kv_container<uint32_t, fb::model::board>
{
public:
    __board() : fb::model::kv_container<uint32_t, fb::model::board>(std::string("json/board.json"))
    { }
    ~__board()
    { }
};

class __bow : public fb::model::kv_container<uint32_t, fb::model::bow>
{
public:
    __bow() : fb::model::kv_container<uint32_t, fb::model::bow>(std::string("json/bow.json"))
    { }
    ~__bow()
    { }
};

class __buy : public fb::model::kv_container<uint32_t, fb::model::kv_container<uint32_t, fb::model::buy>>
{
public:
    __buy() : fb::model::kv_container<uint32_t, fb::model::kv_container<uint32_t, fb::model::buy>>(std::string("json/buy.json"))
    { }
    ~__buy()
    { }
};

class __buy_attribute : public fb::model::kv_container<uint32_t, fb::model::buy_attribute>
{
public:
    __buy_attribute() : fb::model::kv_container<uint32_t, fb::model::buy_attribute>(std::string("json/buy_attribute.json"))
    { }
    ~__buy_attribute()
    { }
};

class __cash : public fb::model::kv_container<uint32_t, fb::model::cash>
{
public:
    __cash() : fb::model::kv_container<uint32_t, fb::model::cash>(std::string("json/cash.json"))
    { }
    ~__cash()
    { }
};

class __combine : public fb::model::array_container<fb::model::combine>
{
public:
    __combine() : fb::model::array_container<fb::model::combine>(std::string("json/combine.json"))
    { }
    ~__combine()
    { }
};

class __consume : public fb::model::kv_container<uint32_t, fb::model::consume>
{
public:
    __consume() : fb::model::kv_container<uint32_t, fb::model::consume>(std::string("json/consume.json"))
    { }
    ~__consume()
    { }
};

class __door : public fb::model::kv_container<uint32_t, fb::model::door>
{
public:
    __door() : fb::model::kv_container<uint32_t, fb::model::door>(std::string("json/door.json"))
    { }
    ~__door()
    { }
};

class __door_pair : public fb::model::kv_container<uint32_t, fb::model::door_pair>
{
public:
    __door_pair() : fb::model::kv_container<uint32_t, fb::model::door_pair>(std::string("json/door_pair.json"))
    { }
    ~__door_pair()
    { }
};

class __drop : public fb::model::kv_container<std::string, fb::model::drop>
{
public:
    __drop() : fb::model::kv_container<std::string, fb::model::drop>(std::string("json/drop.json"))
    { }
    ~__drop()
    { }
};

class __equipment : public fb::model::kv_container<uint32_t, fb::model::equipment>
{
public:
    __equipment() : fb::model::kv_container<uint32_t, fb::model::equipment>(std::string("json/equipment.json"))
    { }
    ~__equipment()
    { }
};

class __helmet : public fb::model::kv_container<uint32_t, fb::model::helmet>
{
public:
    __helmet() : fb::model::kv_container<uint32_t, fb::model::helmet>(std::string("json/helmet.json"))
    { }
    ~__helmet()
    { }
};

class __item : public fb::model::kv_container<uint32_t, fb::model::item>
{
public:
    __item() : fb::model::kv_container<uint32_t, fb::model::item>(std::string("json/item.json"))
    { }
    ~__item()
    { }
};

class __life : public fb::model::array_container<fb::model::life>
{
public:
    __life() : fb::model::array_container<fb::model::life>(std::string("json/life.json"))
    { }
    ~__life()
    { }
};

class __map : public fb::model::kv_container<uint32_t, fb::model::map>
{
public:
    __map() : fb::model::kv_container<uint32_t, fb::model::map>(std::string("json/map.json"))
    { }
    ~__map()
    { }
};

class __mob : public fb::model::kv_container<uint32_t, fb::model::mob>
{
public:
    __mob() : fb::model::kv_container<uint32_t, fb::model::mob>(std::string("json/mob.json"))
    { }
    ~__mob()
    { }
};

class __mob_spawn : public fb::model::kv_container<uint32_t, fb::model::array_container<fb::model::mob_spawn>>
{
public:
    __mob_spawn() : fb::model::kv_container<uint32_t, fb::model::array_container<fb::model::mob_spawn>>(std::string("json/mob_spawn.json"))
    { }
    ~__mob_spawn()
    { }
};

class __mob_spawn_attribute : public fb::model::kv_container<uint32_t, fb::model::mob_spawn_attribute>
{
public:
    __mob_spawn_attribute() : fb::model::kv_container<uint32_t, fb::model::mob_spawn_attribute>(std::string("json/mob_spawn_attribute.json"))
    { }
    ~__mob_spawn_attribute()
    { }
};

class __npc : public fb::model::kv_container<uint32_t, fb::model::npc>
{
public:
    __npc() : fb::model::kv_container<uint32_t, fb::model::npc>(std::string("json/npc.json"))
    { }
    ~__npc()
    { }
};

class __npc_spawn : public fb::model::kv_container<uint32_t, fb::model::array_container<fb::model::npc_spawn>>
{
public:
    __npc_spawn() : fb::model::kv_container<uint32_t, fb::model::array_container<fb::model::npc_spawn>>(std::string("json/npc_spawn.json"))
    { }
    ~__npc_spawn()
    { }
};

class __npc_spawn_attribute : public fb::model::kv_container<uint32_t, fb::model::npc_spawn_attribute>
{
public:
    __npc_spawn_attribute() : fb::model::kv_container<uint32_t, fb::model::npc_spawn_attribute>(std::string("json/npc_spawn_attribute.json"))
    { }
    ~__npc_spawn_attribute()
    { }
};

class __object : public fb::model::array_container<fb::model::object>
{
public:
    __object() : fb::model::array_container<fb::model::object>(std::string("json/object.json"))
    { }
    ~__object()
    { }
};

class __pack : public fb::model::kv_container<uint32_t, fb::model::pack>
{
public:
    __pack() : fb::model::kv_container<uint32_t, fb::model::pack>(std::string("json/pack.json"))
    { }
    ~__pack()
    { }
};

class __promotion : public fb::model::kv_container<fb::model::enum_value::CLASS, fb::model::kv_container<uint8_t, fb::model::promotion>>
{
public:
    __promotion() : fb::model::kv_container<fb::model::enum_value::CLASS, fb::model::kv_container<uint8_t, fb::model::promotion>>(std::string("json/promotion.json"))
    { }
    ~__promotion()
    { }
};

class __promotion_attribute : public fb::model::kv_container<fb::model::enum_value::CLASS, fb::model::promotion_attribute>
{
public:
    __promotion_attribute() : fb::model::kv_container<fb::model::enum_value::CLASS, fb::model::promotion_attribute>(std::string("json/promotion_attribute.json"))
    { }
    ~__promotion_attribute()
    { }
};

class __reward : public fb::model::kv_container<std::string, fb::model::reward>
{
public:
    __reward() : fb::model::kv_container<std::string, fb::model::reward>(std::string("json/reward.json"))
    { }
    ~__reward()
    { }
};

class __ring : public fb::model::kv_container<uint32_t, fb::model::ring>
{
public:
    __ring() : fb::model::kv_container<uint32_t, fb::model::ring>(std::string("json/ring.json"))
    { }
    ~__ring()
    { }
};

class __sell : public fb::model::kv_container<uint32_t, fb::model::kv_container<uint32_t, fb::model::sell>>
{
public:
    __sell() : fb::model::kv_container<uint32_t, fb::model::kv_container<uint32_t, fb::model::sell>>(std::string("json/sell.json"))
    { }
    ~__sell()
    { }
};

class __sell_attribute : public fb::model::kv_container<uint32_t, fb::model::sell_attribute>
{
public:
    __sell_attribute() : fb::model::kv_container<uint32_t, fb::model::sell_attribute>(std::string("json/sell_attribute.json"))
    { }
    ~__sell_attribute()
    { }
};

class __shield : public fb::model::kv_container<uint32_t, fb::model::shield>
{
public:
    __shield() : fb::model::kv_container<uint32_t, fb::model::shield>(std::string("json/shield.json"))
    { }
    ~__shield()
    { }
};

class __spell : public fb::model::kv_container<uint32_t, fb::model::spell>
{
public:
    __spell() : fb::model::kv_container<uint32_t, fb::model::spell>(std::string("json/spell.json"))
    { }
    ~__spell()
    { }
};

class __warp : public fb::model::kv_container<uint32_t, fb::model::array_container<fb::model::warp>>
{
public:
    __warp() : fb::model::kv_container<uint32_t, fb::model::array_container<fb::model::warp>>(std::string("json/warp.json"))
    { }
    ~__warp()
    { }
};

class __warp_attribute : public fb::model::kv_container<uint32_t, fb::model::warp_attribute>
{
public:
    __warp_attribute() : fb::model::kv_container<uint32_t, fb::model::warp_attribute>(std::string("json/warp_attribute.json"))
    { }
    ~__warp_attribute()
    { }
};

class __weapon : public fb::model::kv_container<uint32_t, fb::model::weapon>
{
public:
    __weapon() : fb::model::kv_container<uint32_t, fb::model::weapon>(std::string("json/weapon.json"))
    { }
    ~__weapon()
    { }
};

class __world : public fb::model::kv_container<uint16_t, fb::model::array_container<fb::model::world>>
{
public:
    __world() : fb::model::kv_container<uint16_t, fb::model::array_container<fb::model::world>>(std::string("json/world.json"))
    { }
    ~__world()
    { }
};

class __world_attribute : public fb::model::kv_container<uint16_t, fb::model::world_attribute>
{
public:
    __world_attribute() : fb::model::kv_container<uint16_t, fb::model::world_attribute>(std::string("json/world_attribute.json"))
    { }
    ~__world_attribute()
    { }
};

class __world_group : public fb::model::kv_container<std::string, fb::model::kv_container<std::string, fb::model::world_group>>
{
public:
    __world_group() : fb::model::kv_container<std::string, fb::model::kv_container<std::string, fb::model::world_group>>(std::string("json/world_group.json"))
    { }
    ~__world_group()
    { }
};

class __world_group_attribute : public fb::model::kv_container<std::string, fb::model::world_group_attribute>
{
public:
    __world_group_attribute() : fb::model::kv_container<std::string, fb::model::world_group_attribute>(std::string("json/world_group_attribute.json"))
    { }
    ~__world_group_attribute()
    { }
};


class container
{
public:
    fb::model::__ability ability;
    fb::model::__ability_attribute ability_attribute;
    fb::model::__armor armor;
    fb::model::__auxiliary auxiliary;
    fb::model::__board board;
    fb::model::__bow bow;
    fb::model::__buy buy;
    fb::model::__buy_attribute buy_attribute;
    fb::model::__cash cash;
    fb::model::__combine combine;
    fb::model::__consume consume;
    fb::model::__door door;
    fb::model::__door_pair door_pair;
    fb::model::__drop drop;
    fb::model::__equipment equipment;
    fb::model::__helmet helmet;
    fb::model::__item item;
    fb::model::__life life;
    fb::model::__map map;
    fb::model::__mob mob;
    fb::model::__mob_spawn mob_spawn;
    fb::model::__mob_spawn_attribute mob_spawn_attribute;
    fb::model::__npc npc;
    fb::model::__npc_spawn npc_spawn;
    fb::model::__npc_spawn_attribute npc_spawn_attribute;
    fb::model::__object object;
    fb::model::__pack pack;
    fb::model::__promotion promotion;
    fb::model::__promotion_attribute promotion_attribute;
    fb::model::__reward reward;
    fb::model::__ring ring;
    fb::model::__sell sell;
    fb::model::__sell_attribute sell_attribute;
    fb::model::__shield shield;
    fb::model::__spell spell;
    fb::model::__warp warp;
    fb::model::__warp_attribute warp_attribute;
    fb::model::__weapon weapon;
    fb::model::__world world;
    fb::model::__world_attribute world_attribute;
    fb::model::__world_group world_group;
    fb::model::__world_group_attribute world_group_attribute;

public:
    void load(const std::function<void(float)>& callback)
    {
        std::queue<std::function<void()>> queue;
#pragma region enqueue
        queue.push([this]() { this->ability.load(); });
        queue.push([this]() { this->ability_attribute.load(); });
        queue.push([this]() { this->armor.load(); });
        queue.push([this]() { this->auxiliary.load(); });
        queue.push([this]() { this->board.load(); });
        queue.push([this]() { this->bow.load(); });
        queue.push([this]() { this->buy.load(); });
        queue.push([this]() { this->buy_attribute.load(); });
        queue.push([this]() { this->cash.load(); });
        queue.push([this]() { this->combine.load(); });
        queue.push([this]() { this->consume.load(); });
        queue.push([this]() { this->door.load(); });
        queue.push([this]() { this->door_pair.load(); });
        queue.push([this]() { this->drop.load(); });
        queue.push([this]() { this->equipment.load(); });
        queue.push([this]() { this->helmet.load(); });
        queue.push([this]() { this->item.load(); });
        queue.push([this]() { this->life.load(); });
        queue.push([this]() { this->map.load(); });
        queue.push([this]() { this->mob.load(); });
        queue.push([this]() { this->mob_spawn.load(); });
        queue.push([this]() { this->mob_spawn_attribute.load(); });
        queue.push([this]() { this->npc.load(); });
        queue.push([this]() { this->npc_spawn.load(); });
        queue.push([this]() { this->npc_spawn_attribute.load(); });
        queue.push([this]() { this->object.load(); });
        queue.push([this]() { this->pack.load(); });
        queue.push([this]() { this->promotion.load(); });
        queue.push([this]() { this->promotion_attribute.load(); });
        queue.push([this]() { this->reward.load(); });
        queue.push([this]() { this->ring.load(); });
        queue.push([this]() { this->sell.load(); });
        queue.push([this]() { this->sell_attribute.load(); });
        queue.push([this]() { this->shield.load(); });
        queue.push([this]() { this->spell.load(); });
        queue.push([this]() { this->warp.load(); });
        queue.push([this]() { this->warp_attribute.load(); });
        queue.push([this]() { this->weapon.load(); });
        queue.push([this]() { this->world.load(); });
        queue.push([this]() { this->world_attribute.load(); });
        queue.push([this]() { this->world_group.load(); });
        queue.push([this]() { this->world_group_attribute.load(); });
#pragma endregion

        auto size = queue.size();
        auto process = 0;
        auto mutex = std::mutex();
        auto fn = [&]() 
        {
            while (true)
            {
                std::function<void()> job;
                {
                    auto _ = std::lock_guard(mutex);
                    if(queue.empty())
                        break;

                    job = queue.front();
                    queue.pop();
                }
                
                job();

                {
                    auto _ = std::lock_guard(mutex);
                    auto percent = (++process * 100) / float(size);
                    callback(percent);
                }
            }
        };

        auto tasks = std::queue<std::future<void>>();
        for(int i = 0; i < std::thread::hardware_concurrency(); i++)
        {
            tasks.push(std::async(std::launch::async, fn));
        }
    }
};
#pragma endregion

#pragma region build function
template <typename T> typename std::enable_if<is_default<T>::value, T>::type build(const Json::Value& json)
{
    return T(json);
}

template <typename T> typename std::enable_if<std::is_enum<T>::value, T>::type build(const Json::Value& json)
{
    if(json.isString())
        return T(enum_value::enum_parse<T>(build<std::string>(json)));
    else
        return T(json.asInt());
}

template <typename T> typename std::enable_if<std::is_pointer<T>::value, T>::type build(const Json::Value& json)
{
    if(json.isNull())
        return nullptr;
    else
        return new typename std::pointer_traits<T>::element_type(json);
}

template <typename T> typename std::enable_if<is_vector<T>::value, T>::type build(const Json::Value& json)
{
    auto result = std::vector<typename T::value_type>();
    for (auto i = json.begin(); i != json.end(); i++)
    {
        result.push_back(build<typename T::value_type>(*i));
    }
    return result;
}

template <typename T> typename std::enable_if<is_map<T>::value, T>::type build(const Json::Value& json)
{
    return std::map<typename T::key_type, typename T::mapped_type>();
}

template <typename T> typename std::enable_if<is_optional<T>::value, T>::type build(const Json::Value& json)
{
    if (json.isNull())
        return std::nullopt;
    else
        return std::optional<typename T::value_type>(build<typename T::value_type>(json));
}

template <typename T> typename std::enable_if<is_unique<T>::value, T>::type build(const Json::Value& json)
{
    if (json.isNull())
        return std::unique_ptr<typename std::pointer_traits<T>::element_type>();
    else
        return std::unique_ptr<typename std::pointer_traits<T>::element_type>(build<typename std::pointer_traits<T>::element_type*>(json));
}

template <> int8_t build<int8_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(build<std::string>(json).c_str()) : json.asInt();
    return static_cast<int8_t>(result);
}

template <> uint8_t build<uint8_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(build<std::string>(json).c_str()) : json.asUInt();
    return static_cast<uint8_t>(result);
}

template <> int16_t build<int16_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(build<std::string>(json).c_str()) : json.asInt();
    return static_cast<int16_t>(result);
}

template <> uint16_t build<uint16_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(build<std::string>(json).c_str()) : json.asUInt();
    return static_cast<uint16_t>(result);
}

template <> int build<int>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(build<std::string>(json).c_str()) : json.asInt();
    return static_cast<int>(result);
}

template <> uint32_t build<uint32_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(build<std::string>(json).c_str()) : json.asUInt();
    return static_cast<uint32_t>(result);
}

template <> int64_t build<int64_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(build<std::string>(json).c_str()) : json.asInt64();
    return static_cast<int64_t>(result);
}

template <> uint64_t build<uint64_t>(const Json::Value& json)
{
    auto result = json.isString() ? std::atoi(build<std::string>(json).c_str()) : json.asUInt64();
    return static_cast<uint64_t>(result);
}

template <> std::string build<std::string>(const Json::Value& json)
{
    if (json.isNull())
        return "";
    else if (string_encoding_func != nullptr)
        return string_encoding_func(json.asString());
    else
        return json.asString();
}

template <> float build<float>(const Json::Value& json)
{
    return json.asFloat();
}

template <> double build<double>(const Json::Value& json)
{
    return json.asDouble();
}

template <> bool build<bool>(const Json::Value& json)
{
    return json.asBool();
}

template <> boost::posix_time::ptime build<boost::posix_time::ptime>(const Json::Value& json)
{
    try
    {
        return boost::posix_time::time_from_string(build<std::string>(json));
    }
    catch (std::exception& e)
    {
        return boost::posix_time::ptime();
    }
}

template <> std::chrono::milliseconds build<std::chrono::milliseconds>(const Json::Value& json)
{
    static const auto regex = boost::xpressive::sregex::compile("((?P<day>\\d+)\.)?(?P<hour>\\d{1,2}):(?P<min>\\d{1,2}):(?P<sec>\\d{1,2})(?:.(?P<ms>\\d+))?");
    auto what = boost::xpressive::smatch();
    auto data = build<std::string>(json);
    if (boost::xpressive::regex_match(data, what, regex) == false)
        throw std::runtime_error("cannot parse timespan");

    auto day = what["day"].matched ? std::stoi(what["day"].str()) : 0;
    auto hours = std::stoi(what["hour"].str());
    auto mins = std::stoi(what["min"].str());
    auto secs = std::stoi(what["sec"].str());
    auto ms = what["ms"].matched ? std::stoi(what["ms"].str()) : 0;

    auto count = day;
    count = count * 24 + hours;
    count = count * 60 + mins;
    count = count * 60 + secs;
    count = count * 1000 + ms;
    return std::chrono::milliseconds(count);
}

template <> dsl build<dsl>(const Json::Value& json)
{
    return dsl(json);
}

template <> date_range build<date_range>(const Json::Value& json)
{
    return date_range(build<std::optional<boost::posix_time::ptime>>(json["Begin"]), build<std::optional<boost::posix_time::ptime>>(json["End"]));
}
#pragma endregion

} }

#endif