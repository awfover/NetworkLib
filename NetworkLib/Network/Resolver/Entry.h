#pragma once

#include <Network/Endpoint.h>

#include <Network/Resolver/Query.h>

namespace network {

	namespace resolver {

		template<class InternetProtocol>
		class Entry {
		public:
			using Ptr = typename std::shared_ptr<Entry>;
			using Query = typename Query<InternetProtocol>;
			using Endpoint = typename Endpoint<InternetProtocol>;

		public:
			Entry(const typename Query::Ptr query, PADDRINFOA i) noexcept
				: _query(query),
				_family(i->ai_family),
				_type(i->ai_socktype),
				_protocol(i->ai_protocol),
				_endpoint(i->ai_addr, i->ai_addrlen) {}

			static Ptr New(const typename Query::Ptr query, PADDRINFOA i) noexcept {
				auto e = std::make_shared<Entry>(query, i);
				return e;
			}

			bool Is(const InternetProtocol &ip) const noexcept {
				return Is(ip.Family(), ip.Type(), ip.Protocol());
			}

			bool Is(int family, int type, int protocol) const noexcept {
				if ((_family != 0) && (family != _family)) {
					return false;
				}
				if ((_type != 0) && (type != _type)) {
					return false;
				}
				if ((_protocol != 0) && (protocol != _protocol)) {
					return false;
				}
				return true;
			}

			int Family() const noexcept {
				return _family;
			}

			int Type() const noexcept {
				return _type;
			}

			int Protocol() const noexcept {
				return _protocol;
			}

			const Endpoint &GetEndpoint() const noexcept {
				return _endpoint;
			}

		private:
			int _family;
			int _type;
			int _protocol;

			Endpoint _endpoint;

			const typename Query::Ptr _query;
		};

	}

}