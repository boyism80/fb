#ifndef __BOARD_H__
#define __BOARD_H__

#include <string>
#include <vector>
#include <ctime>
#include "model/session/session.h"
#include "model/mmo/mmo.h"
#include "module/string/string.h"
#include "module/stream/stream.h"

namespace fb { namespace game {

class board
{
#pragma region enum
public:
    enum button_enabled : uint8_t { NEXT = 0x01, WRITE = 0x02 };
#pragma endregion

#pragma region forward nested declaration
public:
    class article;
    class section;
#pragma endregion

#pragma region type definition
public:
    DECLARE_EXCEPTION(auth_exception, fb::game::message::board::NOT_AUTH)

    using unique_sections = std::vector<std::unique_ptr<section>>;
#pragma endregion

#pragma region private field
private:
    unique_sections                 _sections;
#pragma endregion

#pragma region constructor / destructor
public:
    board();
    ~board();
#pragma endregion

#pragma region public method
public:
    const unique_sections&          sections() const;
    section*                        at(uint32_t index) const;

    section*                        add(const std::string& name);
    void                            remove(uint32_t index);
    void                            clear();
#pragma endregion

#pragma region operator
public:
    section*                        operator [] (uint32_t index);
#pragma endregion
};

class board::article
{
public:
    DECLARE_EXCEPTION(not_found_exception, fb::game::message::board::ARTICLE_NOT_EXIST)

#pragma region private field
private:
    uint16_t                        _id;
    std::string                     _title;
    std::string                     _content;
    std::string                     _writer;
    uint8_t                         _month, _day;
    uint8_t                         _color;
    bool                            _deleted;
#pragma endregion

#pragma region constructor / destructor
public:
    article(uint16_t id, const std::string& title, const std::string& content, const std::string& writer, uint8_t month, uint8_t day, uint8_t color = 0x00, bool deleted = false);
    ~article();
#pragma endregion

#pragma region public method
public:
    uint16_t                        id() const;
    const std::string&              title() const;
    const std::string&              content() const;
    const std::string&              writer() const;
    uint8_t                         month() const;
    uint8_t                         day() const;
    uint8_t                         color() const;
    bool                            deleted() const;
    void                            deleted(bool value);
#pragma endregion
};

class board::section : private std::vector<std::unique_ptr<board::article>>
{
public:
    DECLARE_EXCEPTION(not_found_exception, fb::game::message::board::SECTION_NOT_EXIST)

public:
    using std::vector<std::unique_ptr<board::article>>::operator[];
    using std::vector<std::unique_ptr<board::article>>::begin;
    using std::vector<std::unique_ptr<board::article>>::end;
    using std::vector<std::unique_ptr<board::article>>::cbegin;
    using std::vector<std::unique_ptr<board::article>>::cend;
    using std::vector<std::unique_ptr<board::article>>::rbegin;
    using std::vector<std::unique_ptr<board::article>>::rend;
    using std::vector<std::unique_ptr<board::article>>::size;

private:
    std::string          _title;

public:
    section(const std::string& title);
    ~section();

public:
    const std::string&              title() const;
    board::article*                 add(const std::string& title, const std::string& content, const session& session, uint8_t color = 0x00);
    board::article*                 add(const std::string& title, const std::string& content, const std::string& writer, uint8_t color = 0x00);
    bool                            remove(uint16_t id);
    const board::article*           at(uint32_t index) const;
    const board::article*           find(uint16_t id) const;
};

} }

#endif // !__BOARD_H__
