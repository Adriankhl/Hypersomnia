#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <chrono>
#include <algorithm>

#include <fstream>
#include <experimental/filesystem>

#include "augs/ensure.h"
#include "augs/filesystem/path.h"

namespace augs {
	template <class T, class... Args>
	auto with_exceptions(Args&&... args) {
		auto stream = T(std::forward<Args>(args)...);
		stream.exceptions(T::failbit | T::badbit);
		return stream;
	}

	inline auto open_binary_output_stream(const augs::path_type& path) {
		auto s = with_exceptions<std::ofstream>();
		s.open(path, std::ios::out | std::ios::binary);
		return s;
	}

	inline auto open_binary_input_stream(const augs::path_type& path) {
		auto s = with_exceptions<std::ifstream>();
		s.open(path, std::ios::in | std::ios::binary);
		return s;
	}

	using ifstream_error = std::ifstream::failure;
	
	inline std::chrono::system_clock::time_point last_write_time(const path_type& path) {
		return std::experimental::filesystem::last_write_time(path);
	}

	inline bool file_exists(const path_type& path) {
		return std::experimental::filesystem::exists(path);
	}

	inline void remove_file(const path_type& path) {
		std::experimental::filesystem::remove(path);
	}

	inline path_type first_free_path(const path_type path_template) {
		for (std::size_t candidate = 0;; ++candidate) {
			const auto candidate_path = candidate ? 
				typesafe_sprintf(path_template.string(), typesafe_sprintf("-%x", candidate))
				: typesafe_sprintf(path_template.string(), "")
			;

			if (!file_exists(candidate_path)) {
				return candidate_path;
			}
		}
	}

	inline path_type switch_path(
		const path_type canon_path,
		const path_type local_path
	) {
		if (file_exists(local_path)) {
			return local_path;
		}
		else {
			return canon_path;
		}
	}

	template <class C = char>
	auto file_to_string(const path_type& path, const C = C()) {
		auto t = with_exceptions<std::basic_ifstream<C>>();
		t.open(path);

		std::basic_stringstream<C> buffer;
		buffer << t.rdbuf();

		return buffer.str();
	}

	inline bool file_exists_and_non_empty(const path_type& path) {
		try {
			return file_to_string(path).size() > 0;
		}
		catch(...) {
			return false;
		}
	}

	template <class S>
	void save_as_text(const path_type& path, const S& text) {
		auto out = with_exceptions<std::ofstream>();
		out.open(path, std::ios::out);
		out << text;
	}

	template <class S>
	void save_as_text_if_different(const path_type& path, const S& text) {
		if (!file_exists(path) || text != file_to_string(path)) {
			auto out = with_exceptions<std::ofstream>();
			out.open(path, std::ios::out);
			out << text;
		}
	}
}