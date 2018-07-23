#pragma once

#include <Network/Parser/Parser.h>

namespace network {

	namespace ftp {

		namespace parser {

			// User/group names must match [a-z_][a-z0-9_-]*[$]
			// https://fossies.org/linux/shadow/libmisc/chkname.c
			class UserGroupNameParser : public network::parser::Parser {
			public:
				UserGroupNameParser(std::string &name) noexcept 
					: Parser(), 
					_name(name),
					_has_first(false) {}

				void Eoi() override {
					Finish(_name.size() > 0);
				}

			private:
				bool _has_first;
				std::string &_name;

			private:
				void Parse() override {
					if (Finished()) {
						return;
					}

					char c;
					const char *start = Cur();
					while (!Empty()) {
						c = Next();
						if (!_has_first) {
							if (isspace(c)) {
								continue;
							}
							if ((c != '_') && !isalpha(c)) {
								Finish(false);
								return;
							}

							_has_first = true;
							start = Cur() - 1;
							_name.resize(0);
						}
						else {
							if (c == '$') {
								Finish(true);
								break;
							}
							if ((c != '_') && (c != '-') && !isalnum(c)) {
								Back();
								Finish(true);
								break;
							}
						}
					}

					size_t inc_size = Cur() - start;
					size_t old_size = _name.size();
					size_t new_size = old_size + inc_size;
					_name.resize(new_size);
					std::memcpy(&_name[old_size], start, inc_size);
				}
			};

		}

	}

}