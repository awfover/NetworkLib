#pragma once

#include <unordered_map>

namespace network {

	namespace ftp {

		enum class CmdType {
			BAD = 0,
			USER,
			PASS,
			ACCT,
			CWD,
			CDUP,
			SMNT,
			REIN,
			QUIT,
			PORT,
			PASV,
			TYPE,
			STRU,
			MODE,
			RETR,
			STOR,
			STOU,
			APPE,
			ALLO,
			REST,
			RNFR,
			RNTO,
			ABOR,
			DELE,
			RMD,
			MKD,
			PWD,
			LIST,
			NLST,
			SITE,
			SYST,
			STAT,
			HELP,
			NOOP
		};

		const std::unordered_map<CmdType, std::string> CMD_TEXT_TABLE = {
			{ CmdType::BAD, "BAD" },
			{ CmdType::USER, "USER" },
			{ CmdType::PASS, "PASS" },
			{ CmdType::ACCT, "ACCT" },
			{ CmdType::CWD,  "CWD" },
			{ CmdType::CDUP, "CDUP" },
			{ CmdType::SMNT, "SMNT" },
			{ CmdType::REIN, "REIN" },
			{ CmdType::QUIT, "QUIT" },
			{ CmdType::PORT, "PORT" },
			{ CmdType::PASV, "PASV" },
			{ CmdType::TYPE, "TYPE" },
			{ CmdType::STRU, "STRU" },
			{ CmdType::MODE, "MODE" },
			{ CmdType::RETR, "RETR" },
			{ CmdType::STOR, "STOR" },
			{ CmdType::STOU, "STOU" },
			{ CmdType::APPE, "APPE" },
			{ CmdType::ALLO, "ALLO" },
			{ CmdType::REST, "REST" },
			{ CmdType::RNFR, "RNFR" },
			{ CmdType::RNTO, "RNTO" },
			{ CmdType::ABOR, "ABOR" },
			{ CmdType::DELE, "DELE" },
			{ CmdType::RMD,  "RMD" },
			{ CmdType::MKD,  "MKD" },
			{ CmdType::PWD,  "PWD" },
			{ CmdType::LIST, "LIST" },
			{ CmdType::NLST, "NLST" },
			{ CmdType::SITE, "SITE" },
			{ CmdType::SYST, "SYST" },
			{ CmdType::STAT, "STAT" },
			{ CmdType::HELP, "HELP" },
			{ CmdType::NOOP, "NOOP" }
		};

		static const std::string &CmdTypeToText(CmdType t) {
			auto i = CMD_TEXT_TABLE.find(t);
			if (i == CMD_TEXT_TABLE.end()) {
				i = CMD_TEXT_TABLE.find(CmdType::BAD);
			}
			return i->second;
		}

		class Cmd {
		public:
			Cmd() noexcept
				: _t(CmdType::BAD) {}

			template<class ...Args>
			Cmd(CmdType t, Args &&...args) noexcept
				: _t(t) {
				util::string::JoinSpace(_s, CmdTypeToText(t), std::forward<Args>(args)...);
				_s += "\r\n";
			}

			Cmd &operator=(Cmd &&c) {
				_t = std::move(c._t);
				_s = std::move(c._s);
				return *this;
			}

			CmdType Type() const noexcept {
				return _t;
			}

			const std::string &Str() const noexcept {
				return _s;
			}

		private:
			CmdType _t;
			std::string _s;
		};

	}

}