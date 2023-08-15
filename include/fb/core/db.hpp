#include "db.h"

using namespace fb::db;

template <typename... Values>
void fb::db::exec_f(const std::string& name, const std::string& format, Values... values)
{
	auto sql = fstring(format.c_str(), std::forward<Values>(values)...);
	exec(name, sql);
}

template <typename... Values>
result_type fb::db::co_exec_f(const std::string& name, const std::string& format, Values... values)
{
	auto sql = fstring(format.c_str(), std::forward<Values>(values)...);
	return co_exec(name, sql);
}