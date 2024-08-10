# MicroPicoDrive
## _An internal replacement for the Sinclair QL Microdrive without_

UPDATE 10/08/2024

<img src="Images/Main Photo.png"  width="700">

> [!NOTE]
> _This Fork is updating the information_

This branch is for the new 1.4 version of the MicroPicoDrive.
This version was created by Silverio MRS (AKA Popopo) from [Dr. Gusman's original version](https://github.com/gusmanb/micropicodrive).

# Overview



# Objectives of the project


## Changelog over the original project
New 1.4 design changes:

+ Place it in any position (MDV_1 or MDV_2) without physical modification.
+ IDC Cable now go straight to the connector on the QL MB.
+ Removed Status LED from Cartridge due to redundancy information with QL Case LED.
+ Add MicroSD footprint to the Cartridge. From now on, it will be cheaper & elegant to build it up.
+ The board is shorter than original one reducing possible case clashes.
+ RaspBerryPi Pico now is soldered on the bottom of the Driver PCB.
+ The Power regulator has changed the place, allowing to the board be placed in any position (MDV_1 or MDV_2).
+ Solved problem with QL Speaker.
+ Solved problem with QL metal chassis under keyboard.
+ Solved problem with right lateral QL case and the regulator.
+ Sound System ready for sound emulation or new features related with it (Working in progress).


## What is coming (TODO functionalities)

Majority of users find the device functional and accomplish their expectations, however still it keeps some nice surprises powering it up.

- **Software/firmware**
- [x] Config file to customize preferences (optional).
- [x] New screen modes with bigger fonts (optional).
- [ ] Direct writing (optional).
- **Hardware**
- [ ] Sound emulation of the motor (WiP) (optional).
- [ ] MIDI player (WiP) (optional).
- [ ] Bluetooth cartridge and its APP to load images from other devices (PC, Phone, Tablet...) (optional).


### Some extra add-ons that could come
- [ ] External version of the MDP.
- [ ] Interface I MDP version.

---

# Audio visual material
The next videos were made in order to make easier to everyone the understanding about the integration and use on Sinclair QL systems. 
- [Presentation](https://youtu.be/yyiCoWN6E_A) of the project and its initial goals.
- [How to install it](https://youtu.be/CpoAiAcQRLA) points to keep in mind to not damage your Sinclair QL when installing the MDP into it.
- [User's Guide I](https://youtu.be/wyJjXfhqH4E) user's guide to change the firmware, prepare the SD, customize your preferences & browse it.