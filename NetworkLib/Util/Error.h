#pragma once

#include <memory>
#include <string>
#include <sstream>
#include <functional>

namespace util {

	namespace error {

#define REGISTER_ERROR(kind) \
    class kind##Error : public BasicError { \
    public: \
        kind##Error(kind##ErrorCode code) noexcept \
            : BasicError(static_cast<int>(code)) {} \
        kind##Error(kind##ErrorCode code, const std::string &desc) noexcept \
            : BasicError(static_cast<int>(code), desc) {} \
        kind##Error(kind##ErrorCode code, const char *desc) noexcept \
            : BasicError(static_cast<int>(code), desc) {} \
        kind##Error(BasicError &&err) noexcept \
            : BasicError(std::move(err)) {} \
        kind##Error(kind##Error &&err) = default; \
        kind##Error(const kind##Error &err) = default; \
        kind##Error &operator=(kind##Error &&err) = default; \
        kind##Error &operator=(const kind##Error &err) = default; \
		Error Error() const noexcept { \
			return Error::From(*this); \
		} \
        const char *Kind() const override { \
            return _kind.c_str(); \
        } \
    private: \
        std::string _what; \
        const std::string _kind = #kind; \
    }

		class BasicError {
		public:
			using Ptr = typename std::shared_ptr<BasicError>;

		public:
			BasicError()
				: _code(0) {}

			BasicError(int code) noexcept 
				: _code(code) {}

			BasicError(int code, const std::string &desc) noexcept
				: _code(code), _desc(desc) {}

			BasicError(int code, const char *desc) noexcept
				: _code(code), _desc(desc) {}

			BasicError(BasicError &&err) = default;
			BasicError(const BasicError &err) = default;

			BasicError &operator=(BasicError &&err) = default;
			BasicError &operator=(const BasicError &err) = default;

			virtual const char *What() {
				if (!_what.empty()) {
					try {
						std::ostringstream os;
						os << Kind() << " Error Code " << _code;
						if (!_desc.empty()) {
							os << ": " << _desc;
						}
						_what = os.str();
					}
					catch (const std::runtime_error &e) {
						return e.what();
					}
					catch (...) {
						return "Unknown Error";
					}
				}
				return _what.c_str();
			}

			virtual const char *Kind() const = 0;

			operator bool() const {
				return (_code != 0);
			}

		private:
			int _code;
			std::string _desc;
			std::string _what;
		};

		class Error {
		public:
			Error() noexcept {}
			Error(Error &&err) = default;
			Error(const Error &err) = default;

			Error &operator=(Error &&err) = default;
			Error &operator=(const Error &err) = default;

			template<class T>
			Error &operator=(const T &t) {
				*this = Error::From<T>(t);
				return *this;
			}

			const char *What() {
				if (_err) {
					return _err->What();
				}
				return nullptr;
			}

			operator bool() const {
				return (_err && !!(*_err));
			}

			template<class T>
			static
				std::enable_if_t<std::is_base_of_v<BasicError, T>, Error>
				From(const T &t) {
				Error err;
				err._err = std::static_pointer_cast<BasicError>(std::make_shared<T>(t));
				return err;
			}

			static Error None() noexcept {
				return Error();
			}

		private:
			BasicError::Ptr _err;
		};

		enum class IOErrorCode {
			NONE = 0,
			OPEN_FILE_FAILED,
			READ_FAILED,
			WRITE_FAILED,
			READ_INTO_NULL,
		};

		REGISTER_ERROR(IO);

		enum class RuntimeErrorCode {
			NONE = 0,
			NULL_POINTER,
			MEMORY_LIMIT_EXCEEDED,
		};

		REGISTER_ERROR(Runtime);

		namespace lib {

			using SocketErrorCode = int;
			REGISTER_ERROR(Socket);

		}

	}

}
