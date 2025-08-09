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
Server::Server(const WEPP_HANDLER_FUNC _handler,
               const WEPP_POST_HANDLER_SUCCESS_FUNC _postHandler,
               const size_t &_threadCount)
    : m_THREAD_COUNT(_threadCount), m_handlerFunc(_handler),
      m_postHandlerFunc(_postHandler) {
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
    std::this_thread::sleep_for(std::chrono::microseconds(25));

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
  std::vector<unsigned char> buffer;
  SSL *connection;
  int output;
  size_t pollSize;

  // Poll for new connections
  if (GNetworking::SocketPoll(GetServerSocket(), GNetworkingPOLLIN)) {
    connection = SSL_new(m_sslCTX);
    SSL_set_fd(connection, GNetworking::SocketAccept(GetServerSocket()));
    GetClientSockets().push_back(connection);
    GLog::Log(GLog::LOG_DEBUG, "Opened Connection on Socket FD: " +
                                   std::to_string(SSL_get_fd(connection)));

    pollSize = GNetworking::SocketPollSize(SSL_get_fd(connection));
    buffer.resize(pollSize);
    GNetworking::SocketPeek(SSL_get_fd(connection), (char *)buffer.data(), buffer.size(), 0);

    output = SSL_accept(connection);
    GLog::Log(GLog::LOG_DEBUG, "SSL handshake attempt output: " + std::to_string(output));

    if (output < 0) {
      GParsing::HTTPResponse redirectResponse;
      GParsing::HTTPRequest req;

      GLog::Log(GLog::LOG_TRACE, "SSL handshake failed. Redirecting to HTTPS.");
      req.ParseRequest(buffer);

      std::string hostValue;
      for(const auto & header : req.headers) {
        if (header.first == "Host") {
          if (header.second.size() != 1) {
            break;
          } else {
            hostValue = header.second[0];
          }
        }
      }

      if (hostValue != "") {
        const std::string findHTTP = "http://";
        if (hostValue.find(findHTTP) == 0) {
          hostValue.replace(0, findHTTP.length(), "https://");
        }
        else if (hostValue.find('/') == 0) {
          hostValue.insert(0, "https:/");
        }
        else {
          hostValue.insert(0, "https://");
        }

        redirectResponse.response_code = 301;
        redirectResponse.response_code_message = "Moved Permanently";
        redirectResponse.headers.push_back({"Location", {hostValue}});
        redirectResponse.version = "HTTP/1.1";
        redirectResponse.message.clear();
        auto respBuffer = redirectResponse.CreateResponse();

        GNetworking::SocketSend(SSL_get_fd(connection), (char *)respBuffer.data(), respBuffer.size(), 0);
      }

      GNetworking::SocketShutdown(SSL_get_fd(connection),
                                  GNetworkingSHUTDOWNRDWR);
    }
  }
}

void Server::_HandleClients() {
  WEPP_HANDLER_FUNC handlerFunc = m_handlerFunc;
  WEPP_POST_HANDLER_SUCCESS_FUNC postHandlerFunc = m_postHandlerFunc;

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
          std::thread(&Server::_HandleOnThread, this, GetClientSockets()[i],
                      handlerFunc, postHandlerFunc);
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
      GLog::Log(GLog::LOG_DEBUG, "Amount of active sockets: " +
                                     std::to_string(GetClientSockets().size()));
    }
  }
}

size_t Server::_FindRequestSize(SSL *_client) {
  constexpr size_t PEEK_INCREMENT = 512;
  int32_t recvSize = 0;
  int32_t recvOutput = recvSize;
  std::vector<unsigned char> buffer;

  if (GNetworking::SocketPoll(SSL_get_fd(_client), GNetworkingPOLLHUP)) {
    GLog::Log(GLog::LOG_WARNING, '[' + std::to_string(SSL_get_fd(_client)) +
                                     "]: Failed to peek on socket");
    GNetworking::SocketShutdown(SSL_get_fd(_client), GNetworkingSHUTDOWNRDWR);
    return 0;
  }

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
  if (GNetworking::SocketPoll(SSL_get_fd(_client), GNetworkingPOLLHUP)) {
    GLog::Log(GLog::LOG_WARNING, '[' + std::to_string(SSL_get_fd(_client)) +
                                     "]: Failed to read on socket");
    GNetworking::SocketShutdown(SSL_get_fd(_client), GNetworkingSHUTDOWNRDWR);
    return;
  }

  m_mutex.lock();
  SSL_read(_client, _buffer.data(), _buffer.size());
  m_mutex.unlock();
}

void Server::_SendBuffer(SSL *_client,
                         const std::vector<unsigned char> &_buffer,
                         bool _close) {
  GLog::Log(GLog::LOG_TRACE,
            '[' + std::to_string(SSL_get_fd(_client)) + "]: Sending response");
  if (GNetworking::SocketPoll(SSL_get_fd(_client), GNetworkingPOLLHUP)) {
    GLog::Log(GLog::LOG_WARNING, '[' + std::to_string(SSL_get_fd(_client)) +
                                     "]: Failed to send on socket");
    GNetworking::SocketShutdown(SSL_get_fd(_client), GNetworkingSHUTDOWNRDWR);
    return;
  }

  m_mutex.lock();
  SSL_write(_client, _buffer.data(), _buffer.size());
  if (_close) {
    GNetworking::SocketShutdown(SSL_get_fd(_client), GNetworkingSHUTDOWNRDWR);
  }
  m_mutex.unlock();
}

void Server::_HandleOnThread(SSL *_client, WEPP_HANDLER_FUNC _handler,
                             WEPP_POST_HANDLER_SUCCESS_FUNC _postHandler) {
  bool closeConnection;
  GParsing::HTTPRequest req;
  GParsing::HTTPResponse resp;
  GParsing::HTTPResponse intermediateResp;
  std::vector<unsigned char> recvBuffer;
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

  GLog::Log(GLog::LOG_TRACE, '[' + std::to_string(clientSocket) +
                                 "]: Sending request to handler");
  if (_handler(req, resp, closeConnection)) {
    GLog::Log(GLog::LOG_TRACE,
              '[' + std::to_string(clientSocket) +
                  "]: Request successful, sending to post handler");
    if (_postHandler(req, intermediateResp)) {
      GLog::Log(GLog::LOG_TRACE,
                '[' + std::to_string(clientSocket) +
                    "]: Post handler successful, sending to client");
      _SendBuffer(_client, intermediateResp.CreateResponse(), false);
    }
  }

  _SendBuffer(_client, resp.CreateResponse(), closeConnection);
}

GNetworking::GNetworkingSocket &Server::GetServerSocket() {
  return m_serverSocket;
}
std::vector<SSL *> &Server::GetClientSockets() { return m_clientSockets; }
} // namespace Wepp
