#pragma once
#include "augs/readwrite/memory_stream.h"
#include "augs/misc/serialization_buffers.h"

template <bool C>
struct initial_arena_state_payload {
	maybe_const_ref_t<C, cosmos_solvable_significant> signi;
	maybe_const_ref_t<C, online_mode_and_rules> mode;
	maybe_const_ref_t<C, server_vars> vars;
};

using ref_net_stream = augs::basic_ref_memory_stream<message_bytes_type>;
using cref_net_stream = augs::basic_ref_memory_stream<const message_bytes_type>;

template <class T, class P>
bool unsafe_read_message(
	T& msg,
	P& payload
) {
	auto s = cref_net_stream(msg.bytes);
	augs::read_bytes(s, payload);

	return true;
}

template <class T, class P>
bool unsafe_write_message(
	T& msg,
	const P& payload
) {
	auto s = ref_net_stream(msg.bytes);
	augs::write_bytes(s, payload);

	return true;
}

namespace net_messages {
	bool client_welcome::to_payload(
		decltype(client_welcome::payload)& output
	) {
		output = std::move(payload);
		return true;
	}

	bool client_welcome::from_payload(
		decltype(client_welcome::payload)&& input
	) {
		payload = std::move(input);
		return true;
	}

	bool client_entropy::to_payload(
		total_client_entropy& output
	) {
		// TODO SECURITY: don't blindly trust the client!!!
		// TODO BANDWIDTH: optimize entropy i/o

		return unsafe_read_message(*this, output);
	}

	bool client_entropy::from_payload(
		total_client_entropy&& input
	) {
		return unsafe_write_message(*this, input);
	}

	bool initial_arena_state::to_payload(
		augs::serialization_buffers& buffers,
		const initial_arena_state_payload<false> in
	) {
		const auto data = reinterpret_cast<const std::byte*>(GetBlockData());
		const auto size = static_cast<std::size_t>(GetBlockSize());

		if (size < sizeof(uint32_t)) {
			return false;
		}

		const auto uncompressed_size = *reinterpret_cast<const uint32_t*>(data);
	
		/*
			TODO: validate uncompressed_size with some predefined max solvable size.
		*/

		if (uncompressed_size > 100 * 1024 * 1024) {
			return false;
		}

		buffers.serialization.resize(uncompressed_size);

		augs::decompress(
			data + sizeof(uint32_t),
			size - sizeof(uint32_t),
			buffers.serialization
		);

		auto s = augs::cref_memory_stream(buffers.serialization);

		augs::read_bytes(s, in.signi);
		augs::read_bytes(s, in.mode);
		augs::read_bytes(s, in.vars);

		return true;
	}

	const std::vector<std::byte>* initial_arena_state::from_payload(
		augs::serialization_buffers& buffers,
		const initial_arena_state_payload<true> in
	) {
		auto write_all_to = [&in](auto& s) {
			augs::write_bytes(s, in.signi);
			augs::write_bytes(s, in.mode);
			augs::write_bytes(s, in.vars);
		};

		{
			augs::byte_counter_stream s;
			write_all_to(s);
			buffers.serialization.reserve(s.size() + sizeof(uint32_t));
		}

		auto s = buffers.make_serialization_stream();

		const auto real_size = static_cast<uint32_t>(buffers.serialization.size());
		augs::write_bytes(s, real_size);

		write_all_to(s);

		augs::compress(buffers.compression_state, buffers.serialization, buffers.compressed);

		return std::addressof(buffers.compressed);
	}
}
