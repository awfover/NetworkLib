#pragma once

#include <string>
#include <functional>

#include <Util/Buffer.h>

namespace network {

	namespace parser {

		class Parser {
		public:
			enum class State {
				ONGOING,
				SUCCEEDED,
				FAILED,
			};

		public:
			void Input(const std::string &s) {
				_sta = _cur = s.size() ? &s[0] : 0;
				_end = _cur + s.size();
				Parse();
			}

			void Input(const char *start, const char *end) {
				_sta  = _cur = start;
				_end = end;
				Parse();
			}

			void Input(const util::buffer::ConstBuffer &b) {
				_sta = _cur = static_cast<const char *>(b.Data());
				_end = _sta + b.Size();
				Parse();
			}

			virtual void Eoi() {
				if (!Finished()) {
					Finish(false);
				}
			}

			bool Finished() const noexcept {
				return (_state != State::ONGOING);
			}

			bool Failed() const noexcept {
				return (_state == State::FAILED);
			}

			bool Succeeded() const noexcept {
				return (_state == State::SUCCEEDED);
			}

			size_t Count() const noexcept {
				return (_cur - _sta);
			}

		protected:
			const char *_sta;
			const char *_cur;
			const char *_end;

			State _state;

		protected:
			Parser() noexcept 
				: _sta(0), 
				_cur(0), 
				_end(0),
				_state(State::ONGOING) {}

			virtual void Parse() = 0;

			void Finish(bool success) noexcept {
				_state = success ? State::SUCCEEDED : State::FAILED;
			}

			const char *Cur() const noexcept {
				return _cur;
			}

			const char *End() const noexcept {
				return _end;
			}

			char Char() const noexcept {
				return *_cur;
			}

			void Back() noexcept {
				--_cur;
			}

			char Next() noexcept {
				return *(_cur++);
			}

			bool Empty() const noexcept {
				return (_cur >= _end);
			}

			bool Test(char c) const noexcept {
				return (!Empty() && (Char() == c));
			}

			bool Test(std::function<int(char)> fn) const noexcept {
				return (!Empty() && fn(Char()));
			}

			bool Try(char c) noexcept {
				if (Test(c)) {
					Next();
					return true;
				}
				return false;
			}

			bool Try(std::function<int(char)> fn) noexcept {
				if (Test(fn)) {
					Next();
					return true;
				}
				return false;
			}

			void Skip(size_t n) noexcept {
				_cur += std::min<size_t>(n, _end - _cur);
			}

			void Skip(const std::string &s) {
				bool b;
				while (!Empty()) {
					b = false;
					for (auto const &c : s) {
						if (Try(c)) {
							b = true;
							break;
						}
					}

					if (!b) {
						return;
					}
				}
			}

			void Skip(char c) {
				while (!Empty()) {
					if (!Try(c)) {
						return;
					}
				}
			}

			void SkipSpaces() {
				Skip(' ');
			}
		};

	}

}