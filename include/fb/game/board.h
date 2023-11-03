#ifndef __BOARD_H__
#define __BOARD_H__

#include <string>
#include <vector>
#include <ctime>
#include <fb/game/session.h>

namespace fb { namespace game {

class board_section
{
public:
    const uint32_t                  id;
    const std::string               title;

public:
    board_section(uint32_t id, const std::string& title) : id(id), title(title)
    {}
    ~board_section() = default;
};

class board_article_new
{
public:
    const uint32_t                  id;
    const uint32_t                  section;
    const uint32_t                  uid;
    const uint8_t                   month, day;
    const std::string               uname;
    const std::string               title;
    const std::string               contents;

public:
    board_article_new(uint32_t id, uint32_t section, uint32_t uid, const std::string& uname, const std::string& title, uint8_t month, uint8_t day, const std::string& contents = "") : 
        id(id), section(section), uid(uid), uname(uname), title(title), month(month), day(day), contents(contents)
    { }
    ~board_article_new() = default;
};

class board_article
{
public:
    const uint32_t                  id;
    const uint32_t                  section;
    const std::string               title;
    const std::string               content;
    const std::string               writer;
    const uint8_t                   month, day;
    const uint8_t                   color;
    const bool                      deleted;

public:
    board_article(uint32_t id, uint32_t section, const std::string& title, const std::string& content, const std::string& writer, uint8_t month, uint8_t day, uint8_t color, bool deleted) : 
        id(id), section(section), title(title), content(content), writer(writer), month(month), day(day), color(color), deleted(deleted)
    {}
    ~board_article() = default;
};

class board
{
public:
    enum class button_enabled : uint8_t { NEXT = 0x01, WRITE = 0x02 };

public:
    class article;
    class section;

public:
    DECLARE_EXCEPTION(auth_exception, fb::game::message::board::NOT_AUTH)

    using unique_sections = std::vector<std::unique_ptr<section>>;

private:
    unique_sections                 _sections;

public:
    board();
    ~board();

public:
    const unique_sections&          sections() const;
    section*                        at(uint32_t index) const;

    section*                        add(const std::string& name);
    void                            remove(uint32_t index);
    void                            clear();

public:
    section*                        operator [] (uint32_t index);
};

class board::article
{
public:
    DECLARE_EXCEPTION(not_found_exception, fb::game::message::board::ARTICLE_NOT_EXIST)

private:
    uint16_t                        _id;
    std::string                     _title;
    std::string                     _content;
    std::string                     _writer;
    uint8_t                         _month, _day;
    uint8_t                         _color;
    bool                            _deleted;

public:
    article(uint16_t id, const std::string& title, const std::string& content, const std::string& writer, uint8_t month, uint8_t day, uint8_t color = 0x00, bool deleted = false);
    ~article();

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
