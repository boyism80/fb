#include "db.h"

using namespace fb::db;

template <typename... Values>
void fb::db::context::exec_f(const std::string& name, const std::string& format, Values... values)
{
	auto sql = fstring(format.c_str(), std::forward<Values>(values)...);
	this->exec(name, sql);
}

template <typename... Values>
result_type fb::db::context::co_exec_f(const std::string& name, const std::string& format, Values... values)
{
	auto sql = fstring(format.c_str(), std::forward<Values>(values)...);
	return this->co_exec(name, sql);
}