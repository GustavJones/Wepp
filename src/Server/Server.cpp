#include "Server/Server.hpp"
#include "GLog/Log.hpp"
#include "GNetworking/Socket.hpp"
#include "GParsing/GParsing.hpp"
#include <chrono>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <openssl/ssl.h>
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

  // Poll for new connections
  if (GNetworking::SocketPoll(GetServerSocket(), GNetworkingPOLLIN)) {
    connection = SSL_new(m_sslCTX);
    SSL_set_fd(connection, GNetworking::SocketAccept(GetServerSocket()));
    GetClientSockets().push_back(connection);
    GLog::Log(GLog::LOG_DEBUG, "Opened Connection on Socket FD: " +
                                   std::to_string(SSL_get_fd(connection)));
    GLog::Log(GLog::LOG_DEBUG, "SSL handshake attempt: " +
                                   std::to_string(SSL_accept(connection)));
  }
}

void Server::_HandleClients() {
  std::vector<std::thread> threadPool(m_THREAD_COUNT);
  size_t threadIndex = 0;
  int output;

  // Poll for reading on clients
  for (size_t i = 0; i < GetClientSockets().size(); i++) {
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

void Server::_HandleOnThread(SSL *_client) {
  GParsing::HTTPRequest req;
  GParsing::HTTPRequest resp;
  constexpr size_t PEEK_INCREMENT = 512;
  int64_t recvSize = 0;
  int64_t recvOutput = recvSize;
  std::vector<unsigned char> recvBuffer;
  std::vector<unsigned char> sendBuffer;
  GNetworking::GNetworkingSocket clientSocket = SSL_get_fd(_client);

  if (GNetworking::SocketPoll(clientSocket, GNetworkingPOLLIN)) {
    while (recvOutput >= recvSize) {
      recvSize += PEEK_INCREMENT;
      recvBuffer.resize(recvSize);
      m_mutex.lock();
      recvOutput = SSL_peek(_client, recvBuffer.data(), recvSize);
      m_mutex.unlock();
    }

    if (recvOutput <= 0) {
      GLog::Log(GLog::LOG_WARNING, '[' + std::to_string(clientSocket) +
                                       "]: Unable to read on socket");
      m_mutex.lock();
      GNetworking::SocketShutdown(clientSocket, GNetworkingSHUTDOWNRDWR);
      m_mutex.unlock();
      return;
    }

    recvBuffer.resize(recvOutput);
    m_mutex.lock();
    recvOutput = SSL_read(_client, recvBuffer.data(), recvBuffer.size());
    m_mutex.unlock();

    req.ParseRequest(recvBuffer);

    GLog::Log(GLog::LOG_TRACE, (char *)recvBuffer.data());
    resp.method = GParsing::HTTPMethod::GPARSING_GET;
    resp.version = "HTTP/1.1";
    resp.uri = req.uri;
    resp.headers.push_back({"Connection", {"close"}});
    sendBuffer = resp.CreateRequest();

    m_mutex.lock();
    SSL_write(_client, sendBuffer.data(), sendBuffer.size());
    GNetworking::SocketShutdown(clientSocket, GNetworkingSHUTDOWNRDWR);
    m_mutex.unlock();
  }
}

GNetworking::GNetworkingSocket &Server::GetServerSocket() {
  return m_serverSocket;
}
std::vector<SSL *> &Server::GetClientSockets() { return m_clientSockets; }
} // namespace Wepp
