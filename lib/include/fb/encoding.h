#ifndef __ENCODING_H__
#define __ENCODING_H__

#ifdef _WIN32
#include <Windows.h>
#else
#include <locale>
#include <iconv.h>
#include <codecvt>
#endif
#include <cstdint>
#include <wchar.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string>
#include <cstring>
#include <vector>

namespace fb {

/**
 * @brief      This class describes a platform.
 */
enum class PLATFORM
{
    Windows,
    Linux,
    Both
};

/**
 * @brief      { function_description }
 *
 * @param[in]  utf8  The utf 8
 *
 * @return     { description_of_the_return_value }
 */
std::string cp949(const std::string& utf8);
/**
 * @brief      { function_description }
 *
 * @param[in]  cp949  The cp 949
 *
 * @return     { description_of_the_return_value }
 */
std::string utf8(const std::string& cp949);
/**
 * @brief      { function_description }
 *
 * @param[in]  utf8  The utf 8
 * @param[in]  p     { parameter_description }
 *
 * @return     { description_of_the_return_value }
 */
std::string CP949(const std::string& utf8, PLATFORM p = PLATFORM::Linux);
/**
 * @brief      { function_description }
 *
 * @param[in]  cp949  The cp 949
 * @param[in]  p      { parameter_description }
 *
 * @return     { description_of_the_return_value }
 */
std::string UTF8(const std::string& cp949, PLATFORM p = PLATFORM::Linux);

/**
 * @brief      { function_description }
 *
 * @param[in]  m     { parameter_description }
 *
 * @return     { description_of_the_return_value }
 */
std::wstring W(const std::string& m);
/**
 * @brief      { function_description }
 *
 * @param[in]  w     { parameter_description }
 *
 * @return     { description_of_the_return_value }
 */
std::string M(const std::wstring& w);

/**
 * @brief      { function_description }
 *
 * @param[in]  name  The name
 *
 * @return     { description_of_the_return_value }
 */
std::string name_with(const std::string& name);
/**
 * @brief      { function_description }
 *
 * @param[in]  name     The name
 * @param[in]  postfix  The postfix
 *
 * @return     { description_of_the_return_value }
 */
std::string name_with(const std::string& name, const std::pair<std::string, std::string>& postfix);

/**
 * @brief      { function_description }
 *
 * @param[in]  str   The string
 *
 * @return     { description_of_the_return_value }
 */
bool assert_korean(const std::string& str);

} // namespace fb

#endif