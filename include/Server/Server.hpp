#pragma once
#include "GNetworking/Socket.hpp"
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <openssl/ssl.h>
#include <string>
#include <vector>

namespace Wepp {
class Server {
private:
  const size_t m_THREAD_COUNT;

  GNetworking::GNetworkingSocket m_serverSocket;
  std::vector<GNetworking::GNetworkingSocket> m_clientSockets;

  SSL_CTX *m_sslCTX;

  std::mutex m_mutex;

public:
  Server(const size_t &_threadCount = 4);
  Server(Server &&) = delete;
  Server(const Server &) = delete;
  Server &operator=(Server &&) = delete;
  Server &operator=(const Server &) = delete;
  ~Server();

  void Run(const std::string &_address, const uint16_t _port);

  GNetworking::GNetworkingSocket &GetServerSocket();
  std::vector<GNetworking::GNetworkingSocket> &GetClientSockets();
  const size_t &GetThreadCount();

private:
  void _Setup(const std::string &_address, const uint16_t _port);

  void _MainLoop();

  void _Cleanup();

  void _AcceptConnections();

  void _HandleClients();

  void _CloseConnections();

  void _HandleOnThread(SSL *_client);
};
} // namespace Wepp
