#include <string>

#ifndef __STRING_H__
#define __STRING_H__

namespace fb { namespace login { namespace message {

    namespace account
    {
        static constexpr const char* INVALID_NAME = "이름이 길거나 적합하지 않습니다.";
        static constexpr const char* ALREADY_LOGIN = "이미 접속중입니다.";
        static constexpr const char* NOT_FOUND_NAME = "존재하지 않는 이름입니다.";
        static constexpr const char* PASSWORD_SIZE = "암호는 4자 이상 8자 이하";
        static constexpr const char* INVALID_PASSWORD = "비밀번호가 올바르지 않습니다.";
        static constexpr const char* SIMPLE_PASSWORD = "암호가 단순합니다.";
        static constexpr const char* SUCCESS_REGISTER_ACCOUNT = "등록완료, 이어하기를 선택하세요.";
        static constexpr const char* SUCCESS_CHANGE_PASSWORD = "변경됐다리";
        static constexpr const char* INVALID_BIRTHDAY = "생년월일이 올바르지 않습니다.";
        static constexpr const char* NEW_PW_EQUALIZATION = "기존 암호화 동일합니다.";
        static constexpr const char* ALREADY_EXISTS = "이미 존재하는 이름입니다.";
    }

} } }

namespace fb { namespace game { namespace message {

    namespace assets 
    {
        static constexpr const char* MAP_LOADED = " * [{:3.2f}] 맵 정보를 읽었습니다. ({})";
        static constexpr const char* REGEX_LOADED = " * [{:3.2f}] 정규표현식 정보를 읽었습니다. ({})";
        static constexpr const char* WORLD_MAP_LOADED = " * [{:3.2f}] 월드맵 정보를 읽었습니다. ({})";
        static constexpr const char* DOOR_LOADED = " * [{:3.2f}] 도어 정보를 읽었습니다. ({})";
        static constexpr const char* SPELL_LOADED = " * [{:3.2f}] 스펠 정보를 읽었습니다. ({})";
        static constexpr const char* WARP_LOADED = " * [{:3.2f}] 워프 정보를 읽었습니다. ({})";
        static constexpr const char* ITEM_LOADED = " * [{:3.2f}] 아이템 정보를 읽었습니다. ({})";
        static constexpr const char* ITEM_MIX_LOADED = " * [{:3.2f}] 조합 정보를 읽었습니다. ({})";
        static constexpr const char* SELL_LOADED = " * [{:3.2f}] 판매 정보를 읽었습니다. ({})";
        static constexpr const char* BUY_LOADED = " * [{:3.2f}] 구매 정보를 읽었습니다. ({})";
        static constexpr const char* NPC_LOADED = " * [{:3.2f}] NPC 정보를 읽었습니다. ({})";
        static constexpr const char* MOB_LOADED = " * [{:3.2f}] 몹 정보를 읽었습니다. ({})";
        static constexpr const char* DROP_LOADED = " * [{:3.2f}] 드롭 정보를 읽었습니다. ({})";
        static constexpr const char* NPC_SPAWN_LOADED = " * [{:3.2f}] NPC 스폰 정보를 읽었습니다. ({})";
        static constexpr const char* MOB_SPAWN_LOADED = " * [{:3.2f}] 몹 스폰 정보를 읽었습니다. ({})";
        static constexpr const char* CLASS_LOADED = " * [{:3.2f}] 클래스 정보를 읽었습니다. ({})";
        static constexpr const char* BOARD_LOADED = " * [{:3.2f}] 게시판 정보를 읽었습니다. ({})";
        static constexpr const char* MAP_ALL_LOADED = " * [100%] 총 {}개의 맵 정보를 읽었습니다.";
        static constexpr const char* REGEX_ALL_LOADED = " * [100%] 총 {}개의 정규표현식 정보를 읽었습니다.";
        static constexpr const char* WORLD_MAP_ALL_LOADED = " * [100%] 총 {}개의 월드맵 정보를 읽었습니다.";
        static constexpr const char* DOOR_ALL_LOADED = " * [100%] 총 {}개의 도어 정보를 읽었습니다.";
        static constexpr const char* SPELL_ALL_LOADED = " * [100%] 총 {}개의 스펠 정보를 읽었습니다.";
        static constexpr const char* WARP_ALL_LOADED = " * [100%] 총 {}개의 워프 정보를 읽었습니다.";
        static constexpr const char* ITEM_ALL_LOADED = " * [100%] 총 {}개의 아이템 정보를 읽었습니다.";
        static constexpr const char* ITEM_MIX_ALL_LOADED = " * [100%] 총 {}개의 조합 정보를 읽었습니다.";
        static constexpr const char* SELL_ALL_LOADED = " * [100%] 총 {}개의 판매 정보를 읽었습니다.";
        static constexpr const char* BUY_ALL_LOADED = " * [100%] 총 {}개의 구매 정보를 읽었습니다.";
        static constexpr const char* NPC_ALL_LOADED = " * [100%] 총 {}개의 NPC 정보를 읽었습니다.";
        static constexpr const char* MOB_ALL_LOADED = " * [100%] 총 {}개의 몹 정보를 읽었습니다.";
        static constexpr const char* DROP_ALL_LOADED = " * [100%] 총 {}개의 드롭 정보를 읽었습니다.";
        static constexpr const char* NPC_SPAWN_ALL_LOADED = " * [100%] 총 {}개의 NPC 스폰 정보를 읽었습니다.";
        static constexpr const char* MOB_SPAWN_ALL_LOADED = " * [100%] 총 {}개의 몹 스폰 정보를 읽었습니다.";
        static constexpr const char* CLASS_ALL_LOADED = " * [100%] 총 {}개의 클래스 정보를 읽었습니다.";
        static constexpr const char* BOARD_ALL_LOADED = " * [100%] 총 {}개의 게시판 정보를 읽었습니다.";
        static constexpr const char* INVALID_SEX = "성별을 확인할 수 없습니다.";
        static constexpr const char* INVALID_DEATH_PENALTY = "아이템 데스 패널티를 확인할 수 없습니다.";
        static constexpr const char* INVALID_MOB_SIZE = "몹 크기 형식을 확인할 수 없습니다.";
        static constexpr const char* INVALID_MOB_OFFENSIVE = "몹 공격 타입을 확인할 수 없습니다.";
        static constexpr const char* CANNOT_LOAD_MAP_DATA = "맵 데이터를 읽을 수 없습니다.";
        static constexpr const char* CANNOT_LOAD_MAP_BLOCK = "맵 블록 데이터를 읽을 수 없습니다.";
        static constexpr const char* INVALID_NPC_NAME = "존재하지 않는 NPC입니다.";
        static constexpr const char* INVALID_MAP_NAME = "존재하지 않는 맵입니다.";
        static constexpr const char* INVALID_NPC_DIRECTION = "NPC의 방향이 올바르지 않습니다.";
        static constexpr const char* INVALID_NPC_POSITION = "NPC의 위치가 올바르지 않습니다.";
        static constexpr const char* INVALID_MOB_NAME = "올바르지 않은 몹 이름입니다.";
        static constexpr const char* INVALID_ITEM_NAME = "올바르지 않은 아이템 이름입니다.";
    }

    namespace item
    {
        static constexpr const char* CANNOT_PICKUP_ANYMORE = "더 이상 가질 수 없습니다.";
    }
    
    namespace group 
    {
        static constexpr const char* CANNOT_FIND_TARGET = "대상을 찾을 수 없습니다.";
        static constexpr const char* DISABLED_MINE = "그룹 거부 상태입니다.";
        static constexpr const char* DISABLED_TARGET = "상대방이 그룹 거부 상태입니다.";
        static constexpr const char* NOT_OWNER = "그룹장만 할 수 있습니다.";
        static constexpr const char* FULL_MEMBER = "자리가 없습니다.";
        static constexpr const char* ALREADY_JOINED = "님은 이미 그룹 참여 중입니다.";
        static constexpr const char* JOINED = "님 그룹에 참여";
        static constexpr const char* LEFT = "님 그룹 탈퇴";
    }
    
    namespace exception 
    {
        static constexpr const char* RIDDING = "말을 타고는 할 수 없습니다.";
        static constexpr const char* GHOST = "귀신은 할 수 없습니다.";
        static constexpr const char* REQUIRE_CLASS = "직업을 선택해야 합니다.";
        static constexpr const char* NO_CONVEYANCE = "탈 것이 없습니다.";
        static constexpr const char* DISGUISE = "변신 중에는 할 수 없습니다.";
        static constexpr const char* CANNOT_DROP_ITEM = "버릴 수 없는 물건입니다.";
        static constexpr const char* CANNOT_THROW_ITEM = "던질 수 없는 물건입니다.";
        static constexpr const char* INVENTORY_OVERFLOW = "소지품이 꽉 찼습니다.";
        static constexpr const char* INVALID_TARGET = "대상이 올바르지 않습니다.";
    }
    
    namespace trade 
    {
        static constexpr const char* NOT_TRADING = "교환중이 아닙니다.";
        static constexpr const char* NOT_SELECTED = "선택된 아이템이 없습니다.";
        static constexpr const char* SUCCESS = "교환에 성공했습니다.";
        static constexpr const char* FAILED = "교환에 실패했습니다.";
        static constexpr const char* NOTIFY_LOCK_TO_PARTNER = "상대방이 확인을 눌렀습니다.";
        static constexpr const char* CANCELLED_BY_ME = "내가 교환을 취소했습니다.";
        static constexpr const char* CANCELLED_BY_PARTNER = "상대방이 교환을 취소했습니다.";
        static constexpr const char* INVALID_COUNT = "갯수가 올바르지 않습니다.";
        static constexpr const char* NOT_ALLOWED_TO_TRADE = "교환이 불가능한 아이템입니다.";
        static constexpr const char* REFUSED_BY_ME = "교환 거부 상태입니다.";
        static constexpr const char* REFUSED_BY_PARTNER = "님은 교환 거부 상태입니다.";
        static constexpr const char* PARTNER_ALREADY_TRADING = "님은 이미 교환 중입니다.";
        static constexpr const char* PARTNER_INVISIBLE = "대상이 보이지 않습니다.";
        static constexpr const char* PARTNER_TOO_FAR = "님과 너무 멀리 떨어져 있습니다.";
    }
    
    namespace mix 
    {
        static constexpr const char* SUCCESS = "성공하였습니다.";
        static constexpr const char* FAILED = "실패하였습니다.";
    }
    
    namespace ride 
    {
        static constexpr const char* UNRIDE = "말에 타고 있지 않습니다.";
        static constexpr const char* ALREADY_RIDE = "이미 타고 있습니다.";
        static constexpr const char* ON = "말에 탔습니다.";
        static constexpr const char* OFF = "말에서 내렸습니다.";
    }
    
    namespace money 
    {
        static constexpr const char* DROP = "돈을 버렸습니다.";
        static constexpr const char* FULL = "더 이상 돈을 가질 수 없습니다.";
    }
    
    namespace level 
    {
        static constexpr const char* UP = "레벨이 올랐습니다";
    }
    
    namespace board 
    {
        static constexpr const char* WRITE = "글을 작성하였습니다";
        static constexpr const char* ARTICLE_NOT_EXIST = "게시글이 존재하지 않습니다.";
        static constexpr const char* SECTION_NOT_EXIST = "섹션이 존재하지 않습니다.";
        static constexpr const char* NOT_AUTH = "권한이 없습니다.";
        static constexpr const char* SUCCESS_DELETE = "글이 삭제되었습니다.";
        static constexpr const char* TOO_LONG_TITLE = "게시글 제목이 너무 깁니다.";
        static constexpr const char* TOO_LONG_CONTENTS = "게시글 내용이 너무 깁니다.";
    }
    
    namespace door 
    {
        static constexpr const char* OPEN = "문을 열었습니다.";
        static constexpr const char* CLOSE = "문을 닫았습니다.";
        static constexpr const char* LOCK = "문을 잠궜습니다.";
        static constexpr const char* UNLOCK = "문을 열었습니다.";
        static constexpr const char* LOCKED = "문이 잠겼습니다.";
    }

    namespace error
    {
        static constexpr const char* UNKNOWN = "올바르지 않은 명령입니다.";
    }

} } }

#endif