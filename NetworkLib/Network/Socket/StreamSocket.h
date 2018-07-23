#pragma once

#include <WinSock2.h>

#include <Network/Error.h>
#include <Network/Socket/Socket.h>

#include <Util/IO.h>
#include <Util/Error.h>
#include <Util/Buffer.h>

namespace network {

	template<class InternetProtocol>
	class StreamSocket : public Socket<InternetProtocol> {
	public:
		using Ptr = typename std::shared_ptr<StreamSocket>;

	public:
		StreamSocket(util::io::IOContext &ctx, const InternetProtocol &protocol)
			: Socket<InternetProtocol>(ctx, protocol) {}

		virtual ~StreamSocket() {}

		size_t Send(const util::buffer::ConstBuffer &b, util::error::Error &err) {
			int ret = send(_s, static_cast<const char *>(b.Data()), b.Size(), 0);
			
			GetSocketError(err, ret);
			if (err) {
				Close(err);
				return 0;
			}

			return (size_t)ret;
		}

		size_t Receive(const util::buffer::MutableBuffer &b, util::error::Error &err) {
			int ret = recv(_s, static_cast<char *>(b.Data()), b.Size(), 0);
			if (ret == 0) {
				Close(err);
				return 0;
			}

			GetSocketError(err, ret);
			if (err) {
				Close(err);
				return 0;
			}

			return (size_t)ret;
		}

		size_t ReadSome(const util::buffer::MutableBuffer &b, util::error::Error &err) {
			return Receive(b, err);
		}

		size_t WriteSome(const util::buffer::ConstBuffer &b, util::error::Error &err) {
			return Send(b, err);
		}
	};

}
