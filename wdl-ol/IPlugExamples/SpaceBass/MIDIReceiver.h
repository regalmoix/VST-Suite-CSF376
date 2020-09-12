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
	int mOffset;
	
public:
	MIDIReceiver() :
		mNumKeys(0),
		mOffset(0) {
		for (int i = 0; i < keyCount; i++) {
			mKeyStatus[i] = false;
		}
	};

	Signal2< int, int > noteOn;
	Signal2< int, int > noteOff;

	inline bool getKeyStatus(int keyIndex) const { return mKeyStatus[keyIndex]; }
	inline int getNumKeys() const { return mNumKeys; }
	void advance();
	void onMessageReceived(IMidiMsg* midiMessage);
	inline void Flush(int nFrames) { mMidiQueue.Flush(nFrames); mOffset = 0; }
	inline void Resize(int blockSize) { mMidiQueue.Resize(blockSize); }
};