#pragma once

#include <Network/Ftp/Type.h>
#include <Network/Parser/Parser.h>

namespace network {

	namespace ftp{

		namespace parser {

			class FileStatusParser : public network::parser::Parser {
			public:
				FileStatusParser(fs::file_type &file_type, fs::perms &file_perm) noexcept
					: Parser(),
					_pos(0),
					_file_type(file_type),
					_file_perm(file_perm) {}

			private:
				std::uint8_t _pos;

				fs::file_type &_file_type;
				fs::perms &_file_perm;

			private:
				void Parse() override {
					if (Finished()) {
						return;
					}

					char c;
					while (!Empty()) {
						if (_pos == 0) {
							SkipSpaces();
							if (Empty()) {
								return;
							}
						}

						c = Next();
						if (_pos == 0) {
							_file_perm = fs::perms::none;
							if (!FileType(_file_type, c)) {
								Finish(false);
								return;
							}
						}
						else if ((_pos >= 1) && (_pos <= 9)) {
							if (!FilePerm(_file_perm, _pos - 1, c)) {
								Finish(false);
								return;
							}
						}
						else if (_pos == 10) {
							// ignore
							Finish(true);
							return;
						}

						++_pos;
					}
				}

				bool FileType(fs::file_type &type, char c) noexcept {
					using fs::file_type;

					switch (c) {
					case '-':
						type = file_type::regular;
						break;

					case 'b':
						type = file_type::block;
						break;

					case 'c':
						type = file_type::character;
						break;

					case 'd':
						type = file_type::directory;
						break;

					case 'l':
						type = file_type::symlink;
						break;

					case 'p':
						type = file_type::fifo;
						break;

					case 's':
						type = file_type::socket;
						break;

					default:
						type = file_type::unknown;
					}

					return true;
				}

				bool FilePerm(fs::perms &perm, uint8_t i, char c) noexcept {
					using fs::perms;

					uint8_t ro = i / 3;
					uint8_t co = i % 3;
					switch (c) {
					case '-':
						break;

					case 'r':
						if (co != 0) {
							return false;
						}
						else if (ro == 0) {
							perm |= perms::owner_read;
						}
						else if (ro == 1) {
							perm |= perms::group_read;
						}
						else if (ro == 2) {
							perm |= perms::others_read;
						}
						else {
							return false;
						}
						break;

					case 'w':
						if (ro == 0) {
							perm |= perms::owner_write;
						}
						else if (ro == 1) {
							perm |= perms::group_write;
						}
						else if (ro == 2) {
							perm |= perms::others_write;
						}
						else {
							return false;
						}
						break;

					case 'x':
						if (ro == 0) {
							perm |= perms::owner_exec;
						}
						else if (ro == 1) {
							perm |= perms::group_exec;
						}
						else if (ro == 2) {
							perm |= perms::others_exec;
						}
						else {
							return false;
						}
						break;
					}

					return true;
				}
			};

		}

	}

}