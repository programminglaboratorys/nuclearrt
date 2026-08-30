#ifdef NUCLEAR_BACKEND_SDL2

#include "SDL2AudioBackend.h"


#include "Application.h"
#include <iostream>
#include <filesystem>
#include <SDL2/SDL.h>
#include "SoundBank.h"
#include "SDL2Backend.h"
#include "./libs/stb_vorbis.c" // OGG SUPPORT
#define DR_MP3_IMPLEMENTATION
#include "./libs/dr_mp3.h" // MP3 SUPPORT

#ifndef SDL_clamp
#define SDL_clamp(x, a, b) SDL_min(SDL_max((x), (a)), (b))
#endif

//fix for AudioCallback race condition
class AudioStreamLock {
public:
	explicit AudioStreamLock(SDL_AudioDeviceID device) : device(device) {
		Lock();
	}

	~AudioStreamLock() {
		Unlock();
	}

	void Lock() {
		if (device && !locked) {
			SDL_LockAudioDevice(device);
			locked = true;
		}
	}

	void Unlock() {
		if (locked) {
			SDL_UnlockAudioDevice(device);
			locked = false;
		}
	}

	AudioStreamLock(const AudioStreamLock&) = delete;
	AudioStreamLock& operator=(const AudioStreamLock&) = delete;

private:
	SDL_AudioDeviceID device = 0;
	bool locked = false;
};

SDL_AudioDeviceID SDL2AudioBackend::audio_device = NULL;

void SDLCALL SDL2AudioBackend::AudioCallback(void *userdata, Uint8 *stream, int len)
{
	AudioStreamLock lock(audio_device);
	auto& channels = static_cast<SDL2AudioBackend*>(userdata)->channels;
	int frames = len / (sizeof(float) * 2);
	float mixBuffer[8192 * 2] = {0}; // Initilaze array so no garbage data is found
	if (frames > 8192) frames = 8192;
	for (int i = 0; i < frames; ++i) {
		float left = 0.0f, right = 0.0f;
		for (int ch = 1; ch < SDL_arraysize(channels); ++ch) {
			Channel& channel = channels[ch];
			if (!channel.stream || channel.pause) continue;
			float tempData[2] = {0};
			int getData = SDL_AudioStreamGet(channel.stream, tempData, sizeof(tempData));
			if (getData <= 0) { // Channel has finished playing.
				channel.finished = true;
				continue;
			}
			channel.position += getData / (sizeof(float) * 2);
			// Prepare volume + pan handling
			float angle = (channel.pan + 1.0f) * 0.25f * 3.14159265358979323846f;
			float leftGain = SDL_cosf(angle) * channel.volume;
			float rightGain = SDL_sinf(angle) * channel.volume;
			left += tempData[0] * leftGain;
			right += tempData[1] * rightGain;
		}
		left = fmaxf(-1.0f, fminf(left, 1.0f));
		right = fmaxf(-1.0f, fminf(right, 1.0f));

		mixBuffer[i * 2 + 1] = right;
		mixBuffer[i * 2 + 0] = left;
	}
	SDL_memset(stream, 0, len);
	SDL_memcpy(stream, mixBuffer, static_cast<unsigned long long>(frames) * 2 * sizeof(float));  // Using static_cast from what visual studio recommended
}

void SDL2AudioBackend::Initialize()
{
	spec.freq = 44100;
	spec.channels = 2;
	spec.format = AUDIO_F32SYS;
	spec.samples = 1024;
	spec.callback = AudioCallback;
	spec.userdata = this;
	audio_device = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, 0);
	if (!audio_device) {
		backend->GetPlatform()->Log("SDL_OpenAudioDevice Error : " + std::string(SDL_GetError()));
		return;
	}
	SDL_PauseAudioDevice(audio_device, 0);
	backend->GetPlatform()->Log("Opened Audio Device.");
}

void SDL2AudioBackend::Deinitialize()
{	
	if (audio_device) {
		SDL_PauseAudioDevice(audio_device, 1);
	}
	AudioStreamLock lock(audio_device);
	// cleanup audio
	while (!sampleFiles.empty()) DiscardSampleFile(sampleFiles.begin()->first);
	for (int i = 1; i < SDL_arraysize(channels); i++) {
		if (!channels[i].stream) continue;
		if (channels[i].data) {
			SDL_free(channels[i].data);
			channels[i].data = nullptr;
			channels[i].data_len = 0;
		}
		SDL_AudioStreamClear(channels[i].stream);
		SDL_FreeAudioStream(channels[i].stream);
		channels[i].stream = nullptr;
	}

	lock.Unlock();
	if (audio_device) {
		SDL_CloseAudioDevice(audio_device);
		audio_device = 0;
	}
	backend->GetPlatform()->Log("AudioBackend shut down successfully.");
}

bool SDL2AudioBackend::LoadSample(int id, int channel) {
	AudioStreamLock lock(audio_device);

	if (id < 0) return false;
	if (channels[channel].data && channels[channel].curHandle == id) {
		return true;
	}
	SoundInfo* soundInfo = SoundBank::Instance().GetSound(id);
	if (!soundInfo) {
		backend->GetPlatform()->Log("SoundBank Error: Sound ID " + std::to_string(id) + " not found!");
		return false;
	}
	std::vector<uint8_t> data = backend->platform->GetPakFile().GetData("sounds/" + std::to_string(id) + "." + soundInfo->Type);
	if (data.empty()) {
		backend->GetPlatform()->Log("PakFile::GetData Error: Sample with id " + std::to_string(id) + " not found");
		return false;
	}
	if (soundInfo->Type == "wav") {
		SDL_RWops* stream = SDL_RWFromMem(data.data(), static_cast<int>(data.size()));
		if (!SDL_LoadWAV_RW(stream, 1, &channels[channel].spec, &channels[channel].data, &channels[channel].data_len)) {
			backend->GetPlatform()->Log("SDL_LoadWAV_RW Error (WAV) : " + std::string(SDL_GetError()));
			return false;
		}
	}
	else if (soundInfo->Type == "ogg") {
		int channels, samplerate;
		short* output = nullptr;
		int numSamples = stb_vorbis_decode_memory(data.data(), data.size(), &channels, &samplerate, &output);
		if (numSamples <= 0 || !output) {
			backend->GetPlatform()->Log("stb_vorbis_decode_memory failed.");
			return false;
		}
		int totalSamples = numSamples * channels;
		this->channels[channel].data_len = totalSamples * sizeof(short);
		this->channels[channel].data = (Uint8*)SDL_malloc(this->channels[channel].data_len);
		SDL_memcpy(this->channels[channel].data, output, this->channels[channel].data_len);
		this->channels[channel].spec.freq = samplerate;
		this->channels[channel].spec.channels = static_cast<Uint8>(channels);
		this->channels[channel].spec.format = AUDIO_S16SYS;
		free(output);
	}
	else if (soundInfo->Type == "mp3") {
		drmp3 mp3;
		if (!drmp3_init_memory(&mp3, data.data(), data.size(), NULL)) {
			backend->GetPlatform()->Log("Failed to decode mp3 data.");
			drmp3_uninit(&mp3);
			return false;
		}
		drmp3_uint64 frameCount = drmp3_get_pcm_frame_count(&mp3);
		if (frameCount == 0) {
			backend->GetPlatform()->Log("No sample frames in MP3");
			drmp3_uninit(&mp3);
			return false;
		}
		int totalSamples = static_cast<int>(frameCount * mp3.channels);
		Uint32 dataLen = totalSamples * sizeof(int16_t);
		channels[channel].data = (Uint8*)SDL_malloc(dataLen);
		drmp3_uint64 framesRead = drmp3_read_pcm_frames_s16(&mp3, frameCount, (drmp3_int16*)channels[channel].data);
		if (!channels[channel].data) {
			backend->GetPlatform()->Log("Bad MP3 Data");
			drmp3_uninit(&mp3);
			return false;
		}
		(void)framesRead;
		channels[channel].data_len = dataLen;
		channels[channel].spec.channels = static_cast<Uint8>(mp3.channels);
		channels[channel].spec.format = AUDIO_S16SYS;
		channels[channel].spec.freq = mp3.sampleRate;
		drmp3_uninit(&mp3);
	}
	else {
		backend->GetPlatform()->Log("Audio Data Type" + soundInfo->Type + "not supported.");
		return false;
	}
	channels[channel].name = soundInfo->Name;
	return true;
}
bool SDL2AudioBackend::LoadSampleFile(std::string path) {
	AudioStreamLock lock(audio_device);
	backend->GetPlatform()->Log("Loading Sample File : " + path);
	std::filesystem::path fullPath = path;
	std::string type = fullPath.extension().string();
	SampleFile sampleFile;
	if (type == ".wav") {
		if (!SDL_LoadWAV(path.c_str(), &sampleFile.spec, &sampleFile.data, &sampleFile.data_len)) {
			backend->GetPlatform()->Log("Failed to load WAV file : " + std::string(SDL_GetError()));
			return false;
		}
		backend->GetPlatform()->Log("Loaded WAV Sample File : " + path);
	}
	else if (type == ".ogg") {
		int channels, samplerate;
		short* output = nullptr;
		int numSamples = stb_vorbis_decode_filename(path.c_str(), &channels, &samplerate, &output);
		if (numSamples <= 0 || !output) {
			backend->GetPlatform()->Log("Failed to load OGG file : " + path);
			return false;
		}
		int totalSamples = numSamples * channels;
		sampleFile.data_len = totalSamples * sizeof(short);
		sampleFile.data = (Uint8*)SDL_malloc(sampleFile.data_len);
		SDL_memcpy(sampleFile.data, output, sampleFile.data_len);
		sampleFile.spec.freq = samplerate;
		sampleFile.spec.format = AUDIO_S16SYS;
		sampleFile.spec.channels = static_cast<Uint8>(channels);
		free(output);
		backend->GetPlatform()->Log("Loaded OGG file : " + path);
	}
	else if (type == ".mp3") {
		drmp3 mp3;
		if (!drmp3_init_file(&mp3, path.c_str(), NULL)) {
			backend->platform->Log("Couldn't find MP3 file");
			return false;
		}
		drmp3_uint64 frameCount = drmp3_get_pcm_frame_count(&mp3);
		if (frameCount == 0) {
			backend->GetPlatform()->Log("No sample frames in MP3");
			drmp3_uninit(&mp3);
			return false;
		}
		int totalSamples = static_cast<int>(frameCount * mp3.channels);
		Uint32 dataLen = totalSamples * sizeof(int16_t);
		sampleFile.data = (Uint8*)SDL_malloc(dataLen);
		drmp3_uint64 framesRead = drmp3_read_pcm_frames_s16(&mp3, frameCount, (drmp3_int16*)sampleFile.data);
		if (!sampleFile.data) {
			backend->platform->Log("Bad MP3 Data.");
			drmp3_uninit(&mp3);
			return false;
		}
		(void)framesRead;
		sampleFile.data_len = dataLen;
		sampleFile.spec.channels = static_cast<Uint8>(mp3.channels);
		sampleFile.spec.format = AUDIO_S16SYS;
		sampleFile.spec.freq = mp3.sampleRate;
		drmp3_uninit(&mp3);
	}
	else {
		backend->GetPlatform()->Log("Audio File" + type + "not supported.");
		return false;
	}
	sampleFile.pathName = path;
	sampleFiles.emplace(sampleFile.pathName, sampleFile);
	return true;
}
int SDL2AudioBackend::FindSample(std::string name) {
	SoundInfo* soundInfo = SoundBank::Instance().GetSoundName(name);
	if (soundInfo) {
		return soundInfo->Handle;
	}
	return -1;
}

void SDL2AudioBackend::PlaySample(int id, int channel, int loops, int freq, bool uninterruptable, float volume, float pan) {
	AudioStreamLock lock(audio_device);
	if (channel < 1 || channel >= SDL_arraysize(channels)) {
		bool channelFound = false;
		for (int i = 1; i < SDL_arraysize(channels); i++) {
			if (!channels[i].stream && !channels[i].data && !channels[i].lock) {
				channel = i;
				channelFound = true;
				break;
			}
		}

		if (!channelFound) {
			channel = 48;
		}
	}
	else { // Channel is given.
		if (channels[channel].stream && channels[channel].data) // a sound is playing
		{
			if (channels[channel].uninterruptable && !uninterruptable) {
				//backend->GetPlatform()->Log("Channel " + std::to_string(channel) + " is uninterruptable. Can't play sample.");
				return;
			}
		}
	}
	
	//clear out channel
	StopSample(channel, true);
	
	if (!LoadSample(id, channel)) return;

	channels[channel].stream = SDL_NewAudioStream(channels[channel].spec.format, channels[channel].spec.channels, channels[channel].spec.freq, spec.format, spec.channels, spec.freq);
	if (!channels[channel].stream) {
		backend->GetPlatform()->Log("SDL_NewAudioStream error : " + std::string(SDL_GetError()));
		channels[channel].stream = nullptr;
		return;
	}
	channels[channel].loop = (loops <= 0);
	channels[channel].position = 0;
	channels[channel].pause = false;
	channels[channel].uninterruptable = uninterruptable;
	if (channels[channel].loop) SDL_AudioStreamPut(channels[channel].stream, channels[channel].data, channels[channel].data_len);
	else {
		for (int i = 1; i <= loops; i++) {
			SDL_AudioStreamPut(channels[channel].stream, channels[channel].data, channels[channel].data_len);
		}
	}
	if (volume > -1) channels[channel].volume = volume;
	if (pan != -2 ) channels[channel].pan = pan;
	channels[channel].curHandle = id;
	SetSampleVolume(mainVol, channel, true); // Set volume to the main one.
	
	//backend->GetPlatform()->Log("Sample ID " + std::to_string(id) + " is now playing at channel " + std::to_string(channel) + ".");
}

void SDL2AudioBackend::PlaySampleFile(std::string path, int channel, int loops) {
	AudioStreamLock lock(audio_device);
	auto it = sampleFiles.find(path);
	if (it == sampleFiles.end()) {
		backend->GetPlatform()->Log("Can't find sample path.");
		return;
	}
	SampleFile& sampleFile = it->second;
	if (channels[channel].stream || channels[channel].data || channels[channel].lock || channels[channel].uninterruptable) return;
	StopSample(channel, true);
	channels[channel].data = (Uint8*)SDL_malloc(sampleFile.data_len);
	SDL_memcpy(channels[channel].data, sampleFile.data, sampleFile.data_len);
	channels[channel].data_len = sampleFile.data_len;
	channels[channel].spec = sampleFile.spec;

	channels[channel].stream = SDL_NewAudioStream(channels[channel].spec.format, channels[channel].spec.channels, channels[channel].spec.freq, spec.format, spec.channels, spec.freq);
	if (!channels[channel].stream) {
		backend->GetPlatform()->Log("SDL_NewAudioStream error : " + std::string(SDL_GetError()));
		channels[channel].stream = nullptr;
		return;
	}
	channels[channel].loop = (loops <= 0);
	channels[channel].position = 0;
	channels[channel].pause = false;
	channels[channel].name = sampleFile.pathName;
	channels[channel].uninterruptable = false;
	if (channels[channel].loop) SDL_AudioStreamPut(channels[channel].stream, channels[channel].data, channels[channel].data_len);
	else {
		for (int i = 1; i <= loops; i++) {
			SDL_AudioStreamPut(channels[channel].stream, channels[channel].data, channels[channel].data_len);
		}
	}
	SetSampleVolume(mainVol, channel, true);
	DiscardSampleFile(path);
}

void SDL2AudioBackend::DiscardSampleFile(std::string path) {
	AudioStreamLock lock(audio_device);
	auto it = sampleFiles.find(path);
	if (it == sampleFiles.end()) {
		backend->GetPlatform()->Log("Can't find sample path.");
		return;
	}
	SampleFile& sampleFile = it->second;
	if (sampleFile.data) {
		SDL_free(sampleFile.data);
		sampleFile.data = nullptr;
	}
	sampleFiles.erase(it);
}
// ALL SAMPLE CONDITIONS HERE

bool SDL2AudioBackend::SampleState(int id, bool channel, bool pause) {
	AudioStreamLock lock(audio_device);
	if (id == -1 && !channel && !pause) { // No Sample is playing
		int countStream = 0;
		for (int i = 1; i < SDL_arraysize(channels); i++) {
			if (channels[i].stream) countStream++;
		}
		if (countStream == 0) return true;
		else return false;
	}
	if (channel) { // Check if channel is not playing/paused
		if (id < 1 || id >= SDL_arraysize(channels)) return false;
		if (pause && channels[id].pause) return true;
		if (!channels[id].stream && !pause) return true;
	}
	if (id > -1 && !channel) { // Check for specific sample not playing/paused.
		for (int i = 1; i < SDL_arraysize(channels); i++) {
			//TODO: come back to this to verify paused state
			if (channels[i].curHandle == id) {
				if (pause == channels[i].pause) {
					return true;
				}
				if (channels[i].stream && !pause) return true;
			}
		}
	}
	return false;
}
void SDL2AudioBackend::PauseSample(int id, bool channel, bool pause) {
	AudioStreamLock lock(audio_device);
	if (id == -1 && !channel) { // Pause/Resume all sounds
		for (int i = 1; i < SDL_arraysize(channels); i++) {
			PauseSample(i, true, pause);
		}
	}
	if (channel) { // Pause/Resume specific channel
		if (id < 1 || id >= SDL_arraysize(channels)) return;
		if (channels[id].stream) {
			channels[id].pause = pause;
		}
		return;
	}
	if (id > -1 && !channel) { // Pause/Resume sample handle.
		for (int i = 1; i < SDL_arraysize(channels); i++) {
			if (channels[i].curHandle == id) PauseSample(i, true, pause);
		}
	}
}
void SDL2AudioBackend::SetSamplePan(float pan, int id, bool channel) {
	AudioStreamLock lock(audio_device);
	bool setMain = false;
	pan = pan / 100;
	if (pan < -1.0f) pan = -1.0f;
	if (pan > 1.0f) pan = 1.0f;
	if (!channel && id <= -1) { // Set Main Pan
		setMain = true;
		mainPan = pan;
		for (int i = 1; i < SDL_arraysize(channels); ++i) {
			channels[i].pan = channels[i].pan + mainPan;
		}
	}
	if (channel) { // Set Channel Pan
		setMain = false;
		if (id < 1 || id >= SDL_arraysize(channels)) return;
		if (channels[id].stream) channels[id].pan = pan;
	}
	if (id > -1 && !channel) { // Set Sample Pan
		setMain = false;
		for (int i = 1; i < SDL_arraysize(channels); i++) {
			if (channels[i].curHandle == id) channels[i].pan = pan;
			else continue;
		}
	}
}
CValue SDL2AudioBackend::GetSamplePan(int id, bool channel) {
	AudioStreamLock lock(audio_device);
	if (id == -1 && !channel) return CValue(mainPan);
	if (channel) { // Get Channel Volume
		if (id < 1 || id >= SDL_arraysize(channels)) return CValue(0);
		return CValue(channels[id].pan * 100);
	}
	if (!channel && id >= 0) {
		for (int i = 1; i < SDL_arraysize(channels); i++) {
			if (channels[i].curHandle == id) return CValue(channels[i].pan * 100);
		}
	}
	return CValue(0);
}
void SDL2AudioBackend::SetSamplePos(int pos, int id, bool channel)
{
	AudioStreamLock lock(audio_device);
	if (channel) {
		if (id < 0 || id >= SDL_arraysize(channels)) return;
		if (!channels[id].data || !channels[id].stream) return;
		int length = channels[id].data_len / (sizeof(float) * 2);
		pos = SDL_clamp(pos, 0, length);
		channels[id].position = pos;
		SDL_AudioStreamClear(channels[id].stream);
		Uint8* positionData = channels[id].data + static_cast<int>(pos) * sizeof(float) * 2;
		Uint32 positionLength = channels[id].data_len - pos * sizeof(float) * 2;
		SDL_AudioStreamPut(channels[id].stream, positionData, positionLength);
	}
	else {
		if (id < 0) return;
		for (int i = 0; i < SDL_arraysize(channels); ++i) {
			if (channels[i].curHandle == id) SetSamplePos(pos, i, true);
		}
	}
}
void SDL2AudioBackend::SetSampleVolume(float volume, int id, bool channel) {
	AudioStreamLock lock(audio_device);
	bool setMain = false;
	if (id == -1 && !channel) { // Set Main Volume
		mainVol = volume;
		setMain = true;
		for (int i = 1; i < SDL_arraysize(channels); i++) {
			SetSampleVolume(mainVol, i, true);
		}
	}
	if (channel) { // Set Channel Volume
		if (id < 1 || id >= SDL_arraysize(channels)) return;
		if (channels[id].stream) {
			channels[id].volume = volume / 100;
			if (!setMain) channels[id].volume = volume / 100;
			else {
				mainVol = (volume / 100) * channels[id].volume;
				channels[id].volume = mainVol;
			}	
		}
	}
	if (id > -1 && !channel) { // Set Sample Volume
		for (int i = 1; i < SDL_arraysize(channels); i++) {
			if (channels[i].curHandle == id) SetSampleVolume(volume, i, true);
			else continue;
		}
	}
}

CValue SDL2AudioBackend::GetSampleVolume(CValue id) {
	AudioStreamLock lock(audio_device);
	if (id.GetIntValue() == -1) return CValue(mainVol);
	if (id.GetIntValue() > -1) { // Get Sample Volume
		for (int i = 1; i < SDL_arraysize(channels); i++) {
			if (channels[i].curHandle == id.GetIntValue() && channels[i].stream) return CValue(channels[i].volume);
		}
	}
	return CValue(0);
}

CValue SDL2AudioBackend::GetSampleVolume(std::string name) {
	return GetSampleVolume(FindSample(name));
}

CValue SDL2AudioBackend::GetChannelVolume(CValue id) {
	AudioStreamLock lock(audio_device);
	if (id.GetIntValue() < 1 || id.GetIntValue() >= SDL_arraysize(channels)) return CValue(-1);
	return CValue(channels[id.GetIntValue()].volume);
}

CValue SDL2AudioBackend::GetChannelName(CValue channel) {
	AudioStreamLock lock(audio_device);
	if (channel.GetIntValue() < 0 || channel.GetIntValue() >= SDL_arraysize(channels)) return CValue("");
	return CValue(channels[channel.GetIntValue()].name);
}

void SDL2AudioBackend::LockChannel(int channel, bool unlock) {
	AudioStreamLock lock(audio_device);
	if (channel < 0 || channel >= SDL_arraysize(channels)) return;
	channels[channel].lock = !unlock;
}

CValue SDL2AudioBackend::GetSampleDuration(int id, bool channel) {
	AudioStreamLock lock(audio_device);
	if (channel && id > 0 && id < SDL_arraysize(channels)) return CValue(static_cast<int>(channels[id].data_len));
	if (!channel && id > -1) {
		for (int i = 1; i < SDL_arraysize(channels); ++i) if (channels[i].curHandle == id) return CValue(static_cast<int>(channels[i].data_len));
	}
	return CValue(0);
}

CValue SDL2AudioBackend::GetSamplePos(int id, bool channel) {
	AudioStreamLock lock(audio_device);
	if (channel && id > 0 && id < SDL_arraysize(channels)) return CValue(channels[id].position);
	if (!channel && id > -1) {
		for (int i = 1; i < SDL_arraysize(channels); ++i) if (channels[i].curHandle == id) return CValue(channels[i].position);
	}
	return CValue(0);
}

void SDL2AudioBackend::StopSample(int id, bool channel) {
	AudioStreamLock lock(audio_device);
	if (id == -1) { // Stop any sample
		for (int i = 1; i < SDL_arraysize(channels); i++) {
			StopSample(i, true);
		}
		return;
	}
	if (channel) { // check for the channel
		if (id < 1 || id >= SDL_arraysize(channels)) return;
		if (channels[id].stream) {
			SDL_FreeAudioStream(channels[id].stream);
			channels[id].stream = nullptr;
			if (channels[id].data) {
				SDL_free(channels[id].data);
			}
			channels[id].data = nullptr;
			channels[id].data_len = 0;
		}
		channels[id].name = "";
		channels[id].curHandle = -1;
		channels[id].uninterruptable = false;
		channels[id].position = 0;
		return;
	}
	if (!channel && id > -1) { // check for sample handle
		for (int i = 1; i < SDL_arraysize(channels); i++) {
			if (channels[i].curHandle == id) StopSample(i, true);
		}
	}
}
void SDL2AudioBackend::UpdateSample() {
	// desktop uses SDL3 so this is not needed here
	// 	if (!Application::Instance().GetAppData()->GetSampleFocus()) {
	// 	const bool focused = backend->platform && backend->GetPlatform()->IsWindowFocused();
	// 	if (!sampleFocusGainApplied || focused != lastWindowFocused) {
	// 		SDL_SetAudioDeviceGain(audio_device, focused ? 1.0f : 0.0f);
	// 		lastWindowFocused = focused;
	// 		sampleFocusGainApplied = true;
	// 	}
	// }
	// else {
	// 	sampleFocusGainApplied = false;
	// }
	AudioStreamLock lock(audio_device);
	for (int i = 1; i < SDL_arraysize(channels); ++i) {
		channels[i].volume = SDL_clamp(channels[i].volume, 0, 1); // Clamp Volume
		if (channels[i].stream) {
			if (channels[i].finished) {
				channels[i].finished = false;
				if (!channels[i].loop) {
					StopSample(i, true);
					continue;
				}
				else SDL_AudioStreamPut(channels[i].stream, channels[i].data, channels[i].data_len);
			}
		}
		else continue;
	}
}
#endif
