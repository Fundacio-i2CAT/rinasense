FreeRTOS RINA library for microcontroller platforms
====

## 1. Overview
This piece of software is a RINA library implementation for FreeRTOS, 
so that RINA communication capabilities are supported by devices that 
use this RTOS (for information about RINA please visit 
http://pouzinsociety.org). Initially the RINA implementation can be used over the 
ESP32 hardware platform, using RINA over WiFi and RINA over Bluetooth.
This RINA library will be interoperable with IRATI, a RINA implementation
for Linux Operating Systems (https://github.com/IRATI/stack).

## 2. Development workflow
Tihs project has the following branches:

* **master**: contains the most up to date stable code, may not contain features 
that have been developed and are merged in *dev* but are yet not stable enough.

* **dev**: contains all finished and tested features. When the dev branch is 
considered enough, it can be merged into *master*.

* **feature dev branches**: each feature is developed in a dedicated branch, and 
merged into *dev* when they are ready.

To develop a new feature or a fix for a bug, the workflow is the following (as a 
prerequisite you must fork the main rinasense repo):

1. Create an issue summarising the work to be done (https://github.com/Fundacio-i2CAT/rinasense/issues/new/choose). 
Assign it to someone and add one or more labels.

2. Create a new branch from the *dev* branch. The branch name must follow this 
conventions: <issue number>-<one_or_more_descriptive_words>

3. Start developing the feature. Pull from *dev* as required to avoid merge 
conflicts later. Once the feature is developed, create a pull request to merge
the branch into *dev*.

4. Once the pull request has been accepted and merged into *dev*, remove the 
specific development branch.

