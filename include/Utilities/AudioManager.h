#pragma once
#include <string>
#include <map>
#include "allegro.h"
#include "Core/GameDefines.h"

class AudioManager
{
  	public:
		static AudioManager &Instance()
		{
			static AudioManager instance;
			return instance;
		}

		void PlaySFX(const std::string &name) { PlaySFX(name, AUDIO_MAX_VOLUME, AUDIO_PAN_CENTER); }
		void PlaySFX(const std::string &name, int volume, int pan);
		bool IsSFXPlaying(const std::string &name);

		void PlayMusic(const std::string &name, bool loop = true);
		void StopMusic();
		bool IsMusicPlaying() const { return currentMidi != nullptr; }
		std::string GetCurrentTrack() const { return currentTrackName; }

		void SetMusicVolume(int volume);
		void SetSFXVolume(int volume);
		int GetMusicVolume() const { return musicVolume; }
		int GetSFXVolume() const { return sfxVolume; }

		void UnloadAll();

  	private:
		AudioManager() { }
		~AudioManager() { UnloadAll(); }
		AudioManager(const AudioManager &) = delete;
		AudioManager &operator=(const AudioManager &) = delete;
		SAMPLE *LoadOrGetSample(const std::string &name);

  	private:
		std::map<std::string, SAMPLE *> sampleCache;
		std::map<std::string, int> activeVoices;
		MIDI *currentMidi = nullptr;
		std::string currentTrackName;
		int musicVolume = AUDIO_MAX_VOLUME;
		int sfxVolume = AUDIO_MAX_VOLUME;
};