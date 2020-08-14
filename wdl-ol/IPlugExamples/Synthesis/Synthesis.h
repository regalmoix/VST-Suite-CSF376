#ifndef __SYNTHESIS__
#define __SYNTHESIS__

#include "IPlug_include_in_plug_hdr.h"
#include "Oscillator.h"
#include "MIDIReceiver.h"
#include "EnvelopeGenerator.h"
#include "Filter.h"

class Synthesis : public IPlug
{
public:
  Synthesis(IPlugInstanceInfo instanceInfo);
  ~Synthesis();

  void Reset();
  void OnParamChange(int paramIdx);
  void ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames);
  void ProcessMidiMsg(IMidiMsg* pMsg);
  inline int GetNumKeys() const { return mMidiReceiver.getNumKeys(); };
  inline bool GetKeyStatus(int key) const { return mMidiReceiver.getKeyStatus(key); };
  static const int virtualKeyboardMinimumNoteNumber = 48;
  int lastVirtualKeyboardNoteNumber;
private:
  double mFrequency;
  void CreatePresets();
  Oscillator mOscillator;
  MIDIReceiver mMidiReceiver;
  IControl* mVirtualKeyboard;
  EnvelopeGenerator mEnvelopeGenerator;
  Filter mFilter;
  EnvelopeGenerator mFilterEnvelopeGenerator;
  double filterEnvelopeAmount;
  void processVirtualKeyboard();
  inline void onNoteOn(const int noteNumber, const int velocity) { 
	  mEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_ATTACK); 
	  mFilterEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_ATTACK);
  };
  inline void onNoteOff(const int noteNumber, const int velocity) { 
	  mEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_RELEASE); 
	  mFilterEnvelopeGenerator.enterStage(EnvelopeGenerator::ENVELOPE_STAGE_RELEASE);
  };
  inline void onBeganEnvelopeCycle() { mOscillator.setMuted(false); }
  inline void onFinishedEnvelopecycle() { mOscillator.setMuted(true); }
};

#endif
