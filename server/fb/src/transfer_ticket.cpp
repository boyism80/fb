#include <fb/config.h>
#include <fb/hmac.h>
#include <fb/transfer_ticket.h>

#include <chrono>
#include <cstdint>
#include <cstring>
#include <format>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using namespace fb;

const std::string& transfer_ticket::secret()
{
    static const std::string value = [] {
        constexpr uint8_t expected[] = {0x5b, 0xdc, 0xc1, 0x46, 0xbf, 0x60, 0x75, 0x4e, 0x6a, 0x04, 0x24,
                                        0x26, 0x08, 0x95, 0x75, 0xc7, 0x5a, 0x00, 0x3f, 0x08, 0x9d, 0x27,
                                        0x39, 0x83, 0x9d, 0xec, 0x58, 0xb9, 0x64, 0xec, 0x38, 0x43};
        auto              digest     = fb::hmac_sha256("Jefe", 4, "what do ya want for nothing?", 28);
        if (std::memcmp(digest.data(), expected, sizeof(expected)) != 0)
            throw std::runtime_error("HMAC-SHA256 self-test failed");

        auto value = fb::config<std::optional<std::string>>("transfer_secret");
        if (value.has_value() == false)
            throw std::runtime_error("transfer_secret is not set in config");

        if (value->size() < MIN_SECRET_SIZE)
            throw std::runtime_error(std::format("transfer_secret must be at least {} bytes", MIN_SECRET_SIZE));

        return value.value();
    }();

    return value;
}

uint32_t transfer_ticket::now()
{
    auto elapsed = std::chrono::system_clock::now().time_since_epoch();
    return static_cast<uint32_t>(std::chrono::duration_cast<std::chrono::seconds>(elapsed).count());
}

uint64_t transfer_ticket::make_nonce()
{
    thread_local auto engine =
        std::mt19937_64((uint64_t(std::random_device{}()) << 32) | uint64_t(std::random_device{}()));
    return engine();
}

transfer_ticket::tag_type transfer_ticket::sign(uint32_t world, uint8_t host, const uint8_t* data, size_t size)
{
    auto input = std::vector<uint8_t>();
    input.reserve(sizeof(world) + sizeof(host) + size);
    input.push_back(static_cast<uint8_t>(world >> 24));
    input.push_back(static_cast<uint8_t>(world >> 16));
    input.push_back(static_cast<uint8_t>(world >> 8));
    input.push_back(static_cast<uint8_t>(world));
    input.push_back(host);
    input.insert(input.end(), data, data + size);

    auto& key    = secret();
    auto  digest = fb::hmac_sha256(key.data(), key.size(), input.data(), input.size());
    auto  tag    = tag_type{};
    std::memcpy(tag.data(), digest.data(), TAG_SIZE);
    return tag;
}

bool transfer_ticket::nonce_cache::insert(uint64_t nonce)
{
    auto _   = std::lock_guard(this->_mutex);
    auto now = clock::now();
    if (now - this->_purged_at >= PURGE_INTERVAL)
    {
        std::erase_if(this->_nonces, [now](const auto& pair) {
            return now - pair.second > RETENTION;
        });
        this->_purged_at = now;
    }

    return this->_nonces.try_emplace(nonce, now).second;
}
