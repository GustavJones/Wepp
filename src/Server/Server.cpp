#include "Server/Server.hpp"
#include "GLog/Log.hpp"
#include "GNetworking/Socket.hpp"
#include "GParsing/GParsing.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
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
  GLog::Log(GLog::LOG_DEBUG, "Server Setup");
  if (GNetworking::SocketSetup() != 0) {
    throw std::runtime_error("Sockets Setup error");
  }

  GetServerSocket() =
      GNetworking::SocketCreate(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (GetServerSocket() == GNetworkingInvalidSocket) {
    throw std::runtime_error("Cannot create server socket");
  }

  int value = 1;
  if (GNetworking::SocketSetOption(GetServerSocket(), SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value)) != 0) {
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
  std::vector<std::thread> threadPool(m_THREAD_COUNT);
  size_t threadIndex = 0;

  // Poll for reading on clients
  for (size_t i = 0; i < GetClientSockets().size(); i++) {
    if (GNetworking::SocketPoll(GetClientSockets()[i], GNetworkingPOLLIN)) {
      if (threadIndex >= m_THREAD_COUNT) {
        threadIndex = 0;
        if (threadPool[threadIndex].joinable()) {
          threadPool[threadIndex].join();
        }
      }

      threadPool[threadIndex] =
          std::thread(&Server::_HandleOnThread, this, GetClientSockets()[i]);
      threadIndex++;
    }
  }

  for (size_t i = 0; i < m_THREAD_COUNT; i++) {
    if (threadPool[i].joinable()) {
      threadPool[i].join();
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

void Server::_HandleOnThread(GNetworking::GNetworkingSocket _client) {
  GParsing::HTTPRequest req;
  GParsing::HTTPResponse resp;
  std::vector<unsigned char> buffer;
  std::string message;
  char character;
  long status;

  m_mutex.lock();
  while (GNetworking::SocketPoll(_client, GNetworkingPOLLIN)) {
    status = GNetworking::SocketRecv(_client, &character, 1, 0);

    if (status != 1) {
      break;
    }

    buffer.push_back(character);
  }

  m_mutex.unlock();

  if (buffer.size() == 0) {
    return;
  }

  message = "Socket FD " + std::to_string(_client) + " : ";

  for(const auto & c : buffer) {
    message += c;
  }

  GLog::Log(GLog::LOG_TRACE, message);

  req.ParseRequest(buffer);

  resp.version = "HTTP/1.1";
  resp.response_code = 200;
  resp.response_code_message = "OK";
  resp.headers.push_back({"Connection", {"close"}});

  buffer = resp.CreateResponse();

  m_mutex.lock();
  GNetworking::SocketSend(_client, (char *)buffer.data(), buffer.size(), 0);
  GNetworking::SocketShutdown(_client, GNetworkingSHUTDOWNRDWR);
  GNetworking::SocketClose(_client);
  m_mutex.unlock();
}

GNetworking::GNetworkingSocket &Server::GetServerSocket() {
  return m_serverSocket;
}
std::vector<GNetworking::GNetworkingSocket> &Server::GetClientSockets() {
  return m_clientSockets;
}
} // namespace Wepp
