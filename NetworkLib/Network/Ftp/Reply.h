#pragma once

#include <memory>
#include <string>
#include <vector>
#include <algorithm>

#include <Network/Ftp/Error.h>

#include <Util/Error.h>

namespace network {

	namespace ftp {

		class Reply {
		public:
			enum class ReplyType {
				BAD = 0,
				PRELIMINARY,
				COMPLETION,
				INTERMEDIATE,
				TRANSIENT_NEGATIVE_COMPLETION,
				PERMANENT_NEGATIVE_COMPLETION,
			};

			using Ptr = std::shared_ptr<Reply>;
			class Sequence;

		public:
			Reply() noexcept
				: _code(0), _last(true) {}

			void Parse(const std::string &s, util::error::Error &err) noexcept {
				size_t size = s.size();
				err = error::FtpError(error::FtpErrorCode::REPLY_BAD_CODE);
				if (size < 3) {
					return;
				}

				if (!std::all_of(s.begin() + 1, s.begin() + 3, isdigit)) {
					return;
				}

				if ((s[0] < '1') || (s[1] > '5')) {
					return;
				}

				if ((size >= 4) && (s[3] != ' ') && (s[3] != '-')) {
					return;
				}

				if ((size > 4) && (s[3] == '-')) {
					_last = false;
				}

				_msg = s.substr(4);
				_code = (uint16_t)std::stoi(s.substr(0, 3));

				err = error::Error();
				return;
			}

			ReplyType Type() const noexcept {
				return (ReplyType)(_code / 100);
			}

			uint16_t Code() const noexcept {
				return _code;
			}

			const std::string &Msg() const noexcept {
				return _msg;
			}

			bool Last() const noexcept {
				return _last;
			}

			bool Positive() const noexcept {
				ReplyType t = Type();
				return ((t >= ReplyType::PRELIMINARY) && (t <= ReplyType::INTERMEDIATE));
			}

			bool Negative() const noexcept {
				ReplyType t = Type();
				return ((t >= ReplyType::TRANSIENT_NEGATIVE_COMPLETION) && (t <= ReplyType::PERMANENT_NEGATIVE_COMPLETION));
			}

		private:
			bool _last;
			uint16_t _code;
			std::string _msg;
		};

		class Reply::Sequence : public std::vector<Reply::Ptr> {
		public:
			explicit Sequence() noexcept {}

			~Sequence() {}

			void Parse(const std::string &s, util::error::Error &err) noexcept {
				auto r = std::make_shared<Reply>();
				r->Parse(s, err);
				if (err) {
					return;
				}

				push_back(r);
			}

			void Clear() noexcept {
				clear();
			}

			Reply::ReplyType Type() const noexcept {
				if (size() == 0) {
					return ReplyType::BAD;
				}
				return front()->Type();
			}

			uint16_t Code() const noexcept {
				if (size() == 0) {
					return 0;
				}
				return front()->Code();
			}

			bool End() const noexcept {
				if (size() == 0) {
					return false;
				}
				return back()->Last();
			}

			bool Positive() const noexcept {
				if (size() == 0) {
					return false;
				}
				return front()->Positive();
			}

			bool Negative() const noexcept {
				if (size() == 0) {
					return false;
				}
				return front()->Negative();
			}
		};

	}

}