#pragma once
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wextra-tokens"
#include "IPlug_include_in_plug_hdr.h"
#pragma clang diagnostic pop

#include "IMidiQueue.h"
#include "GallantSignal.h"
using Gallant::Signal2;

class MIDIReceiver {
private:
	IMidiQueue mMidiQueue;
	static const int keyCount = 128;
	int mNumKeys;
	bool mKeyStatus[keyCount];
	int mLastNoteNumber;
	double mLastFrequency;
	int mLastVelocity;
	int mOffset;
	inline double noteNumberToFrequency(int noteNumber) {
		return 440.0 * pow(2.0, (noteNumber - 69.0) / 12.0);
	}
public:
	MIDIReceiver() :
		mNumKeys(0),
		mLastNoteNumber(-1),
		mLastFrequency(-1.0),
		mLastVelocity(0),
		mOffset(0) {
		for (int i = 0; i < keyCount; i++) {
			mKeyStatus[i] = false;
		}
	};

	Signal2< int, int > noteOn;
	Signal2< int, int > noteOff;

	inline bool getKeyStatus(int keyIndex) const { return mKeyStatus[keyIndex]; }
	inline int getNumKeys() const { return mNumKeys; }
	inline int getLastNoteNumber() const { return mLastNoteNumber; }
	inline double getLastFrequency() const { return mLastFrequency; }
	inline int getLastVelocity() const { return mLastVelocity; }
	void advance();
	void onMessageReceived(IMidiMsg* midiMessage);
	inline void Flush(int nFrames) { mMidiQueue.Flush(nFrames); mOffset = 0; }
	inline void Resize(int blockSize) { mMidiQueue.Resize(blockSize); }
};