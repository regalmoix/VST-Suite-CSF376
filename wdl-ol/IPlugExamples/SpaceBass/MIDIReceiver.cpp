#include "MIDIReceiver.h"

void MIDIReceiver::onMessageReceived(IMidiMsg* midiMessage) {
	IMidiMsg::EStatusMsg status = midiMessage->StatusMsg();
	if (status == IMidiMsg::kNoteOn || status == IMidiMsg::kNoteOff) {
		mMidiQueue.Add(midiMessage);
	}
}

void MIDIReceiver::advance() {
	while (!mMidiQueue.Empty()) {
		IMidiMsg* midiMessage = mMidiQueue.Peek();
		if (midiMessage->mOffset > mOffset) break;
		IMidiMsg::EStatusMsg status = midiMessage->StatusMsg();
		int noteNumber = midiMessage->NoteNumber();
		int velocity = midiMessage->Velocity();
		if (status == IMidiMsg::kNoteOn && velocity) {
			if (mKeyStatus[noteNumber] == false) {
				mKeyStatus[noteNumber] = true;
				mNumKeys += 1;
				noteOn(noteNumber, velocity);
			}
		}
		else {
			if (mKeyStatus[noteNumber] == true) {
				mKeyStatus[noteNumber] = false;
				mNumKeys -= 1;
				noteOff(noteNumber, velocity);
			}
		}
		mMidiQueue.Remove();
	}
	mOffset++;
}