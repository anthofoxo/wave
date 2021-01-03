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

	void AudioSource::QueueBuffer(std::shared_ptr<AudioBuffer> buffer)
	{
		m_Queue.push_back(buffer);
	}

	bool AudioSource::IsComplete()
	{
		return m_Queue.empty();
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

	int AudioOutput::OnStreamUpdate(const void* inputStream, void* outputStream, unsigned long frameCount, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
	{
		

		float* output = (float*) outputStream;
		AudioOutput* owner = (AudioOutput*) userData;

		owner->mutex.lock();

		for (unsigned long i = 0; i < frameCount; ++i)
		{
			AudioFrame<2> frame;

			for (int j = owner->m_Sources.size() - 1; j >= 0; --j)
			{
				auto source = owner->m_Sources[j];

				AudioFrame<2> sourceFrame;
				source->NextFrame(sourceFrame);
				frame += sourceFrame;

				if (source->IsComplete())
					owner->m_Sources.erase(owner->m_Sources.begin() + j);
			}

			for (int ci = 0; ci < 2; ++ci)
			{
				*output++ = frame.m_Samples[ci] * owner->m_Volume;
			}
		}
		
		owner->mutex.unlock();

		return 0;
	}

	void AudioOutput::AddSource(std::shared_ptr<AudioSource> buffer)
	{
		mutex.lock();

		m_Sources.push_back(buffer);

		mutex.unlock();
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