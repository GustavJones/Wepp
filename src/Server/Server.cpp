#include "Server/Server.hpp"
#include "GLog/Log.hpp"
#include "GNetworking/Socket.hpp"
#include "GParsing/GParsing.hpp"
#include <chrono>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <openssl/ssl.h>
#include <ostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

namespace Wepp {
Server::Server(const size_t &_threadCount) : m_THREAD_COUNT(_threadCount) {
  GetClientSockets().resize(0);
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
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

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
  // Poll for new connections
  if (GNetworking::SocketPoll(GetServerSocket(), GNetworkingPOLLIN)) {
    GetClientSockets().push_back(GNetworking::SocketAccept(GetServerSocket()));
    GLog::Log(
        GLog::LOG_DEBUG,
        "Opened Connection on Socket FD: " +
            std::to_string(GetClientSockets()[GetClientSockets().size() - 1]));
  }
}

void Server::_HandleClients() {
  std::vector<std::pair<std::thread, SSL *>> threadPool(m_THREAD_COUNT);
  size_t threadIndex = 0;

  // Poll for reading on clients
  for (size_t i = 0; i < GetClientSockets().size(); i++) {
    if (GNetworking::SocketPoll(GetClientSockets()[i], GNetworkingPOLLIN)) {
      threadIndex = i % m_THREAD_COUNT;

      // Cleanup running thread in pool
      if (threadPool[threadIndex].first.joinable()) {
        threadPool[threadIndex].first.join();
        SSL_free(threadPool[threadIndex].second);
      }

      // Create new thread
      threadPool[threadIndex].second = SSL_new(m_sslCTX);
      SSL_set_fd(threadPool[threadIndex].second, GetClientSockets()[i]);
      if (SSL_accept(threadPool[threadIndex].second) <= 0) {
        SSL_free(threadPool[threadIndex].second);
        continue;
      }

      threadPool[threadIndex].first = std::thread(
          &Server::_HandleOnThread, this, threadPool[threadIndex].second);
    }
  }

  for (size_t i = 0; i < m_THREAD_COUNT; i++) {
    if (threadPool[i].first.joinable()) {
      threadPool[i].first.join();

      SSL_free(threadPool[i].second);
    }
  }
}

void Server::_CloseConnections() {
  // Poll for closing sockets
  for (int32_t i = GetClientSockets().size() - 1; i >= 0; i--) {
    if (GNetworking::SocketPoll(GetClientSockets()[i], GNetworkingPOLLHUP)) {
      GLog::Log(GLog::LOG_DEBUG,
                "Closing Socket FD: " + std::to_string(GetClientSockets()[i]));
      GNetworking::SocketShutdown(GetClientSockets()[i],
                                  GNetworkingSHUTDOWNRDWR);
      GNetworking::SocketClose(GetClientSockets()[i]);
      GetClientSockets().erase(GetClientSockets().begin() + i);
    }
  }
}

void Server::_HandleOnThread(SSL *_client) {
  GNetworking::GNetworkingSocket clientSocket = SSL_get_fd(_client);
}

GNetworking::GNetworkingSocket &Server::GetServerSocket() {
  return m_serverSocket;
}
std::vector<GNetworking::GNetworkingSocket> &Server::GetClientSockets() {
  return m_clientSockets;
}
} // namespace Wepp
