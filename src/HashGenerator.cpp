// Header include.
#include "HashGenerator.h"

#if THINGSBOARD_ENABLE_OTA

// Library includes.
#include <sstream>
#include <iomanip>

HashGenerator::HashGenerator() :
    m_ctx()
{
    // Nothing to do
}

HashGenerator::~HashGenerator(void) {
    // Ensures to clean up the mbedtls memory after it has been used
    mbedtls_md_free(&m_ctx);
}

void HashGenerator::start(const mbedtls_md_type_t& type) {
    // MBEDTLS Version 3 is a major breaking changes were accessing the internal structures requires the MBEDTLS_PRIVATE macro
#if MBEDTLS_VERSION_MAJOR < 3
    if (m_ctx.hmac_ctx != nullptr && m_ctx.md_ctx != nullptr && m_ctx.md_info != nullptr) {
#else
    if (m_ctx.MBEDTLS_PRIVATE(hmac_ctx) != nullptr && m_ctx.MBEDTLS_PRIVATE(md_ctx) != nullptr && m_ctx.MBEDTLS_PRIVATE(md_info) != nullptr) {
#endif
        mbedtls_md_free(&m_ctx);
    }
    // Initialize the context
    mbedtls_md_init(&m_ctx);
    // Choose the hash function
    mbedtls_md_setup(&m_ctx, mbedtls_md_info_from_type(type), 0);
    // Start the hash
    mbedtls_md_starts(&m_ctx);
}

bool HashGenerator::update(const uint8_t* data, const size_t& len) {
    return mbedtls_md_update(&m_ctx, data, len) == 0;
}

void HashGenerator::get_hash_string(unsigned char *hash) {
    finish(hash);
}

void HashGenerator::finish(unsigned char *hash) {
    mbedtls_md_finish(&m_ctx, hash);
}

#endif // THINGSBOARD_ENABLE_OTA
