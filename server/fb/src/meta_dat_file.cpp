#include <fb/meta_dat_file.h>
#include <fb/encoding.h>
#include <fb/stream_reader.h>
#include <zlib.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <format>

namespace fb {

namespace {

std::vector<uint8_t> inflate_bytes(const uint8_t* in, size_t in_size)
{
    auto run = [&](int window_bits) -> std::vector<uint8_t> {
        z_stream stream{};
        if (inflateInit2(&stream, window_bits) != Z_OK)
            throw std::runtime_error("zlib inflateInit failed");

        stream.next_in  = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(in));
        stream.avail_in = static_cast<uInt>(in_size);

        auto out = std::vector<uint8_t>(in_size * 4 + 64);
        int  ret = Z_OK;
        do
        {
            if (stream.total_out >= out.size())
                out.resize(out.size() * 2);
            stream.next_out  = out.data() + stream.total_out;
            stream.avail_out = static_cast<uInt>(out.size() - stream.total_out);
            ret              = inflate(&stream, Z_NO_FLUSH);
        } while (ret == Z_OK);

        inflateEnd(&stream);
        if (ret != Z_STREAM_END)
            throw std::runtime_error("zlib inflate failed");
        out.resize(stream.total_out);
        return out;
    };

    try
    {
        return run(MAX_WBITS);
    }
    catch (const std::runtime_error&)
    {
        return run(-MAX_WBITS);
    }
}

struct tree_node
{
    std::string              name;
    std::vector<std::string> children;
};

std::vector<tree_node> parse_tree(std::vector<uint8_t>& data)
{
    auto reader    = fb::stream_reader<big_endian>(data);
    auto top_count = reader.read<uint16_t>();
    auto nodes     = std::vector<tree_node>{};
    nodes.reserve(top_count);

    for (uint16_t i = 0; i < top_count; ++i)
    {
        auto name        = reader.read<std::string>();
        auto child_count = reader.read<uint16_t>();
        auto children    = std::vector<std::string>{};
        children.reserve(child_count);
        for (uint16_t c = 0; c < child_count; ++c)
            children.push_back(reader.read<std::string, uint16_t>());
        nodes.push_back({std::move(name), std::move(children)});
    }
    return nodes;
}

uint8_t to_u8(std::string_view s)
{
    return static_cast<uint8_t>(std::stoul(std::string(s)));
}

uint32_t to_u32(std::string_view s)
{
    return static_cast<uint32_t>(std::stoul(std::string(s)));
}

std::string trim_copy(std::string_view s)
{
    auto begin = s.find_first_not_of(" \t");
    if (begin == std::string_view::npos)
        return std::string{};

    auto end = s.find_last_not_of(" \t");
    return std::string(s.substr(begin, end - begin + 1));
}

} // namespace

void meta_dat_file::load(std::string_view path, bool parse_collections)
{
    this->_entries.clear();
    this->_groups.clear();

    auto file = std::filesystem::path(std::string(path));
    if (std::filesystem::exists(file) == false)
        throw std::runtime_error(std::format("Meta.dat not found: {}", file.string()));

    auto size = std::filesystem::file_size(file);
    auto buf  = std::vector<uint8_t>(static_cast<size_t>(size));
    auto in   = std::ifstream(file, std::ios::binary);
    if (!in.read(reinterpret_cast<char*>(buf.data()), static_cast<std::streamsize>(size)))
        throw std::runtime_error(std::format("failed to read Meta.dat: {}", file.string()));

    auto le = fb::stream_reader<little_endian>(buf);
    auto be = fb::stream_reader<big_endian>(buf);

    auto count      = le.read<uint32_t>();
    auto index_size = le.read<uint32_t>();
    auto names      = std::vector<std::pair<std::string, uint32_t>>{};
    names.reserve(count);
    for (uint32_t i = 0; i < count; ++i)
    {
        auto nlen = le.read<uint16_t>();
        auto wide = std::wstring(nlen, L'\0');
        for (uint16_t w = 0; w < nlen; ++w)
            wide[w] = static_cast<wchar_t>(le.read<uint16_t>());

        be.seek(le.seek());
        auto compressed_size = be.read<uint32_t>();
        be.read<uint32_t>();
        le.seek(be.seek());
        names.emplace_back(M(wide), compressed_size);
    }

    le.seek(8 + index_size);
    auto plains = std::vector<std::vector<uint8_t>>{};
    this->_entries.reserve(count);
    plains.reserve(count);
    for (auto& [name, compressed_size] : names)
    {
        if (le.readable_size() < compressed_size)
            throw std::runtime_error(std::format("Meta.dat blob truncated: {}", name));

        auto entry = meta_dat_entry{};
        entry.name = std::move(name);
        entry.compressed.resize(compressed_size);
        le.read(entry.compressed.data(), compressed_size);

        auto plain = inflate_bytes(reinterpret_cast<const uint8_t*>(entry.compressed.data()), compressed_size);
        entry.crc  = static_cast<uint32_t>(crc32(0L, plain.data(), static_cast<uInt>(plain.size())));
        this->_entries.push_back(std::move(entry));
        plains.push_back(std::move(plain));
    }

    if (parse_collections == false)
        return;

    for (size_t i = 0; i < this->_entries.size(); ++i)
    {
        if (this->_entries[i].name != "GroupNames")
            continue;
        for (const auto& node : parse_tree(plains[i]))
        {
            auto group = meta_dat_collection_group{};
            group.id   = to_u8(node.name);
            group.name = node.children.empty() ? std::string{} : trim_copy(node.children.front());
            this->_groups.push_back(std::move(group));
        }
    }

    for (size_t i = 0; i < this->_entries.size(); ++i)
    {
        if (this->_entries[i].name != "Collections")
            continue;
        for (const auto& node : parse_tree(plains[i]))
        {
            if (node.children.size() < 4)
                continue;
            auto item        = meta_dat_collection_item{};
            item.name        = trim_copy(node.name);
            item.group_id    = to_u8(node.children[0]);
            item.slot        = to_u8(node.children[1]);
            item.look        = to_u32(node.children[2]);
            item.description = node.children[3];
            for (auto& group : this->_groups)
            {
                if (group.id != item.group_id)
                    continue;
                if (group.items.size() <= item.slot)
                    group.items.resize(item.slot + 1);
                group.items[item.slot] = std::move(item);
                break;
            }
        }
    }
}

const std::vector<meta_dat_entry>& meta_dat_file::entries() const
{
    return this->_entries;
}

const meta_dat_entry* meta_dat_file::find(std::string_view name) const
{
    for (const auto& entry : this->_entries)
    {
        if (entry.name == name)
            return &entry;
    }
    return nullptr;
}

std::vector<std::pair<std::string, uint32_t>> meta_dat_file::crc_list() const
{
    auto list = std::vector<std::pair<std::string, uint32_t>>{};
    list.reserve(this->_entries.size());
    for (const auto& entry : this->_entries)
        list.emplace_back(entry.name, entry.crc);
    return list;
}

const std::vector<meta_dat_collection_group>& meta_dat_file::groups() const
{
    return this->_groups;
}

const meta_dat_collection_group* meta_dat_file::group(uint8_t id) const
{
    for (const auto& group : this->_groups)
    {
        if (group.id == id)
            return &group;
    }
    return nullptr;
}

const meta_dat_collection_item* meta_dat_file::find_item(std::string_view name) const
{
    for (const auto& group : this->_groups)
    {
        for (const auto& item : group.items)
        {
            if (item.name.empty())
                continue;
            if (item.name == name)
                return &item;
        }
    }
    return nullptr;
}

const meta_dat_collection_item* meta_dat_file::item(uint8_t group_id, uint8_t slot) const
{
    auto* group = this->group(group_id);
    if (group == nullptr || slot >= group->items.size())
        return nullptr;

    auto& entry = group->items[slot];
    if (entry.name.empty())
        return nullptr;

    return &entry;
}

} // namespace fb
