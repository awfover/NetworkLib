#pragma once

#include <ctime>
#include <locale>
#include <iomanip>
#include <sstream>

#include <Network/Parser/Parser.h>

namespace network {

	namespace parser {

		const uint8_t TIMESTAMP_STR_SIZE = 12;
		const char RECENT_TIMESTAMP[] = "%b %d %H:%M";
		const char NON_RECENT_TIMESTAMP[] = "%b %d %Y";

		// "%b %2d %5Y"
		// "%b %2d %2H:%2M"
		class TimestampParser : public Parser {
		public:
			TimestampParser(time_t &time)
				: Parser(),
				_time(time) {
				_str.reserve(TIMESTAMP_STR_SIZE);
			}

		private:
			time_t &_time;
			std::string _str;

		private:
			void Parse() override {
				if (Finished()) {
					return;
				}

				char c;
				while (!Empty()) {
					c = Next();
					if (_str.empty()) {
						if (isspace(c)) {
							continue;
						}
					}

					_str += c;
					if (_str.size() == TIMESTAMP_STR_SIZE) {
						std::tm t;
						std::memset(&t, 0, sizeof(t));

						std::istringstream is(_str);
						const char *fmt;
						if (_str[TIMESTAMP_STR_SIZE - 3] == ':') {
							fmt = RECENT_TIMESTAMP;

							std::time_t curt = std::time(nullptr);
							std::tm cur;
							gmtime_s(&cur, &curt);
							t.tm_year = cur.tm_year;
						}
						else {
							fmt = NON_RECENT_TIMESTAMP;
						}

						is >> std::get_time(&t, fmt);
						if (is.fail()) {
							Finish(false);
						}
						else {
							Finish(true);
							_time = std::mktime(&t);
						}
						return;
					}
				}
			}
		};

	}

}