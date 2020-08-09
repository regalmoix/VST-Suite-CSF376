#include "Synthesis.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "IKeyboardControl.h"
#include "resource.h"
#include <math.h>
const int kNumPrograms = 5;

enum EParams
{
    mWaveform=0,
    mAttack,
    mDecay,
    mSustain,
    mRelease,
    kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,
  kKeybX=1,
  kKeybY=230
};

Synthesis::Synthesis(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), 
    lastVirtualKeyboardNoteNumber(virtualKeyboardMinimumNoteNumber-1)
{
  TRACE;

  //arguments are: name, defaultVal, minVal, maxVal, step, label

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  //pGraphics->AttachPanelBackground(&COLOR_RED);
  pGraphics->AttachBackground(BG_ID, BG_FN);

  IBitmap whiteKeyImage = pGraphics->LoadIBitmap(WHITE_KEY_ID, WHITE_KEY_FN,6);
  IBitmap blackKeyImage = pGraphics->LoadIBitmap(BLACK_KEY_ID, BLACK_KEY_FN);

  int keyCoordinates[12] = { 0,7,12,20,24,36,43,48,56,60,69,72 };
  mVirtualKeyboard = new IKeyboardControl(this, kKeybX, kKeybY, virtualKeyboardMinimumNoteNumber, 5, &whiteKeyImage, &blackKeyImage, keyCoordinates);
  pGraphics->AttachControl(mVirtualKeyboard);

  GetParam(mWaveform)->InitEnum("Waveform", OSCILLATOR_MODE_SINE, kNumOscillatorModes);
  GetParam(mWaveform)->SetDisplayText(0, "Sine");
  IBitmap waveformBitmap = pGraphics->LoadIBitmap(WAVEFORM_ID, WAVEFORM_FN, 4);
  pGraphics->AttachControl(new ISwitchControl(this, 24, 53, mWaveform, &waveformBitmap));

  IBitmap knobBitmap = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, 64); //adsr knobs

  GetParam(mAttack)->InitDouble("Attack", 0.01, 0.01, 10.0, 0.001);
  GetParam(mAttack)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 95, 34, mAttack, &knobBitmap));
  
  GetParam(mDecay)->InitDouble("Decay", 0.5, 0.01, 15.0, 0.001);
  GetParam(mDecay)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 177, 34, mDecay, &knobBitmap));

  GetParam(mSustain)->InitDouble("Sustain", 0.1, 0.001, 1.0, 0.001);
  GetParam(mSustain)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 259, 34, mSustain, &knobBitmap));

  GetParam(mRelease)->InitDouble("Release", 1.0, 0.001, 15.0, 0.001);
  GetParam(mRelease)->SetShape(3);
  pGraphics->AttachControl(new IKnobMultiControl(this, 341, 34, mRelease, &knobBitmap));

  AttachGraphics(pGraphics);

  //MakePreset("preset 1", ... );
  CreatePresets();

  mMidiReceiver.noteOn.Connect(this, &Synthesis::onNoteOn);
  mMidiReceiver.noteOff.Connect(this, &Synthesis::onNoteOff);
  mEnvelopeGenerator.beganEnvelopeCycle.Connect(this, &Synthesis::onBeganEnvelopeCycle);
  mEnvelopeGenerator.finishedEnvelopeCycle.Connect(this, &Synthesis::onFinishedEnvelopecycle);
}

Synthesis::~Synthesis() {}

void Synthesis::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
    // Mutex is already locked for us.

    double* leftOutput = outputs[0];
    double* rightOutput = outputs[1];
    processVirtualKeyboard();
    for (int i = 0; i < nFrames; ++i) {
        mMidiReceiver.advance();
        int velocity = mMidiReceiver.getLastVelocity();
        
        mOscillator.setFrequency(mMidiReceiver.getLastFrequency());

        //leftOutput[i] = rightOutput[i] = mOscillator.nextSample()* velocity / 127.0;
        
        leftOutput[i] = rightOutput[i] = mOscillator.nextSample() * mEnvelopeGenerator.nextSample() * velocity / 127.0;
    }
    mMidiReceiver.Flush(nFrames);
}

void Synthesis::Reset()
{
  TRACE;
  IMutexLock lock(this);
  mOscillator.setSampleRate(GetSampleRate());
  mEnvelopeGenerator.setSampleRate(GetSampleRate());
}

void Synthesis::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);
  switch (paramIdx) {
  case mWaveform:
      mOscillator.setMode(static_cast<OscillatorMode>(GetParam(mWaveform)->Int()));
      break;
  case mAttack:
  case mDecay:
  case mSustain:
  case mRelease:
      mEnvelopeGenerator.setStageValue(static_cast<EnvelopeGenerator::EnvelopeStage>(paramIdx), GetParam(paramIdx)->Value());
  }
}

void Synthesis::CreatePresets() {
    MakeDefaultPreset((char*)"-", kNumPrograms);
    
}

void Synthesis::ProcessMidiMsg(IMidiMsg* pMsg) {
    mMidiReceiver.onMessageReceived(pMsg);
    mVirtualKeyboard->SetDirty();
}

void Synthesis::processVirtualKeyboard() {
    IKeyboardControl* virtualKeyboard = (IKeyboardControl*) mVirtualKeyboard;
    int virtualKeyboardNoteNumber = virtualKeyboard->GetKey() + virtualKeyboardMinimumNoteNumber;
    if (lastVirtualKeyboardNoteNumber >= virtualKeyboardMinimumNoteNumber && virtualKeyboardNoteNumber != lastVirtualKeyboardNoteNumber) {
        IMidiMsg midiMessage;
        midiMessage.MakeNoteOffMsg(lastVirtualKeyboardNoteNumber, 0);
        mMidiReceiver.onMessageReceived(&midiMessage);
    }
    if (lastVirtualKeyboardNoteNumber >= virtualKeyboardMinimumNoteNumber && virtualKeyboardNoteNumber != lastVirtualKeyboardNoteNumber) {
        IMidiMsg midiMessage;
        midiMessage.MakeNoteOnMsg(virtualKeyboardNoteNumber, virtualKeyboard->GetVelocity(), 0);
        mMidiReceiver.onMessageReceived(&midiMessage);
    }
    lastVirtualKeyboardNoteNumber = virtualKeyboardNoteNumber;
}