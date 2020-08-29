#pragma once
#include "Voice.h"

class VoiceManager {
public:
	void onNoteOn(int noteNumber, int velocity);
	void onNoteOff(int noteNumber, int velocity);
	double nextSample();
	void setSampleRate(double sampleRate) {
		EnvelopeGenerator::setSampleRate(sampleRate);
		for (int i = 0; i < NumberOfVoices; i++) {
			Voice& voice = voices[i];
			voice.mOscillatorOne.setSampleRate(sampleRate);
			voice.mOscillatorTwo.setSampleRate(sampleRate);
		}
		mLFO.setSampleRate(sampleRate);
	}
private:
	static const int NumberOfVoices = 64;
	Voice voices[NumberOfVoices];
	Oscillator mLFO;
	Voice* findFreeVoice();
};