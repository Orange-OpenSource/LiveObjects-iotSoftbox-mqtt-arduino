# LiveObject IoT Client - IoTSoftBox-MQTT

This repository contains LiveObject IoT Client Library (used to connect devices to LiveObject Server from our partners).

Visit [Datavenue Live Objects - complete guide](https://liveobjects.orange-business.com/doc/html/lo_manual.html).

And mainly the [Device mode](https://liveobjects.orange-business.com/doc/html/lo_manual.html#MQTT_MODE_DEVICE) section.

Please, have a look to the [user manual](docs/liveobjects_starterkit_arduino_v1.1.pdf) to have a presentation of the library and to be more familiar with it.

**We use the [Arduino IDE](https://www.arduino.cc/en/Main/Software) for all our operations.** Download and install it before going any further.

## Table of content

- [Requirement](#requirement)
	- [Hardware](#hardware)
	- [Software](#software)
	- [LiveObjects API Key](#liveobjects-api-key)
	- [Setup the LiveObjects header file](#setup-the-liveobjects-header-file)
		- [Configuration files](#configuration-files)
		- [API key](#api-key)
		- [Security](#security)
- [LinkIt-ONE](#linkit-one)
- [Usage](#usage)
	- [Library Installation](#library-installation)
	- [Build an example](#build-an-example)
	- [Launch](#launch)
	- [Debug](#debug)
- [Libraries](#libraries)
	- [jsmn](#jsmn)
	- [mbedTLS](#mbedtls)
	- [paho mqtt](#paho-mqtt)
- [Application Control](#application-control)
	- [Live Objects Portal](#live-objects-portal)
	- [Live Objects Swagger](#live-objects-swagger)
	

## Requirement

### Hardware

* An Arduino compatible platform like :
	1. the Mediatek [LinkIt-ONE](https://labs.mediatek.com/en/platform/linkit-one)
		* SIM card to use GSM/GPRS communication interface.
* Internet connectivity

Note: The mbedtls implementation requires a board with the RAM size > = 16 K Bytes.

### Software

* [Arduino IDE](https://www.arduino.cc/en/Main/Software). Tested with ARDUINO 1.8.1
* Install additional packages/libraries (to use your board and communication shield). 
	* If you’re using the LinkIt One board, you will need:
		* to download the Mediatek SDK. Procedure is described [here](https://docs.labs.mediatek.com/resource/linkit-one/en/getting-started/get-started-on-windows/install-the-arduino-ide-and-linkit-one-sdk)

### LiveObjects API Key

Visit [IoT Soft Box powered by Datavenue](https://liveobjects.orange-business.com/v2/#/sdk)

1. You need to request the creation of a developer account.
1. Then, with your LiveObjects user identifier, login to the [Live Objects portal](https://liveobjects.orange-business.com/#/login).
1. Go in 'Configuration - API key' tab, and add a new API key.
**Don't forget to copy this API key value** in a local and secure place during this operation.

### Setup the LiveObjects header file

The config directory can be edited after the library installation.

#### Configuration files

Once the library has been installed (see [below](#library-installation)), you will be able to customize the applications behaviors. The files are located in this folder `<Arduino user dir>\libraries\iotsoftbox_mqtt_arduino\src\config`.

#### API key
In the config directory of every examples, you will find 3 files to customize the behavior of the library.
Edit this files to change some values, in particular the **LiveObjects API key** in `liveobjects_dev_params.h`.

#### Security
From this file (`liveobjects_dev_params.h`) you can also disable TLS By switching `#define SECURITY_ENABLED 1` to 0.
If the security is disabled your device will communicate in plain text with the platform.

By disabling the security, MbedTLS code's will still be embedded because it is used by the resource appliance for MD5 purpose.

You can avoid to compile mbedTLS by uncommenting `//#define LOC_FEATURE_MBEDTLS 0` in  `liveobjects_dev_config.h` but resource related feature won't be available.

## LinkIt-ONE

If you want to use a LinkIt-ONE, you need to install the LinkIt-ONE SDK.

To do so follow this [link](https://docs.labs.mediatek.com/resource/linkit-one/en/getting-started/get-started-on-windows/install-the-arduino-ide-and-linkit-one-sdk) and go to step 4.

After that you will be able to compile and upload program to the LinkIt-ONE

## Usage
### Library Installation

> - Download the zip file from web site https://github.com/Orange-OpenSource/LiveObjects-iotSoftbox-mqtt-arduino.
>	**iotsoftbox_mqtt_arduino.zip**, containing library and examples for the following boards:
>	1. Mediatek LinkIt One
> - Open your Arduino IDE and add this library **Sketch** -> **Include Library** -> **Add .ZIP Library** and Select the zip file : `iotsoftbox_mqtt_arduino.zip`
> - Need to update the file `<Arduino user dir>\libraries\iotsoftbox_mqtt_arduino\src\config\liveobjects_dev_params.h` to set your Live Objects Tenant API key
> - Open example sketch **File** -> **Examples** -> **LiveObjects iotsoftbox** -> **liveobjects_sample_basic_mdk** (or liveobjects_sample_basic for an arduino board)
> - For LintIt One board, edit `liveobjects_sample_basic_mdk.h`  file to set your SIM parameters: **GPRS_APN** , **GPRS_USERNAME** , **GPRS_PASSWORD**

### Select the correct board

- Mediatek LinkIt One:
	* **Tools -> Boards -> LinkIt One**
	* **Tools -> Programmer -> LinkIt Firmware Updater**

### Build an example

To build an example in the IDE, just use Sketch -> Verify/Compile.

### Launch

First, check that the correct board is chosen in Tools -> Boards. Also verify that the IDE is using the correct COM port (Tools -> Port).
To upload a program to your board: Sketch -> Upload.
The card will launch the program after the upload.

### Debug

You can change the debug Level (more or less verbose) inside each example.
```
#define DBG_DFT_MAIN_LOG_LEVEL <Debug Level>
```
It goes from 1 (only errors) to 6 (everything).

## Libraries

Here is a list of the third-party libraries used in this library and their utilities:

### jsmn

[jsmn](https://github.com/zserge/jsmn) (pronounced like 'jasmine') is a minimalistic JSON parser in C. It can be easily integrated into resource-limited or embedded projects.

### mbedTLS

[mbed TLS](https://github.com/ARMmbed/mbedtls) offers an SSL library with an intuitive API and readable source code, so you can actually understand what the code does. Also the mbed TLS modules are as loosely coupled as possible and written in the portable C language. This allows you to use the parts you need, without having to include the total library.

### paho mqtt

[paho mqtt](https://github.com/eclipse/paho.mqtt.embedded-c) is part of the Eclipse Paho project, which provides open-source client implementations of MQTT and MQTT-SN messaging protocols aimed at new, existing, and emerging applications for the Internet of Things.

## Application Control

### Live Objects Portal

Using your Live Objects user account, go to [Live Objects Portal](https://liveobjects.orange-business.com/#/login).

### Live Objects Swagger

Go in [Live Objects Swagger User Interface](https://liveobjects.orange-business.com/swagger-ui/index.html).
