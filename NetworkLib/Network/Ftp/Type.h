#pragma once

#include <ctime>
#include <string>
#include <cstdint>
#include <filesystem>

namespace network {

	namespace ftp {

		namespace fs = std::experimental::filesystem;

		struct File {
			using Ptr = typename std::shared_ptr<File>;
			using List = typename std::vector<Ptr>;

			fs::file_type type;
			fs::perms perm;
			uint64_t links;
			std::string owner;
			std::string group;
			uint64_t size;
			time_t last_mod_time;
			std::string name;
		};

		static bool IsDirectory(const File *file) {
			return (file->type == fs::file_type::directory);
		}

	}

}