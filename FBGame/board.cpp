#include "board.h"


//
// item methods
//
fb::game::board::article::article(uint16_t id, const std::string& title, const std::string& content, const std::string& owner, uint8_t month, uint8_t day, uint8_t color) : 
    _id(id),
    _title(title),
    _content(content),
    _owner(owner),
    _month(month), _day(day),
    _color(color)
{}

fb::game::board::article::~article()
{}

uint16_t fb::game::board::article::id() const
{
    return this->_id;
}

const std::string& fb::game::board::article::title() const
{
    return this->_title;
}

const std::string& fb::game::board::article::content() const
{
    return this->_content;
}

const std::string& fb::game::board::article::owner() const
{
    return this->_owner;
}

uint8_t fb::game::board::article::month() const
{
    return this->_month;
}

uint8_t fb::game::board::article::day() const
{
    return this->_day;
}

uint8_t fb::game::board::article::color() const
{
    return this->_color;
}


//
// section methods
//
fb::game::board::section::section(const std::string & title) : 
    _title(title)
{
}

fb::game::board::section::~section()
{
    for(auto article : this->_articles)
        delete article;
}

const std::string& fb::game::board::section::title() const
{
    return this->_title;
}

const std::vector<fb::game::board::article*>& fb::game::board::section::articles() const
{
    return this->_articles;
}

fb::game::board::article* fb::game::board::section::add(uint16_t id, const std::string& title, const std::string& content, const session& session, uint8_t color)
{
    return this->add(id, title, content, session.name(), color);
}

fb::game::board::article* fb::game::board::section::add(uint16_t id, const std::string& title, const std::string& content, const std::string& owner, uint8_t color)
{
    auto time = std::time(0);
    auto now = std::localtime(&time);

    auto allocated = new article(id, title, content, owner, now->tm_mon + 1, now->tm_mday, color);
    this->_articles.push_back(allocated);

    return allocated;
}



//
// board methods
//
fb::game::board::board()
{
}

fb::game::board::~board()
{
    for(auto section : this->_sections)
        delete section;
}

const std::vector<fb::game::board::section*>& fb::game::board::sections() const
{
    return this->_sections;
}

fb::game::board::section* fb::game::board::at(uint32_t index)
{
    return this->_sections[index];
}

fb::game::board::section* fb::game::board::add(const std::string& name)
{
    auto section = new board::section(name);
    this->_sections.push_back(section);

    return section;
}

void fb::game::board::remove(uint32_t index)
{
    this->_sections.erase(this->_sections.begin() + index);
}

fb::game::board::section* fb::game::board::operator[](uint32_t index)
{
    return this->at(index);
}

fb::ostream fb::game::board::make_sections_stream() const
{
    ostream                 ostream;

    ostream.write_u8(0x31)
        .write_u8(0x01)
        .write_u16(this->_sections.size());

    for(int i = 0; i < this->_sections.size(); i++)
    {
        const auto section = this->_sections[i];
        const auto& section_title = section->title();

        ostream.write_u16(i)
            .write_u8(section_title.size())
            .write(section_title.c_str(), section_title.size());
    }

    return ostream;
}

fb::ostream fb::game::board::make_articles_stream(uint16_t section_id, uint16_t offset) const
{
    ostream                 ostream;
    const auto              section = this->_sections[section_id];
    const auto&             section_name = section->title();

    ostream.write_u8(0x31)
        .write_u8(0x02)
        .write_u8(0x01) // 레벨제한 맞으면 3 아니면 1
        .write_u16(section_id)
        .write_u8(section_name.size())
        .write(section_name.c_str(), section_name.size());


    const auto&             articles = section->articles();
    if(offset == 0x7FFF)
        offset = articles.size() - 1;
    uint16_t                reverse_offset = articles.size() - (offset + 1);

    int                     count = std::min(size_t(20), articles.size() - reverse_offset);
    ostream.write_u8(count);
    for(auto i = articles.rbegin() + reverse_offset; i != articles.rend(); i++)
    {
        const auto          article = *i;
        const auto&         title = article->title();
        const auto&         owner = article->owner();

        ostream.write_u8(0x00)
            .write_u16(article->id())
            .write_u8(owner.size())
            .write(owner.c_str(), owner.size())
            .write_u8(article->month())
            .write_u8(article->day())
            .write_u8(title.size())
            .write(title.c_str(), title.size());

        if(--count == 0)
            break;
    }

    ostream.write_u8(0x00);
    return ostream;
}
