#pragma once

#include <cstdint>

#include <WinSock2.h>

namespace network {

	class AddressV4 {
	public:
		AddressV4() {}

		std::uint32_t to_uint32() const noexcept {
			return htonl(_addr.s_addr);
		}

	private:
		IN_ADDR _addr;
	};

}