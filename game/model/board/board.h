#ifndef __BOARD_H__
#define __BOARD_H__

#include <string>
#include <vector>
#include <ctime>
#include <module/stream/stream.h>
#include <model/session/session.h>
#include <model/mmo/mmo.h>

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
        DECLARE_EXCEPTION(not_found_exception, message::board::ARTICLE_NOT_EXIST)

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
    class section : private std::vector<article*>
    {
    public:
        DECLARE_EXCEPTION(not_found_exception, message::board::SECTION_NOT_EXIST)

    public:
        using std::vector<article*>::operator[];
        using std::vector<article*>::begin;
        using std::vector<article*>::end;
        using std::vector<article*>::cbegin;
        using std::vector<article*>::cend;
        using std::vector<article*>::rbegin;
        using std::vector<article*>::rend;
        using std::vector<article*>::size;

    private:
        std::string          _title;

    public:
        section(const std::string& title);
        ~section();

    private:
        iterator                        it(uint16_t id);
        const_iterator                  const_it(uint16_t id) const;

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
    DECLARE_EXCEPTION(auth_exception, message::board::NOT_AUTH)

private:
    std::vector<section*>               _sections;

public:
    board();
    ~board();

public:
    const std::vector<section*>&        sections() const;
    section*                            at(uint32_t index) const;

    section*                            add(const std::string& name);
    void                                remove(uint32_t index);
    void                                clear();

public:
    section*                            operator [] (uint32_t index);
};

} }

#endif // !__BOARD_H__
