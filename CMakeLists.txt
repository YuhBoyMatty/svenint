cmake_minimum_required(VERSION 3.3.2)

project(SvenInt)

set(SVENINT_DLL "svenint")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(SVENINT_SOURCE_FILES
	dllmain.cpp
	gamedata.cpp
	stdafx.cpp

	utils/GOST_28147.c
	utils/detours.cpp
	utils/memalloc.cpp
	utils/memory_utils.cpp
	utils/menu_styles.cpp
	utils/prof.cpp

	udis86/src/decode.c
	udis86/src/itab.c
	udis86/src/syn.c
	udis86/src/syn-att.c
	udis86/src/syn-intel.c
	udis86/src/udis86.c

	modules/client.cpp
	modules/config.cpp
	modules/hooks.cpp
	modules/menu.cpp
	modules/opengl.cpp
	modules/scripts.cpp
	modules/server.cpp

	glew/src/glew.c

	imgui/imgui.cpp
	imgui/imgui_draw.cpp
	imgui/imgui_tables.cpp
	imgui/imgui_widgets.cpp
	imgui/backends/imgui_impl_opengl3.cpp

	strafe/strafe.cpp

	game/CBaseEngineClient.cpp
	game/CBaseExtraPlayerInfo.cpp
	game/CBasePlayerMove.cpp
	game/CBaseWeaponsResource.cpp
	game/commandline.cpp
	game/convar.cpp
	game/cvar.cpp
	game/dbg.cpp
	game/game_hooks.cpp
	game/game_utils.cpp
	game/hook_events.cpp
	game/keyvalues_parser.cpp
	game/localplayer.cpp
	game/mathlib.cpp
	game/messagebuffer.cpp

	scripts/lua/lapi.c
	scripts/lua/lauxlib.c
	scripts/lua/lbaselib.c
	scripts/lua/lcode.c
	scripts/lua/lcorolib.c
	scripts/lua/lctype.c
	scripts/lua/ldblib.c
	scripts/lua/ldebug.c
	scripts/lua/ldo.c
	scripts/lua/ldump.c
	scripts/lua/lfunc.c
	scripts/lua/lgc.c
	scripts/lua/linit.c
	scripts/lua/liolib.c
	scripts/lua/llex.c
	scripts/lua/lmathlib.c
	scripts/lua/lmem.c
	scripts/lua/loadlib.c
	scripts/lua/lobject.c
	scripts/lua/lopcodes.c
	scripts/lua/loslib.c
	scripts/lua/lparser.c
	scripts/lua/lstate.c
	scripts/lua/lstring.c
	scripts/lua/lstrlib.c
	scripts/lua/ltable.c
	scripts/lua/ltablib.c
	scripts/lua/ltests.c
	scripts/lua/ltm.c
	scripts/lua/lua.c
	scripts/lua/lundump.c
	scripts/lua/lutf8lib.c
	scripts/lua/lvm.c
	scripts/lua/lzio.c

	scripts/lua_cvar.cpp
	scripts/lua_debug.cpp
	scripts/lua_entity_dictionary.cpp
	scripts/lua_entity_vars.cpp
	scripts/lua_global_vars.cpp
	scripts/lua_input_manager.cpp
	scripts/lua_logic.cpp
	scripts/lua_mod.cpp
	scripts/lua_player_move.cpp
	scripts/lua_random.cpp
	scripts/lua_triggers.cpp
	scripts/lua_usercmd.cpp
	scripts/lua_vector.cpp
	scripts/scripts_binding.cpp

	features/features.cpp
	features/exploit_action_burst.cpp
	features/exploit_airstuck.cpp
	features/exploit_fakelag.cpp
	features/hud_chat_colors.cpp
	features/hud_chat_history.cpp
	features/hud_grenade_timer.cpp
	features/hud_remap_colors.cpp
	features/hud_speedometer.cpp
	features/hud_vote_popup.cpp
	features/misc_antisoundspam.cpp
	features/misc_bypass_cvar_query.cpp
	features/misc_clientmenu.cpp
	features/misc_demo_capture.cpp
	features/misc_demo_message.cpp
	features/misc_edge_pixels_player.cpp
	features/misc_entity_list.cpp
	features/misc_game_patches.cpp
	features/misc_ignore_different_maps.cpp
	features/misc_message_spammer.cpp
	features/misc_models_downloader.cpp
	# features/misc_mute_manager.cpp
	features/misc_nosleep.cpp
	features/misc_private_chat.cpp
	features/misc_random_generator.cpp
	features/misc_soundcache.cpp
	features/misc_votebot.cpp
	features/mov_airrun.cpp
	features/mov_autojump.cpp
	features/mov_ceilclip.cpp
	features/mov_ducktap.cpp
	features/mov_edgejump.cpp
	features/mov_fastrun.cpp
	features/mov_jumpbug.cpp
	features/mov_selfsink.cpp
	features/mov_strafer.cpp
	features/mov_use_key.cpp
	features/mov_wallstrafe.cpp
	features/player_aim.cpp
	features/player_antiafk.cpp
	features/player_autoreload.cpp
	features/player_camhack.cpp
	features/player_color_pulsator.cpp
	features/player_dynamic_glow.cpp
	features/player_firstperson_roaming.cpp
	features/player_flashlight.cpp
	features/player_freeze.cpp
	features/player_gib_abuse.cpp
	features/player_keyspam.cpp
	features/player_lookat.cpp
	features/player_npc_abuse.cpp
	features/player_silent_angles.cpp
	features/player_speedhack.cpp
	features/player_spinner.cpp
	features/player_stick.cpp
	features/player_thirdperson.cpp
	features/r_bsp.cpp
	features/r_chams.cpp
	features/r_drawing.cpp
	features/r_draw_context.cpp
	features/r_draw_entities.cpp
	features/r_fog.cpp
	features/r_frame_skipper.cpp
	features/r_lightmap.cpp
	features/r_models_replacement.cpp
	features/r_skybox.cpp
	features/r_viewmodel_tweaks.cpp
	features/r_wallhack.cpp
	features/shader_bloom.cpp
	features/shader_chromatic_aberration.cpp
	features/shader_color_correction.cpp
	features/shader_depth_buffer.cpp
	features/shader_dof_blur.cpp
	features/shader_menu_background_blur.cpp
	features/shader_motion_blur.cpp
	features/shader_ssao.cpp
	features/shader_vignette.cpp
	features/st_aim_angles.cpp
	features/st_hud.cpp
	features/st_input_manager.cpp
	features/st_landing_prediction.cpp
	features/st_legit_mode.cpp
	features/st_player_hulls.cpp
	features/st_revive_area.cpp
	features/st_revive_boost_info.cpp
	features/st_revive_info.cpp
	features/st_timer.cpp
	features/st_timescale.cpp
	features/st_usables.cpp
	features/visual_crosshair.cpp
	features/visual_esp.cpp
	features/visual_friends.cpp
	features/visual_hitmarkers.cpp
	features/visual_nofade.cpp
	features/visual_noshake.cpp
	features/visual_noviewentity.cpp
	features/visual_player_push_direction.cpp
	features/visual_player_sight_direction.cpp
	features/visual_projectile_prediction.cpp
	features/visual_radar.cpp
)

include_directories(./)
include_directories(glew/include)
include_directories(imgui)

add_library(${SVENINT_DLL} SHARED ${SVENINT_SOURCE_FILES})

if (MSVC)
	target_link_libraries(${SVENINT_DLL} OpenGL32)
	target_sources(${SVENINT_DLL} PRIVATE imgui/backends/imgui_impl_win32.cpp)
else()
	# find_library(SDL2_LIB
	# 	NAMES SDL2
	# 	PATHS "${CMAKE_CURRENT_SOURCE_DIR}/SDL/lib/x86"
	# 	NO_DEFAULT_PATH
	# )

	link_directories(${CMAKE_CURRENT_SOURCE_DIR}/SDL2/lib/x86) # I compiled x64 build for Linux lol

	target_link_libraries(${SVENINT_DLL} dl GL GLX pthread SDL2 ${SDL2_LIB})
	target_sources(${SVENINT_DLL} PRIVATE imgui/backends/imgui_impl_sdl2.cpp features/visual_ignore_unicode.cpp)

	add_definitions(-DSDL_PROTOTYPES_ONLY)
	add_definitions(-DIMGUI_USE_SDL)
	#add_definitions(-DSINT_USE_GLEW)

	include_directories(SDL2/include)
endif()

add_definitions(-D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE -D_WINSOCK_DEPRECATED_NO_WARNINGS)

target_compile_definitions(${SVENINT_DLL} PRIVATE
	GLEW_STATIC
	LIBUDIS86_EXPORTS
	SVENINT_EXPORTS
)

if (MSVC)
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /DNDEBUG /GR-")
	set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
else()
	set(CMAKE_C_FLAGS "-m32 -s -fpic -std=c++14 -w")
	set(CMAKE_CXX_FLAGS "-m32 -s -fpic -std=c++14 -w")
	set(CMAKE_SHARED_LINKER_FLAGS "-m32 -s -fpic -std=c++14 -w")
	set(CMAKE_STATIC_LINKER_FLAGS "-m32 -s -fpic -std=c++14 -w")
	set(CMAKE_EXECUTABLE_LINKER_FLAGS "-m32 -s -fpic -std=c++14 -w")

	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} -O3 -s -DNDEBUG -fpic -fpermissive -Wl,--strip-all -fvisibility=hidden")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS} -O3 -s -DNDEBUG -fpic -fpermissive -Wl,--strip-all -fvisibility=hidden")
	set(CMAKE_CXX_FLAGS_DEBUG "-Os -fpic -fpermissive")
endif()
