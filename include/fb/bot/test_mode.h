#ifndef FB_BOT_TEST_MODE_H
#define FB_BOT_TEST_MODE_H

namespace fb::bot {
/**
 * @brief      Enumeration for different bot test modes
 */
enum class test_mode
{
    LOAD_TEST,       // 부하 테스트 (기존 동작)
    INTEGRATION_TEST // 통합 테스트 (새로운 기능)
};
} // namespace fb::bot

#endif // FB_BOT_TEST_MODE_H
