#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <Util/Error.h>

namespace util {

	namespace serializer {

		class Serializer {
		public:
			explicit Serializer(std::istream &is, util::error::Error &err)
				: _is(&is),
				_err(err) {}

			explicit Serializer(std::ostream &os, util::error::Error &err)
				: _os(&os),
				_err(err) {}

			template<class T>
			std::enable_if_t<std::is_integral_v<T>, Serializer>
				&operator<<(const T &t) {
				if (!_err) {
					_os->write((const char *)&t, sizeof(T));
					if (_os->fail()) {
						_err = util::error::IOError(util::error::IOErrorCode::WRITE_FAILED);
					}
				}

				return  *this;
			}

			template<class T>
			std::enable_if_t<std::is_integral_v<typename T::size_type>, Serializer>
				&operator<<(const T &t) {
				if (!_err) {
					T::size_type size = t.size();
					(*this) << size;
					for (auto const &i : t) {
						(*this) << i;
					}
				}

				return *this;
			}

			template<class T>
			std::enable_if_t<std::is_pointer_v<T>, Serializer>
				&operator<<(const T &t) {
				if (!_err) {
					if (t) {
						(*this) << *t;
					}
					else {
						using TT = std::remove_pointer_t<T>;
						(*this) << TT();
					}
				}

				return *this;
			}

			template<class T>
			Serializer &operator<<(const std::shared_ptr<T> &t) {
				if (!_err) {
					if (t) {
						(*this) << *t;
					}
					else {
						using TT = std::remove_cv_t<T>;
						(*this) << TT();
					}
				}

				return *this;
			}

			template<class T>
			std::enable_if_t<std::is_integral_v<T>, Serializer>
				&operator>>(T &t) {
				if (!_err) {
					_is->read((char *)&t, sizeof(T));
					if (_is->fail()) {
						_err = util::error::IOError(util::error::IOErrorCode::READ_FAILED);
					}
				}

				return  *this;
			}

			template<class T>
			std::enable_if_t<std::is_integral_v<typename T::size_type>, Serializer>
				&operator>>(T &t) {
				if (!_err) {
					T::size_type size;
					(*this) >> size;

					t.resize(size);
					for (T::size_type i = 0; i < size; i++) {
						(*this) >> t[i];
					}
				}

				return *this;
			}

			template<class T>
			std::enable_if_t<std::is_pointer_v<T>, Serializer>
				&operator>>(T &t) {
				if (!_err) {
					if (t) {
						(*this) >> *t;
					}
					else {
						err = util::error::IOError(util::error::IOErrorCode::READ_INTO_NULL);
					}
				}

				return *this;
			}

			template<class T>
			Serializer &operator>>(const std::shared_ptr<T> &t) {
				if (!_err) {
					if (!t) {
						t = std::make_shared<T>();
					}
					(*this) >> *t;
				}

				return *this;
			}

		private:
			util::error::Error &_err;

			std::istream *_is;
			std::ostream *_os;
		};

	}

}