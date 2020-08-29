#ifndef __SPACEBASS__
#define __SPACEBASS__

#include "IPlug_include_in_plug_hdr.h"
#include "MIDIReceiver.h"
#include "VoiceManager.h"

class SpaceBass : public IPlug
{
public:
  SpaceBass(IPlugInstanceInfo instanceInfo);
  ~SpaceBass();

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
  MIDIReceiver mMidiReceiver;
  IControl* mVirtualKeyboard;
  double lfoFilterModAmount;
  double filterEnvelopeAmount;
  void processVirtualKeyboard();
  void CreateParams();
  void CreateGraphics();
  VoiceManager voiceManager;
};

#endif
