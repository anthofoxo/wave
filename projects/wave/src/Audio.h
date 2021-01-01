#pragma once

#include <vector>
#include <memory>

#include <portaudio.h>

#define AF_STANDARD_SAMPLE_RATE 44100

namespace AF
{
	class AudioBuffer final
	{
	public:
		AudioBuffer(size_t frames, size_t channels);
		~AudioBuffer();

		bool IsComplete();
		float NextSample();

		float* m_Buffer;
		size_t m_BufferSize;

		size_t m_Position;
		size_t m_Limit;

		size_t m_Frames;
		size_t m_Channels;
	};

	class AudioOutput final
	{
	public:
		AudioOutput(int channels, int sampleRate);
		~AudioOutput();

		void QueueBuffer(std::shared_ptr<AudioBuffer> buffer);
		inline bool IsReady() const { return m_Success; }

		static int OnStreamUpdate(const void* inputStream, void* outputStream, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData);

		float m_Volume = 1.0f;

		int m_Channels;
		int m_SampleRate;
		bool m_Success;
		PaStream* m_Stream;
		std::vector<std::shared_ptr<AudioBuffer>> m_Queue;
	};

	class AudioMaster final
	{
	public:
		AudioMaster();
		~AudioMaster();

		std::shared_ptr<AudioOutput> CreateAudioOutput(int channels, int sampleRate);
		void DeleteAudioOutput(std::shared_ptr<AudioOutput> output);

		inline bool IsReady() const { return s_LibraryReady; }
	private:
		static bool s_LibraryReady;
		std::vector<std::shared_ptr<AudioOutput>> m_Outputs;
	};
}