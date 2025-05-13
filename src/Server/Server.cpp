#include "Server/Server.hpp"
#include "GLog/Log.hpp"
#include "GNetworking/Socket.hpp"
#include "GParsing/GParsing.hpp"
#include <chrono>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <ios>
#include <openssl/ssl.h>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace Wepp {
Server::Server(const size_t &_threadCount, const std::filesystem::path &_dataDir) : m_THREAD_COUNT(_threadCount), m_dataDir(_dataDir) {
  GetClientSockets().resize(0);

  if (!std::filesystem::exists(m_dataDir)) {
    std::filesystem::create_directories(_dataDir);
  }
}

Server::~Server() {}

void Server::Run(const std::string &_address, const uint16_t _port) {
  GLog::Log(GLog::LOG_TRACE, "Starting Server");

  _Setup(_address, _port);

  _MainLoop();

  _Cleanup();
}

void Server::_MainLoop() {
  while (true) {
    std::this_thread::sleep_for(std::chrono::microseconds(5));

    _AcceptConnections();
    _HandleClients();
    _CloseConnections();
  }
}

void Server::_Setup(const std::string &_address, const uint16_t _port) {
  const SSL_METHOD *serverMethod = TLS_server_method();
  GLog::Log(GLog::LOG_DEBUG, "Server Setup");
  if (GNetworking::SocketSetup() != 0) {
    throw std::runtime_error("Sockets Setup error");
  }

  m_sslCTX = SSL_CTX_new(serverMethod);
  if (!m_sslCTX) {
    throw std::runtime_error("Cannot create OpenSSL Context");
  }

  if (SSL_CTX_use_certificate_file(m_sslCTX, "ssl.crt.pem", SSL_FILETYPE_PEM) <=
      0) {
    throw std::runtime_error("Cannot assign ssl.crt.pem to SSL context");
  }

  if (SSL_CTX_use_PrivateKey_file(m_sslCTX, "ssl.key.pem", SSL_FILETYPE_PEM) <=
      0) {
    throw std::runtime_error("Cannot assign ssl.key.pem to SSL context");
  }

  GetServerSocket() =
      GNetworking::SocketCreate(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (GetServerSocket() == GNetworkingInvalidSocket) {
    throw std::runtime_error("Cannot create server socket");
  }

  int value = 1;
  if (GNetworking::SocketSetOption(GetServerSocket(), SOL_SOCKET, SO_REUSEADDR,
                                   &value, sizeof(value)) != 0) {
    throw std::runtime_error("Cannot set socket option SO_REUSEADDR");
  }

  if (GNetworking::SocketBind(GetServerSocket(), _address, _port) != 0) {
    throw std::runtime_error("Cannot bind server socket to address");
  }

  if (GNetworking::SocketListen(GetServerSocket()) != 0) {
    throw std::runtime_error("Cannot liston on binded server socket");
  }
}

void Server::_Cleanup() {
  GLog::Log(GLog::LOG_DEBUG, "Server Cleanup");
  if (GNetworking::SocketShutdown(GetServerSocket(), GNetworkingSHUTDOWNRDWR) !=
      0) {
    throw std::runtime_error("Server Socket shutdown error");
  }

  if (GNetworking::SocketClose(GetServerSocket()) != 0) {
    throw std::runtime_error("Server Socket close error");
  }

  if (GNetworking::SocketCleanup() != 0) {
    throw std::runtime_error("Sockets cleanup error");
  }

  SSL_CTX_free(m_sslCTX);
}

void Server::_AcceptConnections() {
  SSL *connection;
  int output;

  // Poll for new connections
  if (GNetworking::SocketPoll(GetServerSocket(), GNetworkingPOLLIN)) {
    connection = SSL_new(m_sslCTX);
    SSL_set_fd(connection, GNetworking::SocketAccept(GetServerSocket()));
    GetClientSockets().push_back(connection);
    GLog::Log(GLog::LOG_DEBUG, "Opened Connection on Socket FD: " +
                                   std::to_string(SSL_get_fd(connection)));

    output = SSL_accept(connection);
    GLog::Log(GLog::LOG_DEBUG,
              "SSL handshake attempt: " + std::to_string(output));

    if (output < 0) {
      GLog::Log(GLog::LOG_TRACE, "Shutdown socket");
      GNetworking::SocketShutdown(SSL_get_fd(connection),
                                  GNetworkingSHUTDOWNRDWR);
    }
  }
}

void Server::_HandleClients() {
  std::vector<std::thread> threadPool(m_THREAD_COUNT);
  size_t threadIndex = 0;
  int output;

  // Poll for reading on clients
  for (size_t i = 0; i < GetClientSockets().size(); i++) {
    if (GNetworking::SocketPoll(SSL_get_fd(GetClientSockets()[i]),
                                GNetworkingPOLLHUP)) {
      continue;
    }

    if (GNetworking::SocketPoll(SSL_get_fd(GetClientSockets()[i]),
                                GNetworkingPOLLIN)) {
      threadIndex = i % m_THREAD_COUNT;

      // Cleanup running thread in pool
      if (threadPool[threadIndex].joinable()) {
        threadPool[threadIndex].join();
      }

      // Create new thread
      threadPool[threadIndex] =
          std::thread(&Server::_HandleOnThread, this, GetClientSockets()[i]);
    }
  }

  for (size_t i = 0; i < m_THREAD_COUNT; i++) {
    if (threadPool[i].joinable()) {
      GLog::Log(GLog::LOG_TRACE, "Joining thread - " + std::to_string(i));
      threadPool[i].join();
    }
  }
}

void Server::_CloseConnections() {
  GNetworking::GNetworkingSocket sock;

  // Poll for closing sockets
  for (int32_t i = GetClientSockets().size() - 1; i >= 0; i--) {
    sock = SSL_get_fd(GetClientSockets()[i]);

    if (GNetworking::SocketPoll(sock, GNetworkingPOLLHUP)) {
      GLog::Log(GLog::LOG_DEBUG,
                "Closing Socket FD: " +
                    std::to_string(SSL_get_fd(GetClientSockets()[i])));
      GNetworking::SocketShutdown(sock, GNetworkingSHUTDOWNRDWR);
      GNetworking::SocketClose(sock);
      SSL_free(GetClientSockets()[i]);
      GetClientSockets().erase(GetClientSockets().begin() + i);
    }
  }
}

size_t Server::_FindRequestSize(SSL *_client) {
  constexpr size_t PEEK_INCREMENT = 512;
  int32_t recvSize = 0;
  int32_t recvOutput = recvSize;
  std::vector<unsigned char> buffer;

  while (recvOutput >= recvSize) {
    recvSize += PEEK_INCREMENT;
    buffer.resize(recvSize);
    m_mutex.lock();
    recvOutput = SSL_peek(_client, buffer.data(), buffer.size());
    m_mutex.unlock();
  }

  if (recvOutput <= 0) {
    return 0;
  } else {
    return recvOutput;
  }
}

void Server::_ReadBuffer(SSL *_client, std::vector<unsigned char> &_buffer) {
  m_mutex.lock();
  SSL_read(_client, _buffer.data(), _buffer.size());
  m_mutex.unlock();
}

void Server::_SendBuffer(SSL *_client,
                         const std::vector<unsigned char> &_buffer,
                         bool _close) {
  m_mutex.lock();
  SSL_write(_client, _buffer.data(), _buffer.size());
  if (_close) {
    GNetworking::SocketShutdown(SSL_get_fd(_client), GNetworkingSHUTDOWNRDWR);
  }
  m_mutex.unlock();
}

size_t Server::_FileSize(const std::filesystem::path &_filename) {
  size_t output;
  std::fstream file;
  file.open(_filename, std::ios::in | std::ios::ate);
  output = file.tellg();
  file.close();
  return output;
}

void Server::_ReadFile(const std::filesystem::path &_filename,
                       std::vector<unsigned char> &_buffer) {
  std::fstream file;
  file.open(_filename, std::ios::in | std::ios::binary);
  file.read((char *)_buffer.data(), _buffer.size());
  file.close();
}

bool Server::_HasHostHeader(const GParsing::HTTPRequest &_req) {
  for (const auto &header : _req.headers) {
    if (header.first == "Host" || header.first == "host") {
      return true;
    }
  }

  return false;
}

void Server::_HandleOnThread(SSL *_client) {
  GParsing::HTTPRequest req;
  GParsing::HTTPResponse resp;
  std::vector<unsigned char> recvBuffer;
  std::vector<unsigned char> sendBuffer;
  GNetworking::GNetworkingSocket clientSocket = SSL_get_fd(_client);

  size_t recvSize;

  if (!GNetworking::SocketPoll(clientSocket, GNetworkingPOLLIN)) {
    return;
  }

  recvSize = _FindRequestSize(_client);

  if (recvSize <= 0) {
    GLog::Log(GLog::LOG_WARNING, '[' + std::to_string(clientSocket) +
                                     "]: Unable to read on socket");
    m_mutex.lock();
    GNetworking::SocketShutdown(clientSocket, GNetworkingSHUTDOWNRDWR);
    m_mutex.unlock();
    return;
  }

  recvBuffer.resize(recvSize);
  _ReadBuffer(_client, recvBuffer);
  GLog::Log(GLog::LOG_TRACE, (char *)recvBuffer.data());
  req.ParseRequest(recvBuffer);

  if (!_HasHostHeader(req)) {
    GLog::Log(GLog::LOG_WARNING, '[' + std::to_string(clientSocket) + "]: Connection request did not provide a Host header");
    resp.version = "HTTP/1.1";
    resp.response_code = 400;
    resp.response_code_message = "Bad Request";
    resp.headers.push_back({"Connection", {"close"}});
    sendBuffer = resp.CreateResponse();
    _SendBuffer(_client, sendBuffer);
    return;
  } else {
    GLog::Log(GLog::LOG_DEBUG, '[' + std::to_string(clientSocket) + "]: Sending Continue Response");
    resp.version = "HTTP/1.1";
    resp.response_code = 100;
    resp.response_code_message = "Continue";
    resp.headers.push_back({"Connection", {"keep-alive"}});
    sendBuffer = resp.CreateResponse();
    _SendBuffer(_client, sendBuffer, false);
  }

  if (req.uri.find('/') == req.uri.find("//")) {
    int index = req.uri.find("//") + 2;
    req.uri.erase(0, index);
  }

  req.uri.erase(0, req.uri.find('/') + 1);

  if (req.uri == "") {
    req.uri = "index.html";
  }

  resp.version = "HTTP/1.1";
  resp.response_code = 200;
  resp.response_code_message = "OK";

  GLog::Log(GLog::LOG_TRACE, '[' + std::to_string(clientSocket) + "]: Requesting URI - " + std::filesystem::absolute(req.uri).string());
  if (std::filesystem::exists(std::filesystem::absolute(m_dataDir / req.uri))) {
    sendBuffer.resize(_FileSize(std::filesystem::absolute(m_dataDir / req.uri)));
    _ReadFile(std::filesystem::absolute(m_dataDir / req.uri), sendBuffer);

    resp.message = sendBuffer;
  }
  else {
    resp.response_code = 404;
    resp.response_code_message = "Not Found";
  }

  resp.headers.push_back({"Connection", {"close"}});

  sendBuffer = resp.CreateResponse();
  _SendBuffer(_client, sendBuffer);
}

GNetworking::GNetworkingSocket &Server::GetServerSocket() {
  return m_serverSocket;
}
std::vector<SSL *> &Server::GetClientSockets() { return m_clientSockets; }
} // namespace Wepp
