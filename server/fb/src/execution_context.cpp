#include <fb/async_executor.h>
#include <fb/execution_context.h>

using namespace fb;

namespace {

thread_local std::shared_ptr<execution_context> active_frame;

} // namespace

execution_context::execution_context() :
    _slots(std::make_shared<slot_storage>())
{ }

std::shared_ptr<execution_context> execution_context::create()
{
    return std::shared_ptr<execution_context>(new execution_context());
}

std::shared_ptr<execution_context> execution_context::current()
{
    return frame(token());
}

std::shared_ptr<execution_context> execution_context::active()
{
    return active_frame;
}

void execution_context::active(std::shared_ptr<execution_context> value)
{
    active_frame = std::move(value);
}

async::propagation::token execution_context::token()
{
    return async::propagation::capture();
}

async::propagation::token execution_context::token(async::propagation::token value)
{
    if (value)
        return value;

    return token();
}

async::propagation::token execution_context::token(std::shared_ptr<execution_context> value)
{
    return std::static_pointer_cast<void>(std::move(value));
}

std::shared_ptr<execution_context> execution_context::frame(async::propagation::token value)
{
    if (value == nullptr)
        return nullptr;

    return std::static_pointer_cast<execution_context>(value);
}

void execution_context::pending(async::propagation::token value)
{
    if (value)
        async::propagation::set_pending(std::move(value));
}

void execution_context::install_propagation_hooks(async_executor&)
{
    auto& hooks = async::propagation::runtime();

    hooks.capture = []() -> async::propagation::token {
        return token(active());
    };

    hooks.install = [](async::propagation::token value) {
        active(frame(std::move(value)));
    };

    hooks.restore = [](async::propagation::token value) {
        active(frame(std::move(value)));
    };
}
