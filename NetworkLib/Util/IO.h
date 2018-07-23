#pragma once

#include <functional>

#include <Util/Error.h>
#include <Util/Buffer.h>
#include <Util/String.h>
#include <Util/Thread.h>

namespace util {

	namespace io {

		using IOContext = util::thread::ThreadPool;

		template<typename SyncReadStream, typename MutableBuffer>
		size_t Read(SyncReadStream &s, const MutableBuffer &b, util::error::Error &err) {
			for (size_t nread = 0; nread < b.Size();) {
				nread += s.ReadSome(b + nread, err);
				if (ec) {
					return nread;
				}
			}
			return b.Size();
		}

		template<typename SyncReadStream, typename DynamicBuffer, typename ...Patterns>
		size_t ReadUntil(SyncReadStream &s, DynamicBuffer &b, util::error::Error &err, Patterns &&...patterns) {
			size_t bsize = 0;
			size_t nread = 0;
			size_t nsearched = 0;
			DynamicBuffer::const_iterator cur;
			DynamicBuffer::const_iterator end;
			util::string::Pattern<DynamicBuffer::ValueType> pattern(std::forward<Patterns>(patterns)...);
			for (;;) {
				bsize = b.Size();

				cur = b.begin() + nsearched;
				end = b.end();
				for (; cur != end; ++cur) {
					++nsearched;
					if (pattern.Consume(*cur)) {
						return nsearched;
					}
				}

				nread = std::min<std::size_t>(
					std::max<size_t>(512, b.Capacity() - bsize),
					std::min<size_t>(65536, b.MaxSize() - bsize));

				auto tmpb = b.Prepare(nread, err);
				if (err) {
					break;
				}

				nread = s.ReadSome(tmpb, err);
				if (err) {
					break;
				}

				b.Commit(nread);
			}
			return nsearched;
		}

		template<typename SyncReadStream>
		size_t Write(SyncReadStream &s, const util::buffer::ConstBuffer &b, util::error::Error &err) {
			for (size_t nwrite = 0; nwrite < b.Size();) {
				nwrite += s.WriteSome(b + nwrite, err);
				if (err) {
					return nwrite;
				}
			}
			return b.Size();
		}

	}

}