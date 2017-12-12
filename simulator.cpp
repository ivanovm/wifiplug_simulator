#include "Simple-Web-Server/server_http.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <boost/filesystem.hpp>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <vector>

using namespace std;

using HttpServer = SimpleWeb::Server<SimpleWeb::HTTP>;

/**
 * Get the machine primary IP address.
 *
 * Example taken from http://stackoverflow.com/a/3120382/1136400
 */
string getPrimaryIp() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        _exit(EXIT_FAILURE);
    }

    const char* kGoogleDnsIp = "8.8.8.8";
    uint16_t kDnsPort = 53;
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(kGoogleDnsIp);
    serv.sin_port = htons(kDnsPort);

    int err = connect(sock, (const sockaddr*)&serv, sizeof(serv));
    if (err == -1) {
        _exit(EXIT_FAILURE);
    }

    sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (sockaddr*)&name, &namelen);
    if (err == -1) {
        _exit(EXIT_FAILURE);
    }

    char ipBuf[48];  // IPv6 max = 8 * 4 + 7 = 39; + '\0' = 40
    const char* p = inet_ntop(AF_INET, &name.sin_addr, ipBuf, sizeof(ipBuf));
    if (NULL == p) {
        _exit(EXIT_FAILURE);
    }

    if (0 != ::close(sock)) {
        _exit(EXIT_FAILURE);
    }

    return ipBuf;
}

int main() {
  // HTTP-server at port 8080 using 1 thread
  // Unless you do more heavy non-threaded processing in the resources,
  // 1 thread is usually faster than several threads
  HttpServer server;
  server.config.port = 80;

  bool isPlugOn = false;

  server.resource["^/on"]["GET"] = [&isPlugOn](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> /*request*/) {
    thread work_thread([response, &isPlugOn] {
        isPlugOn = true;
    });
    work_thread.detach();
  };

  server.resource["^/off"]["GET"] = [&isPlugOn](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> /*request*/) {
    thread work_thread([response, &isPlugOn] {
        isPlugOn = false;
    });
    work_thread.detach();
  };

  std::string ipAddress = getPrimaryIp();

  server.resource["^/state"]["GET"] = [ipAddress, &isPlugOn](shared_ptr<HttpServer::Response> response, shared_ptr<HttpServer::Request> /*request*/) {
    thread work_thread([response, ipAddress, &isPlugOn] {
        std::string stateValue = isPlugOn ? "1" : "0";
        *response << "{id:000001,state:" << stateValue << ",ip:" << ipAddress << "}";
    });
    work_thread.detach();
  };

  server.on_error = [](shared_ptr<HttpServer::Request> /*request*/, const SimpleWeb::error_code & /*ec*/) {
    // Handle errors here
  };

  thread server_thread([&server]() {
    // Start server
    server.start();
  });

  // Wait for server to start so that the client can connect
  this_thread::sleep_for(chrono::seconds(1));

  server_thread.join();
}
