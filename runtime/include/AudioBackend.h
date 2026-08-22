#pragma once

#include <string>
#include "CValue.h"

class AudioBackend {
public:
	virtual void Initialize() {}
	virtual void Deinitialize() {}

	virtual bool LoadSample(int id, int channel) {return false;}
	virtual bool LoadSampleFile(std::string path) {return false;}
	virtual int FindSample(std::string name) {return -1;}
	virtual void PlaySample(int id, int channel, int loops, int freq, bool uninterruptable, float volume, float pan) {}
	virtual void PlaySampleFile(std::string path, int channel, int loops) {}
	virtual void DiscardSampleFile(std::string path) {}
	virtual void StopSample(int id, bool channel) {}
	virtual void PauseSample(int id, bool channel, bool pause) {}
	virtual void SetSampleVolume(float volume, int id, bool channel) {}
	virtual CValue GetSampleVolume(CValue id) {return CValue(0);}
	virtual CValue GetSampleVolume(std::string name) {return 0;}
	virtual CValue GetChannelVolume(CValue id) {return CValue(0);}
	virtual CValue GetChannelName(CValue channel) {return CValue("");}
	virtual void LockChannel(int channel, bool unlock) {}
	virtual void SetSamplePan(float pan, int id, bool channel) {}
	virtual CValue GetSamplePan(int id, bool channel) {return 0;}
	virtual CValue GetSampleFreq(int id, bool channel) {return 0;}
	virtual void SetSampleFreq(int freq, int id, bool channel) {}
	virtual CValue GetSampleDuration(int id, bool channel) {return 0;}
	virtual CValue GetSamplePos(int id, bool channel) {return 0;}
	virtual void SetSamplePos(int pos, int id, bool channel) {}
	virtual void UpdateSample() {}
	virtual bool SampleState(int id, bool channel, bool pauseOrStop) {return false;}
}; 