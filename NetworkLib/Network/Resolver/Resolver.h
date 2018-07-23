#pragma once

#include <memory>
#include <string>
#include <functional>

#include <WS2tcpip.h>

#include <Network/Protocol/Tcp.h>
#include <Network/Address/Address.h>

#include <Network/Resolver/Entry.h>
#include <Network/Resolver/Query.h>
#include <Network/Resolver/Result.h>

#include <Util/IO.h>
#include <Util/Error.h>

namespace network {

	namespace resolver {

		template<class InternetProtocol>
		class Resolver {
		public:
			using Entry = typename Entry<InternetProtocol>;
			using Query = typename Query<InternetProtocol>;
			using Result = typename Result<InternetProtocol>;

		public:
			Resolver(util::io::IOContext &ctx) noexcept
				: _ctx(ctx) {}

			typename Result::Ptr Resolve(const std::string &host, const std::string &service, util::error::Error &err) {
				PADDRINFOA pinfo;
				int ec = GetAddrInfoA(host.c_str(), service.c_str(), nullptr, &pinfo);
				if (ec != 0) {
					err = util::error::lib::SocketError(ec, "GetAddrInfoA");
					return nullptr;
				}

				Query::Ptr q = Query::New(host, service);
				Result::Ptr r = Result::New(q, pinfo);
				FreeAddrInfoA(pinfo);

				return r;
			}

			util::io::IOContext &Context() const noexcept {
				return _ctx;
			}

		private:
			util::io::IOContext &_ctx;
		};

	}

}