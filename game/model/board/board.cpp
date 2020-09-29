#include "model/board/board.h"


fb::game::board::article::article(uint16_t id, const std::string& title, const std::string& content, const std::string& writer, uint8_t month, uint8_t day, uint8_t color, bool deleted) : 
    _id(id),
    _title(title),
    _content(content),
    _writer(writer),
    _month(month), _day(day),
    _color(color),
    _deleted(deleted)
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

const std::string& fb::game::board::article::writer() const
{
    return this->_writer;
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

bool fb::game::board::article::deleted() const
{
    return this->_deleted;
}

void fb::game::board::article::deleted(bool value)
{
    this->_deleted = value;
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
    for(auto article : *this)
        delete article;
}

std::vector<fb::game::board::article*>::iterator fb::game::board::section::it(uint16_t id)
{
    for(auto i = this->begin(); i != this->end(); i++)
    {
        const auto& article = *i;
        if(article->id() == id)
            return i;
    }

    return this->end();
}

std::vector<fb::game::board::article*>::const_iterator fb::game::board::section::const_it(uint16_t id) const
{
    for(auto i = this->cbegin(); i != this->cend(); i++)
    {
        const auto& article = *i;
        if(article->id() == id)
            return i;
    }

    return this->cend();
}

const std::string& fb::game::board::section::title() const
{
    return this->_title;
}

fb::game::board::article* fb::game::board::section::add(const std::string& title, const std::string& content, const session& session, uint8_t color)
{
    return this->add(title, content, session.name(), color);
}

fb::game::board::article* fb::game::board::section::add(const std::string& title, const std::string& content, const std::string& writer, uint8_t color)
{
    auto time = std::time(0);
    auto now = std::localtime(&time);

    auto allocated = new article(uint16_t(this->size()+1), title, content, writer, now->tm_mon + 1, now->tm_mday, color);
    this->push_back(allocated);

    return allocated;
}

bool fb::game::board::section::remove(uint16_t id)
{
    auto found = this->it(id);
    if(found == this->end())
        return false;

    this->erase(found);
    return true;
}

const fb::game::board::article* fb::game::board::section::at(uint32_t index) const
{
    if(index > this->size() - 1)
        return nullptr;

    const auto article = std::vector<board::article*>::at(index);
    if(article->deleted())
        return nullptr;

    return article;
}

const fb::game::board::article* fb::game::board::section::find(uint16_t id) const
{
    auto found = this->const_it(id);
    if(found == this->cend())
        return nullptr;

    return *found;
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

fb::game::board::section* fb::game::board::at(uint32_t index) const
{
    if(index > this->_sections.size() - 1)
        return nullptr;

    return this->_sections.at(index);
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

void fb::game::board::clear()
{
    for(auto section : this->_sections)
        delete section;

    this->_sections.clear();
}

fb::game::board::section* fb::game::board::operator[](uint32_t index)
{
    return this->at(index);
}