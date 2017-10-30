#if BUILD_OPENAL
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include "augs/ensure.h"
#include "augs/audio/OpenAL_error.h"

#include "augs/filesystem/file.h"

#include "augs/audio/sound_data.h"
#include "augs/audio/sound_buffer.h"



#define TRACE_CONSTRUCTORS_DESTRUCTORS 0

#if TRACE_CONSTRUCTORS_DESTRUCTORS
int g_num_buffers = 0;
#endif

namespace augs {
	ALenum get_openal_format_of(const sound_data& d) {
#if BUILD_OPENAL
		if (d.channels == 1) {
			return AL_FORMAT_MONO16;
		}
		else if (d.channels == 2) {
			return AL_FORMAT_STEREO16;
		}

		const bool bad_format = true;
		ensure(!bad_format);
		return AL_FORMAT_MONO8;
#else
		return 0xdeadbeef;
#endif
	}

	single_sound_buffer::single_sound_buffer(const sound_data& data) {
		set_data(data);
	}

	single_sound_buffer::~single_sound_buffer() {
		destroy();
	}

	single_sound_buffer::single_sound_buffer(single_sound_buffer&& b) : 
		computed_length_in_seconds(b.computed_length_in_seconds),
		id(b.id),
		initialized(b.initialized)
	{
		b.initialized = false;
	}

	single_sound_buffer& single_sound_buffer::operator=(single_sound_buffer&& b) {
		destroy();

		computed_length_in_seconds = b.computed_length_in_seconds;
		id = b.id;
		initialized = b.initialized;

		b.initialized = false;

		return *this;
	}

	void single_sound_buffer::destroy() {
		if (initialized) {
#if TRACE_CONSTRUCTORS_DESTRUCTORS
			--g_num_buffers;
			LOG("alDeleteBuffers: %x (now %x buffers)", id, g_num_buffers);
#endif
			AL_CHECK(alDeleteBuffers(1, &id));
			initialized = false;
		}
	}

	ALuint single_sound_buffer::get_id() const {
		return id;
	}

	single_sound_buffer::operator ALuint() const {
		return get_id();
	}

	void single_sound_buffer::set_data(const sound_data& new_data) {
		if (!initialized) {
			AL_CHECK(alGenBuffers(1, &id));

#if TRACE_CONSTRUCTORS_DESTRUCTORS
			++g_num_buffers;
			LOG("alGenBuffers: %x (now %x buffers)", id, g_num_buffers);
#endif
			initialized = true;
		}

		if (new_data.samples.empty()) {
			LOG("WARNING! No samples were sent to a sound buffer.");
			return;
		}

		const auto passed_format = get_openal_format_of(new_data);
		const auto passed_frequency = new_data.frequency;
		const auto passed_bytesize = new_data.samples.size() * sizeof(sound_sample_type);
		computed_length_in_seconds = new_data.compute_length_in_seconds();

#if LOG_AUDIO_BUFFERS
		LOG("Passed format: %x\nPassed frequency: %x\nPassed bytesize: %x", passed_format, passed_frequency, passed_bytesize);
#endif

		AL_CHECK(alBufferData(id, passed_format, new_data.samples.data(), static_cast<ALsizei>(passed_bytesize), static_cast<ALsizei>(passed_frequency)));
	}

	double single_sound_buffer::get_length_in_seconds() const {
		return computed_length_in_seconds;
	}

	const single_sound_buffer& sound_buffer::variation::stereo_or_mono() const {
		return stereo ? *stereo : *mono;
	}

	const single_sound_buffer& sound_buffer::variation::mono_or_stereo() const {
		return mono ? *mono : *stereo;
	}

	sound_buffer::variation::variation(const sound_data& data, const bool generate_mono) {
		if (data.channels == 1) {
			mono = data;
		}
		else if (data.channels == 2) {
			stereo = data;

			if (generate_mono) {
				mono = sound_data(data).to_mono();
			}
		}
		else {
			ensure(false && "Bad format");
		}
	}

	sound_buffer::sound_buffer(const sound_buffer_loading_input input) {
		from_file(input);
	}

	void sound_buffer::from_file(const sound_buffer_loading_input input) {
		const auto& path = input.path_template;

		if (
			const bool many_files = typesafe_sprintf(path.string(), 1) != path;
			many_files
		) {
			for (size_t i = 1;; ++i) {
				const auto target_path = augs::path_type(typesafe_sprintf(path.string(), i));

				if (!augs::file_exists(target_path)) {
					break;
				}

				variations.emplace_back(target_path, input.generate_mono);
			}

			ensure(variations.size() > 0);
		}
		else {
			variations.emplace_back(path, input.generate_mono);
		}
	}
}
