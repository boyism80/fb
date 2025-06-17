#ifndef __ENCODING_H__
#define __ENCODING_H__

#ifndef _WIN32
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
#include <stdexcept>
#include <memory>
#include <sstream>
#include <iomanip>

namespace fb {

/**
 * @brief      Platform enumeration for encoding conversion operations.
 *
 *             Specifies the target platform for character encoding conversions,
 *             allowing platform-specific handling of text encoding differences.
 */
enum class PLATFORM
{
    Windows,
    Linux,
    Both
};

/**
 * @brief      Converts a UTF-8 encoded string to CP949 encoding.
 *
 *             This function converts a UTF-8 encoded string to CP949 encoding.
 *             It is useful for converting UTF-8 strings to CP949 strings.
 *
 * @param[in]  utf8  The UTF-8 encoded string.
 *
 * @return     The string converted to CP949 encoding.
 */
std::string cp949(const std::string& utf8);
/**
 * @brief      Converts a CP949 encoded string to UTF-8 encoding.
 *
 *             This function converts a CP949 encoded string to UTF-8 encoding.
 *             It is useful for converting CP949 strings to UTF-8 strings.
 *
 * @param[in]  cp949  The CP949 encoded string.
 *
 * @return     The string converted to UTF-8 encoding.
 */
std::string utf8(const std::string& cp949);
/**
 * @brief      Converts a UTF-8 encoded string to CP949 encoding, depending on the platform.
 *
 *             This function converts a UTF-8 encoded string to CP949 encoding,
 *             depending on the platform.
 *
 * @param[in]  utf8  The UTF-8 encoded string.
 * @param[in]  p     The target platform for conversion (default: Linux).
 *
 * @return     The string converted to CP949 encoding, or unchanged depending on the platform.
 */
std::string CP949(const std::string& utf8, PLATFORM p = PLATFORM::Linux);
/**
 * @brief      Converts a CP949 encoded string to UTF-8 encoding, depending on the platform.
 *
 *             This function converts a CP949 encoded string to UTF-8 encoding,
 *             depending on the platform.
 *
 * @param[in]  cp949  The CP949 encoded string.
 * @param[in]  p      The target platform for conversion (default: Linux).
 *
 * @return     The string converted to UTF-8 encoding, or unchanged depending on the platform.
 */
std::string UTF8(const std::string& cp949, PLATFORM p = PLATFORM::Linux);

/**
 * @brief      Converts a multibyte string to a wide string.
 *
 *             This function converts a multibyte string to a wide string.
 *             It is useful for converting multibyte strings to wide strings.
 *
 * @param[in]  m  The multibyte string (usually in CP949 encoding).
 *
 * @return     The converted wide string.
 */
std::wstring W(const std::string& m);
/**
 * @brief      Converts a wide string to a multibyte string.
 *
 *             This function converts a wide string to a multibyte string.
 *             It is useful for converting wide strings to multibyte strings.
 *
 * @param[in]  w  The wide string.
 *
 * @return     The converted multibyte string (usually in CP949 encoding).
 */
std::string M(const std::wstring& w);

/**
 * @brief      Appends a Korean object marker (을/를) to the given name.
 *
 *             This function appends a Korean object marker (을/를) to the given name.
 *             It is useful for generating proper Korean grammatical forms.
 *
 * @param[in]  name  The name to append the marker to.
 *
 * @return     The name with the appropriate marker appended (e.g., '채승현을', '거기를').
 */
std::string name_with(const std::string& name);

/**
 * @brief      Appends a specified Korean postfix to the given name.
 *
 *             This function appends a specified Korean postfix to the given name.
 *             It is useful for generating proper Korean grammatical forms.
 *
 * @param[in]  name     The name to append the postfix to.
 * @param[in]  postfix  The postfix pair to append (e.g., {"은", "는"}).
 *
 * @return     The name with the specified Korean postfix appended.
 */
std::string name_with(const std::string& name, const std::pair<std::string, std::string>& postfix);

/**
 * @brief      Checks if the given string contains only Korean characters.
 *
 *             This function checks if the given string contains only Korean characters.
 *             It is useful for validating input strings before processing.
 *
 * @param[in]  str  The string to check.
 *
 * @return     True if the string contains only Korean characters, false otherwise.
 */
bool assert_korean(const std::string& str);

/**
 * @brief      URL-encodes the given string.
 *
 *             This function converts a string into a URL-encoded format,
 *             replacing special characters with their percent-encoded equivalents.
 *             It ensures that the string is safe to use in URLs and HTTP requests.
 *             Reference: https://stackoverflow.com/a/17708801
 *
 * @param[in]  value  The string to encode.
 *
 * @return     The URL-encoded string.
 */
std::string url_encode(const std::string& value);

} // namespace fb

#endif