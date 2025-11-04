#ifndef __BULLETIN_H__
#define __BULLETIN_H__

#include <ctime>
#include <stdint.h>
#include <optional>
#include <string>
#include <fb/model/model.h>

using namespace fb::model::enum_value;

namespace fb::game {

class character;

class bulletin
{
public:
    struct section;
    struct article;

public:
    character& owner;

public:
    bulletin(character& owner);
    ~bulletin() = default;

public:
    void show();
    void show(const fb::model::bulletin& model, const std::list<article>& articles, BULLETIN_BUTTON_ENABLE flag);
    void show(const article& article, BULLETIN_BUTTON_ENABLE flag);
    void message(const std::string& message, bool success);
};

struct bulletin::section
{
    uint32_t               id;
    std::string            title;
    std::optional<uint8_t> min_level;
    std::optional<uint8_t> max_level;
    bool                   admin;

public:
    bool writable(uint8_t level, bool admin) const;
};

struct bulletin::article
{
    uint32_t    id       = 0;
    uint32_t    section  = 0;
    uint32_t    user     = 0;
    uint8_t     month    = 0;
    uint8_t     day      = 0;
    std::string uname    = "";
    std::string title    = "";
    std::string contents = "";
    bool        next     = false;
};

} // namespace fb::game

#endif // !__BULLETIN_H__
