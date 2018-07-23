#pragma once

#include <Network/Address/AddressV4.h>

namespace network {

	class Address {
	public: 
		bool IsV4() const noexcept {
			return (_type == ipv4);
		}

		AddressV4 V4() const noexcept {
			return _addr_v4;
		}

	private:
		enum {
			ipv4,
			ipv6
		} _type;

		AddressV4 _addr_v4;
		// AddressV6 _addr_v6;
	};

}