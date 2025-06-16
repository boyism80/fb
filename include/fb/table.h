#ifndef __FB_TABLE_H__
#define __FB_TABLE_H__

#include <string>
#include <fstream>
#include <sstream>
#include <functional>
#include <json/json.h>
#include <queue>
#include <mutex>
#include <future>
#include <thread>

namespace fb::table {

/**
 * @brief      The type of the callback function.
 */
using handle_callback = std::function<void(Json::Value&, Json::Value&, double)>;

/**
 * @brief      The type of the error function.
 */
using handle_error = std::function<void(Json::Value&, Json::Value&, const std::string& error)>;

/**
 * @brief      Loads a JSON file.
 *
 * @param[in]  path  The path to the JSON file.
 * @param[in]  json  The JSON value to load into.
 *
 * @return     True if the file was loaded successfully, false otherwise.
 */
bool load(const std::string& path, Json::Value& json);

/**
 * @brief      Loads a JSON file.
 *
 * @param[in]  path  The path to the JSON file.
 * @param[in]  callback  The callback function.
 * @param[in]  error  The error function.
 * @param[in]  async  Whether to load asynchronously.
 *
 * @return     The number of rows loaded.
 */
uint32_t load(const std::string& path, const handle_callback& callback, const handle_error& error, bool async = true);

} // namespace fb::table

#endif // !__FB_TABLE_H__
