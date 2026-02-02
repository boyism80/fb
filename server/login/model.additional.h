#ifndef __MODEL_PREPROCESSOR_H__
#define __MODEL_PREPROCESSOR_H__

#include <fb/model/datetime.h>
#include <macro.h>

#define DECLARE_BLOCKED_NAME_CONTAINER_EXTENSION \
                                                 \
public:                                          \
    bool contains_substring(std::string_view name) const;

#define DECLARE_BLOCKED_WORD_CONTAINER_EXTENSION                 \
                                                                 \
public:                                                          \
    bool        contains_substring(std::string_view name) const; \
    std::string filter(std::string_view message) const;

#endif