#include <engine/AL/SoundSource.h>
#include <iostream>

SoundSource::SoundSource()
{
	alGenSources(1, &p_Source);
	alSourcei(p_Source, AL_BUFFER, p_Buffer);
}

SoundSource::~SoundSource()
{
	alDeleteSources(1, &p_Source);
}

void SoundSource::Play(const ALuint& buffer_to_play)
{
	if (buffer_to_play != p_Buffer)
	{
		p_Buffer = buffer_to_play;
		alSourcei(p_Source, AL_BUFFER, (ALint)p_Buffer);
	}

	alSourcePlay(p_Source);
}

void SoundSource::Stop()
{
	alSourceStop(p_Source);
}

void SoundSource::Pause()
{
	alSourcePause(p_Source);
}

void SoundSource::Resume()
{
	alSourcePlay(p_Source);
}

void SoundSource::SetBufferToPlay(const ALuint& buffer_to_play)
{
	if (buffer_to_play != p_Buffer)
	{
		p_Buffer = buffer_to_play;
		alSourcei(p_Source, AL_BUFFER, (ALint)p_Buffer);
	}
}

void SoundSource::SetLooping(const bool& loop)
{
	alSourcei(p_Source, AL_LOOPING, (ALint)loop);
}

bool SoundSource::isPlaying()
{
	ALint playState;
	alGetSourcei(p_Source, AL_SOURCE_STATE, &playState);
	return (playState == AL_PLAYING);
}