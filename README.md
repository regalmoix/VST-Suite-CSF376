# VST Suite for CS F376
This repo is a collection of multiple different VST Plugins (Generators and Effects)
Basic knowledge of Audio Production and DAW is required.

>This repo contains the source code for Projucer projects created as part of Development of Cross-Platform VSTs (Design Project - CSF376, BITS Pilani, Goa Campus).


Suggested resources are [Juce Documentation](https://juce.com/learn/documentation), [Juce Tutorials](https://juce.com/learn/tutorials), [The Audio Programmer](https://www.youtube.com/c/TheAudioProgrammer), and [FreeCodeCamp : Learn Modern C++ by Building an Audio Plugin (w/ JUCE Framework)](https://www.youtube.com/watch?v=i_Iq4_Kd7Rc) 

## Framework

The current projects use Juce exclusively. Get Juce using your package manager (Linux) / [build yourself](https://github.com/juce-framework/JUCE) / [Download](https://juce.com/get-juce/download).
If you're developing on Linux, it is recommended to use build.sh script to build projects. This is a simple wrapper around make.

## Troubleshooting

- Check Projucer configurations for the project. Make sure that the targets are VST3 (& Standalone).
- In Projucer, correctly set Global Paths for Juce, Juce Modules, and User Modules. On Linux Based systems, the paths will likely be `/usr/share/juce`, `/usr/share/juce/modules`, `~/modules`
- Refer existing settings for other Projects
- Prefer using Global Paths for all modules. (Modules > Gear Icon > Enable Global Path for all modules)
- Set `JUCE_VST3_CAN_REPLACE_VST2` disabled for all Module `juce_audio_plugin_client`  
- Add any new files you make using Projucer. Save after adding the new file, and click the Debug and Release Makefiles and Save again to update Makefiles. 

## Configuring Project in Projucer

- Use Basic Audio Plugin Template
- Set Project Line Feed to be `\n`
- Set User Global App Config `true`
- Set Using namespace juce `true`
- Plugin Formats : VST3, Standalone
- Plugin Characteristics : 
	- Generators : Plugin is Synth, Plugin Midi Input 
	- Audio Effect : No Selection
	- Other Plugin Types : Trial and Error
- Plugin VST3 category
	- Generators : Generator, Instrument, Synth
	- Audio Effect : Fx
	- Other Plugin Types : Trial and Error
- C++ Standard : C++17 
- For most others, leave at default values.

## Configuring Exporters in Projucer

This section only details Linux Makefile Exporter Settings.
> Click Linux Makefile
- Extra compiler flags : ` -Wno-unused-parameter `
- GNU Compiler extensions : enabled
> Click Linux Makefile > Debug
- Debug Mode : enabled
- Binary Location : `Builds/Plugin/`
- Recommended Warnings : GCC
- Optimisation : -O0
- Architecture : Native
> Click Linux Makefile > Release
- Debug Mode : disabled
- Binary Location : `Builds/Plugin/`
- Recommended Warnings : GCC
- Link Time Optimisation : enabled
- Optimisation : -O3
- Architecture : Native


## License
Distribution of all plugins in the collection is done under GNU GPLv3.
Juce permits free usage under GPLv3.
