#include <string>

#ifndef __STRING_H__
#define __STRING_H__

namespace fb { namespace login { namespace message {

    namespace account
    {
        static const char* INVALID_NAME = "이름이 길거나 적합하지 않습니다.";
        static const char* ALREADY_LOGIN = "이미 접속중입니다.";
        static const char* NOT_FOUND_NAME = "존재하지 않는 이름입니다.";
        static const char* PASSWORD_SIZE = "암호는 4자 이상 8자 이하";
        static const char* INVALID_PASSWORD = "비밀번호가 올바르지 않습니다.";
        static const char* SIMPLE_PASSWORD = "암호가 단순합니다.";
        static const char* SUCCESS_REGISTER_ACCOUNT = "등록완료, 이어하기를 선택하세요.";
        static const char* SUCCESS_CHANGE_PASSWORD = "변경됐다리";
        static const char* INVALID_BIRTHDAY = "생년월일이 올바르지 않습니다.";
        static const char* NEW_PW_EQUALIZATION = "기존 암호화 동일합니다.";
        static const char* ALREADY_EXISTS = "이미 존재하는 이름입니다.";
    }

} } }

namespace fb { namespace game { namespace message {

    namespace assets 
    {
        static const char* MAP_LOADED = " * [%0.2lf%%] 맵 정보를 읽었습니다. (%s)";
        static const char* DOOR_LOADED = " * [%0.2lf%%] 도어 정보를 읽었습니다. (%s)";
        static const char* SPELL_LOADED = " * [%0.2lf%%] 스펠 정보를 읽었습니다. (%s)";
        static const char* WARP_LOADED = " * [%0.2lf%%] 워프 정보를 읽었습니다. (%s)";
        static const char* ITEM_LOADED = " * [%0.2lf%%] 아이템 정보를 읽었습니다. (%s)";
        static const char* ITEM_MIX_LOADED = " * [%0.2lf%%] 조합 정보를 읽었습니다. (%s)";
        static const char* NPC_LOADED = " * [%0.2lf%%] NPC 정보를 읽었습니다. (%s)";
        static const char* MOB_LOADED = " * [%0.2lf%%] 몹 정보를 읽었습니다. (%s)";
        static const char* DROP_LOADED = " * [%0.2lf%%] 드롭 정보를 읽었습니다. (%s)";
        static const char* NPC_SPAWN_LOADED = " * [%0.2lf%%] NPC 스폰 정보를 읽었습니다. (%s)";
        static const char* MOB_SPAWN_LOADED = " * [%0.2lf%%] 몹 스폰 정보를 읽었습니다. (%s)";
        static const char* CLASS_LOADED = " * [%0.2lf%%] 클래스 정보를 읽었습니다. (%s)";
        static const char* BOARD_LOADED = " * [%0.2lf%%] 게시판 정보를 읽었습니다. (%s)";
        static const char* MAP_ALL_LOADED = " * [100%%] 총 %d개의 맵 정보를 읽었습니다.";
        static const char* DOOR_ALL_LOADED = " * [100%%] 총 %d개의 도어 정보를 읽었습니다.";
        static const char* SPELL_ALL_LOADED = " * [100%%] 총 %d개의 스펠 정보를 읽었습니다.";
        static const char* WARP_ALL_LOADED = " * [100%%] 총 %d개의 워프 정보를 읽었습니다.";
        static const char* ITEM_ALL_LOADED = " * [100%%] 총 %d개의 아이템 정보를 읽었습니다.";
        static const char* ITEM_MIX_ALL_LOADED = " * [100%%] 총 %d개의 조합 정보를 읽었습니다.";
        static const char* NPC_ALL_LOADED = " * [100%%] 총 %d개의 NPC 정보를 읽었습니다.";
        static const char* MOB_ALL_LOADED = " * [100%%] 총 %d개의 몹 정보를 읽었습니다.";
        static const char* DROP_ALL_LOADED = " * [100%%] 총 %d개의 드롭 정보를 읽었습니다.";
        static const char* NPC_SPAWN_ALL_LOADED = " * [100%%] 총 %d개의 NPC 스폰 정보를 읽었습니다.";
        static const char* MOB_SPAWN_ALL_LOADED = " * [100%%] 총 %d개의 몹 스폰 정보를 읽었습니다.";
        static const char* CLASS_ALL_LOADED = " * [100%%] 총 %d개의 클래스 정보를 읽었습니다.";
        static const char* BOARD_ALL_LOADED = " * [100%%] 총 %d개의 게시판 정보를 읽었습니다.";
        static const char* INVALID_SEX = "성별을 확인할 수 없습니다.";
        static const char* INVALID_DEATH_PENALTY = "아이템 데스 패널티를 확인할 수 없습니다.";
        static const char* INVALID_MOB_SIZE = "몹 크기 형식을 확인할 수 없습니다.";
        static const char* INVALID_MOB_OFFENSIVE = "몹 공격 타입을 확인할 수 없습니다.";
        static const char* CANNOT_LOAD_MAP_DATA = "맵 데이터를 읽을 수 없습니다.";
        static const char* CANNOT_LOAD_MAP_BLOCK = "맵 블록 데이터를 읽을 수 없습니다.";
        static const char* INVALID_NPC_NAME = "존재하지 않는 NPC입니다.";
        static const char* INVALID_MAP_NAME = "존재하지 않는 맵입니다.";
        static const char* INVALID_NPC_DIRECTION = "NPC의 방향이 올바르지 않습니다.";
        static const char* INVALID_NPC_POSITION = "NPC의 위치가 올바르지 않습니다.";
        static const char* INVALID_MOB_NAME = "올바르지 않은 몹 이름입니다.";
        static const char* INVALID_ITEM_NAME = "올바르지 않은 아이템 이름입니다.";
    }
    
    namespace group 
    {
        static const char* CANNOT_FIND_TARGET = "대상을 찾을 수 없습니다.";
        static const char* DISABLED_MINE = "그룹 거부 상태입니다.";
        static const char* DISABLED_TARGET = "상대방이 그룹 거부 상태입니다.";
        static const char* NOT_OWNER = "그룹장만 할 수 있습니다.";
        static const char* FULL_MEMBER = "자리가 없습니다.";
        static const char* ALREADY_JOINED = "님은 이미 그룹 참여 중입니다.";
        static const char* JOINED = "님 그룹에 참여";
        static const char* LEFT = "님 그룹 탈퇴";
    }
    
    namespace exception 
    {
        static const char* RIDDING = "말을 타고는 할 수 없습니다.";
        static const char* GHOST = "귀신은 할 수 없습니다.";
        static const char* REQUIRE_CLASS = "직업을 선택해야 합니다.";
        static const char* NO_CONVEYANCE = "탈 것이 없습니다.";
        static const char* DISGUISE = "변신 중에는 할 수 없습니다.";
        static const char* CANNOT_DROP_ITEM = "버릴 수 없는 물건입니다.";
        static const char* CANNOT_THROW_ITEM = "던질 수 없는 물건입니다.";
        static const char* INVENTORY_OVERFLOW = "소지품이 꽉 찼습니다.";
        static const char* INVALID_TARGET = "대상이 올바르지 않습니다.";
    }
    
    namespace trade 
    {
        static const char* NOT_TRADING = "교환중이 아닙니다.";
        static const char* NOT_SELECTED = "선택된 아이템이 없습니다.";
        static const char* SUCCESS = "교환에 성공했습니다.";
        static const char* FAILED = "교환에 실패했습니다.";
        static const char* NOTIFY_LOCK_TO_PARTNER = "상대방이 확인을 눌렀습니다.";
        static const char* CANCELLED_BY_ME = "내가 교환을 취소했습니다.";
        static const char* CANCELLED_BY_PARTNER = "상대방이 교환을 취소했습니다.";
        static const char* INVALID_COUNT = "갯수가 올바르지 않습니다.";
        static const char* NOT_ALLOWED_TO_TRADE = "교환이 불가능한 아이템입니다.";
        static const char* REFUSED_BY_ME = "교환 거부 상태입니다.";
        static const char* REFUSED_BY_PARTNER = "님은 교환 거부 상태입니다.";
        static const char* PARTNER_ALREADY_TRADING = "님은 이미 교환 중입니다.";
        static const char* PARTNER_INVISIBLE = "대상이 보이지 않습니다.";
        static const char* PARTNER_TOO_FAR = "님과 너무 멀리 떨어져 있습니다.";
    }
    
    namespace mix 
    {
        static const char* SUCCESS = "성공하였습니다.";
        static const char* FAILED = "실패하였습니다.";
    }
    
    namespace ride 
    {
        static const char* ON = "말에 탔습니다.";
        static const char* OFF = "말에서 내렸습니다.";
    }
    
    namespace money 
    {
        static const char* DROP = "돈을 버렸습니다.";
        static const char* FULL = "더 이상 돈을 가질 수 없습니다.";
    }
    
    namespace level 
    {
        static const char* UP = "레벨이 올랐습니다";
    }
    
    namespace board 
    {
        static const char* WRITE = "글을 작성하였습니다";
        static const char* ARTICLE_NOT_EXIST = "게시글이 존재하지 않습니다.";
        static const char* SECTION_NOT_EXIST = "섹션이 존재하지 않습니다.";
        static const char* NOT_AUTH = "권한이 없습니다.";
        static const char* SUCCESS_DELETE = "글이 삭제되었습니다.";
    }
    
    namespace door 
    {
        static const char* OPEN = "문을 열었습니다.";
        static const char* CLOSE = "문을 닫았습니다.";
        static const char* LOCK = "문을 잠궜습니다.";
        static const char* UNLOCK = "문을 열었습니다.";
        static const char* LOCKED = "문이 잠겼습니다.";
    }

} } }

#endif