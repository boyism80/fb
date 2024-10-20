#include <fb/stream.h>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

fb::buffer::buffer(size_t size)
{
    this->reserve(size);
}

fb::buffer::buffer(const uint8_t* data, size_t size)
{
    std::vector<uint8_t>::reserve(size);
    std::vector<uint8_t>::assign(data, data + size);
}

fb::buffer::buffer(const buffer & right) : fb::buffer((uint8_t*)right.data(), right.size())
{ }

fb::buffer::~buffer()
{ }

uint32_t fb::buffer::offset() const
{
    return this->_offset;
}

const void* fb::buffer::data() const
{
    return std::vector<uint8_t>::data();
}

fb::buffer fb::buffer::compress() const
{
    uint32_t                    src_size = this->size();
    uint32_t                    dst_size = this->size() * 2;
    uint8_t*                    buffer = new uint8_t[dst_size];

    if(compress2(buffer, (uLongf*)&dst_size, vector<uint8_t>::data(), uint32_t(this->size()), Z_BEST_COMPRESSION) == Z_STREAM_ERROR)
        throw std::runtime_error("cannot compress data");

    auto compressed = fb::buffer(buffer, dst_size);
    delete[] buffer;
    return compressed;
}

fb::buffer fb::buffer::decompress() const
{
    uint32_t src_size = this->size();
    uint32_t dst_size = this->size() * 2;
    uint8_t* buffer = new uint8_t[dst_size];

    if(uncompress(buffer, (uLongf*)& dst_size, vector<uint8_t>::data(), uint32_t(this->size())) != Z_OK)
        throw std::runtime_error("cannot compress data");

    auto decompressed = fb::buffer(buffer, dst_size);
    delete[] buffer;
    return decompressed;
}

void fb::buffer::clear()
{
    std::vector<uint8_t>::clear();
    this->_offset = 0;
}

uint32_t fb::buffer::crc() const
{
    return crc32(0, vector<uint8_t>::data(), (uint32_t)this->size());
}

fb::istream::istream(size_t size) : buffer(size)
{ }

fb::istream::istream(const uint8_t* data, size_t size) : buffer(data, size)
{ }

fb::istream::~istream()
{ }

int8_t fb::istream::read_8()
{
    int8_t          value = (*this)[this->_offset];
    this->_offset++;
    return value;
}

uint8_t fb::istream::read_u8()
{
    return (uint8_t)this->read_8();
}

int16_t fb::istream::read_16(buffer::endian endian)
{
#ifdef _WIN32
    int16_t         value = (*this)[this->_offset] | (*this)[this->_offset + 1] << 8;
    if(endian == fb::buffer::endian::BIG)
        value = _byteswap_ushort(value);
#else
    int16_t         value = (*this)[this->_offset] << 8 | (*this)[this->_offset + 1];
    if(endian == fb::buffer::endian::LITTLE)
        value = __bswap_16(value);
#endif
    this->_offset += sizeof(int16_t);
    return value;
}

uint16_t fb::istream::read_u16(buffer::endian endian)
{
    return (uint16_t)this->read_16(endian);
}

int32_t fb::istream::read_32(buffer::endian endian)
{
#ifdef _WIN32
    int32_t         value = (*this)[this->_offset + 0] <<  0 |
                            (*this)[this->_offset + 1] <<  8 |
                            (*this)[this->_offset + 2] << 16 |
                            (*this)[this->_offset + 3] << 24;
    if(endian == fb::buffer::endian::BIG)
        value = _byteswap_ulong(value);
#else
    int32_t         value = (*this)[this->_offset + 0] << 24 |
                            (*this)[this->_offset + 1] << 16 |
                            (*this)[this->_offset + 2] <<  8 |
                            (*this)[this->_offset + 3] <<  0;
    if(endian == fb::buffer::endian::LITTLE)
        value = __bswap_32(value);
#endif
    this->_offset += sizeof(int32_t);
    return value;
}

uint32_t fb::istream::read_u32(buffer::endian endian)
{
    return (uint32_t)this->read_32(endian);
}

void fb::istream::read(void * buffer, size_t size)
{
    if(buffer != nullptr)
        memcpy(buffer, (uint8_t*)this->data() + this->_offset, size);
    this->_offset += uint32_t(size);
}

std::string fb::istream::readstr()
{
    auto size = strlen((const char*)this->data() + this->_offset);
    char* buffer = new char[size + 1];
    this->read(buffer, size + 1);

    std::string text(buffer);
    delete[] buffer;
    this->_offset += size + 1;
    return UTF8(text, PLATFORM::Linux);
}

std::string fb::istream::readstr_u8()
{
    uint8_t                 size = this->read_u8();
    char                    buffer[0xFF];
    this->read(buffer, size);
    buffer[std::min(size, uint8_t(0xFF))] = 0x00;

    return UTF8(buffer, PLATFORM::Linux);
}

std::string fb::istream::readstr_u16(buffer::endian endian)
{
    uint16_t                size = this->read_u16(endian);
    char                    buffer[0xFF];
    this->read(buffer, size);
    buffer[std::min(size, uint16_t(0xFF))] = 0x00;

    return UTF8(buffer, PLATFORM::Linux);
}

std::string fb::istream::readstr_u32(buffer::endian endian)
{
    uint32_t                size = this->read_u32(endian);
    char                    buffer[0xFF];
    this->read(buffer, size);
    buffer[std::min(size, uint32_t(0xFF))] = 0x00;

    return UTF8(buffer, PLATFORM::Linux);
}

uint32_t fb::istream::readable_size() const
{
    return std::max(uint32_t(0), uint32_t(this->size() - this->_offset));
}

void fb::istream::reset()
{
    this->_offset = 0;
}

void fb::istream::shift(size_t size)
{
    this->_offset = std::min(uint32_t(this->_offset + size), uint32_t(this->capacity() - 1));
}

void fb::istream::flush()
{
    this->erase(this->begin(), this->begin() + this->_offset);
    this->_offset = 0;
}

fb::ostream::ostream(size_t size) : buffer(size)
{ }

fb::ostream::ostream(const uint8_t* data, size_t size) : buffer(size)
{
    this->insert(this->end(), data, data + size);
}

fb::ostream::~ostream()
{ }

fb::ostream& fb::ostream::write_8(int8_t value)
{
    this->push_back(value);
    return *this;
}

fb::ostream& fb::ostream::write_u8(uint8_t value)
{
    this->write_8(value);
    return *this;
}

fb::ostream& fb::ostream::write_16(int16_t value, buffer::endian endian)
{
#ifdef _WIN32
    if(endian == fb::buffer::endian::BIG)
        value = _byteswap_ushort(value);
#else
    if(endian == fb::buffer::endian::BIG)
        value = __bswap_16(value);
#endif
    this->push_back(value & 0xFF);
    this->push_back(value >> 8 & 0xFF);
    return *this;
}

fb::ostream& fb::ostream::write_u16(uint16_t value, buffer::endian endian)
{
    this->write_16(value, endian);
    return *this;
}

fb::ostream& fb::ostream::write_32(int32_t value, buffer::endian endian)
{
#ifdef _WIN32
    if(endian == fb::buffer::endian::BIG)
        value = _byteswap_ulong(value);
#else
    if(endian == fb::buffer::endian::BIG)
        value = __bswap_32(value);
#endif
    this->push_back(value & 0xFF);
    this->push_back(value >> 8 & 0xFF);
    this->push_back(value >> 16 & 0xFF);
    this->push_back(value >> 24 & 0xFF);
    return *this;
}

fb::ostream& fb::ostream::write_u32(uint32_t value, buffer::endian endian)
{
    this->write_32(value, endian);
    return *this;
}

fb::ostream& fb::ostream::writestr_u8(const std::string& value)
{
    auto cp949 = CP949(value, PLATFORM::Linux);
    this->write_u8((uint8_t)cp949.length());
    this->write(cp949.c_str(), cp949.length());
    return *this;
}

fb::ostream& fb::ostream::writestr_u16(const std::string& value, buffer::endian endian)
{
    auto cp949 = CP949(value, PLATFORM::Linux);
    this->write_u16((uint16_t)cp949.length());
    this->write(cp949.c_str(), cp949.length());
    return *this;
}

fb::ostream& fb::ostream::writestr_u32(const std::string& value, buffer::endian endian)
{
    auto cp949 = CP949(value, PLATFORM::Linux);
    this->write_u32((uint32_t)cp949.length());
    this->write(cp949.c_str(), cp949.length());
    return *this;
}

fb::ostream& fb::ostream::write(const void * buffer, size_t size)
{
    this->insert(this->end(), (uint8_t*)buffer, (uint8_t*)buffer + size);
    return *this;
}

fb::ostream& fb::ostream::write(const buffer& wb)
{
    this->insert(this->end(), wb.begin(), wb.end());
    return *this;
}

fb::ostream& fb::ostream::write(const std::string& str, bool uint16)
{
    auto cp949 = CP949(str, PLATFORM::Linux);
    if(uint16)
        this->write_u16((uint16_t)cp949.size());
    else
        this->write_u8((uint8_t)cp949.size());
    
    if(cp949.empty() == false)
        this->write(cp949.c_str(), cp949.size());

    return *this;
}
