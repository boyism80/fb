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
public:
    enum button_enabled : uint8_t { NEXT = 0x01, WRITE = 0x02 };

public:
#pragma region item class
    class article
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
#pragma endregion

#pragma region section class
    class section : private std::vector<std::unique_ptr<article>>
    {
    public:
        DECLARE_EXCEPTION(not_found_exception, fb::game::message::board::SECTION_NOT_EXIST)

    public:
        using std::vector<std::unique_ptr<article>>::operator[];
        using std::vector<std::unique_ptr<article>>::begin;
        using std::vector<std::unique_ptr<article>>::end;
        using std::vector<std::unique_ptr<article>>::cbegin;
        using std::vector<std::unique_ptr<article>>::cend;
        using std::vector<std::unique_ptr<article>>::rbegin;
        using std::vector<std::unique_ptr<article>>::rend;
        using std::vector<std::unique_ptr<article>>::size;

    private:
        std::string          _title;

    public:
        section(const std::string& title);
        ~section();

    public:
        const std::string&              title() const;
        article*                        add(const std::string& title, const std::string& content, const session& session, uint8_t color = 0x00);
        article*                        add(const std::string& title, const std::string& content, const std::string& writer, uint8_t color = 0x00);
        bool                            remove(uint16_t id);
        const article*                  at(uint32_t index) const;
        const article*                  find(uint16_t id) const;
    };
#pragma endregion

public:
    DECLARE_EXCEPTION(auth_exception, fb::game::message::board::NOT_AUTH)

    typedef std::vector<std::unique_ptr<section>> unique_sections;

private:
    unique_sections                     _sections;

public:
    board();
    ~board();

public:
    const unique_sections&              sections() const;
    section*                            at(uint32_t index) const;

    section*                            add(const std::string& name);
    void                                remove(uint32_t index);
    void                                clear();

public:
    section*                            operator [] (uint32_t index);
};

} }

#endif // !__BOARD_H__
