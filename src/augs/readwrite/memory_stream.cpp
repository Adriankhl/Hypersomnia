#include "augs/readwrite/memory_stream.h"
#include "augs/misc/serialization_buffers.h"

namespace augs {
	ref_memory_stream serialization_buffers::make_serialization_stream() {
		serialization.clear();
		return augs::ref_memory_stream(serialization);
	}

	void from_bytes(const std::vector<std::byte>& bytes, trivial_type_marker& object) {
		std::memcpy(
			reinterpret_cast<std::byte*>(std::addressof(object)),
		   	bytes.data(),
		   	bytes.size()
		);
	}

	memory_stream byte_counter_stream::create_reserved_stream() {
		memory_stream reserved;
		reserved.reserve(write_pos);
		return reserved;
	}
}