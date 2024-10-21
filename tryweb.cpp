#include "httplib.h"
#include <fstream>
#include <thread>
#include <string>
#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include <winhttp.h>
#include <mutex>
#include <future>
#include <vector>
#include <sstream>

const bool ID_NAME = true;
const bool AI_ANS = false;

//——————————————————————————————————————————————————————————————————————————————————————————————————————————————————
 std::string get_the_id() {//获取conversation id以供调用使用
    HINTERNET hSession = WinHttpOpen(L"Erzhi/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        std::cerr << "WinHttpOpen failed." << '\n';
        return "";
    }

    HINTERNET hConnect = WinHttpConnect(hSession, L"qianfan.baidubce.com", 443, 0);
    if (!hConnect) {
        std::cerr << "WinHttpConnect failed." << std::endl;
        WinHttpCloseHandle(hSession);
        return "";
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", L"/v2/app/conversation",
        NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        std::cerr << "WinHttpOpenRequest failed." << '\n';
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    const wchar_t* headers = L"Content-Type: application/json\r\nX-Appbuilder-Authorization: Bearer bce-v3/ALTAK-uYK7lgAwa1X5HeSxWbAMf/0e617ec0def18571613d5ba6ad9e944f40db2364";
    const char* jsonData = "{\"app_id\":\"88e73386-abd6-4a50-93aa-2cedd6c556e3\"}";

    WinHttpAddRequestHeaders(hRequest, headers, wcslen(headers), WINHTTP_ADDREQ_FLAG_ADD);
    if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        (LPVOID)jsonData, strlen(jsonData), strlen(jsonData), 0)) {
        std::cerr << "WinHttpSendRequest failed." << std::endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    if (!WinHttpReceiveResponse(hRequest, NULL)) {
        std::cerr << "WinHttpReceiveResponse failed." << std::endl;
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return "";
    }

    DWORD dwSize = 0;
    WinHttpQueryDataAvailable(hRequest, &dwSize);

    std::vector<char> buffer(static_cast<size_t>(dwSize)); // 使用 static_cast 避免警告
    WinHttpReadData(hRequest, buffer.data(), dwSize, NULL);
    std::string response(buffer.begin(), buffer.end());

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return response;
}
 //——————————————————————————————————————————————————————————————————————————————————————————————————————————————————
 std::string conversation(const std::string& response,bool mod) {//切割get_the_ip()返回的内容，获取真正的id
     int jishu = 0;
     std::string ans;
     if (mod)
     {
         for (char ch : response) {
             if (ch == '"') {
                 jishu++;
                 if (jishu == 3) {
                     continue;
                 }
                 if (jishu == 4) {
                     return ans; // Return the extracted string
                 }
             }
             if (jishu == 3) {
                 ans += ch;
             }
         }
     }
     else
     {
         for (char ch : response) {
             if (ch == '"') {
                 jishu++;
                 if (jishu == 11) {
                     continue;
                 }
                 if (jishu == 12) {
                     return ans; // Return the extracted string
                 }
             }
             if (jishu == 11) {
                 ans += ch;
             }
         }
     }
     return ""; // Return empty string if failed to find
 }
 //——————————————————————————————————————————————————————————————————————————————————————————————————————————————————
 // 获取 AI 回答
 std::string get_ai_answer(std::string conversation_id,const std::string& query) {
     HINTERNET hSession = WinHttpOpen(L"Erzhi/1.0",
         WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
         WINHTTP_NO_PROXY_NAME,
         WINHTTP_NO_PROXY_BYPASS, 0);

     if (!hSession) {
         std::cerr << "WinHttpOpen failed." << '\n';
         return "";
     }

     HINTERNET hConnect = WinHttpConnect(hSession, L"qianfan.baidubce.com", 443, 0);
     if (!hConnect) {
         std::cerr << "WinHttpConnect failed." << std::endl;
         WinHttpCloseHandle(hSession);
         return "";
     }

     HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"POST", L"/v2/app/conversation/runs",
         NULL, WINHTTP_NO_REFERER,
         WINHTTP_DEFAULT_ACCEPT_TYPES,
         WINHTTP_FLAG_SECURE);

     if (!hRequest) {
         std::cerr << "WinHttpOpenRequest failed." << '\n';
         WinHttpCloseHandle(hConnect);
         WinHttpCloseHandle(hSession);
         return "";
     }

     const wchar_t* headers = L"Content-Type: application/json\r\nX-Appbuilder-Authorization: Bearer bce-v3/ALTAK-uYK7lgAwa1X5HeSxWbAMf/0e617ec0def18571613d5ba6ad9e944f40db2364";

     // 构造请求数据
     std::string jsonData = "{\"app_id\":\"88e73386-abd6-4a50-93aa-2cedd6c556e3\",\"query\":\"" + query + "\",\"stream\":false,\"conversation_id\":\"" + conversation_id + "\"}";

     WinHttpAddRequestHeaders(hRequest, headers, wcslen(headers), WINHTTP_ADDREQ_FLAG_ADD);
     if (!WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0,
         (LPVOID)jsonData.c_str(), jsonData.length(), jsonData.length(), 0)) {
         std::cerr << "WinHttpSendRequest failed." << std::endl;
         WinHttpCloseHandle(hRequest);
         WinHttpCloseHandle(hConnect);
         WinHttpCloseHandle(hSession);
         return "";
     }

     if (!WinHttpReceiveResponse(hRequest, NULL)) {
         std::cerr << "WinHttpReceiveResponse failed." << std::endl;
         WinHttpCloseHandle(hRequest);
         WinHttpCloseHandle(hConnect);
         WinHttpCloseHandle(hSession);
         return "";
     }

     DWORD dwSize = 0;
     WinHttpQueryDataAvailable(hRequest, &dwSize);

     std::vector<char> buffer(static_cast<size_t>(dwSize));
     WinHttpReadData(hRequest, buffer.data(), dwSize, NULL);
     std::string response_data(buffer.begin(), buffer.end());

     WinHttpCloseHandle(hRequest);
     WinHttpCloseHandle(hConnect);
     WinHttpCloseHandle(hSession);

     return response_data;//conversation(response_data,AI_ANS); // 返回 AI 的回答
 }

 //——————————————————————————————————————————————————————————————————————————————————————————————————————————————————



int main() {
    httplib::Server svr;
    std::cout << get_ai_answer(conversation(get_the_id(), ID_NAME),"hello");
    std::cout << "Web server has been launched\n";

    // 读取 index.html 文件内容
    std::ifstream file("index.html");
    if (!file.is_open()) {
        std::cerr << "Failed to open index.html. Make sure it is in the same directory as the executable.\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string html_content = buffer.str();
    file.close();

    // 获取 IP 信息

    // 处理 GET 请求，并返回 HTML 内容
    svr.Get("/", [html_content](const httplib::Request&, httplib::Response& res) {
        res.set_content(html_content, "text/html");
        });

    // 监听 0.0.0.0 的 80 端口
    svr.listen("0.0.0.0", 11451);

    return 0;
}
