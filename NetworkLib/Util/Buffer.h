#pragma once

#include <string>
#include <algorithm>

namespace util {

	namespace buffer {

		class MutableBuffer {
		public:
			MutableBuffer() noexcept
				: _data(0), _size(0) {}

			MutableBuffer(void *data, size_t size) noexcept
				: _data(data), _size(size) {}

			template<typename Elem, typename Traits, typename Allocator>
			static MutableBuffer From(std::basic_string<Elem, Traits, Allocator> &data) {
				return MutableBuffer((data.size() ? static_cast<void*>(&data[0]) : nullptr), data.size());
			}

			void *Data() const noexcept {
				return _data;
			}

			size_t Size() const noexcept {
				return _size;
			}

			MutableBuffer operator+(size_t n) const noexcept {
				size_t offset = (std::min)(n, _size);
				size_t new_size = _size - offset;
				char *new_data = static_cast<char *>(_data) + offset;
				return MutableBuffer(new_data, new_size);
			}

			MutableBuffer &operator+=(size_t n) noexcept {
				size_t offset = (std::min)(n, _size);
				_data = static_cast<char *>(_data) + offset;
				_size -= offset;
				return *this;
			}

		private:
			void *_data;
			size_t _size;
		};

		class ConstBuffer {
		public:
			ConstBuffer() noexcept 
				: _data(0), _size(0) {}

			ConstBuffer(const void *data, size_t size) noexcept 
				: _data(data), _size(size) {}

			ConstBuffer(const ConstBuffer &b) noexcept
				: _data(b._data), _size(b._size) {}

			template<typename Elem, typename Traits, typename Allocator>
			static ConstBuffer From(const std::basic_string<Elem, Traits, Allocator> &data) {
				return ConstBuffer(data.size() ? &data[0] : nullptr, data.size());
			}

			template<typename Elem, typename Traits, typename Allocator>
			static ConstBuffer From(const std::basic_string<Elem, Traits, Allocator> &data, size_t size) {
				return ConstBuffer(data.size() ? &data[0] : nullptr, std::min<size_t>(size, data.size()));
			}

			const void *Data() const noexcept {
				return _data;
			}

			size_t Size() const noexcept {
				return _size;
			}

			ConstBuffer operator+(size_t n) const noexcept {
				size_t offset = (std::min)(n, _size);
				size_t new_size = _size - offset;
				const char *new_data = static_cast<const char *>(_data) + offset;
				return ConstBuffer(new_data, new_size);
			}

			ConstBuffer &operator+=(size_t n) noexcept {
				size_t offset = (std::min)(n, _size);
				_data = static_cast<const char *>(_data) + offset;
				_size -= offset;
				return *this;
			}

			ConstBuffer Off(size_t n) const {
				size_t offset = (std::min)(n, _size);
				size_t new_size = _size - offset;
				const char *new_data = static_cast<const char *>(_data) + offset;
				return ConstBuffer(new_data, new_size);
			}

		private:
			const void *_data;
			size_t _size;
		};

		template<typename Elem, typename Traits, typename Allocator>
		class DynamicStringBuffer {
		public:
			using String = typename std::basic_string<Elem, Traits, Allocator>;
			using ValueType = String;
			using const_iterator = typename String::const_iterator;

		public:
			DynamicStringBuffer(
				String &s,
				size_t max_size = (std::numeric_limits<size_t>::max)())
				: _s(s), _size(s.size()), _max_size(max_size) {}

			explicit DynamicStringBuffer(DynamicStringBuffer &&other)
				: _s(other._s),
				_size(other._size),
				_max_size(other._max_size) {}

			DynamicStringBuffer &operator=(DynamicStringBuffer &&other) {
				_s = other._s;
				_size = std::move(other._size);
				_max_size = std::move(other._max_size);
			}

			size_t Size() const noexcept {
				return _size;
			}

			size_t MaxSize() const noexcept {
				return _max_size;
			}

			size_t Capacity() const noexcept {
				return _s.capacity();
			}

			MutableBuffer Prepare(size_t n, util::error::Error &err) {
				size_t size = Size();
				size_t max_size = MaxSize();
				if ((size > max_size) || ((max_size - size) < n)) {
					err = util::error::RuntimeError(util::error::RuntimeErrorCode::MEMORY_LIMIT_EXCEEDED);
					return MutableBuffer();
				}

				_s.resize(size + n);
				return (MutableBuffer::From(_s) + _size);
			}

			void Commit(size_t n) {
				_size += (std::min)(n, _s.size() - _size);
				_s.resize(_size);
			}

			void Consume(size_t n) {
				size_t consume_size = (std::min)(n, _size);
				_s.erase(0, consume_size);
				_size -= consume_size;
			}

			void Consume(String &s, size_t n) {
				size_t consume_size = (std::min)(n, _size);
				s = _s.substr(0, consume_size);
				_s.erase(0, consume_size);
				_size -= consume_size;
			}

		// iterator
		public:
			const_iterator begin() const {
				return _s.begin();
			}

			const_iterator end() const {
				return (begin() + _size);
			}

		private:
			String &_s;
			size_t _size;
			const size_t _max_size;
		};

	}

}