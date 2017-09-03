#if BUILD_SOUND_FORMAT_DECODERS
#include <ogg/ogg.h>
#include <vorbis/vorbisfile.h>
#define OGG_BUFFER_SIZE 4096
#endif

#include "augs/audio/sound_data.h"
#include "augs/ensure.h"
#include "augs/filesystem/file.h"
#include "augs/audio/sound_data.h"
#include "augs/build_settings/setting_log_audio_files.h"

namespace augs {
	sound_data::sound_data(const path_type& path) {
		channels = 1;

#if BUILD_SOUND_FORMAT_DECODERS
		const auto extension = path.extension();

		if (extension == ".ogg") {
			std::vector<char> buffer;
			// TODO: throw if the file fails to load as OGG
			// TODO: detect endianess
			int endian = 0;             // 0 for Little-Endian, 1 for Big-Endian
			int bitStream = 0xdeadbeef;
			long bytes = 0xdeadbeef;
			char array[OGG_BUFFER_SIZE]; 

			auto ogg_stdio_file = std::unique_ptr<FILE, decltype(&fclose)>(fopen(path.string().c_str(), "rb"), &fclose);

			OggVorbis_File oggFile;
			ov_open(ogg_stdio_file.get(), &oggFile, NULL, 0);

			const auto* pInfo = ov_info(&oggFile, -1);
			channels = pInfo->channels;
			frequency = pInfo->rate;

			do {
				bytes = ov_read(&oggFile, array, OGG_BUFFER_SIZE, endian, 2, 1, &bitStream);
				buffer.insert(buffer.end(), array, array + bytes);
			} while (bytes > 0);

			samples.resize(buffer.size() / sizeof(sound_sample_type));
			std::memcpy(samples.data(), buffer.data(), buffer.size());

			ov_clear(&oggFile);
		}
		else if (extension == ".wav") {
			auto wav_file = std::unique_ptr<FILE, decltype(&fclose)>(fopen(path.string().c_str(), "rb"), &fclose);

			typedef struct WAV_HEADER {
				/* RIFF Chunk Descriptor */
				uint8_t         RIFF[4];        // RIFF Header Magic header
				uint32_t        ChunkSize;      // RIFF Chunk Size
				uint8_t         WAVE[4];        // WAVE Header
												/* "fmt" sub-chunk */
				uint8_t         fmt[4];         // FMT header
				uint32_t        Subchunk1Size;  // Size of the fmt chunk
				uint16_t        AudioFormat;    // Audio format 1=PCM,6=mulaw,7=alaw,     257=IBM Mu-Law, 258=IBM A-Law, 259=ADPCM
				uint16_t        NumOfChan;      // Number of channels 1=Mono 2=Sterio
				uint32_t        SamplesPerSec;  // Sampling Frequency in Hz
				uint32_t        bytesPerSec;    // bytes per second
				uint16_t        blockAlign;     // 2=16-bit mono, 4=16-bit stereo
				uint16_t        bitsPerSample;  // Number of bits per sample
												/* "data" sub-chunk */
				uint8_t         Subchunk2ID[4]; // "data"  string
				uint32_t        Subchunk2Size;  // Sampled data length
			} wav_hdr;

			wav_hdr wav_header;

			if (fread(&wav_header, 1, sizeof(wav_hdr), wav_file.get()) > 0) {
				if (wav_header.bitsPerSample == 16) {
					channels = wav_header.NumOfChan;
					frequency = wav_header.SamplesPerSec;
					samples.resize(wav_header.Subchunk2Size / sizeof(sound_sample_type));
					fread(samples.data(), sizeof(char), wav_header.Subchunk2Size, wav_file.get());
				}
				else {
					throw sound_decoding_error(
						"%x is a %x-bit WAV. Only supporting 16-bit WAVs.", 
						path, 
						wav_header.bitsPerSample
					);
				}
			}
			else {
				throw sound_decoding_error("Failed to decode %x as WAV file.", path);
			}
			//ensure(wav_header.SamplesPerSec == 44100);
		}

#if LOG_AUDIO_BUFFERS
		LOG("Sound: %x\nSample rate: %x\nChannels: %x\nFormat: %x\nLength in seconds: %x",
			path,
			info.samplerate,
			info.channels,
			info.format,
			compute_length_in_seconds()
		);
#endif
#endif
	}
	
	double sound_data::compute_length_in_seconds() const {
		return static_cast<double>(samples.size()) / (frequency * channels);
	}

	sound_data& sound_data::to_mono() {
		ensure(samples.size() % 2 == 0);

		std::vector<sound_sample_type> output;
		output.resize(samples.size() / 2);

		for (size_t i = 0; i < samples.size(); i += 2) {
			output[i / 2] = (static_cast<int>(samples[i]) + samples[i + 1]) / 2;
		}

		samples = output;
		channels = 1;

		return *this;
	}
}