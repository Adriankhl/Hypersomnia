#include "augs/readwrite/memory_stream.h"

namespace augs {
	void from_bytes(const std::vector<std::byte>& bytes, trivial_type_marker& object) {
		std::memcpy(
			reinterpret_cast<std::byte*>(std::addressof(object)),
		   	bytes.data(),
		   	bytes.size()
		);
	}

	memory_stream::memory_stream(const std::vector<std::byte>& new_buffer)
		: buffer(new_buffer)
	{
		set_write_pos(buffer.size());
	}

	memory_stream::memory_stream(std::vector<std::byte>&& new_buffer)
		: buffer(std::move(new_buffer))
	{
		set_write_pos(buffer.size());
	}

	memory_stream& memory_stream::operator=(const std::vector<std::byte>& new_buffer) {
		buffer = new_buffer;
		set_write_pos(buffer.size());
		set_read_pos(0);
		return *this;
	}

	memory_stream& memory_stream::operator=(std::vector<std::byte>&& new_buffer) {
		buffer = std::move(new_buffer);
		set_write_pos(buffer.size());
		set_read_pos(0);
		return *this;
	}

	void byte_counter_stream::write(const std::byte* const, const std::size_t bytes) {
		write_pos += bytes;
	}

	memory_stream byte_counter_stream::create_reserved_stream() {
		memory_stream reserved;
		reserved.reserve(write_pos);
		return reserved;
	}
}