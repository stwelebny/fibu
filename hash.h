
#include <openssl/evp.h>
#include <openssl/sha.h>

std::string calculateSHA256(const std::string& data) {
    // Create a buffer to hold the hash
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int lengthOfHash = 0;

    // Create a `EVP_MD_CTX` to hold the hash context
    EVP_MD_CTX* context = EVP_MD_CTX_new();

    if(context != nullptr) {
        // Initialize the context to use the desired hash function, SHA-256 in this case
        if(EVP_DigestInit_ex(context, EVP_sha256(), nullptr)) {
            // Update the context with the data
            if(EVP_DigestUpdate(context, data.c_str(), data.size())) {
                // Finalize the context and retrieve the hash
                EVP_DigestFinal_ex(context, hash, &lengthOfHash);
            }
        }
        // Clean up the context
        EVP_MD_CTX_free(context);
    }

    // Convert the hash to a hex string
    std::stringstream ss;
    for(unsigned int i = 0; i < lengthOfHash; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }

    return ss.str();
}

