#pragma once

#include <Util/Error.h>

namespace network {

	namespace ftp {

		namespace error {

			using namespace util::error;

			enum class FtpErrorCode {
				NONE = 0,
				REPLY_BAD_CODE,
				REPLY_BAD_MSG,
				REPLY_NEGATIVE,

				CWD_FAILED,
				LOGIN_FAILED,
				INVALID_HOST_PORT,

				READ_FILE_LIST_FAILED,
				READ_DATA_CONN_FAILED,
				WRITE_DATA_CONN_FAILED,
			};

			REGISTER_ERROR(Ftp);

		}

	}

}