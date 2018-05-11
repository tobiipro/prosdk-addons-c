# prosdk-addons-c - Calibration validation

## What is it
Add-ons for the Tobii Pro SDK.

![alt text](https://www.tobiipro.com/imagevault/publishedmedia/6rkt3jb83qlottsfh1ts/Tobii-Pro-SDK-with-VR-3_1-banner.jpg)


The Tobii Pro SDK is available at: https://www.tobiipro.com/product-listing/tobii-pro-sdk/ <br/>
Documentation to the API: http://developer.tobiipro.com/python.html
Getting started: http://developer.tobiipro.com/c/c-getting-started.html

Do not hesitate to contribute to this project and create issues if you find something that might be wrong or could be improved.

## Installation

* Download or clone this folder.
* Download the Tobii Pro SDK C for your operating system.
* Unpack the Tobii Pro SDK archive to a new directory _sdk_ in root of the addons folder.

### For Windows

* Open Visual Studio and load the solution in the win32 directory.
* Build for the desired configuration (i.e. Debug/Release and x86/x64).
* In order to use the library you need to manually load two DLLs (*tobii_research_addons.dll* and *tobii_research.dll*) in your source or link against their respective lib file. See sample project.

### For Linux

* TODO

### For Mac OS X:

* TODO

## Features

### Calibration Validation

The package contains functionality for validating calibrations by calculating various statistics for different stimuli points. Note: There are no functionality for actually presenting the stimuli points on screen.

#### Example

See [sample.c](./source/sample.c) in the source directory for a working example.

Also, see [screen_based_calibration_validation.h](./source/screen_based_calibration_validation.h) for documentation of the API.
