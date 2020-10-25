#ifndef __INTERNAL_EXCEPTION_H__
#define __INTERNAL_EXCEPTION_H__

#include <exception>
#include "protocol/internal.h"

namespace fb { namespace internal {

class transfer_error : public std::exception
{
public:
    fb::protocol::internal::response::transfer_code code;

public:
    transfer_error(fb::protocol::internal::response::transfer_code code) : code(code)
    {}
};

} }

#endif // !__INTERNAL_EXCEPTION_H__