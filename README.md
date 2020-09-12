# VSTs_CSF376
This repo contains the source code for Projucer projects created as part of Development of Cross-Platform VSTs (Design Project - CSF376).
The JUCE Tutorials folder contains implementations of tutorials on the JUCE website, bound by the terms of the ISC licence under which JUCE's own code for tutorials is distributed.
The repo also contains copies of the following libraries:
1. wdl-ol (https://github.com/olilarkin/wdl-ol) - Oli Larkin's enhancement of Cockos' iPlug, now no longer maintained
2. iPlug2 (https://github.com/iPlug2/iPlug2) - The continuation of iPlug/wdl-ol, still a pre-release.

Within the wdl-ol/IPlugExamples folder are implementations of plugin examples (DigitalDistortion, SpaceBass) from Martin Finke's tutorial series blog (http://www.martin-finke.de/blog/)

A few tips for patching the required libraries before compiling a plugin written in wdl-ol:
1. Steinberg VST3 SDK’s VST2 port folder lacks the following files: aeffect.h and aeffectx.h, both deprecated, but required by wdl-ol. Fix: get files from a VST SDK 2.4 online archive (or a GitHub gist).
2. While patching the VST3 SDK, copy the thread folder in public.sdk as well due to dependencies.
3. Test VST2 builds as 32bit on savihost (64 bit is broken, throws config error) during debug, build as release for 64 bit to test in DAW – Ableton live 10.1 onwards supports VST2 and VST3 on Windows, while Mac is still on AU and VST2.
4. For user-level signals in C++: Open-source Signals library (https://github.com/pbhogan/Signals) by Patrick Hogan .
