# SvenInt v3
A mod for Sven Co-op that improves gameplay.

Supported versions of the game: 5.26, 5.25 and 5.11

Power of Speedrun Tools provided by that mod:
https://youtu.be/QTiYMgazPjE?si=bmS2X2IYqXBer270
https://youtu.be/XsKlNMPGTls?si=Z-0XybjmiOnNJvV8
https://youtu.be/h59zz0tma4U?si=um7QOgV7PSSO6dd6

## Installation
1. Download the release archive for your OS (Windows/Linux)
2. Extract the archive to the game’s root folder
3. Launch the game with the -insecure parameter (just for safe)
4. Injection
4.1 **Windows**
Use any DLL injector to inject library `svenint.dll`

4.2 **Linux** (Ubuntu at least)
Run the script `./load` with sudo, it will automatically inject library `libsvenint.so`.
Note: sometimes load of SvenInt may fail because of incorrect reading of file `gamedata.txt`, in this case restart the game and repeat the injection. To check why SvenInt may not load, go to the root folder of the game, here you will find the logs file `svenint.log`
**Important:** in Linux, by default, enabled feature named `Ignore Unicode` (category Visual) that replaces all Unicode/UTF characters with spaces, it's needed if your game does support ASCII only, disable the feature if you can see (not using SvenInt) Unicode characters

## Gamedata
SvenInt uses patterns, offsets, RVA offsets to lookup any gamedata using file `./svenint/gamedata.txt`, any update of this mod means that you should consider updating your `gamedata.txt`

## Fixing possible issues
* Disable shaders (via launch option or menu) in case your screen appear black, but when you open the main menu it disappears, also, disable it if your game crashes everytime you enter a map
* Disable Models Downloader feature in case you experience crashes from time to time
* If you have bugged sounds on a specified server then you have to follow this path `Sven Co-op/svencoop_downloads/maps/soundcache/`, remove folder that contains IP and Port of the server

## Features
* **Player:** Aim (Aimbot / Ragebot / No Recoil), Anti-AFK, Auto Reload, Cam Hack, Color Pulsator, Dynamic Glow, First Person Roaming, Custom Flashlight, Freeze, Gib Abuse (Fly when you have huge amount of HP), Key Spam, NPC Abuse, Speedhack, Spinner, Stick, Third Person
* **Movement:** Air Run, Auto Jump, Auto Ceil-Clipping, Auto Edgejump, Fastrun, Auto Jumpbug, Auto Selfsink, Auto Strafer, Auto Wallstrafe, Use Key (Change the slowdown cap when holding +use input)
* **HUD:** Chat Colors, Chat History (Source-like), Grenade Timer, Remap Colors, Speedometer (BXT), Custom Vote Popup
* **Visual:** Crosshair, ESP, Friends List, Hit Markers, Ignore Unicode (Linux-only), No Fade, No Shake, No View Entity, Player Push Direction, Player Sight Direction, Projectile Prediction, Radar
* **Render:** BSP, Chams / Glow, Draw Entities, Fog, Frame Skipper, Lightmap, Models Replacement, Skybox Replacement, Viewmodel Tweaks, Wallhack
* **Exploit:** Action Burst, Air Stuck, Fake Lag
* **Misc:** Anti Slowhack, Anti Sound Spam (Studio Events), Bypass Cvar Query, Game Patches (Tertiary Attack Glitch / Unlock 31 FPS at map loading), Ignore Different Map Versions, Models Downloader (Autodownload missing player models), Mute Manager (Useless since 5.26), No Sleep (When the game is minimized), Private Chat (Encrypted messages, use chat command: /pm your text goes next), Soundcache (Saves downloaded soundcache to enter the server faster), Votebot
* **Speedrun Tools:** HUD, Landing Prediction, Legit Mode, Player Hulls Visualization, Revive Area Visualization, Revive Boost Info, Revive Info, Timer, Timescale, Usables Visualization
* **Shaders:** Bloom, Chromatic Aberration, Color Correction, Depth Buffer, DoF Blur, Menu Background Blur, Motion Blur, SSAO, Vignette
* **Configs Save & Auto Load**
* **Menu Settings & Customization**

### Console Variables/Commands
Type in the console the following command: `sc_print_cvars all`.
The command above will print information about each ConVar/ConCommand that belongs to the mod. Use the console command `help <cvarname>` to get detailed information about the given cvar/cmd.

For example, you can bind auto jump feature: 'bind F3 sc_autojump'

### Launch Parameters
* **-sint_noconsole**: disable debug console (enabled by default) [Windows only]
* **-sint_keepconsole**: prevent debug console from closing [Windows only]
* **-sint_noshaders**: disable shaders
* **-sint_nocheckupdates**: disable updates checking

### Building on Windows
* Option 1: Visual Studio
1. Open svenint.sln in Visual Studio
2. Switch build configuration to Release
3. Compile the project

* Option 2: CMake (Quick Build)
1. Run the script:
```
build_win.bat
```

### Building on Linux (Debian/Ubuntu)
1. Install dependencies
```
sudo dpkg --add-architecture i386
sudo apt-get update
sudo apt-get install -y cmake gcc g++ gcc-multilib g++-multilib libglu1-mesa-dev:i386 libsdl2-dev:i386 libsdl2-2.0-0:i386
```
2. Build the project in Release configuration
```
./build_linux
```