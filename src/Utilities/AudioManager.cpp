#include "Utilities/AudioManager.h"
#include "Core/AssetPaths.h"
#include "Core/GameDefines.h"

SAMPLE* AudioManager::LoadOrGetSample(const std::string &name)
{
	auto it = sampleCache.find(name);
	if (it != sampleCache.end())
		return it->second;

	std::string path = std::string(AssetPaths::DIR_SFX) + name + ".WAV";
	SAMPLE *sample = load_sample(path.c_str());
	if (sample)
		sampleCache[name] = sample;
	return sample;
}

bool AudioManager::IsSFXPlaying(const std::string &name)
{
	auto it = activeVoices.find(name);
	if (it == activeVoices.end())
		return false;

	int voice = it->second;
	if (voice < 0)
		return false;

	if (voice_get_position(voice) == -1)
		return false;

	return true;
}

void AudioManager::PlaySFX(const std::string &name, int volume, int pan)
{
	if (IsSFXPlaying(name))
		return;

	SAMPLE *sample = LoadOrGetSample(name);
	if (sample)
	{
		int scaledVolume = (volume * sfxVolume) / AUDIO_MAX_VOLUME;
		int voice = play_sample(sample, scaledVolume, pan, AUDIO_DEFAULT_PITCH, FALSE);
		if (voice >= 0)
		{
			for (auto it = activeVoices.begin(); it != activeVoices.end(); )
			{
				if (it->second == voice)
					it = activeVoices.erase(it);
				else
					it++;
			}
			activeVoices[name] = voice;
		}
	}
}

void AudioManager::PlayMusic(const std::string &name, bool loop)
{
	if (name == currentTrackName && currentMidi)
		return;

	StopMusic();

	std::string path = std::string(AssetPaths::DIR_MUSIC) + name + ".MID";
	currentMidi = load_midi(path.c_str());
	if (currentMidi)
	{
		currentTrackName = name;
		play_midi(currentMidi, loop ? TRUE : FALSE);
		set_volume(-1, musicVolume);
	}
}

void AudioManager::StopMusic()
{
	if (currentMidi)
	{
		stop_midi();
		destroy_midi(currentMidi);
		currentMidi = nullptr;
		currentTrackName.clear();
	}
}

void AudioManager::SetMusicVolume(int volume)
{
	musicVolume = (volume < 0) ? 0 : (volume > AUDIO_MAX_VOLUME ? AUDIO_MAX_VOLUME : volume);
	set_volume(-1, musicVolume);
}

void AudioManager::SetSFXVolume(int volume)
{
	sfxVolume = (volume < 0) ? 0 : (volume > AUDIO_MAX_VOLUME ? AUDIO_MAX_VOLUME : volume);
}

void AudioManager::UnloadAll()
{
	StopMusic();
	for (auto const &[name, sample] : sampleCache)
	{
		if (sample)
			destroy_sample(sample);
	}
	sampleCache.clear();
}