#include <fb/logger.h>
#include <filesystem>
#include <memory>
#ifdef _WIN32
#include <share.h>
#endif

fb::logger::logger(fb::logger::level level, std::string service, std::string name) :
    _level(level),
    _service(std::move(service)),
    _name(std::move(name))
{
    if (this->_service.empty())
        this->_service = "unknown";
    if (this->_name.empty())
        this->_name = "unknown";

    this->_writer = std::thread(&logger::writer_run, this);
}

fb::logger::~logger()
{
    if (this->_writer.joinable() == false)
        return;

    this->_stop.store(true, std::memory_order_relaxed);
    this->_queue_cv.notify_all();
    this->_writer.join();
}

std::string fb::logger::daily_path(std::string_view service)
{
    auto dt = fb::model::datetime();
    auto s  = service.empty() ? "unknown" : service;
    return std::format("logs/{:04}-{:02}-{:02}-{}.log", dt.year(), dt.month(), dt.day(), s);
}

bool fb::logger::has_flag(fb::logger::level level) const
{
    return (this->_level & level) == level;
}

void fb::logger::enqueue(std::string line)
{
    {
        auto lock = std::unique_lock(this->_queue_mutex);
        if (this->_queue.size() >= this->_queue_limit)
            return;

        this->_queue.push_back(std::move(line));
    }
    this->_queue_cv.notify_one();
}

void fb::logger::writer_run()
{
    try
    {
        std::filesystem::create_directories("logs");
    }
    catch (...)
    {
        return;
    }

    auto  current = std::string{};
    FILE* fp      = nullptr;

    while (true)
    {
        auto batch = std::deque<std::string>{};
        {
            auto lock = std::unique_lock(this->_queue_mutex);
            this->_queue_cv.wait(lock, [this] {
                return this->_stop.load(std::memory_order_relaxed) || this->_queue.empty() == false;
            });
            batch.swap(this->_queue);
            if (this->_stop.load(std::memory_order_relaxed) && batch.empty())
                break;
        }

        auto path = daily_path(this->_service);
        if (path != current)
        {
            if (fp != nullptr)
            {
                std::fclose(fp);
                fp = nullptr;
            }
#ifdef _WIN32
            fp = _fsopen(path.c_str(), "a", _SH_DENYNO);
#else
            fp = std::fopen(path.c_str(), "a");
#endif
            if (fp == nullptr)
                return;
            if (std::ftell(fp) == 0)
            {
                static constexpr unsigned char bom[] = {0xEF, 0xBB, 0xBF};
                std::fwrite(bom, 1, sizeof(bom), fp);
            }
            current = std::move(path);
        }

        for (const auto& line : batch)
        {
            std::fwrite(line.data(), 1, line.size(), fp);
            std::fputc('\n', fp);
        }
        std::fflush(fp);
    }

    if (fp != nullptr)
        std::fclose(fp);
}

fb::logger& fb::logger::get()
{
    static std::once_flag              flag;
    static std::unique_ptr<fb::logger> ist;

    std::call_once(flag, [] {
        auto level      = (int)fb::logger::level::NONE;
        auto log_config = fb::config<>("log");
        auto name       = fb::config<std::string>("name", std::string{"unknown"});
        auto service    = std::string{"unknown"};

        if (log_config.isObject() && log_config.isMember("level"))
        {
            for (auto& x : log_config["level"])
            {
                auto value = boost::algorithm::to_lower_copy(x.asString());
                if (value == "debug")
                    level |= (int)fb::logger::level::DEBUG;
                else if (value == "info")
                    level |= (int)fb::logger::level::INFO;
                else if (value == "warn")
                    level |= (int)fb::logger::level::WARN;
                else if (value == "fatal")
                    level |= (int)fb::logger::level::FATAL;
            }
        }

        if (name.empty())
            name = "unknown";

        if (name.starts_with("gateway"))
            service = "gateway";
        else if (name.starts_with("login"))
            service = "login";
        else if (name.starts_with("game"))
            service = "game";

        ist = std::unique_ptr<fb::logger>(
            new fb::logger(static_cast<fb::logger::level>(level), std::move(service), std::move(name)));
    });
    return *ist;
}
