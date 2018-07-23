#pragma once

#include <WinSock2.h>

#include <Network/Endpoint.h>
#include <Network/Resolver/Resolver.h>
#include <Network/Socket/StreamSocket.h>

namespace network {

	class Tcp {
	public:
		using Socket = StreamSocket<Tcp>;
		using Endpoint = Endpoint<Tcp>;
		using Resolver = resolver::Resolver<Tcp>;

		const int TYPE = SOCK_STREAM;
		const int PROTOCOL = IPPROTO_TCP;

	public:
		static Tcp v4() noexcept {
			return Tcp(AF_INET);
		}

		static Tcp v6() noexcept {
			return Tcp(AF_INET6);
		}

		int Family() const noexcept {
			return _family;
		}

		int Type() const noexcept {
			return TYPE;
		}

		int Protocol() const noexcept {
			return PROTOCOL;
		}

	private:
		int _family;

	private:
		explicit Tcp(int family) noexcept
			: _family(family) {}
	};

}
