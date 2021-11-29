# VST Suite for CS F376

This repo is a collection of multiple different VST Plugins (Generators and Effects)
Basic knowledge of Audio Production and DAW is required.

>This repo contains the source code for Projucer projects created as part of Development of Cross-Platform VSTs (Design Project - CSF376, BITS Pilani, Goa Campus).

Suggested resources are [Juce Documentation](https://juce.com/learn/documentation), [Juce Tutorials](https://juce.com/learn/tutorials), [The Audio Programmer](https://www.youtube.com/c/TheAudioProgrammer), [FreeCodeCamp : Learn Modern C++ by Building an Audio Plugin (w/ JUCE Framework)](https://www.youtube.com/watch?v=i_Iq4_Kd7Rc), and [C++ Programming Tutorial - Build a 3-Band Compressor Audio Plugin (w/ JUCE Framework)](https://www.youtube.com/watch?v=Mo0Oco3Vimo)

Note that the Equaliser plugin has already been implemented, so you may want to implement compressor, limiter with sidechain functionality.  

## Framework

The current projects use Juce exclusively. Get Juce using your package manager (Linux) / [build yourself](https://github.com/juce-framework/JUCE) / [Download](https://juce.com/get-juce/download).
If you're developing on Linux, it is recommended to use build.sh script to build projects. This is a simple wrapper around make.
Depends on [dialog](https://bash.cyberciti.biz/guide/A_menu_box) which can be obtained from your package manager.

## Setup Existing Project

- If on Linux, open the project's `.jucer` file using Projucer
- Click on Linux Makefile and save using `Ctrl + S`
- If using VS Code, see [Setup VS Code](#setup-vs-code)

## Configuring New Project in Projucer

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
- See [Configuring Exporters in Projucer](#configuring-exporters-in-projucer)

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

## Setup VS Code

- Install VS Code C/C++ extension
- Set the EOL (End of Line) to `LF`. This can be changed from the bottom bar on the right corner
- After installing Juce, add to the include path of VS Code C/C++ the path to Juce, typically `/usr/share/juce/**`
- To the default include path, also add `${workspaceFolder}/**`
- It is recommended not to open the folder `VST-Suite-CSF376`, since intellisense will find it difficult to associate the correct JuceLibraryCode files
- Instead, it is recommended to open the `VST_CSF376.code-workspace` file, and add the Project Folder you are working on in it.
- You can also add other reference projects to that workspace, if need be.
- Optionally, use clang-format to format the code, use extension `xaver.clang-format`. Set clang-format to be the default formatter in VS Code, `Ctrl+Shift+I` to format the file.
- If you have the `VST-Suite-CSF376/Projects` folder open or you are in the workspace, you can use `Ctrl+Shift+B` to pop a menu to build the appropriate project
- It is recommended to look at the .vscode folder of existing project folders and replicate them in any new Project created in the future

## Troubleshooting

- Use Juce 6 and above. VST3 support on linux does not existing in lower versions of Juce.
- After changing branches / checking out another commit, re open the `.jucer` file in Projucer, re-save the Makefile and then clean build using ./build.sh script
- Check Projucer configurations for the project. Make sure that the targets are VST3 (& Standalone).
- In Projucer, correctly set Global Paths for Juce, Juce Modules, and User Modules. On Linux Based systems, the paths will likely be `/usr/share/juce`, `/usr/share/juce/modules`, `~/modules`
- Refer existing settings for other Projects
- Prefer using Global Paths for all modules. (Modules > Gear Icon > Enable Global Path for all modules)
- Set `JUCE_VST3_CAN_REPLACE_VST2` disabled for all Module `juce_audio_plugin_client`  
- Add any new files you make using Projucer. Save after adding the new file, and click the Debug and Release Makefiles and Save again to update Makefiles.

## License

Distribution of all plugins in the collection is done under GNU GPLv3.
Juce permits free usage under GPLv3.

Other Licenses, if any, are mentioned in the respective subfolders and are applicable to any file in that subfolder.
