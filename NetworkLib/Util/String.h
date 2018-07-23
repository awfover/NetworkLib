#pragma once

#include <string>
#include <vector>
#include <sstream>

namespace util {

	namespace string {

		template<class T>
		void Concat(std::string &s, const T &t) noexcept {
			std::ostringstream os;
			os << t;
			s += os.str();
		}

		class Join {
		public:
			explicit Join(std::string &sep = std::string(" ")) noexcept
				: _sep(sep) {}

			void operator()(std::string &s) const noexcept {
				return;
			}

			template<class T, class ...Args>
			void operator()(std::string &s, const T &t, Args &&...args) const noexcept {
				Concat(s, t);

				if (sizeof...(args) > 0) {
					s += _sep;
					this->operator()(s, std::forward<Args>(args)...);
				}
			}

		private:
			std::string _sep;
		};

		const Join JoinSpace;

		template<class Elem>
		class Pattern {
		public:
			using value_type = typename Elem::value_type;
			using const_iterator = typename Elem::const_iterator;

		public:
			template<class ...Pats>
			Pattern(Pats &&...pats)
				: _matched(false) {
				_pats = { pats... };
				_offs.resize(Size());
				for (size_t i = 0; i < Size(); i++) {
					_offs[i] = _pats[i].begin();
				}
			}

			bool Matched() const noexcept {
				return _matched;
			}

			bool Consume(value_type c) {
				if (_matched) {
					return _matched;
				}

				const_iterator ci;
				for (size_t i = 0; i < Size(); i++) {
					if (Consume(c, i)) {
						return (_matched = true);
					}
				}

				return false;
			}

		private:
			bool _matched;

			std::vector<Elem> _pats;
			std::vector<const_iterator> _offs;

		private:
			size_t Size() const noexcept {
				return _pats.size();
			}

			bool Consume(value_type c, size_t i) {
				const_iterator end = _pats[i].end();
				const_iterator &off = _offs[i];
				if ((off != end) && ((*off) == c)) {
					++off;
					return (off == end);
				}
				return false;
			}
		};

	}

}