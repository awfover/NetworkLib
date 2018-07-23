#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <Network/Address/Address.h>

namespace network {

	template<class InternetProtocol>
	class Endpoint {
	public:
		Endpoint(int family, std::uint16_t port) noexcept {
			if (family == AF_INET) {
				_data.base.v4.sin_family = AF_INET;
				_data.base.v4.sin_port = port;
				_data.base.v4.sin_addr = INADDR_ANY;
			}
			else {
				// TODO: ipv6
			}
		}

		Endpoint(sockaddr *addr, size_t addrlen) {
			if (addr) {
				std::memcpy(&_data, addr, addrlen);
			}
		}

		Endpoint(const Address &addr, std::uint16_t port) noexcept {
			if (addr.IsV4()) {
				_data.base.v4.sin_family = AF_INET;
				_data.base.v4.sin_port = port;
				_data.base.v4.sin_addr = addr.V4().to_uint32();
			}
			else {
				// TODO: ipv6
			}
		}

		sockaddr *Data() noexcept {
			return &_data.base;
		}

		const sockaddr *Data() const noexcept {
			return &_data.base;
		}

		std::size_t Size() const noexcept {
			if (IsV4()) {
				return sizeof(sockaddr_in);
			}
			return sizeof(sockaddr_in6);
		}

		bool IsV4() const noexcept {
			return (_data.base.sa_family == AF_INET);
		}

	private:
		union {
			sockaddr base;
			sockaddr_in v4;
			sockaddr_in6 v6;
		} _data;
	};

}