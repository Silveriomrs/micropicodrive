# Description of Images (MDV & MDP)
## _Overview_

> [!NOTE]
> _Images served only for testing purpose_

The microdrives images presented here are only for testing purpose. Thus you can find software ready to load into your QL system and others that need some works with [MDTools (From Dr. Gusman)](https://github.com/gusmanb/micropicodrive/releases/tag/V1.0.0.0) or another similar tool.

In particular part of the PSION software is not prepared yet to be loaded successfully in a real QL (but Abacus). It allows you to understand the complexity and how MDV images are required to be adapted for emulators or some devices. What means it is not related with an specific hardware solution but how datas are modified by some OSes (_ie. MS Windows_) when they are stored in its file system.

The idea is to give few images to play around directly with QL and enjoy the MicroDrivePico solution (also name here as MPD). Others to learn to control the MDTools and how the QL ecosystem fight between different formats and issues from a highly complex legacy reasons.

If you are new in QL ecosystem, be aware that due to some OS treatment with storage systems (MS Windows mainly) it requires some "headers" or extra info related with data space and "Magic Fields" that exceed the purpose of this info. All that can be solved with the mentioned tools above.

Also, for newbies there is a empty and formatted image that you can use (copy and paste) to use like real Microdrives to save your data or use like you were using real one.

I hope you're enjoying this beautiful solution. :)

## Directory: Main
No particular hardware needs for those images.


| Image | Ready? | Type | Overview |
| ------ | :--: | :--: | ---- |
| ABACUS.MDV | Yes | Office | Developed by PSION software |
| Chess.mdv | Yes | Game |  |
| pacman_patched | No | Game | The image has not right checksum |
| Pitman.MDV | Yes | Game | Developed by Andrei |
| Space Invaders(PD).mdv | Yes | Game |  |
| MyBasic.mdv | Yes | Util | Basic cartridge with a Basic sample |
| newcart.mdv | Yes | Util | Formatted MDV image for your use |
| **CONFIG.CFG** | Yes | Util | _Optional file_ **required to customize the device** |


## Directory: PSION_4
All those belongs to PSION suit. All of them work properly in a stock QL with MDPs. They have the headers 'fixed' with the data attribute saved. They could serve like comparation with the structure image with their twins into the Fail folder.


| Image | Ready? | Type | Overview |
| ------ | :--: | :--: | ---- |
| ARCHIVE.MDV | Yes | Office | Developed by PSION software |
| EASEL.MDV | Yes | Office | Developed by PSION software |
| QUILL.MDV | Yes | Office | Developed by PSION software |


## Directory: FAIL
All those belongs to PSION suit. Those images are to test and play around with convertir MDVs subformats and doesn't work without 'fix' them. Specially to use Dr. Gusman tools or Johan Engdhal's tool. 


| Image | Ready? | Type | Overview |
| ------ | :--: | :--: | ---- |
| ARCHIVE.MDV | No | Office | Developed by PSION software |
| EASEL.MDV | No | Office | Developed by PSION software |
| QUILL.MDV | No | Office | Developed by PSION software |



## Directory: Demos
**Some hardware requirements could be need**. For example: Sound cards, expansion cards, memory expansion, new versions of the demos (due to it presented bugs in the time where they were published), etc. You should be aware of those matters and find the requirements/updates to play around with them.

This directory contains some others with files of many kinds to play around about converting them into MDV images, changing headers and so on.



| Image | Ready? | Type | Overview |
| ------ | :--: | :--: | ---- |
| m86K24 | ? | Demo | Sound card/Specific ROM? |
| smfxteam.mdv | ? | Demo | Sound card/Specific ROM? |
| qldrive11 | X | Dir | Multiple files for testing |
| war | X | Dir | Multiple files for testing |
| WB21_tk2.MDV | yes | Demo | Amiga Workbench demo. Requires memory expansion and TK2 |

**Directories qldrive11 & war**

Those directories contains files of a vary of types in order to play around, and may or may not be packed into MDV images, changing headers and so on.

> [!NOTE]
> Those dirs are placed here to check out how the browsing system of the MDPs is right, ignores not compatible files or contains bugs. So its purpose is not to be loaded into a real QL.

## Directory: more
This folder contain some legacy images from original project (Dr. Gusman's github). Not a specific role yet.