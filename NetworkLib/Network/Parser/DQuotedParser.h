#pragma once

#include <string>

#include <Network/Parser/Parser.h>

namespace network {

	namespace parser {

		class DQuotedParser : public Parser {
		public:
			DQuotedParser(std::string &dquoted) noexcept
				: Parser(),
				_sub_state(SubState::START),
				_dquoted(dquoted) {}

		private:
			enum class SubState {
				START,
				DQUOTE_LEFT,
				STRING,
				ESCAPED
			};

			SubState _sub_state;
			std::string &_dquoted;

		private:
			void Parse() override {
				if (Finished()) {
					return;
				}

				char c;
				const char *start = Cur();
				while (!Finished() && !Empty()) {
					c = Next();
					switch (_sub_state) {
					case SubState::START:
						if (isspace(c)) {
							break;
						}
						if (c == '"') {
							_dquoted.resize(0);
							_sub_state = SubState::DQUOTE_LEFT;
							break;
						}

						return Finish(false);

					case SubState::DQUOTE_LEFT:
						Back();
						start = Cur();
						_sub_state = SubState::STRING;
						break;

					case SubState::STRING:
						if (c == '"') {
							Finish(true);
							break;
						}
						if (c == '\\') {
							_sub_state = SubState::ESCAPED;
							break;
						}
						break;

					case SubState::ESCAPED:
						_sub_state = SubState::STRING;
						break;
					}
				}

				size_t inc_size = Cur() - start - (Finished() ? 1 : 0);
				size_t old_size = _dquoted.size();
				size_t new_size = old_size + inc_size;
				_dquoted.resize(new_size);
				std::memcpy(&_dquoted[old_size], start, inc_size);
			}
		};

	}

}