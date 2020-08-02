#include "Synthesis.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "resource.h"
#include <math.h>
const int kNumPrograms = 5;

enum EParams
{
  kNumParams
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,
};

Synthesis::Synthesis(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), mFrequency(1.)
{
  TRACE;

  //arguments are: name, defaultVal, minVal, maxVal, step, label

  IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
  //pGraphics->AttachPanelBackground(&COLOR_RED);
  pGraphics->AttachBackground(BACKGROUND_ID, BACKGROUND_FN);

  AttachGraphics(pGraphics);

  //MakePreset("preset 1", ... );
  CreatePresets();
}

Synthesis::~Synthesis() {}

void Synthesis::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
    // Mutex is already locked for us.

    double* leftOutput = outputs[0];
    double* rightOutput = outputs[1];
    for (int i = 0; i < nFrames; ++i) {
        mMidiReceiver.advance();
        int velocity = mMidiReceiver.getLastVelocity();
        if (velocity > 0) {
            mOscillator.setFrequency(mMidiReceiver.getLastFrequency());
            mOscillator.setMuted(false);
        }
        else {
            mOscillator.setMuted(true);
        }
        leftOutput[i] = rightOutput[i] = mOscillator.nextSample()* velocity / 127.0;
    }
    mMidiReceiver.Flush(nFrames);
}

void Synthesis::Reset()
{
  TRACE;
  IMutexLock lock(this);
  mOscillator.setSampleRate(GetSampleRate());
}

void Synthesis::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);

}

void Synthesis::CreatePresets() {
    MakeDefaultPreset((char*)"-", kNumPrograms);
    
}

void Synthesis::ProcessMidiMsg(IMidiMsg* pMsg) {
    mMidiReceiver.onMessageReceived(pMsg);
}