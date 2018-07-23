#pragma once

#include <WinSock2.h>

#include <Util/Error.h>

namespace network {

	static void GetSocketError(util::error::Error &err, int ec) {
		if (ec == SOCKET_ERROR) {
			ec = WSAGetLastError();
			if (ec != WSAEWOULDBLOCK) {
				err = util::error::lib::SocketError(ec);
				return;
			}
		}
	}

	namespace error {

		using namespace util::error;

		enum class NetworkErrorCode {
			NONE = 0,
			CLOSED,
		};

		REGISTER_ERROR(Network);

	}

}