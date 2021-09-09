Needed to get LADPSA for building audiopluginhost
Needed to update to Juce 6 for VST3 support (yay has Juce6 binary prebuilt)
Wrote a build script to simplify
make takes CONFIG= variable to specify debug or release
Use -j8 for faster compilation.
Symlink (NOT COPY) the AudioPluginHost app to Builds folder of required plugin (for convinence)
Do not tick MIDI Effect or Synth or anything in ProJucer [otherwise no audio io pins]
Only tick FX, Effects [since audio effect]
Use PulseAudio Sound Server for output in Audiopluginhost
https://github.com/Spyced-Concepts/AudioFilePlayerPlugin/blob/master/Source/PluginEditor.cpp
Changed some of the above code to make it working
https://docs.juce.com/master/tutorial_playing_sound_files.html
