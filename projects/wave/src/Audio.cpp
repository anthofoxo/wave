#include "Audio.h"

#include "Log.h"


#define STB_VORBIS_HEADER_ONLY
#include "vendor/stb_vorbis.c"

namespace AF
{
	AudioBuffer::AudioBuffer(size_t frames, size_t channels)
	{
		m_Buffer = new float[frames * channels];
		m_BufferSize = frames * channels;
		m_Position = 0;
		m_Limit = m_BufferSize;
		m_Frames = frames;
		m_Channels = channels;
	}

	AudioBuffer::~AudioBuffer()
	{
		delete[] m_Buffer;
		m_Buffer = nullptr;
	}

	bool AudioBuffer::IsComplete()
	{
		return m_Position >= m_Limit;
	}

	float AudioBuffer::NextSample()
	{
		if (IsComplete()) return 0;

		return m_Buffer[m_Position++];
	}

	AudioOutput::AudioOutput(int channels, int sampleRate)
	{
		m_Channels = channels;
		m_SampleRate = sampleRate;
		
		PaError err = Pa_OpenDefaultStream(&m_Stream, 0, channels, paFloat32, sampleRate, paFramesPerBufferUnspecified, OnStreamUpdate, this);
		AF_ASSERT(err == paNoError, "PortAudio error: {}", Pa_GetErrorText(err));
		if (err != paNoError)
		{
			m_Success = false;
			return;
		}

		err = Pa_StartStream(m_Stream);
		AF_ASSERT(err == paNoError, "PortAudio error: {}", Pa_GetErrorText(err));
		if (err != paNoError)
		{
			m_Success = false;
			return;
		}

		m_Success = true;
	}

	AudioOutput::~AudioOutput()
	{
		PaError err = Pa_StopStream(m_Stream);
		AF_ASSERT(err == paNoError, "PortAudio error: {}", Pa_GetErrorText(err));

		err = Pa_CloseStream(m_Stream);
		AF_ASSERT(err == paNoError, "PortAudio error: {}", Pa_GetErrorText(err));
	}

	void AudioOutput::QueueBuffer(std::shared_ptr<AudioBuffer> buffer)
	{
		m_Queue.push_back(buffer);
	}

	int AudioOutput::OnStreamUpdate(const void* inputStream, void* outputStream, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
	{
		float* output = (float*) outputStream;
		AudioOutput* owner = (AudioOutput*) userData;

		for (unsigned long i = 0; i < frameCount; ++i)
		{
			if (owner->m_Queue.empty())
			{
				for (int j = 0; j < owner->m_Channels; ++j)
					*output++ = 0.0f;
			}
			else
			{
				auto buffer = owner->m_Queue[0];

				for (int j = 0; j < owner->m_Channels; ++j)
					*output++ = buffer->NextSample() * owner->m_Volume;

				if (buffer->IsComplete())
				{
					owner->m_Queue.erase(owner->m_Queue.begin());
				}
			}
		}
		
		return 0;
	}

	AudioMaster::AudioMaster()
	{
		if (s_LibraryReady) return;

		AF_DEBUG("Initializing PortAudio");
		PaError err = Pa_Initialize();
		AF_ASSERT(err == paNoError, "PortAudio error: {}", Pa_GetErrorText(err));

		if (err == paNoError)
			s_LibraryReady = true;
	}

	AudioMaster::~AudioMaster()
	{
		if (!s_LibraryReady) return;

		AF_DEBUG("Destroying PortAudio");
		PaError err = Pa_Terminate();
		AF_ASSERT(err == paNoError, "PortAudio error: {}", Pa_GetErrorText(err));

		if (err == paNoError)
			s_LibraryReady = false;
	}

	std::shared_ptr<AudioOutput> AudioMaster::CreateAudioOutput(int channels, int sampleRate)
	{
		auto output = std::make_shared<AudioOutput>(channels, sampleRate);
		m_Outputs.push_back(output);
		return output;
	}

	void AudioMaster::DeleteAudioOutput(std::shared_ptr<AudioOutput> output)
	{
		m_Outputs.erase(std::remove(m_Outputs.begin(), m_Outputs.end(), output));
	}

	bool AudioMaster::s_LibraryReady = false;
	

	void Test()
	{
		AudioMaster master = AudioMaster();

		if (master.IsReady())
		{
			auto output = master.CreateAudioOutput(2, AF_STANDARD_SAMPLE_RATE);
			if (output->IsReady())
			{

			}
		}
	}
}