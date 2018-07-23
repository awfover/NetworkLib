#pragma once

#include <chrono>
#include <string>
#include <sstream>
#include <iomanip>

#include <WinSock2.h>

#include <Util/Locale.h>

namespace util {

	namespace time {

		template<class T>
		void ChronoToTimeval(timeval &tv, std::chrono::duration<T> ch) {
			auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(ch).count();
			tv.tv_sec = (long)(ms / 1000);
			tv.tv_usec = (long)((ms % 1000) * 1000);
		}

		void PutTime(std::wstring &out, const std::tm *tm, const char *fmt) {
			std::ostringstream os;
			os << std::put_time(tm, fmt);

			std::string tmp = os.str();
			util::locale::ConverseEncoding(out, tmp);
		}

	}

}