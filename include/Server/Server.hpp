#pragma once
#include "GNetworking/Socket.hpp"
#include "GParsing/GParsing.hpp"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <openssl/ssl.h>
#include <string>
#include <vector>

namespace Wepp {

typedef bool (*WEPP_HANDLER_FUNC)(GParsing::HTTPRequest _req, GParsing::HTTPResponse &_resp, bool &_closeConnection);
typedef bool (*WEPP_POST_HANDLER_SUCCESS_FUNC)(GParsing::HTTPRequest _req, GParsing::HTTPResponse &_resp);

class Server {
private:
  const size_t m_THREAD_COUNT;

  GNetworking::GNetworkingSocket m_serverSocket;
  std::vector<SSL *> m_clientSockets;

  SSL_CTX *m_sslCTX;

  std::mutex m_mutex;
  const std::atomic<WEPP_HANDLER_FUNC> m_handlerFunc;
  const std::atomic<WEPP_POST_HANDLER_SUCCESS_FUNC> m_postHandlerFunc;

public:
  Server(const WEPP_HANDLER_FUNC _handler, const WEPP_POST_HANDLER_SUCCESS_FUNC _postHandler, const size_t &_threadCount = 4);
  Server(Server &&) = delete;
  Server(const Server &) = delete;
  Server &operator=(Server &&) = delete;
  Server &operator=(const Server &) = delete;
  ~Server();

  void Run(const std::string &_address, const uint16_t _port);

  GNetworking::GNetworkingSocket &GetServerSocket();
  std::vector<SSL *> &GetClientSockets();
  const size_t &GetThreadCount();

private:
  void _Setup(const std::string &_address, const uint16_t _port);

  void _MainLoop();

  void _Cleanup();

  void _AcceptConnections();

  void _HandleClients();

  void _CloseConnections();

  void _HandleOnThread(SSL *_client, WEPP_HANDLER_FUNC _handler, WEPP_POST_HANDLER_SUCCESS_FUNC _postHandler);

  size_t _FindRequestSize(SSL *_client);

  void _ReadBuffer(SSL *_client, std::vector<unsigned char> &_buffer);
  void _SendBuffer(SSL *_client, const std::vector<unsigned char> &_buffer,
                   bool _close = true);
};
} // namespace Wepp
