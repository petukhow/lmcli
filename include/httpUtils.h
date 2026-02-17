#pragma once
#include <curl/curl.h>
#include <string>

class Curl {
private:
    CURL *curl;

public:
    Curl(); 
    CURL* get() const;
    Curl(const Curl&) = delete; 
    Curl& operator=(const Curl&) = delete;
    ~Curl();

};

class CurlSlist {
private:
    curl_slist* headers = nullptr;

public:
    CurlSlist() = default; 
    curl_slist* get() const;
    void append(const std::string& header);
    CurlSlist(const CurlSlist&) = delete; 
    CurlSlist& operator=(const CurlSlist&) = delete;
    ~CurlSlist();
};