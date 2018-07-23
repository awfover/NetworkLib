#pragma once

#include <locale>
#include <string>
#include <codecvt>

namespace util {

	namespace locale {

		template<class Facet>
		struct DeletableFacet : Facet
		{
			using Facet::Facet; // inherit constructors
			~DeletableFacet() {}
		};

		static void ConverseEncoding(std::string &out, const std::wstring &in) {
			std::locale loc = std::locale("");
			using F = DeletableFacet<std::codecvt_byname<wchar_t, char, std::mbstate_t>>;
			auto cvt = new F(loc.name());
			std::wstring_convert<F> conv(cvt);
			out = conv.to_bytes(in);
		}

		static void ConverseEncoding(std::wstring &out, const std::string &in) {
			std::locale loc = std::locale("");
			using F = DeletableFacet<std::codecvt_byname<wchar_t, char, std::mbstate_t>>;
			auto cvt = new F(loc.name());
			std::wstring_convert<F> conv(cvt);
			out = conv.from_bytes(in);
		}

	}

}