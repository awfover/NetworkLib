#pragma once

#include <Network/Parser/Parser.h>

namespace network {

	namespace ftp {

		namespace parser {

			class FileNameParser: public network::parser::Parser{
			public:
				FileNameParser(std::string &name) noexcept
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

							Back();
							start = Cur();

							_has_first = true;
							_name.resize(0);
						}
						else {
							if (c == '/') {
								Finish(false);
								return;
							}
							if ((c == '\r') || (c == '\n')) {
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