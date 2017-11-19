#pragma once
#include <experimental/filesystem>
#include "augs/readwrite/byte_readwrite_declaration.h"

std::string to_forward_slashes(std::string);

namespace augs {
	using path_type = std::experimental::filesystem::path;

#if READWRITE_OVERLOAD_TRAITS_INCLUDED
#error "I/O traits were included BEFORE I/O overloads, which may cause them to be omitted under some compilers."
#endif

	template <class Archive>
	void read_object_bytes(
		Archive& ar,
		path_type& storage
	) {
		std::string str;
		augs::read_bytes(ar, str);
		storage = str;
	}

	template <class Archive>
	void write_object_bytes(
		Archive& ar,
		const path_type& storage
	) {
		augs::write_bytes(ar, to_forward_slashes(storage.string()));
	}

	template <class Archive>
	void write_object_bytes(Archive& ar, const std::string& storage) = delete;

	template <class Archive>
	void read_object_bytes(Archive& ar, std::string& storage) = delete;

	template <class Archive>
	void write_object_bytes(Archive& ar, const std::wstring& storage) = delete;

	template <class Archive>
	void read_object_bytes(Archive& ar, std::wstring& storage) = delete;

	inline auto to_display_path(path_type target_path) {
		auto display_path = target_path.filename();
		display_path += " (";
		display_path += target_path.replace_filename("");
		display_path += ")";
		return display_path;
	}
}

namespace std {
	template <>
	struct hash<augs::path_type> {
		size_t operator()(const augs::path_type& k) const {
			return std::experimental::filesystem::hash_value(k);
		}
	};
}
