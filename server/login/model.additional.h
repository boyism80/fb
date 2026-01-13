#ifndef __MODEL_PREPROCESSOR_H__
#define __MODEL_PREPROCESSOR_H__

#include <fb/model/datetime.h>
#include <macro.h>

#define DECLARE_BLOCKED_NAME_CONTAINER_EXTENSION \
                                                 \
public:                                          \
    bool contains_substring(const std::string& name) const;

#endif