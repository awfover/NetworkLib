#pragma once

#include <regex>
#include <sstream>

#include <Network/Parser/Parser.h>
#include <Network/Parser/NumberParser.h>

#include <Util/String.h>

namespace network {

	namespace ftp {

		namespace parser {

			const size_t HOST_PORT_NUM = 6;
			const std::regex HOST_PORT_REGEX("(\\d+),(\\d+),(\\d+),(\\d+),(\\d+),(\\d+)", std::regex_constants::ECMAScript);

			class HostPortParser : public network::parser::Parser {
			public:
				HostPortParser(std::string &host, std::string &port) noexcept
					: Parser(),
					_sub_state(SubState::START),
					_pos(0),
					_host(host),
					_port(port) {}

			private:
				enum class SubState {
					START,
					NUMBER,
					COMMA
				};

				SubState _sub_state;

				uint8_t _pos;
				uint8_t _num[HOST_PORT_NUM];
				std::shared_ptr<network::parser::NumberParser<uint8_t>> _num_parser;

				std::string &_host;
				std::string &_port;

			private:
				void Parse() override {
					if (Finished()) {
						return;
					}

					while (!Empty()) {
						if (_num_parser && !_num_parser->Finished()) {
							_num_parser->Input(Cur(), End());
							if (_num_parser->Failed()) {
								Finish(false);
								return;
							}
							else if (!_num_parser->Finished()) {
								return;
							}

							Skip(_num_parser->Count());
						}

						switch (_sub_state) {
						case SubState::START:
							if (Test(isdigit)) {
								_sub_state = SubState::NUMBER;
								_num_parser = std::make_shared<network::parser::NumberParser<uint8_t>>(_num[_pos++]);
								break;
							}
							if (!Empty()) {
								Next();
							}
							break;

						case SubState::NUMBER:
							if (_pos == HOST_PORT_NUM) {
								_host.reserve(15);
								for (uint8_t i = 0; i < 4; i++) {
									if (i > 0) {
										_host += '.';
									}
									_host += std::to_string(_num[i]);
								}
								_port = std::to_string((static_cast<uint16_t>(_num[4]) << 8) + _num[5]);

								Finish(true);
								return;
							}
							_sub_state = SubState::COMMA;
							break;

						case SubState::COMMA:
							if (!Empty()) {
								if (Try(',')) {
									_sub_state = SubState::NUMBER;
									_num_parser = std::make_shared<network::parser::NumberParser<uint8_t>>(_num[_pos++]);
									break;
								}

								Finish(false);
								return;
							}
							break;

						default:
							Finish(false);
							return;
						}
					}
				}

				/*bool Is8BitNumber(const std::string &s) {
					int n = std::stoi(s);
					return ((n >= 0) && (n < 256));
				}*/
			};

		}

	}

}

