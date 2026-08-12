# Gerbers Description
 _Secction to describe each Gerber option_

---

UPDATE 12/08/2026

> [!NOTE]
> _New revisions of the driver (h) & cartridges (e) with extra EMI & electrical protection and improving signals._.


There system is compound by a driver and its cartridge. It is always possible to improve somehow some elements or factors, so a alive project requires time to time some new releases (when they are justified). 

Sometimes changes are to improve signals, ESD/EMI factors, or power. In other cases it is to bring to the users more options and better implementation/building.

## Driver
Always the last version will result in the best option, but sometimes those changes are not necesesary. If you don't know what to use, choose the newest version. If you are happy with lower protection level or not using some hardware elements you know won't be useful for you (that means you have a good control of what you are doing), choose the alternative from available Gerbers.




## Cartridge
**There are 2 main different shapes** for cartridge which main difference is the size of the handles: **Short** and **Large** one. 

<p align="center">
<img src="../../Images/Difference 1.4b vs 1.4c.png"  width="700">
</p>

The short is more than enough to handle it easily and fits into the old black plastic box where Sinclair used to give PSION software.

The large is easier to extract/insert than the short one, for hands like mine it is nicer. 

How it is designed allows you to use SMD Leds or TH Leds (giving more retro feeling). So when mounting the LEDs you can choose what suits more more to your personality. Keep in mind that schemas reflects resistos values for the LEDs acording to the specs of the used LEDs by myself. TH versions may be happy with 1K resistors but always check it out by yourself according to luminance, max current and max current that RP2040 may source.

<p align="center">
<img src="../../Images/Cartridge Long Wings with TH LEDs.jpg"  width="700">
</p>

Finally, recently I have made some changes for cartridges in order to improve electrical manipulation, so a good part of the pads that were connection to the EDGE socket now are retreived in order to ensure Power Lines are connected before the signals. It is by now working great, but old version has been working for 3 years with non stop (and with HASL finishing).

<p align="center">
<img src="../../Images/Cartridge_1.4eL.jpg"  width="500">
</p>

Pros: Safer electrically, the EDGE contacts now scratch less copper/HASL surface and now it goes on a FR4 fiber that is harder to scratch.
Cons: Has not been tested for years like previous one.