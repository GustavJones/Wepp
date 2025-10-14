#pragma once
#include <openssl/ssl.h>

namespace Wepp {
	struct ClientSocket
	{
		bool encrypted;
		SSL* socket;

		ClientSocket(SSL *const _socket = nullptr, const bool _encrypted = false) : encrypted(_encrypted), socket(_socket) {}

		// Shallow copy only
		ClientSocket(const ClientSocket& _socket) = default;
		ClientSocket& operator=(const ClientSocket& _socket) = default;

		// Move
		ClientSocket(ClientSocket&& _socket) {
			*this = _socket;
		}

		ClientSocket& operator=(ClientSocket&& _socket) {
			encrypted = _socket.encrypted;
			socket = _socket.socket;

			_socket.encrypted = false;
			_socket.socket = nullptr;

			return *this;
		}
	};
} // namespace Wepp