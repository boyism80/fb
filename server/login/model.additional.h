#ifndef __MODEL_PREPROCESSOR_H__
#define __MODEL_PREPROCESSOR_H__

#include <fb/model/datetime.h>
#include <fb/model/substring_matcher.h>
#include <macro.h>

#define DECLARE_BLOCKED_NAME_CONTAINER_CUSTOM_CONSTRUCTOR \
    ___blocked_name();                                    \
    ___blocked_name(const ___blocked_name&) = delete;     \
    ~___blocked_name()                      = default;

#define DECLARE_BLOCKED_NAME_CONTAINER_EXTENSION          \
                                                          \
public:                                                   \
    bool contains_substring(std::string_view name) const; \
                                                          \
private:                                                  \
    fb::model::substring_matcher _matcher;

#define DECLARE_BLOCKED_WORD_CONTAINER_CUSTOM_CONSTRUCTOR \
    ___blocked_word();                                    \
    ___blocked_word(const ___blocked_word&) = delete;     \
    ~___blocked_word()                      = default;

#define DECLARE_BLOCKED_WORD_CONTAINER_EXTENSION                 \
                                                                 \
public:                                                          \
    bool        contains_substring(std::string_view name) const; \
    std::string filter(std::string_view message) const;          \
                                                                 \
private:                                                         \
    fb::model::substring_matcher _matcher;

#endif