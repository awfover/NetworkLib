#pragma once

#include <vector>
#include <memory>

#include <Network/Resolver/Entry.h>
#include <Network/Resolver/Query.h>

namespace network {

	namespace resolver {

		template<class InternetProtocol>
		class Result : public std::vector<typename Entry<InternetProtocol>::Ptr> {
		public:
			using Ptr = typename std::shared_ptr<Result>;
			using Entry = typename Entry<InternetProtocol>;
			using Query = typename Query<InternetProtocol>;

			friend Ptr;

		public:
			Result(typename Query::Ptr query) noexcept
				: _query(query) {}

			static Ptr New(typename Query::Ptr query, PADDRINFOA i) {
				auto r = std::make_shared<Result>(query);

				Entry::Ptr e;
				for (; i; i = i->ai_next) {
					e = Entry::New(query, i);
					r->push_back(e);
				}

				return r;
			}

		private:
			typename Query::Ptr _query;
		};

	}

}