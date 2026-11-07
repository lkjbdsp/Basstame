# lkjb BassTame

## Build steps:

The project contains solutions for Visual Studio 2022. There are no macOS/XCode project files as I don't have a Mac anymore to test it. But It shouldn't be to hard to get a current AI LLM to create a CMakeLists.txt from the Visual Studio solution and project files.

- copy VST3 SDK and VST SDK files in according directories
- build Cairo in YCAIRO/Cairo_Graphics/projects/cairo.sln
- build BassTame in IPlugExamples\BassTame/BassTame.sln

If only the VST3 SDK should be build remove the vst2 project from the qrange.sln solution.

## 3rd Party Libraries used with BassTame

BassTame uses IPlug-Youlean. See readme.txt for more information.
