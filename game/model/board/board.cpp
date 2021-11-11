#include "model/board/board.h"


fb::game::board::article::article(uint16_t id, const std::string& title, const std::string& content, const std::string& writer, uint8_t month, uint8_t day, uint8_t color, bool deleted) : 
    _id(id),
    _title(title),
    _content(content),
    _writer(writer),
    _month(month), _day(day),
    _color(color),
    _deleted(deleted)
{ }

fb::game::board::article::~article()
{ }

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
{ }

fb::game::board::section::~section()
{ }

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

    auto article = std::make_unique<fb::game::board::article>(uint16_t(this->size()+1), title, content, writer, now->tm_mon + 1, now->tm_mday, color);
    auto ptr = article.get();
    this->push_back(std::move(article));
    return ptr;
}

bool fb::game::board::section::remove(uint16_t id)
{
    return std::remove_if
    (
        this->begin(), 
        this->end(),
        [id] (std::unique_ptr<article>& x)
        {
            return x->id() == id;
        }
    ) != this->end();
    return true;
}

const fb::game::board::article* fb::game::board::section::at(uint32_t index) const
{
    if(index > this->size() - 1)
        return nullptr;

    const auto& article = std::vector<std::unique_ptr<board::article>>::at(index);
    if(article->deleted())
        return nullptr;

    return article.get();
}

const fb::game::board::article* fb::game::board::section::find(uint16_t id) const
{
    auto found = std::find_if
    (
        this->cbegin(),
        this->cend(),
        [id] (const std::unique_ptr<article>& x)
        {
            return x->id() == id;
        }
    );

    if(found == this->cend())
        return nullptr;

    return found->get();
}



//
// board methods
//
fb::game::board::board()
{ }

fb::game::board::~board()
{ }

const std::vector<std::unique_ptr<fb::game::board::section>>& fb::game::board::sections() const
{
    return this->_sections;
}

fb::game::board::section* fb::game::board::at(uint32_t index) const
{
    if(index > this->_sections.size() - 1)
        return nullptr;

    return this->_sections[index].get();
}

fb::game::board::section* fb::game::board::add(const std::string& name)
{
    auto section = new fb::game::board::section(name); std::make_unique<board::section>(name);
    this->_sections.push_back(std::unique_ptr<fb::game::board::section>(section));
    return section;
}

void fb::game::board::remove(uint32_t index)
{
    this->_sections.erase(this->_sections.begin() + index);
}

void fb::game::board::clear()
{
    this->_sections.clear();
}

fb::game::board::section* fb::game::board::operator[](uint32_t index)
{
    return this->at(index);
}