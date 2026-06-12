#include "Utilities/AudioManager.h"

SAMPLE* AudioManager::LoadOrGetSample(const std::string &name)
{
	auto it = sampleCache.find(name);
	if (it != sampleCache.end())
		return it->second;

	std::string path = ".\\SFX\\" + name + ".WAV";
	SAMPLE *sample = load_sample(path.c_str());
	if (sample)
		sampleCache[name] = sample;
	return sample;
}

void AudioManager::PlaySFX(const std::string &name, int volume, int pan)
{
	SAMPLE *sample = LoadOrGetSample(name);
	if (sample)
	{
		int scaledVolume = (volume * sfxVolume) / 255;
		play_sample(sample, scaledVolume, pan, 1000, FALSE);
	}
}

void AudioManager::PlayMusic(const std::string &name, bool loop)
{
	if (name == currentTrackName && currentMidi)
		return;

	StopMusic();

	std::string path = ".\\MUSIC\\" + name + ".MID";
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
	musicVolume = (volume < 0) ? 0 : (volume > 255 ? 255 : volume);
	set_volume(-1, musicVolume);
}

void AudioManager::SetSFXVolume(int volume)
{
	sfxVolume = (volume < 0) ? 0 : (volume > 255 ? 255 : volume);
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