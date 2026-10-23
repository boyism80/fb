#include <fb/bot/client_profile.h>

#include <fb/config.h>

#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <format>
#include <random>
#include <stdexcept>
#include <string>
#include <system_error>
#include <vector>

namespace {

std::optional<fb::bot::client_profile> g_configured;

} // namespace

namespace fb::bot {

std::string client_profile::to_string() const
{
    return std::format("{}/{}", fb::protocol::to_string(this->version), fb::protocol::to_string(this->ui_mode));
}

std::optional<client_profile> client_profile::parse(uint16_t version, std::string_view ui_mode)
{
    if (fb::protocol::is_supported(version) == false)
        return std::nullopt;

    auto upper = std::string(ui_mode);
    std::transform(upper.begin(), upper.end(), upper.begin(), [](unsigned char c) {
        return static_cast<char>(std::toupper(c));
    });

    auto profile = client_profile{.version = static_cast<fb::protocol::CLIENT_VERSION>(version)};
    if (upper.empty() || upper == "OLD")
        profile.ui_mode = fb::protocol::CLIENT_UI_MODE::OLD;
    else if (upper == "NEW")
        profile.ui_mode = fb::protocol::CLIENT_UI_MODE::NEW;
    else
        return std::nullopt;

    if (profile.ui_mode == fb::protocol::CLIENT_UI_MODE::NEW && profile.version != fb::protocol::CLIENT_VERSION::v651)
        return std::nullopt;

    return profile;
}

std::optional<client_profile> client_profile::parse(std::string_view text)
{
    auto separator    = text.find(':');
    auto version_text = text.substr(0, separator);
    auto ui_mode_text = separator == std::string_view::npos ? std::string_view{} : text.substr(separator + 1);

    auto version     = uint16_t{0};
    auto version_end = version_text.data() + version_text.size();
    auto [ptr, ec]   = std::from_chars(version_text.data(), version_end, version);
    if (ec != std::errc{} || ptr != version_end)
        return std::nullopt;

    return parse(version, ui_mode_text);
}

void client_profile::configure(const client_profile& profile)
{
    g_configured = profile;
}

client_profile client_profile::configured()
{
    if (g_configured.has_value())
        return g_configured.value();

    auto profile = parse(fb::config<uint16_t>("client:version", 550), fb::config<std::string>("client:ui_mode", "OLD"));
    return profile.value_or(client_profile{});
}

client_profile client_profile::pick()
{
    const auto& mix = fb::config_node("client:mix");
    if (mix.isArray() == false || mix.empty())
        return configured();

    auto profiles = std::vector<client_profile>{};
    auto weights  = std::vector<uint32_t>{};
    for (const auto& entry : mix)
    {
        auto profile = parse(static_cast<uint16_t>(entry["version"].asUInt()), entry.get("ui_mode", "OLD").asString());
        if (profile.has_value() == false)
            throw std::runtime_error(std::format("invalid client:mix entry: {}", entry.toStyledString()));

        profiles.push_back(profile.value());
        weights.push_back(entry.get("weight", 1).asUInt());
    }

    thread_local auto generator    = std::mt19937(std::random_device{}());
    auto              distribution = std::discrete_distribution<size_t>(weights.begin(), weights.end());
    return profiles[distribution(generator)];
}

} // namespace fb::bot
