## 1.5.2 - 2017-08-16
* fixed ambient lights (thx raynorpat for contributing some code!)
* fixed black objects: some objects (mostly rag dolls) appeared completely black until beeing moved the first time.
* unified texture loading
    * all texture loading (uncompressed textures, compressed textures, cube maps, image programs) goes through the same code path
    * all textures are now immutable
    * added support for cubemaps from dds files
    * added support for RGTC/3Dc-compressed normal maps
    * channels (red/alpha) of tga normal maps are no longer swapped at load time    
* fixed several effects for RoE (some things are still missing for full RoE support, but at least its playable now)
* improved linux support
    * fixed core profile context 
    * fixed several compilation issues (compiles now fine with gcc 6.2 and clang 4.0)
    * fixed some keyboard layout issues
    * added shell script to install required build tools on an ubuntu system
* fixed light/shadow flickering (in certain scenarios everything in a lights range seemed to be completely shadowed or not, depending on your view point and direction)
* fixed vid_restart, reloadEngine and loading of mods
* fixed a few minor issues with Visual Studio 2017 (compiles now fine with VS2013, VS2015 and VS2017)

## 1.5.1 (alpha) - 2016-06-27
* AMD texturing issues solved (r_amdWorkaround is gone)
* Lots of performance improvements
	* optimized draw calls and state changes
	* use a 4k*4k dynamic shadow map atlas to batch shadow maps
	* modified dmap to generate optimized occlusion geometry for shadow map rendering
		* occlusion geometry is stored in a separate *.ocl file (*.map and *.proc are still the same)
		* ocl file optional, maps without ocl file work just fine (at the expense of performance)
		* dmap option 'exportObj' will export occlusion geometry to obj files (i needed that for debugging, not sure whether it is useful to others)
		* fhDOOM release includes ocl files for all maps of the original game
	* more aggressive culling
	* performance improvements are huge on AMD (on nVidia the improvements are not that significant, but still noticeable).
* fixed (time)demo rendering
* implemented shadow mapping for parallel lights
	* uses cascade shadow maps
	* not final, there are still some things to improve (flickering, light bleeding)
* improved shadow filtering
	* poisson filtering
	* softness is constant across the light range, making shadows alot easier to tweak
* fixed some smaller editor issues
* changed required OpenGL version to 3.3 (core profile)
	*will not necessarily stay at 3.3, future releases may require 4.x again (depends on what exactly i will work on)
	* still requires GLSL extension "GL_ARB_shading_language_420pack" (should be available with all modern drivers even on older hardware, at least on AMD and nVidia)
* added soft-particles and soft-shadow-quality options to main menu
* removed cvars:
	* r_amdWorkaround
	* r_smQuality (replaced by r_smForceLod)
* new/modified cvars:
	* r_smForceLod (same as previous r_smQuality, forces a certain lod on all lights)
	* r_smLodBias (increase lod on all lights)
	* r_smUseStaticOcclusion (enable/disable static occlusion geometry from ocl files)
	* com_showFPS (0=Off, 1=FPS, 2=ms, 3=FPS+ms)
	* com_showBackendStats (show various backend perf counters)
	* g_projectileLightLodBias (reduce shadow quality from projectile lights, usually not noticable)
	* g_muzzleFlashLightLodBias (reduce shadow quality from muzzle flashes, usually not noticable)	

## 1.5.0 (alpha) - 2016-04-10
* CMake build system
* Compiles fine with Visual Studio 2013 (you need to have the MFC Multibyte Addon installed) and Visual Studio 2015
* Replaced deprecated DirectX SDK by Windows 8.1 SDK 
* Use GLEW for OpenGL extension handling (replaced all qgl\* calls by normal gl\* calls)
* Contains all game assets from latest official patch 1.31
* OpenGL 4.3 core profile  
	* Fixed function and ARB2 assembly shaders are completely gone
	* Re-wrote everything with GLSL  
	* Game and Tools!
* Soft shadows via shadow mapping
	* Alpha-tested surfaces can cast shadows 
	* Configurable globally as a default (useful for original maps) and for each light individually
	* Softness
	* Brightness
	* Shadow mapping and Stencil Shadows can be freely mixed
* Soft particles
* Parallax occlusion mapping (expects height data in the alpha channel of the specular map)
* Added support for Qt based tools
* optional (can be excluded from the build, so you are not forced to install Qt to build fhDOOM)
* Implemented some basic helpers
	* RenderWidget, so the engine can render easily to the GUI
	* several input widgets for colors, numbers and vectors
	* utility functions to convert between Qt and id types (Strings, Colors, etc.)
	* re-implemented the light editor as an example (+some improvements)
		* useable ingame and from editor/radiant
		* cleaned up layout 
		* removed unused stuff
		* added advanced shadow options (related to shadow mapping)
		* show additional material properties (eg the file where it is defined)
* Several smaller changes
	* Detect base directory automatically by walking up current directory path
	* Added new r_mode's for HD/widescreen resolutions (r_mode also sets the aspect ratio)
	* Set font size of console via con_fontScale
	* Set size of console via con_size
	* Minor fixes and cleanups in editor code (e.g. fixed auto save, removed some unused options)
	* Renamed executable and game dll (the intention was to have fhDOOM installed next to the original doom binaries. Not sure if that still works)
	* Moved maya import stuff from dll to a stand-alone command line tool (maya2md5)
	* Compile most 3rd party stuff as separate libraries
	* Fixed tons of warnings	
* new/modified cvars:
	* image_usePrecompressedTextures is 0 by default, so HD texture packs (like Wulfen) can be used more easily.
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
	* r_smQuality <-1|0|1|2>: set quality/size of shadow maps
		* -1: choose dynamically based on light size and distance from viewer
		* 0: high quality (1024x1024)
		* 1: mid quality (512x512)
		* 2: low quality (256x256)
	* r_glCoreProfile <0|1>: enable/disable opengl core profile (requires full restart of the game)
	* r_glDebugOutput <0|1|2>: OpenGL debug messages, requires core profile
		* 0: No debug message
		* 1: async debug messages
		* 2: sync debug messages   
	* con_fontScale <float>: scale font size in console
	* con_size <float>: scale console (not immediately visible, close and re-open the console to see it)
	* r_amdWorkaround <0|1|2>: enable temporary workaround for AMD hardware
		* 0: workaround disabled
		* 1: workaround enabled only on AMD hardware
		* 2: workaround always enabled  
