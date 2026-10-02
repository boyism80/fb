#include <fb/hmac.h>

#include <algorithm>
#include <bit>
#include <cstdint>
#include <cstring>

using namespace fb;

sha256::sha256() :
    _state{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19}
{ }

void sha256::transform(const uint8_t* block)
{
    uint32_t w[64];
    for (int i = 0; i < 16; i++)
    {
        w[i] = (uint32_t(block[i * 4]) << 24) | (uint32_t(block[i * 4 + 1]) << 16) | (uint32_t(block[i * 4 + 2]) << 8) |
               uint32_t(block[i * 4 + 3]);
    }
    for (int i = 16; i < 64; i++)
    {
        auto s0 = std::rotr(w[i - 15], 7) ^ std::rotr(w[i - 15], 18) ^ (w[i - 15] >> 3);
        auto s1 = std::rotr(w[i - 2], 17) ^ std::rotr(w[i - 2], 19) ^ (w[i - 2] >> 10);
        w[i]    = w[i - 16] + s0 + w[i - 7] + s1;
    }

    auto a = this->_state[0];
    auto b = this->_state[1];
    auto c = this->_state[2];
    auto d = this->_state[3];
    auto e = this->_state[4];
    auto f = this->_state[5];
    auto g = this->_state[6];
    auto h = this->_state[7];
    for (int i = 0; i < 64; i++)
    {
        auto s1  = std::rotr(e, 6) ^ std::rotr(e, 11) ^ std::rotr(e, 25);
        auto ch  = (e & f) ^ (~e & g);
        auto t1  = h + s1 + ch + K[i] + w[i];
        auto s0  = std::rotr(a, 2) ^ std::rotr(a, 13) ^ std::rotr(a, 22);
        auto maj = (a & b) ^ (a & c) ^ (b & c);
        auto t2  = s0 + maj;

        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    this->_state[0] += a;
    this->_state[1] += b;
    this->_state[2] += c;
    this->_state[3] += d;
    this->_state[4] += e;
    this->_state[5] += f;
    this->_state[6] += g;
    this->_state[7] += h;
}

void sha256::update(const void* data, size_t size)
{
    auto bytes     = static_cast<const uint8_t*>(data);
    this->_length += size;
    while (size > 0)
    {
        auto n = std::min(size, BLOCK_SIZE - this->_buffered);
        std::memcpy(this->_buffer.data() + this->_buffered, bytes, n);
        this->_buffered += n;
        bytes           += n;
        size            -= n;

        if (this->_buffered == BLOCK_SIZE)
        {
            this->transform(this->_buffer.data());
            this->_buffered = 0;
        }
    }
}

sha256::digest_type sha256::final()
{
    auto bit_length = this->_length * 8;

    auto pad = uint8_t{0x80};
    this->update(&pad, 1);

    auto zero = uint8_t{0};
    while (this->_buffered != BLOCK_SIZE - sizeof(uint64_t))
    {
        this->update(&zero, 1);
    }

    uint8_t length_bytes[sizeof(uint64_t)];
    for (int i = 0; i < 8; i++)
    {
        length_bytes[i] = static_cast<uint8_t>(bit_length >> (56 - i * 8));
    }
    this->update(length_bytes, sizeof(length_bytes));

    auto digest = digest_type{};
    for (int i = 0; i < 8; i++)
    {
        digest[i * 4]     = static_cast<uint8_t>(this->_state[i] >> 24);
        digest[i * 4 + 1] = static_cast<uint8_t>(this->_state[i] >> 16);
        digest[i * 4 + 2] = static_cast<uint8_t>(this->_state[i] >> 8);
        digest[i * 4 + 3] = static_cast<uint8_t>(this->_state[i]);
    }
    return digest;
}

sha256::digest_type fb::hmac_sha256(const void* key, size_t key_size, const void* data, size_t data_size)
{
    auto block = std::array<uint8_t, sha256::BLOCK_SIZE>{};
    if (key_size > sha256::BLOCK_SIZE)
    {
        auto hasher = sha256();
        hasher.update(key, key_size);
        auto digest = hasher.final();
        std::memcpy(block.data(), digest.data(), digest.size());
    }
    else
    {
        std::memcpy(block.data(), key, key_size);
    }

    auto inner_pad = block;
    auto outer_pad = block;
    for (size_t i = 0; i < sha256::BLOCK_SIZE; i++)
    {
        inner_pad[i] ^= 0x36;
        outer_pad[i] ^= 0x5c;
    }

    auto inner = sha256();
    inner.update(inner_pad.data(), inner_pad.size());
    inner.update(data, data_size);
    auto inner_digest = inner.final();

    auto outer = sha256();
    outer.update(outer_pad.data(), outer_pad.size());
    outer.update(inner_digest.data(), inner_digest.size());
    return outer.final();
}

bool fb::constant_time_equal(const void* lhs, const void* rhs, size_t size)
{
    auto l    = static_cast<const volatile uint8_t*>(lhs);
    auto r    = static_cast<const volatile uint8_t*>(rhs);
    auto diff = uint8_t{0};
    for (size_t i = 0; i < size; i++)
    {
        diff |= l[i] ^ r[i];
    }
    return diff == 0;
}
