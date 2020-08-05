#include <module/stream/stream.h>

fb::buffer::buffer(size_t size) : _position(0), _offset(0)
{
    this->reserve(size);
}

fb::buffer::buffer(const uint8_t* data, size_t size) : _position(0), _offset(0), std::vector<uint8_t>(data, data + size)
{}

fb::buffer::~buffer()
{
}

uint32_t fb::buffer::position() const
{
    return this->_position;
}

fb::buffer fb::buffer::compress() const
{
    static uint8_t              buffer[65536];
    uint32_t                    size = sizeof(buffer);

    compress2(buffer, (uLongf*)&size, this->data(), uint32_t(this->size()), Z_BEST_COMPRESSION);
    buffer[size] = 0x00;
    return fb::buffer(buffer, size);
}

uint32_t fb::buffer::crc() const
{
    return crc32(0, this->data(), (uint32_t)this->size());
}

fb::istream::istream(size_t size) : buffer(size)
{
}

fb::istream::istream(const uint8_t* data, size_t size) : buffer(data, size)
{
}

fb::istream::~istream()
{
}

int8_t fb::istream::read_8()
{
    int8_t          value = (*this)[this->_position + this->_offset];
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
    int16_t         value = (*this)[this->_position + this->_offset] | (*this)[this->_position + this->_offset + 1] << 8;
    if(endian == buffer::endian::BIG)
        value = _byteswap_ushort(value);
#else
    int16_t         value = (*this)[this->_position + this->_offset] << 8 | (*this)[this->_position + this->_offset + 1];
    if(byte_order == buffer::byte_order::LITTLE_ENDIAN)
        value = _byteswap_ushort(value);
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
    int32_t         value = (*this)[this->_position + this->_offset + 0] <<  0 |
                            (*this)[this->_position + this->_offset + 1] <<  8 |
                            (*this)[this->_position + this->_offset + 2] << 16 |
                            (*this)[this->_position + this->_offset + 3] << 24;
    if(endian == buffer::endian::BIG)
        value = _byteswap_ulong(value);
#else
    int32_t         value = (*this)[this->_position + this->_offset + 0] <<  0 |
                            (*this)[this->_position + this->_offset + 1] <<  8 |
                            (*this)[this->_position + this->_offset + 2] << 16 |
                            (*this)[this->_position + this->_offset + 3] << 24;
    if(byte_order == buffer::byte_order::LITTLE_ENDIAN)
        value = _byteswap_ulong(value);
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
		memcpy(buffer, this->data() + this->_position + this->_offset, size);
    this->_offset += uint32_t(size);
}

std::string fb::istream::readstr_u8()
{
	uint8_t					size = this->read_u8();
	char					buffer[0xFF];
	this->read(buffer, size);
	buffer[std::min(size, uint8_t(0xFF))] = 0x00;

	return std::string(buffer);
}

std::string fb::istream::readstr_u16(buffer::endian endian)
{
	uint16_t				size = this->read_u16(endian);
	char					buffer[0xFF];
	this->read(buffer, size);
	buffer[std::min(size, uint16_t(0xFF))] = 0x00;

	return std::string(buffer);
}

std::string fb::istream::readstr_u32(buffer::endian endian)
{
    uint32_t				size = this->read_u32(endian);
    char					buffer[0xFF];
    this->read(buffer, size);
    buffer[std::min(size, uint32_t(0xFF))] = 0x00;

    return std::string(buffer);
}

uint32_t fb::istream::readable_size() const
{
    return std::max(uint32_t(0), uint32_t(this->size() - this->_position - this->_offset));
}

void fb::istream::reset()
{
    this->_offset = 0;
}

void fb::istream::shift(size_t size)
{
    this->_position = std::min(uint32_t(this->_position + size), uint32_t(this->capacity() - 1));
    this->_offset = uint32_t(this->_offset > size ? this->_offset - size : 0);
}

void fb::istream::flush()
{
    this->erase(this->begin(), this->begin() + this->_position);
    this->_position = 0;
}

fb::ostream::ostream(size_t size) : buffer(size)
{
}

fb::ostream::ostream(const uint8_t* data, size_t size) : buffer(size)
{
    this->insert(this->end(), data, data + size);
}

fb::ostream::~ostream()
{
}

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
    if(endian == buffer::endian::LITTLE)
    {
        this->push_back(value & 0xFF);
        this->push_back(value >> 8 & 0xFF);
    }
    else
    {
        this->push_back(value >> 8 & 0xFF);
        this->push_back(value & 0xFF);
    }
#else
    if(endian == buffer::endian::LITTLE)
    {
        this->push_back(value >> 8 & 0xFF);
        this->push_back(value & 0xFF);
    }
    else
    {
        this->push_back(value & 0xFF);
        this->push_back(value >> 8 & 0xFF);
    }
#endif
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
    if(endian == buffer::endian::LITTLE)
    {
        this->push_back(value & 0xFF);
        this->push_back(value >> 8 & 0xFF);
        this->push_back(value >> 16 & 0xFF);
        this->push_back(value >> 24 & 0xFF);
    }
    else
    {
        this->push_back(value >> 24 & 0xFF);
        this->push_back(value >> 16 & 0xFF);
        this->push_back(value >> 8 & 0xFF);
        this->push_back(value & 0xFF);
    }
#else
    if(endian == buffer::endian::LITTLE)
    {
        this->push_back(value >> 24 & 0xFF);
        this->push_back(value >> 16 & 0xFF);
        this->push_back(value >> 8 & 0xFF);
        this->push_back(value & 0xFF);
    }
    else
    {
        this->push_back(value & 0xFF);
        this->push_back(value >> 8 & 0xFF);
        this->push_back(value >> 16 & 0xFF);
        this->push_back(value >> 24 & 0xFF);
    }
#endif
    
    return *this;
}

fb::ostream& fb::ostream::write_u32(uint32_t value, buffer::endian endian)
{
    this->write_32(value, endian);
    return *this;
}

fb::ostream& fb::ostream::writestr_u8(const std::string& value)
{
    this->write_u8((uint8_t)value.length());
    this->write(value.c_str(), value.length());
    return *this;
}

fb::ostream& fb::ostream::writestr_u16(const std::string& value, buffer::endian endian)
{
    this->write_u16((uint16_t)value.length());
    this->write(value.c_str(), value.length());
    return *this;
}

fb::ostream& fb::ostream::writestr_u32(const std::string& value, buffer::endian endian)
{
    this->write_u32((uint32_t)value.length());
    this->write(value.c_str(), value.length());
    return *this;
}

fb::ostream& fb::ostream::write(const void * buffer, size_t size)
{
    this->insert(this->end(), (uint8_t*)buffer, (uint8_t*)buffer + size);
    return *this;
}

fb::ostream& fb::ostream::write(const ostream& wb)
{
    this->insert(this->end(), wb.begin(), wb.end());
    return *this;
}

fb::ostream& fb::ostream::write(const std::string& str, bool uint16)
{
	if(uint16)
		this->write_u16((uint16_t)str.size());
	else
		this->write_u8((uint8_t)str.size());
    
	if(str.empty() == false)
		this->write(str.c_str(), str.size());

	return *this;
}
