#pragma once

#include <regex>

#include <Network/Ftp/Type.h>
#include <Network/Ftp/Parser/FileNameParser.h>
#include <Network/Ftp/Parser/FileStatusParser.h>
#include <Network/Ftp/Parser/UserGroupNameParser.h>

#include <Network/Parser/NumberParser.h>
#include <Network/Parser/TimestampParser.h>

#include <Util/String.h>

namespace network {

	namespace ftp {

		namespace parser {

			class FileListParser : public network::parser::Parser {
			public:
				FileListParser(File::List &files) noexcept
					: Parser(),
					_sub_state(SubState::SOL),
					_sub_parser(nullptr),
					_files(files) {}

				void Eoi() override {
					if (_sub_state == SubState::NAME) {
						_sub_parser->Eoi();
						if (_sub_parser->Succeeded()) {
							CommitFile();
							Finish(true);
						}
						else {
							Finish(false);
						}
						return;
					}
					Finish((_sub_state == SubState::SOL) || (_sub_state == SubState::EOL));
				}

			private:
				enum class SubState {
					SOL,
					STATUS,
					LINKS,
					OWNER,
					GROUP,
					SIZE,
					LAST_MOD_TIME,
					NAME,
					EOL
				};

				SubState _sub_state;
				std::shared_ptr<Parser> _sub_parser;

				File _file;
				File::List &_files;

			private:
				void Parse() override {
					while (!Empty()) {
						if (_sub_parser) {
							_sub_parser->Input(Cur(), End());
							if (!_sub_parser->Finished()) {
								return;
							}
							if (_sub_parser->Failed()) {
								Finish(false);
								return;
							}

							Skip(_sub_parser->Count());
						}

						switch (_sub_state) {
						case SubState::SOL:
							_sub_state = SubState::STATUS;
							_sub_parser = std::make_shared<FileStatusParser>(_file.type, _file.perm);
							break;

						case SubState::STATUS:
							_sub_state = SubState::LINKS;
							_sub_parser = std::make_shared<network::parser::NumberParser<uint64_t>>(_file.links);
							break;

						case SubState::LINKS:
							_sub_state = SubState::OWNER;
							_sub_parser = std::make_shared<UserGroupNameParser>(_file.owner);
							break;

						case SubState::OWNER:
							_sub_state = SubState::GROUP;
							_sub_parser = std::make_shared<UserGroupNameParser>(_file.group);
							break;

						case SubState::GROUP:
							_sub_state = SubState::SIZE;
							_sub_parser = std::make_shared<network::parser::NumberParser<uint64_t>>(_file.size);
							break;

						case SubState::SIZE:
							_sub_state = SubState::LAST_MOD_TIME;
							_sub_parser = std::make_shared<network::parser::TimestampParser>(_file.last_mod_time);

							break;

						case SubState::LAST_MOD_TIME:
							_sub_state = SubState::NAME;
							_sub_parser = std::make_shared<FileNameParser>(_file.name);
							break;

						case SubState::NAME:
							_sub_state = SubState::EOL;
							_sub_parser = nullptr;
							break;

						case SubState::EOL:
							CommitFile();

							Try('\r');
							if (Try('\n')) {
								_sub_state = SubState::SOL;
							}
							break;

						default:
							Finish(false);
							return;
						}
					}
				}

				void CommitFile() {
					_files.push_back(std::make_shared<File>(_file));
				}
			};

		}

	}

}
