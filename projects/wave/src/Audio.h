#pragma once

#include <vector>
#include <memory>
#include <mutex>

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

	template<int t_Channels>
	class AudioFrame final
	{
	public:
		AudioFrame()
		{
			for (int i = 0; i < t_Channels; ++i)
				m_Samples[i] = 0.0f;
		}

		float m_Samples[t_Channels];

		void operator+= (AudioFrame<t_Channels>& other)
		{
			for (int i = 0; i < t_Channels; ++i)
				m_Samples[i] += other.m_Samples[i];
		}
	};

	class AudioSource final
	{
	public:
		void QueueBuffer(std::shared_ptr<AudioBuffer> buffer);
		bool IsComplete();

		template<int t_Channels>
		void NextFrame(AudioFrame<t_Channels>& frame)
		{
			if (m_Queue.empty())
			{
				for (int i = 0; i < t_Channels; ++i)
					frame.m_Samples[i] = 0.0f;

				return;
			}

			auto buffer = m_Queue[0];

			{
				if (t_Channels == buffer->m_Channels)
				{
					for (int i = 0; i < t_Channels; ++i)
						frame.m_Samples[i] = buffer->NextSample();
				}
				else if(t_Channels == 2 && buffer->m_Channels == 1)
				{
					for (int i = 0; i < buffer->m_Channels; ++i)
					{
						float sample = buffer->NextSample();;

						frame.m_Samples[i] = sample;
						frame.m_Samples[i + 1] = sample;
					}
				}
			}
			

			if (buffer->IsComplete()) m_Queue.erase(m_Queue.begin());

		}


		std::vector<std::shared_ptr<AudioBuffer>> m_Queue;
	};

	class AudioOutput final
	{
	public:
		AudioOutput(int channels, int sampleRate);
		~AudioOutput();

		void AddSource(std::shared_ptr<AudioSource> buffer);
		inline bool IsReady() const { return m_Success; }

		static int OnStreamUpdate(const void* inputStream, void* outputStream, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData);

		float m_Volume = 1.0f;

		int m_Channels;
		int m_SampleRate;
		bool m_Success;
		PaStream* m_Stream;
		std::vector<std::shared_ptr<AudioSource>> m_Sources;

	private:
		std::mutex mutex;
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