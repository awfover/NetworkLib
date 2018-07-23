#pragma once

#include <thread>
#include <functional>

#include <WinSock2.h>

#include <Network/Error.h>
#include <Network/Endpoint.h>
#include <Network/Resolver/Resolver.h>

#include <Util/IO.h>
#include <Util/Error.h>
#include <Util/Buffer.h>
#include <Util/Thread.h>

namespace network {

	template<class InternetProtocol>
	class Socket {
	public:
		using Resolver = typename resolver::Resolver<InternetProtocol>;
		using Endpoint = typename Endpoint<InternetProtocol>;

		using Ptr = typename std::shared_ptr<Socket>;

	public:
		Socket(util::io::IOContext &ctx, const InternetProtocol &protocol)
			: _ctx(ctx),
			_protocol(protocol) {
			_s = socket(_protocol.Family(), _protocol.Type(), _protocol.Protocol());
		}

		virtual ~Socket() {
			// ignore all errors
			util::error::Error err;
			Shutdown(SD_BOTH, err);
			Close(err);
		}

		void Close(util::error::Error &err) {
			if (!IsOpen()) {
				return;
			}

			int ret = closesocket(_s);
			GetSocketError(err, ret);

			_s = -1;
		}

		void Shutdown(int how, util::error::Error &err) {
			if (!IsOpen()) {
				return;
			}

			int ret = shutdown(_s, how);
			GetSocketError(err, ret);
		}

		bool IsOpen() const noexcept {
			return (_s != -1);
		}

		bool IsOpen(util::error::Error &err) const {
			if (IsOpen()) {
				return true;
			}

			err = network::error::NetworkError(network::error::NetworkErrorCode::CLOSED);
			return false;
		}

		void Connect(const typename Resolver::Result::Ptr &endpoints, util::error::Error &err) {
			for (auto &i : (*endpoints)) {
				if (i->Is(_protocol)) {
					Connect(i->GetEndpoint(), err);
					return;
				}
			}
		}

		void Connect(const std::string &host, const std::string &port, util::error::Error &err) {
			Tcp::Resolver resolver(_ctx);
			Tcp::Resolver::Result::Ptr endpoints = resolver.Resolve(host, port, err);
			if (err) {
				return;
			}

			Connect(endpoints, err);
			if (err) {
				return;
			}
		}

		void Connect(const Endpoint &pe, util::error::Error &err) {
			if (!IsOpen(err)) {
				return;
			}
			
			int ret = connect(_s, pe.Data(), pe.Size());
			GetSocketError(err, ret);
			if (err) {
				Close(err);
			}
		}

		util::io::IOContext &IOContext() const noexcept {
			return _ctx;
		}

	protected:
		SOCKET _s;
		util::io::IOContext &_ctx;
		InternetProtocol _protocol;
	};

}
