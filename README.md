# Odljev   
*("drain" in Bosnian)*   

Welcome to Odljev's repository!

## Table of contents

  * [About](#about)
  * [Features](#features)
  * [Building](#building)
  * [Plans](#plans)
  * [Notes](#notes)
  
## About

Odljev is a small techno-stealth game set in 80s Yugoslavia. This is where you can access its source code.    
Its code is based on eXistence's fhDoom, which you should totally check out: [fhDoom](https://github.com/eXistence/fhDOOM)   

## Features

Some of the most significant changes include:
  * Blending of up to 3 materials per model, with a couple of new parameters for material defs
  * One-pass PBR shader (no more 2004 Phong shading with specular maps, now you gotta use roughness (R channel), metallic (G channel), and AO (B channel))
  * Interactive computer terminals, which you can actually type into
  * "Use" functionality (e.g. press F to open a door in front of you)
  * Leaning left-right
  * Ability to pick up physical objects like boxes, ragdolls etc.
  * Text display entity that scans in characters (think of CoD4: MW intro mission text)
  * Sliding
  * Cool view bobbing stuff
  * [LibreCoop](https://github.com/Stradex/H.A.S.T.E.) integration, also present in the game [H.A.S.T.E.](https://www.indiedb.com/games/haste-a-free-to-play-fps)

## Building

Dependencies:
  * Visual Studio 2013 or Visual Studio 2015 or Visual Studio 2017. Community versions are just fine, but VS2013 needs MBCS-Addon.
  * cmake 3.2+ (make sure cmake.exe is in your PATH)
  * Windows 8.1 SDK
  * optional: Qt 5.4 (32bit)
  * optional: Maya 2011 SDK (32bit)

Setup:
  * Clone repository
  * Run `cmake_msvc201x.cmd` or use CMake manually to generate 
    * if you want to build with Qt tools enabled, there are two ways to tell cmake where Qt is installed: 
      * run `cmake_msvc201x.cmd -DQTDIR=<QTDIR>`
      * or set an env var in windows: `QT_MSVC201x_X86=<QTDIR>`
      * `<QTDIR>` must point to your Qt installation so that `<QTDIR>/bin` contains all the Qt binaries (dll and exe files).
      * Please keep in mind that fhDOOM needs a 32bit build of Qt
  * Compile fhDOOM from Visual Studio
    * Debug and Run fhDOOM directly from Visual Studio
    * fhDOOM will pick up the base directory in your repository automatically
  * Generating distributable zip files: There are three special build targets that generate distributable zip files:
    * `dist`: generates a zip file that contains fhDOOM and all required files from the official 1.31 patch (this is what is usually released)
    * `dist_nopatch`: same as `dist` but without the files from the 1.31 patch
    * `sdk`: generates a SDK to build only a game dll for fhDOOM (this is currently not used, not sure if its still working)

## Plans

A couple of the bigger plans for the future include:
  * Physics: integrate Bullet physics library
  * Tools: make changes to the level editor (likely an external level editor), eventually making changes to how map/level geometry works
  * Graphics and rendering: real-time cubemap, image-based lighting, volumetric lighting effect, nice-looking water
  * Model parsing: add support for FBX
  * Build: x64 support
  * Entities: Source-like I/O support, improved vehicles etc.
  * Write some damn DOCUMENTATION <-<

## Notes

Compatibility is broken with vanilla Doom 3, last time I checked.
Sorry, but this fork is meant for standalone game development, and it'd very likely just look a bit out of place for D3 (considering the view bobbing, the ability to pick up objects etc.).   

I didn't test this with Linux, so I definitely don't know how it's going to work. But, I haven't done any Windows-specific stuff in there, so it should be fine.

## cvars

Odljev added a couple of CVars:

  * g_infiniSlide <0|1>: Turns the infinite sliding bug on or off, requires server restart
  * Various view bobbing CVars:
    * v_viewSwayYaw<float>
    * v_viewSwayPitch<float>
    * v_viewJumpAngleFactor<float>
    * v_viewSwayCycleY<float>
    * v_viewSwayCycleX<float>
    * v_viewCycleB<float>
    * v_viewCycleA<float>
    * v_weaponSwayYaw<float>
    * v_weaponSwayPitch<float>
    * v_weaponYawPushByRoll<float>
    * v_weaponYawPush<float>
    * v_weaponSprintPulldown<float>
    * v_weaponJumpPush<float>
    * v_weaponJumpPitch<float>
    * v_weaponMoveRollSide<float>
    * v_rollSide<float>
    * v_crouchPushFr<float>
    * v_crouchPushUp<float>
    * v_weaponSwayUp<float>
    * v_weaponSwaySide<float>
    * v_weaponSwayForward<float>
  * net_clientCoopDebug<0|1>: Enables debugging for the co-op mode

fhDOOM added and changed a couple of cvars. This list of cvars might be interesting:

  * r_mode <0..15>: set resolution and aspect ratio. Resolution can also be set via GUI.
    * 0..8: (unchanged, 4:3)
    * 9: 1280x720 (16:9)
    * 10: 1366x768 (16:9)
    * 11: 1440x900 (16:10)
    * 12: 1600x900 (16:9)
    * 13: 1680x1050 (16:10)
    * 14: 1920x1080 (16:9)
    * 15: 1920x1200 (16:10)
  * r_shadows <0|1|2>: default shadow mode, can also be selected via GUI.
    * 0: force shadows off
    * 1: use stencil shadows
    * 2: use shadow mapping
  * r_specularScale <float>: scale specularity globally
  * r_specularExp <float>: sharpness of specular reflections
  * r_shading <0|1>: shading model
    * 0: Blinn-Phong
    * 1: Phong
  * r_pomEnabled <0|1>: enable/disable parallax occlusion mapping, requires special specular maps with height information in alpha channel
  * r_pomMaxHeight <float>: adjust max displacement
  * r_smBrightness <float>: adjust default brightness of soft shadows
  * r_smSoftness <float>: scale default softness of soft shadows (higher values will lead to artifacts)
  * r_smPolyOffsetFactor <float>: depth-offset of shadow maps (increasing this will fix artifacts from high softness, but will also increase light bleeding)
  * r_smForceLod <-1|0|1|2>: set lod (quality/size) of shadow maps for all lights
    * -1: choose dynamically based on light size and distance from viewer
    * 0: high quality (1024x1024)
    * 1: mid quality (512x512)
    * 2: low quality (256x256)
  * r_smLodBias <0|1|2>: apply lod bias to lights to reduce quality without forcing all lights to the same lod.
  * r_smUseStaticOcclusion <0|1>: enable/disable static occlusion geometry from ocl files
  * r_glCoreProfile <0|1>: enable/disable opengl core profile (requires full restart of the game)
  * r_glDebugOutput <0|1|2>: OpenGL debug messages, requires core profile
    * 0: No debug message
    * 1: async debug messages
    * 2: sync debug messages   
  * con_fontScale <float>: scale font size in console
  * con_size <float>: scale console (not immediately visible, close and re-open the console to see it)
  * com_showFPS <0|1|2|3>: 0=Off, 1=FPS, 2=ms, 3=FPS+ms
  * com_showBackendStats <0|1>: show various backend perf counters
  * g_projectileLightLodBias <0|1|2>: reduce shadow quality from projectile lights, usually not noticable
  * g_muzzleFlashLightLodBias <0|1|2>: reduce shadow quality from muzzle flashes, usually not noticable
