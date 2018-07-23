#pragma once

#include <string>

namespace network {

	namespace resolver {

		template<class InternetProtocol>
		class Query {
		public:
			using Ptr = typename std::shared_ptr<Query>;

		public:
			Query(const std::string &host, const std::string &service) noexcept
				: _host(host), _service(service) {}

			static Ptr New(const std::string &host, const std::string &service) noexcept {
				return std::make_shared<Query>(host, service);
			}

			const std::string &Host() const noexcept {
				return _host;
			}

			const std::string &Service() const noexcept {
				return _service;
			}

		private:
			std::string _host;
			std::string _service;
		};

	}

}