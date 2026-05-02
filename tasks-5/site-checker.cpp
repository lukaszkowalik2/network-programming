#include <iostream>
#include <string>
#include <curl/curl.h>

size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;
    userp->append((char*)contents, totalSize);
    return totalSize;
}

int main() {
    CURL* curl;
    CURLcode res;
    std::string readBuffer;
    std::string url = "http://th.if.uj.edu.pl/";
    std::string expectedString = "Institute of Theoretical Physics";

    curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Błąd inicjalizacji CURL" << std::endl;
        return 2;
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

    res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        std::cerr << "Błąd połączenia: " << curl_easy_strerror(res) << std::endl;
        curl_easy_cleanup(curl);
        return 1;
    }

    long response_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

    char* content_type = nullptr;
    curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &content_type);

    bool status_ok = (response_code == 200);
    bool is_html = (content_type != nullptr && std::string(content_type).find("text/html") != std::string::npos);
    bool content_match = (readBuffer.find(expectedString) != std::string::npos);

    curl_easy_cleanup(curl);

    if (status_ok && is_html && content_match) {
        std::cout << "OK: Strona dziala poprawnie." << std::endl;
        return 0;
    } else {
        std::cerr << "BLAD: Warunki nie zostaly spelnione:" << std::endl;
        if (!status_ok) std::cerr << "- Kod statusu: " << response_code << std::endl;
        if (!is_html) std::cerr << "- Typ zawartosci: " << (content_type ? content_type : "nieznany") << std::endl;
        if (!content_match) std::cerr << "- Brak frazy: '" << expectedString << "'" << std::endl;
        return 1;
    }
}
