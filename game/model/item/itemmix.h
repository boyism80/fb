#ifndef __ITEM_MIX_H__
#define __ITEM_MIX_H__

#include "model/item/item.equipment.h"

namespace fb { namespace game {

class itemmix
{
#pragma region forward nested declaration
private:
    struct element;

public:
    class builder;
#pragma endregion

#pragma region type definition
public:
    DECLARE_EXCEPTION(no_match_exception, "조합할 수 없습니다.")
#pragma endregion

#pragma region public field
public:
    std::vector<element>                require;   // 재료 아이템
    std::vector<element>                success;   // 성공시 얻는 아이템
    std::vector<element>                failed;    // 실패시 얻는 아이템
    float                               percentage;
#pragma endregion

#pragma region constructor / destructor
public:
    itemmix(float percentage = 100.0f) : percentage(percentage) { }
    itemmix(const class itemmix& right) : 
        require(right.require.begin(), right.require.end()),
        success(right.success.begin(), right.success.end()),
        failed(right.failed.begin(), right.failed.end()),
        percentage(right.percentage)
    { }
#pragma endregion

#pragma region private method
private:
    bool                                contains(const item* item) const;
#pragma endregion

#pragma region public method
public:
    void                                require_add(fb::game::item::master* item, uint32_t count);
    void                                success_add(fb::game::item::master* item, uint32_t count);
    void                                failed_add(fb::game::item::master* item, uint32_t count);
    bool                                matched(const std::vector<item*>& items) const;
#pragma endregion
};


#pragma region nested class
class itemmix::builder : private std::vector<fb::game::item*>
{
private:
    session&                            _owner;

public:
    builder(session& owner);
    ~builder();

public:
    builder&                            push(uint8_t index);
    bool                                mix();
};
#pragma endregion

#pragma region nested structure
struct itemmix::element
{
public:
    fb::game::item::master*             item;       // 재료 아이템
    uint32_t                            count;      // 갯수

public:
    element(fb::game::item::master* item, uint32_t count) : item(item), count(count) { }
    element(const element& right) : item(right.item), count(right.count) { }
};
#pragma endregion

} }

#endif // !__ITEM_MIX_H__