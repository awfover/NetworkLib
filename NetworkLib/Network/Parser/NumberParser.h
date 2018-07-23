#pragma once

#include <Network/Ftp/Type.h>
#include <Network/Parser/Parser.h>

namespace network {

	namespace parser {

		template<class Number>
		class NumberParser : public network::parser::Parser {
		public:
			NumberParser(Number &num) noexcept
				: Parser(),
				_start(0),
				_len(0),
				_num(num),
				_max((std::numeric_limits<Number>::max)()) {}

			void Eoi() override {
				Finish(Valid());
			}

		private:
			char _start;
			uint8_t _len;

			Number &_num;
			const Number _max;

		private:
			void Parse() override {
				if (Finished()) {
					return;
				}

				char c;
				Number tmp;
				while (!Empty()) {
					if (_len == 0) {
						SkipSpaces();
						if (Empty()) {
							return;
						}

						_start = Char();
						_num = static_cast<Number>(0);
					}

					c = Char();
					if (!isdigit(c)) {
						Finish(Valid());
						return;
					}
					else {
						if ((_num > 0) && (_max / _num < 10)) {
							Finish(false);
							return;
						}

						tmp = static_cast<Number>(c - '0');
						_num = _num * static_cast<Number>(10);
						if (_max - _num < tmp) {
							Finish(false);
							return;
						}

						++_len;
						_num += tmp;
						Next();
					}
				}
			}

			bool Valid() const noexcept {
				return (((_len > 1) && (_start != '0')) || (_len == 1));
			}
		};

	}

}