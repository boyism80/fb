#ifndef __FB_CRYPTOR_H__
#define __FB_CRYPTOR_H__

/**
 * @file    crypto.h
 * @brief   Cryptographic system for secure data transmission and protocol protection
 * @author  FB Development Team
 *
 * @details This file implements the cryptographic system used for securing data
 *          transmission between clients and servers in the FB 2D MMORPG. The system
 *          provides encryption, decryption, and data wrapping capabilities using
 *          custom algorithms with lookup tables and sequence-based operations.
 *
 *          Key features:
 *          - Custom encryption/decryption algorithms with lookup table optimization
 *          - Sequence-based cryptographic operations for enhanced security
 *          - Data wrapping and unwrapping with size headers for protocol integrity
 *          - Configurable encryption keys and types for different security contexts
 *          - XOR-based cryptographic operations with key cycling for performance
 *          - Random key generation for dynamic security establishment
 *          - Parameter validation to ensure cryptographic integrity
 *          - Support for partial and full data stream encryption
 *          - Memory-safe key management with proper cleanup
 *          - Integration with the game protocol stack for seamless security
 *
 * @note    This cryptographic system is specifically designed for the FB game
 *          protocol and provides the security foundation for all client-server
 *          communications in the distributed game architecture.
 */

#include <fb/stream.h>
#include <algorithm>
#include <cstring>

namespace fb {

/**
 * @brief      A cryptographic class for encrypting and decrypting data streams.
 *
 *             This class provides encryption/decryption functionality using a custom
 *             algorithm with lookup tables and sequence-based operations. It supports
 *             wrapping/unwrapping data with headers and maintains encryption state.
 */
class crypto
{
public:
    /**
     * @brief      Hexadecimal lookup tables for encryption operations.
     */
    static uint32_t HEX_TABLE[10][0x100];

    /**
     * @brief      The size of the encryption key in bytes.
     */
    static constexpr uint32_t KEY_SIZE = 0x09;

private:
    uint8_t  _type     = 0;
    uint8_t* _key      = nullptr;
    uint8_t  _sequence = 0;

public:
    /**
     * @brief      Constructs a crypto object with default parameters.
     *
     *             Uses type 0 and default key "NexonInc.".
     */
    crypto();

    /**
     * @brief      Constructs a crypto object with specified type and key.
     *
     * @param[in]  types  The encryption type (used as index into HEX_TABLE).
     * @param[in]  key    The encryption key (must be at least KEY_SIZE bytes).
     */
    crypto(uint8_t types, const uint8_t* key);

    /**
     * @brief      Copy constructor.
     *
     * @param[in]  crt   The crypto object to copy.
     */
    crypto(const crypto& crt);

    /**
     * @brief      Destructor that cleans up allocated key memory.
     */
    ~crypto();

private:
    /**
     * @brief      Performs XOR-based cryptographic operation on data.
     *
     * @param[in]  source  The source data to encrypt/decrypt.
     * @param      dest    The destination buffer for the result.
     * @param[in]  size    The size of data to process.
     * @param[in]  key     The key to use for the operation.
     * @param[in]  ksize   The key size for cycling through key data.
     */
    void crypt(const uint8_t* source, uint8_t* dest, uint32_t size, const uint8_t* key, uint32_t ksize);

public:
    /**
     * @brief      Encrypts a portion of the data stream.
     *
     * @param      data    The data stream to encrypt.
     * @param[in]  offset  The offset within the stream to start encryption.
     * @param[in]  size    The number of bytes to encrypt.
     *
     * @return     The new size of the encrypted data.
     */
    uint32_t encrypt(fb::stream& data, uint32_t offset, uint32_t size);

    /**
     * @brief      Encrypts the entire data stream.
     *
     * @param      data  The data stream to encrypt.
     *
     * @return     The new size of the encrypted data, or -1 if data is empty.
     */
    uint32_t encrypt(fb::stream& data);

    /**
     * @brief      Decrypts a portion of the data stream.
     *
     * @param      data    The data stream to decrypt.
     * @param[in]  offset  The offset within the stream to start decryption.
     * @param[in]  size    The number of bytes to decrypt.
     *
     * @return     The new size of the decrypted data.
     */
    uint32_t decrypt(fb::stream& data, uint32_t offset, uint32_t size);

    /**
     * @brief      Decrypts the entire data stream.
     *
     * @param      data  The data stream to decrypt.
     *
     * @return     The new size of the decrypted data, or -1 if data is empty.
     */
    uint32_t decrypt(fb::stream& data);

    /**
     * @brief      Wraps data with a header containing size information.
     *
     * @param      data    The data stream to wrap.
     * @param[in]  offset  The offset within the stream to insert the header.
     *
     * @return     The new size including the header, or -1 if data is empty.
     */
    uint32_t wrap(fb::stream& data, uint32_t offset) const;

    /**
     * @brief      Wraps the entire data stream with a header.
     *
     * @param      data  The data stream to wrap.
     *
     * @return     The new size including the header, or -1 if data is empty.
     */
    uint32_t wrap(fb::stream& data) const;

    /**
     * @brief      Unwraps data by removing the header.
     *
     * @param      data    The data stream to unwrap.
     * @param[in]  offset  The offset within the stream where the header starts.
     *
     * @return     The new size without the header, or -1 if insufficient data.
     */
    uint32_t unwrap(fb::stream& data, uint32_t offset) const;

    /**
     * @brief      Unwraps the entire data stream by removing the header.
     *
     * @param      data  The data stream to unwrap.
     *
     * @return     The new size without the header, or -1 if insufficient data.
     */
    uint32_t unwrap(fb::stream& data) const;

public:
    /**
     * @brief      Gets the encryption type.
     *
     * @return     The current encryption type.
     */
    uint8_t type() const;

    /**
     * @brief      Gets the encryption key.
     *
     * @return     A pointer to the encryption key data.
     */
    const uint8_t* key() const;

public:
    /**
     * @brief      Assignment operator.
     *
     * @param[in]  crt   The crypto object to assign from.
     *
     * @return     A reference to this crypto object.
     */
    crypto& operator= (const crypto& crt);

public:
    /**
     * @brief      Generates a new crypto object with random parameters.
     *
     *             In debug builds, uses predictable values for testing.
     *             In release builds, uses random type and key values.
     *
     * @return     A new crypto object with generated parameters.
     */
    static crypto generate();

    /**
     * @brief      Validates encryption parameters.
     *
     * @param[in]  type   The encryption type to validate.
     * @param[in]  key    The encryption key to validate.
     * @param[in]  ksize  The key size to validate.
     *
     * @return     True if parameters are valid, false otherwise.
     */
    static bool validate(uint8_t type, const uint8_t* key, uint8_t ksize);
};

} // namespace fb

#endif // !__FB_CRYPTOR_H__
