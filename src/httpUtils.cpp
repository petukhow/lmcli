#include "httpsUtils.h"
#include <curl/curl.h>
#include <curl/easy.h>
#include <stdexcept>

// =====================================
//                Curl
// =====================================

Curl::Curl() {
    curl = curl_easy_init();
    if (!curl) throw std::runtime_error("Failed to initialize CURL");
}  

CURL* Curl::get() const {
    return curl;
 }

Curl::~Curl() {
    curl_easy_cleanup(curl);
}

// =====================================
//               CurlSlist
// =====================================

void CurlSlist::append(const std::string& header) {
    headers = curl_slist_append(headers, header.c_str());
}

curl_slist* CurlSlist::get() const {
    return headers;
}

CurlSlist::~CurlSlist() {
    curl_slist_free_all(headers);
}