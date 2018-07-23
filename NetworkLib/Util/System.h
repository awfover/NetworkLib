#pragma once

#include <UI/stdafx.h>

#include <Util/Error.h>

namespace util {

	namespace system {

		namespace error {

			using namespace util::error;

			enum class SystemErrorCode {
				NONE = 0,
				GET_CURRENT_DIR_FAILED,
			};

			REGISTER_ERROR(System);

		}

		static void GetCurrentDir(std::wstring &dir, util::error::Error &err) {
			dir.resize(_MAX_PATH);
			DWORD size = GetCurrentDirectory(_MAX_PATH, &dir[0]);
			dir.resize(size);
			if (size == 0) {
				err = error::SystemError(error::SystemErrorCode::GET_CURRENT_DIR_FAILED);
			}
		}

		template<class Elem>
		class BaseDir {
		public:
			using String = std::basic_string<Elem, std::char_traits<Elem>, std::allocator<Elem>>;

		public:
			explicit BaseDir(Elem c) noexcept
				: _c(c) {}

			void operator()(String &str) const {
				operator()(str, str);
			}

			void operator()(String &out, const String &in) const {
				if (in.empty()) {
					return;
				}

				auto p = in.find_last_of(_c);
				if (p == String::npos) {
					out = in;
				}
				else {
					if (p == 0) {
						++p;
					}
					out = in.substr(0, p);
				}
			}

		private:
			Elem _c;
		};

		const BaseDir<char> PosixBaseDirA('/');
		const BaseDir<wchar_t> PosixBaseDirW('/');
		const BaseDir<char> WinBaseDirA('\\');
		const BaseDir<wchar_t> WinBaseDirW('\\');

		template<class Elem>
		class FileName {
		public:
			using String = std::basic_string<Elem, std::char_traits<Elem>, std::allocator<Elem>>;

		public:
			explicit FileName(Elem c) noexcept
				: _c(c) {}

			void operator()(String &str) const {
				operator()(str, str);
			}

			void operator()(String &out, const String &in) const {
				if (in.empty()) {
					return;
				}

				auto p = in.find_last_of(_c);
				if (p == String::npos) {
					out = in;
				}
				else {
					out = in.substr(p + 1);
				}
			}

		private:
			Elem _c;
		};

		const FileName<char> PosixFileNameA('/');
		const FileName<wchar_t> PosixFileNameW('/');
		const FileName<char> WinFileNameA('\\');
		const FileName<wchar_t> WinFileNameW('\\');

		template<class Elem>
		class JoinPath {
		public:
			using String = std::basic_string<Elem, std::char_traits<Elem>, std::allocator<Elem>>;

		public:
			explicit JoinPath(Elem c) noexcept 
				: _c(c) {}

			void operator()(String &base, const String &name) const {
				if (!base.empty() && (base.back() != _c)) {
					base += _c;
				}
				base += name;
			}

			template<class ...Args>
			void operator()(String &base, const String &name, Args &&...args) const {
				operator()(base, name);
				if (sizeof...(args) > 0) {
					operator()(base, std::forward<Args>(args)...);
				}
			}

		private:
			Elem _c;
		};

		const JoinPath<char> PosixJoinPathA('/');
		const JoinPath<wchar_t> PosixJoinPathW('/');
		const JoinPath<char> WinJoinPathA('\\');
		const JoinPath<wchar_t> WinJoinPathW('\\');

	}

}