#ifndef __BOARD_H__
#define __BOARD_H__

#include <string>
#include <vector>
#include <ctime>
#include "stream.h"
#include "session.h"
#include "mmo.h"

namespace fb { namespace game {

class board
{
public:
#pragma region item class
    class article
    {
    public:
        DECLARE_EXCEPTION(not_found_exception, "게시글이 존재하지 않습니다.")

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
        DECLARE_EXCEPTION(not_found_exception, "섹션이 존재하지 않습니다.")

    public:
        using std::vector<article*>::operator[];
        using std::vector<article*>::begin;
        using std::vector<article*>::end;
        using std::vector<article*>::rbegin;
        using std::vector<article*>::rend;
        using std::vector<article*>::size;

    private:
        std::string          _title;

    public:
        section(const std::string& title);
        ~section();

    public:
        const std::string&              title() const;
        article*                        add(uint16_t id, const std::string& title, const std::string& content, const session& session, uint8_t color = 0x00);
        article*                        add(uint16_t id, const std::string& title, const std::string& content, const std::string& writer, uint8_t color = 0x00);
        const article*                  at(uint32_t index) const;
    };
#pragma endregion

public:
    DECLARE_EXCEPTION(auth_exception, "권한이 없습니다.")

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

public:
    section*                            operator [] (uint32_t index);

public:
    ostream                             make_sections_stream() const;
    ostream                             make_articles_stream(uint16_t section_id, uint16_t offset) const;
    ostream                             make_article_stream(uint16_t section_id, uint16_t article_id, const session& session) const;
    ostream                             make_delete_stream(uint16_t section_id, uint16_t article_id, const session& session) const;
    ostream                             make_message_stream(const std::string& message, bool success) const;
};

} }

#endif // !__BOARD_H__
