#include "SpaceBass.h"
#include "IPlug_include_in_plug_src.h"
#include "IControl.h"
#include "IKeyboardControl.h"
#include "resource.h"
#include <math.h>
#include <functional>
const int kNumPrograms = 5;
const double parameterStep = 0.001;
enum EParams
{
       //oscs
    mOsc1Waveform=0,
    mOsc1PitchMod,
    mOsc2Waveform,
    mOsc2PitchMod,
    mOscMix,
    //filters
    mFilterMode,
    mFilterCutoff,
    mFilterResonance,
    mFilterLfoAmount,
    mFilterEnvAmount,
    //lfo
    mLFOWaveform,
    mLFOFrequency,
    //volume env
    mVolumeEnvAttack,
    mVolumeEnvDecay,
    mVolumeEnvSustain,
    mVolumeEnvRelease,
    //filter env
    mFilterEnvAttack,
    mFilterEnvDecay,
    mFilterEnvSustain,
    mFilterEnvRelease,
    kNumParams
};

typedef struct {
    const char* name;
    const int x;
    const int y;
    const double defaultVal;
    const double minVal;
    const double maxVal;
} parameterProperties_struct;

const parameterProperties_struct parameterProperties[kNumParams]=
{
    {.name = "Osc 1 Waveform", .x = 30, .y = 75},
    {.name = "Osc 1 Pitch Mod", .x = 69, .y = 61, .defaultVal = 0.0, .minVal = 0.0, .maxVal = 1.0},
    {.name = "Osc 2 Waveform", .x = 203, .y = 75},
    {.name = "Osc 2 Pitch Mod", .x = 242, .y = 61, .defaultVal = 0.0, .minVal = 0.0, .maxVal = 1.0},
    {.name = "Osc Mix", .x = 130, .y = 61, .defaultVal = 0.5, .minVal = 0.0, .maxVal = 1.0},
    {.name = "Filter Mode", .x = 30, .y = 188},
    {.name = "Filter Cutoff", .x = 69, .y = 174, .defaultVal = 0.99, .minVal = 0.0, .maxVal = 0.99},
    {.name = "Filter Resonance", .x = 124, .y = 174, .defaultVal = 0.0, .minVal = 0.0, .maxVal = 1.0},
    {.name = "Filter LFO Amount", .x = 179, .y = 174, .defaultVal = 0.0, .minVal = 0.0, .maxVal = 1.0},
    {.name = "Filter Envelope Amount", .x = 234, .y = 174, .defaultVal = 0.0, .minVal = -1.0, .maxVal = 1.0},
    {.name = "LFO Waveform", .x = 30, .y = 298},
    {.name = "LFO Frequency", .x = 69, .y = 284, .defaultVal = 6.0, .minVal = 0.01, .maxVal = 30.0},
    {.name = "Volume Env Attack", .x = 323, .y = 61, .defaultVal = 0.01, .minVal = 0.01, .maxVal = 10.0},
    {.name = "Volume Env Decay", .x = 378, .y = 61, .defaultVal = 0.5, .minVal = 0.01, .maxVal = 15.0},
    {.name = "Volume Env Sustain", .x = 433, .y = 61, .defaultVal = 0.1, .minVal = 0.001, .maxVal = 1.0},
    {.name = "Volume Env Release", .x = 488, .y = 61, .defaultVal = 1.0, .minVal = 0.01, .maxVal = 15.0},
    {.name = "Filter Env Attack", .x = 323, .y = 174, .defaultVal = 0.01, .minVal = 0.01, .maxVal = 10.0},
    {.name = "Filter Env Decay", .x = 378, .y = 174, .defaultVal = 0.5, .minVal = 0.01, .maxVal = 15.0},
    {.name = "Filter Env Sustain", .x = 433, .y = 174, .defaultVal = 0.1, .minVal = 0.001, .maxVal = 1.0},
    {.name = "Filter Env Release", .x = 488, .y = 174, .defaultVal = 1.0, .minVal = 0.01, .maxVal = 15.0}
};

enum ELayout
{
  kWidth = GUI_WIDTH,
  kHeight = GUI_HEIGHT,
  kKeybX=62,
  kKeybY=425
};

SpaceBass::SpaceBass(IPlugInstanceInfo instanceInfo)
  :	IPLUG_CTOR(kNumParams, kNumPrograms, instanceInfo), 
    lastVirtualKeyboardNoteNumber(virtualKeyboardMinimumNoteNumber-1),
    filterEnvelopeAmount(0.0),
    lfoFilterModAmount(0.1)
{
  TRACE;

  //arguments are: name, defaultVal, minVal, maxVal, step, label

  CreateParams();
  CreateGraphics();
  CreatePresets();

  mMidiReceiver.noteOn.Connect(&voiceManager, &VoiceManager::onNoteOn);
  mMidiReceiver.noteOff.Connect(&voiceManager, &VoiceManager::onNoteOff);
}

SpaceBass::~SpaceBass() {}

void SpaceBass::ProcessDoubleReplacing(double** inputs, double** outputs, int nFrames)
{
    // Mutex is already locked for us.

    double* leftOutput = outputs[0];
    double* rightOutput = outputs[1];
    processVirtualKeyboard();
    for (int i = 0; i < nFrames; ++i) {
        mMidiReceiver.advance();
        leftOutput[i] = rightOutput[i] = voiceManager.nextSample() ;
    }
    mMidiReceiver.Flush(nFrames);
}

void SpaceBass::Reset()
{
  TRACE;
  IMutexLock lock(this);
  double sampleRate=GetSampleRate();
  voiceManager.setSampleRate(sampleRate);
}

void SpaceBass::OnParamChange(int paramIdx)
{
  IMutexLock lock(this);
  IParam* param = GetParam(paramIdx);
  if (paramIdx == mLFOWaveform) {
      voiceManager.setLFOMode(static_cast<Oscillator::OscillatorMode>(param->Int()));
  }
  else if (paramIdx == mLFOFrequency) {
      voiceManager.setLFOFrequency(param->Value());
  }
  else {
      using std::placeholders::_1;
      using std::bind;
      VoiceManager::VoiceChangerFunction changer;
      switch (paramIdx) {
      case mOsc1Waveform:
          changer = bind(&VoiceManager::setOscillatorMode, _1, 1, static_cast<Oscillator::OscillatorMode>(param->Int()));
          break;
      case mOsc1PitchMod:
          changer = bind(&VoiceManager::setOscillatorPitchMod, _1, 1, param->Value());
          break;
      case mOsc2Waveform:
          changer = bind(&VoiceManager::setOscillatorMode, _1, 2, static_cast<Oscillator::OscillatorMode>(param->Int()));
          break;
      case mOsc2PitchMod:
          changer = bind(&VoiceManager::setOscillatorPitchMod, _1, 2, param->Value());
          break;
      case mOscMix:
          changer = bind(&VoiceManager::setOscillatorMix, _1, param->Value());
          break;
          // Filter Section:
      case mFilterMode:
          changer = bind(&VoiceManager::setFilterMode, _1, static_cast<Filter::FilterMode>(param->Int()));
          break;
      case mFilterCutoff:
          changer = bind(&VoiceManager::setFilterCutoff, _1, param->Value());
          break;
      case mFilterResonance:
          changer = bind(&VoiceManager::setFilterResonance, _1, param->Value());
          break;
      case mFilterLfoAmount:
          changer = bind(&VoiceManager::setFilterLFOAmount, _1, param->Value());
          break;
      case mFilterEnvAmount:
          changer = bind(&VoiceManager::setFilterEnvAmount, _1, param->Value());
          break;
          // Volume Envelope:
      case mVolumeEnvAttack:
          changer = bind(&VoiceManager::setVolumeEnvelopeStageValue, _1, EnvelopeGenerator::ENVELOPE_STAGE_ATTACK, param->Value());
          break;
      case mVolumeEnvDecay:
          changer = bind(&VoiceManager::setVolumeEnvelopeStageValue, _1, EnvelopeGenerator::ENVELOPE_STAGE_DECAY, param->Value());
          break;
      case mVolumeEnvSustain:
          changer = bind(&VoiceManager::setVolumeEnvelopeStageValue, _1, EnvelopeGenerator::ENVELOPE_STAGE_SUSTAIN, param->Value());
          break;
      case mVolumeEnvRelease:
          changer = bind(&VoiceManager::setVolumeEnvelopeStageValue, _1, EnvelopeGenerator::ENVELOPE_STAGE_RELEASE, param->Value());
          break;
          // Filter Envelope:
      case mFilterEnvAttack:
          changer = bind(&VoiceManager::setFilterEnvelopeStageValue, _1, EnvelopeGenerator::ENVELOPE_STAGE_ATTACK, param->Value());
          break;
      case mFilterEnvDecay:
          changer = bind(&VoiceManager::setFilterEnvelopeStageValue, _1, EnvelopeGenerator::ENVELOPE_STAGE_DECAY, param->Value());
          break;
      case mFilterEnvSustain:
          changer = bind(&VoiceManager::setFilterEnvelopeStageValue, _1, EnvelopeGenerator::ENVELOPE_STAGE_SUSTAIN, param->Value());
          break;
      case mFilterEnvRelease:
          changer = bind(&VoiceManager::setFilterEnvelopeStageValue, _1, EnvelopeGenerator::ENVELOPE_STAGE_RELEASE, param->Value());
          break;
      }
      voiceManager.changeAllVoices(changer);
  }
}

void SpaceBass::CreatePresets() {
    MakeDefaultPreset((char*)"-", kNumPrograms);
    
}

void SpaceBass::ProcessMidiMsg(IMidiMsg* pMsg) {
    mMidiReceiver.onMessageReceived(pMsg);
    mVirtualKeyboard->SetDirty();
}

void SpaceBass::processVirtualKeyboard() {
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

void SpaceBass::CreateParams() {
    for (int i = 0; i < kNumParams; i++) {
        IParam* param = GetParam(i);
        const parameterProperties_struct& properties = parameterProperties[i];
        switch (i) {
        case mOsc1Waveform:
        case mOsc2Waveform:
            param->InitEnum(properties.name, Oscillator::OSCILLATOR_MODE_SAW, Oscillator::kNumOscillatorModes);
            param->SetDisplayText(0, properties.name);
            break;
        case mLFOWaveform:
            param->InitEnum(properties.name, Oscillator::OSCILLATOR_MODE_TRIANGLE, Oscillator::kNumOscillatorModes);
            param->SetDisplayText(0, properties.name);
            break;
        case mFilterMode:
            param->InitEnum(properties.name, Filter::FILTER_MODE_LOWPASS, Filter::kNumFilterModes);
            break;
        default:
            param->InitDouble(properties.name, properties.defaultVal, properties.minVal, properties.maxVal, parameterStep);
            break;
        }
    }
    GetParam(mFilterCutoff)->SetShape(2);
    GetParam(mVolumeEnvAttack)->SetShape(3);
    GetParam(mFilterEnvAttack)->SetShape(3);
    GetParam(mVolumeEnvDecay)->SetShape(3);
    GetParam(mFilterEnvDecay)->SetShape(3);
    GetParam(mVolumeEnvSustain)->SetShape(2);
    GetParam(mFilterEnvSustain)->SetShape(2);
    GetParam(mVolumeEnvRelease)->SetShape(3);
    GetParam(mFilterEnvRelease)->SetShape(3);
    for (int i = 0; i < kNumParams; i++) {
        OnParamChange(i);
    }
}

void SpaceBass::CreateGraphics() {
    IGraphics* pGraphics = MakeGraphics(this, kWidth, kHeight);
    pGraphics->AttachBackground(BG_ID, BG_FN);
    IBitmap whiteKeyImage = pGraphics->LoadIBitmap(WHITE_KEY_ID, WHITE_KEY_FN, 6);
    IBitmap blackKeyImage = pGraphics->LoadIBitmap(BLACK_KEY_ID, BLACK_KEY_FN);
    int keyCoordinates[12] = { 0,10,17,30,35,52,61,68,79,85,97,102 };
    mVirtualKeyboard = new IKeyboardControl(this, kKeybX, kKeybY, virtualKeyboardMinimumNoteNumber, 4, &whiteKeyImage, &blackKeyImage, keyCoordinates);
    pGraphics->AttachControl(mVirtualKeyboard);

    IBitmap waveformBitmap = pGraphics->LoadIBitmap(WAVEFORM_ID, WAVEFORM_FN, 4);
    IBitmap filterModeBitmap = pGraphics->LoadIBitmap(FILTERMODE_ID, FILTERMODE_FN, 3);
    IBitmap knobBitmap = pGraphics->LoadIBitmap(KNOB_ID, KNOB_FN, 64);

    for (int i = 0; i < kNumParams; i++) {
        const parameterProperties_struct& properties = parameterProperties[i];
        IControl* control;
        IBitmap* graphic;
        switch (i) {
        case mOsc1Waveform:
        case mOsc2Waveform:
        case mLFOWaveform:
            graphic = &waveformBitmap;
            control = new ISwitchControl(this, properties.x, properties.y, i, graphic);
            break;
        case mFilterMode:
            graphic = &filterModeBitmap;
            control = new ISwitchControl(this, properties.x, properties.y, i, graphic);
            break;
        default:
            graphic = &knobBitmap;
            control = new IKnobMultiControl(this, properties.x, properties.y, i, graphic);
            break;
        }
        pGraphics->AttachControl(control);
    }
    AttachGraphics(pGraphics);
}