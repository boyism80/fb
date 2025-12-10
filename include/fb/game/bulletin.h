#ifndef __BULLETIN_H__
#define __BULLETIN_H__

#include <ctime>
#include <stdint.h>
#include <optional>
#include <string>
#include <fb/model/model.h>

namespace fb::game {

using namespace fb::model::enum_value;

class character;

class bulletin
{
public:
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
    void message(const std::string& message, bool success, bool unknown = false);
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
