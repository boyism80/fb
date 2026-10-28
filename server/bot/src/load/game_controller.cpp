#include <fb/bot/load/game_controller.h>

#include <async/awaitable_get.h>
#include <fb/bot/container.h>
#include <fb/bot/game_bot.h>
#include <fb/config.h>
#include <fb/logger.h>
#include <fb/model/loader.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <unordered_set>

using namespace fb::bot::load;

game_bot_controller::game_bot_controller(bot_container& container) :
    fb::bot::game_bot_controller(container)
{ }

void game_bot_controller::initialize()
{
    table::item->hook.build = &game_bot_controller::build_item;
    async::awaitable_get(fb::model::loader(this->container).run());

    auto data_path   = fb::config<std::string>("load:data", "config/load.json");
    auto revive_npcs = std::unordered_set<uint32_t>{};
    auto speed       = 1.0;
    try
    {
        auto file = std::ifstream(data_path);
        auto root = Json::Value{};
        file >> root;
        for (auto& npc : root["revive_npcs"])
        {
            revive_npcs.insert(npc.asUInt());
        }

        for (auto& json : root["personas"])
        {
            this->_personas.emplace_back(json);
        }
        this->_spread   = root["spread"];
        this->_learning = root["learning"].get("ratio", 0.0).asDouble();
        this->_learner.configure(root["learning"]);
        speed = root.get("speed", 1.0).asDouble();

        this->_tutorial = tutorial(root["tutorial"]);
        this->_job      = job(root["job"]);
        this->_experience.configure(root["experience"]);

        auto items_file = std::ifstream(root["supplies"].get("catalog", "config/load-items.json").asString());
        auto items      = Json::Value{};
        items_file >> items;
        this->_supplies = supplies(items, root["supplies"]);

        auto catalog_file = std::ifstream(root["spells"].get("catalog", "config/load-spells.json").asString());
        auto catalog      = Json::Value{};
        catalog_file >> catalog;
        this->_spellbook = spellbook(catalog, root["spells"]);

        auto quests_file = std::ifstream(root["quests"].get("catalog", "config/load-quests.json").asString());
        auto quests      = Json::Value{};
        quests_file >> quests;
        this->_questbook = questbook(quests, root["quests"]);
    }
    catch (std::exception& e)
    {
        fb::logger::warn("load data: cannot read {}: {}", data_path, e.what());
    }
    auto sobj = fb::sobj_tbl_file{};
    sobj.load(fb::config<std::string>("load:sobj", "../game/SObj.tbl"));
    this->_knowledge =
        std::make_unique<knowledge>(fb::config<std::string>("load:maps", "../game/maps"), sobj, revive_npcs);

    auto messages_path = fb::config<std::string>("load:messages", "config/load-messages.json");
    try
    {
        auto file = std::ifstream(messages_path);
        auto root = Json::Value{};
        file >> root;
        this->_messages = messages(root);
    }
    catch (std::exception& e)
    {
        fb::logger::warn("load messages: cannot read {}: {}", messages_path, e.what());
    }
    this->_trace.open(std::format("logs/load-trace-{}.jsonl", fb::config<uint32_t>("load:process", 0)));
    {
        auto file       = std::ifstream(data_path, std::ios::binary);
        auto text       = std::string{std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()};
        auto entry      = Json::Value{};
        entry["event"]  = "config";
        entry["data"]   = data_path;
        entry["hash"]   = std::format("{:016x}", std::hash<std::string>{}(text));
        entry["users"]  = fb::config<uint32_t>("spawn_count", 0);
        entry["offset"] = fb::config<uint32_t>("load:account_offset", 0);
        entry["speed"]  = speed;
        this->_trace.write("", entry);
    }
    for (auto& command : fb::config<>("load")["setup"])
    {
        this->_setup.push_back(fb::CP949(command.asString(), fb::PLATFORM::WINDOWS));
    }

    if (this->_personas.empty())
        this->_personas.emplace_back();
    for (auto& persona : this->_personas)
    {
        persona.accelerate(speed);
    }

    this->bind_thread_timer(&game_bot_controller::on_timer, 100ms);
    this->bind_timer(&game_bot_controller::on_metrics,
                     std::chrono::milliseconds(fb::config<uint32_t>("load:metrics_interval_ms", 10000)));

    if (this->container.link.enabled())
    {
        this->_census.journal().enable();
        this->_hazards.journal().enable();
        this->_market.journal().enable();
        this->_roster.journal().enable();
        this->_crowd.journal().enable();
        this->_learner.journal().enable();
        this->_experience.journal().enable();
        this->container.link.on("sync", [this](const Json::Value& message) {
            for (auto& event : message["events"])
            {
                auto kind = event["kind"].asString();
                if (kind == "census")
                    this->_census.apply(event);
                else if (kind == "hazard")
                    this->_hazards.apply(event);
                else if (kind == "market")
                    this->_market.apply(event);
                else if (kind == "roster")
                    this->_roster.apply(event);
                else if (kind == "crowd")
                    this->_crowd.apply(event);
                else if (kind == "learner")
                    this->_learner.apply(event);
                else if (kind == "fight")
                    this->_experience.apply(event);
            }
        });
        this->bind_timer(&game_bot_controller::on_sync, 500ms);
    }
}

async::task<void> game_bot_controller::on_sync()
{
    auto events = Json::Value{Json::arrayValue};
    for (auto* journal : {&this->_census.journal(),
                          &this->_hazards.journal(),
                          &this->_market.journal(),
                          &this->_roster.journal(),
                          &this->_crowd.journal(),
                          &this->_learner.journal(),
                          &this->_experience.journal()})
    {
        for (auto& event : journal->drain())
        {
            events.append(event);
        }
    }

    if (events.empty() == false)
    {
        auto message      = Json::Value{};
        message["type"]   = "sync";
        message["events"] = std::move(events);
        this->container.link.send(message);
    }
    co_return;
}

async::task<void> game_bot_controller::on_timer(const fb::model::datetime& now, std::thread::id id)
{
    auto thread = this->container.threads.at(id);
    auto params = thread->data<bot_thread_params>();
    if (params == nullptr)
        co_return;

    auto guard = this->_bots.enter_read();
    for (auto& [_, bot] : params->bots)
    {
        if (guard.value().contains(bot->id) == false)
            continue;

        auto typed_bot = std::static_pointer_cast<game_bot>(bot);
        if (typed_bot->inited())
        {
            auto lock  = std::lock_guard(this->_brains_mutex);
            auto found = this->_brains.find(typed_bot->name());
            if (found != this->_brains.end())
                found->second->check(*typed_bot);
        }
        else if (typed_bot->oid() != 0 && typed_bot->map() != 0xFFFF && typed_bot->has_position() &&
                 typed_bot->name().empty() == false)
        {
            auto selected = std::shared_ptr<brain>{};
            {
                auto lock  = std::lock_guard(this->_brains_mutex);
                auto found = this->_brains.find(typed_bot->name());
                if (found != this->_brains.end())
                {
                    selected = found->second;
                }
                else
                {
                    auto seed    = static_cast<uint32_t>(std::hash<std::string>{}(typed_bot->name()) ^
                                                      fb::config<uint32_t>("load:seed", 0));
                    auto weights = std::vector<double>{};
                    for (auto& persona : this->_personas)
                    {
                        weights.push_back(persona.ratio);
                    }
                    auto random      = std::mt19937(seed);
                    auto index       = std::discrete_distribution<size_t>(weights.begin(), weights.end())(random);
                    auto persona     = this->_personas.at(index).vary(this->_spread, random);
                    persona.learning = std::uniform_real_distribution<double>(0.0, 1.0)(random) < this->_learning;
                    selected         = std::make_shared<brain>(*this->_knowledge,
                                                       persona,
                                                       this->_tutorial,
                                                       this->_job,
                                                       this->_supplies,
                                                       this->_spellbook,
                                                       this->_questbook,
                                                       this->_census,
                                                       this->_hazards,
                                                       this->_market,
                                                       this->_roster,
                                                       this->_crowd,
                                                       this->_learner,
                                                       this->_experience,
                                                       this->_messages,
                                                       this->_trace,
                                                       this->_metrics,
                                                       seed);
                    this->_brains.insert({typed_bot->name(), selected});
                    fb::logger::info("load brain: bot={} persona={}",
                                     typed_bot->name(),
                                     this->_personas.at(index).name);
                }
            }

            typed_bot->inited(true);
            auto builder = thread->new_builder<void>();
            builder.func = [selected, typed_bot, setup = this->_setup](auto&) -> async::task<void> {
                for (auto& command : setup)
                {
                    typed_bot->send(*typed_bot->make<fb::protocol::game::request::chat>(false, command));
                    co_await typed_bot->thread()->sleep(200ms);
                }
                co_await selected->run(typed_bot);
            };
            builder.enqueue();
        }
    }
    co_return;
}

async::task<void> game_bot_controller::on_metrics()
{
    auto goals   = std::array<uint32_t, GOAL_COUNT>{};
    auto classes = std::array<uint32_t, CLASS_COUNT>{};
    auto online  = uint32_t{0};
    {
        auto lock = std::lock_guard(this->_brains_mutex);
        for (auto& [_, brain] : this->_brains)
        {
            auto goal = brain->goal();
            if (goal.has_value() == false)
                continue;

            online++;
            goals[static_cast<size_t>(goal.value())]++;

            auto cls = brain->cls();
            if (cls.has_value())
                classes[static_cast<size_t>(cls.value())]++;
        }
    }

    auto root       = Json::Value{};
    root["time"]    = std::format("{}", std::chrono::system_clock::now());
    root["online"]  = online;
    root["kills"]   = static_cast<Json::UInt64>(this->_metrics.kills.load());
    root["deaths"]  = static_cast<Json::UInt64>(this->_metrics.deaths.load());
    root["exp"]     = static_cast<Json::UInt64>(this->_metrics.exp.load());
    root["stuck"]   = static_cast<Json::UInt64>(this->_metrics.stuck.load());
    root["fights"]  = static_cast<Json::UInt64>(this->_experience.fights());
    root["learner"] = this->_learner.state();
    this->_learner.save();
    for (size_t i = 0; i < GOAL_COUNT; i++)
    {
        root["goals"][GOAL_NAMES.at(i)]    = goals.at(i);
        root["failures"][GOAL_NAMES.at(i)] = static_cast<Json::UInt64>(this->_metrics.failures.at(i).load());
    }

    for (size_t i = 0; i < CLASS_COUNT; i++)
    {
        root["classes"][enum_tostring(static_cast<CLASS>(i))] = classes.at(i);
    }

    auto writer           = Json::StreamWriterBuilder{};
    writer["indentation"] = "";
    auto line             = Json::writeString(writer, root);

    std::filesystem::create_directories("logs");
    auto file = std::ofstream(std::format("logs/load-metrics-{}.jsonl", fb::config<uint32_t>("load:process", 0)),
                              std::ios::app);
    file << line << std::endl;
    fb::logger::info("load metrics: {}", line);

    root["type"] = "metrics";
    this->container.link.send(root);
    co_return;
}

async::task<void> game_bot_controller::on_bot_connected(game_bot& bot)
{
    // Send game login packet with transfer buffer.
    // Bootstrap 0x10 always uses the v550 layout; the packed client_version field
    // inside the transfer blob still establishes the session version.
    using login_request = fb::protocol::game::request::login<fb::protocol::CLIENT_VERSION::v550>;
    bot.send(login_request(bot.transfer_buffer()), false, true);

    // Bot is now managed by bot_controller's thread-safe collection
    co_return;
}

async::task<void> game_bot_controller::on_bot_disconnected(game_bot& bot)
{
    this->_market.withdraw(bot.name(), {});
    this->_market.release(bot.name());
    this->_roster.remove(bot.name());
    co_await super::on_bot_disconnected(bot);
    // Bot is automatically removed from bot_controller's thread-safe collection
    co_return;
}
