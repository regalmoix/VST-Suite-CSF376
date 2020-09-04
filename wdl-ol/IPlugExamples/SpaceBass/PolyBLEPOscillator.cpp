#include "PolyBLEPOscillator.h"

double PolyBLEPOscillator::poly_blep(double t) {
	double dt = mPhaseIncrement / twoPI;
	if (t < dt) {
		t /= dt;
		return t + t - t * t - 1.0;
	}
	else if (t > 1.0 - dt) {
		t = (t - 1.0) / dt;
		return t * t + t + t + 1.0;
	}
	else return 0.0;
}

double PolyBLEPOscillator::nextSample() {
	double value = 0.0;
	double t = mPhase / twoPI;
	if (mOscillatorMode == OSCILLATOR_MODE_SINE) {
		value = naiveWaveformForMode(OSCILLATOR_MODE_SINE);
	}
	else if (mOscillatorMode == OSCILLATOR_MODE_SAW) {
		value = naiveWaveformForMode(OSCILLATOR_MODE_SAW);
		value -= poly_blep(t);
	}
	else {
		value = naiveWaveformForMode(OSCILLATOR_MODE_SQUARE);
		value += poly_blep(t);
		value -= poly_blep(fmod(t + 0.5, 1.0));
		if (mOscillatorMode == OSCILLATOR_MODE_TRIANGLE) {
			value = mPhaseIncrement * value + (1 - mPhaseIncrement) * lastOutput;
			lastOutput = value;
		}
	}
	mPhase += mPhaseIncrement;
	while (mPhase >= twoPI) {
		mPhase -= twoPI;
	}
	return value;
}