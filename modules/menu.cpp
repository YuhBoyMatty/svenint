#include <vector>
#include <algorithm>

#include <sys.h>
#include <dbg.h>
#include <convar.h>
#include <ISvenModAPI.h>

#include "opengl.h"
#include "menu.h"

#include "imgui.h"
#include "imgui_impl_win32.h"

#if IMGUI_USE_GL3
#include "imgui_impl_opengl3.h"
#else
#include "imgui_impl_opengl2.h"
#endif

#include "../features/models_manager.h"
#include "../features/thirdperson.h"
#include "../features/menu_colors.h"

#include "../utils/menu_styles.h"
#include "../utils/menu_fonts.hpp"
#include "../config.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"

#ifdef SVENINT_OBFUSCATE

#ifndef xs
#include "../utils/xorstr.h"
#endif

#else

#ifdef xs
#undef xs
#define (_str) (_str)
#else
#define (_str) (_str)
#endif

#endif

// Types of fonts
#define FONT_SVENINT ( 0 )
#define FONT_L4D2 ( 1 )

// Sizes of fonts
#define FONT_BIG ( 0 )
#define FONT_SMALL ( 1 )
#define FONT_DEFAULT ( 2 )

extern uint64 g_ullSteam64ID;

//-----------------------------------------------------------------------------
// Signatures
//-----------------------------------------------------------------------------

DECLARE_HOOK(BOOL, APIENTRY, wglSwapBuffers, HDC);
DECLARE_HOOK(BOOL, WINAPI, SetCursorPos, int, int);

LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

CMenuModule g_MenuModule;

int g_iMenuState = 0;

bool g_bMenuEnabled = false;
bool g_bMenuClosed = true;

float g_flMenuOpenTime = -1.f;
float g_flMenuCloseTime = -1.f;

//-----------------------------------------------------------------------------
// Static vars
//-----------------------------------------------------------------------------

static ImFont *pMenuFonts[][3] =
{
	{ NULL, NULL, NULL }, // SvenInt
	{ NULL, NULL, NULL } // L4D2
};

static CCommand s_DummyCommand;

static HWND hGameWnd = NULL;
static WNDPROC hGameWndProc = NULL;

static int selectedTab = 0, selectedSubTab0 = 0, selectedSubTab1 = 0, selectedSubTab2 = 0, selectedSubTab3 = 0, selectedSubTab4 = 0;

// Tabs Strings Vars

static std::string MainTabNames[] = { (ICON_FA_EYE "  Visuals"), (ICON_FA_CROSSHAIRS "  HUD"), (ICON_FA_GLOBE "  Utility"), (ICON_FA_SAVE "  Configs"), (ICON_FA_COG "  Settings") };

static std::string VisualsSubTabs[] = { ("Render"), ("ESP"), ("Chams"), ("Glow"), ("Flashlight"), ("Wallhack"), ("BSP"), ("Models Manager"), ("Shaders"), ("Misc") };

static std::string HUDSubTabs[] = { ("General"), ("Speedometer"), ("Radar"), ("Chat Colors"), ("Custom Vote Popup") };

static std::string UtilitySubTabs[] = { ("Player"), ("Color Pulsator"), ("Fake Lag"), ("Anti-AFK"), ("Spammer"), ("Speedrun Tools"), ("Misc") };

static std::string ConfigsSubTab[] = { ("SvenInt") };

static std::string SettingsSubTab[] = { ("Menu"), ("Game") };

// Features Strings Vars

static obfuscated_string draw_entities_items[] =
{
	("0 - Default"),
	("1 - Draw Bones"),
	("2 - Draw Hitboxes"),
	("3 - Draw Model & Hitboxes"),
	("4 - Draw Hulls"),
	("5 - Draw Players Bones"),
	("6 - Draw Players Hitboxes"),
	("7 - Don't Draw Player Models")
};

static obfuscated_string esp_style[] = { ("0 - Default"), ("1 - SAMP"), ("2 - Left 4 Dead") };
static obfuscated_string esp_process_items[] = { ("0 - Everyone"), ("1 - Entities"), ("2 - Players") };
static obfuscated_string esp_box_items[] = { ("0 - Off"), ("1 - Default"), ("2 - Coal"), ("3 - Corner") };

static obfuscated_string chams_items[] = { ("0 - Disable"), ("1 - Flat"), ("2 - Texture"), ("3 - Material") };

static obfuscated_string glow_items[] = { ("0 - Disable"), ("1 - Glow Outline"), ("2 - Glow Shell"), ("3 - Ghost") };

static obfuscated_string ca_types[] = { ("0 - Default"), ("1 - Barrel Distortion"), ("2 - Linear Barrel Distortion") };

static obfuscated_string dof_interps[] = { ("0 - Linear"), ("1 - Simple Spline"), ("2 - Parabolic"), ("3 - Parabolic Inverted"), ("4 - Cubic") };

static obfuscated_string strafe_dir_items[] = { ("0 - To the left"), ("1 - To the right"), ("2 - Best strafe"), ("3 - View angles") };
static obfuscated_string strafe_type_items[] = { ("0 - Max. acceleration"), ("1 - Max. angle"), ("2 - Max. deceleration"), ("3 - Const speed") };

static obfuscated_string fakelag_type_items[] = { ("0 - Dynamic"), ("1 - Maximum"), ("2 - Jitter"), ("3 - Break Lag Compensation") };
static obfuscated_string fakelag_move_items[] = { ("0 - Everytime"), ("1 - On Land"), ("2 - On Move"), ("3 - In Air") };

static obfuscated_string antiafk_items[] = {
	("0 - Off"),
	("1 - Step Forward & Back"),
	("2 - Spam Gibme"),
	("3 - Spam Kill"),
	("4 - Walk Around & Spam Inputs"),
	("5 - Walk Around"),
	("6 - Go Right") };

static obfuscated_string trace_type[] = { ("0 - Trace Line"), ("1 - Trace Hull") };

static obfuscated_string radar_type[] = { ("0 - Round"), ("1 - Square") };

static obfuscated_string revive_boost_direction_type[] = { ("0 - Box"), ("1 - Line") };

static obfuscated_string theme_items[] =
{
	("SvenInt"),
	("Dark"),
	("Light"),
	("Classic"),
	("Berserk"),
	("Deep Dark"),
	("Carbon"),
	("Corporate Grey"),
	("Grey"),
	("Dark Light"),
	("Soft Dark"),
	("Gold & Black"),
	("Monochrome"),
	("Pink"),
	("Half-Life"),
	("Sven-Cope"),
	("Custom"),
};

static obfuscated_string font_items[] =
{
	("SvenInt"),
	("Left 4 Dead 2")
};

static obfuscated_string tool_tips_visuals[] =
{
	("- Draw Entities: Changes drawing method of all entities or only players\n"
	"- Light Map: Overrides entire game lightmap, basically a customizable fullbright\n"
	"- Skip Frames: Skips game frames on a set interval, can boost or lower your FPS\n"),
	("Displays entities information like Name, HP, Armor, etc. with fully customizable look"),
	("As short for Chameleon, Chams are a method of wallhacks but with colors!"),
	("Fully customizable entities/players animated glow!"),
	("Shows shape and position of spawns and any selected game trigger from the list with customizable look"),
	("- Depth Buffer: Shows the depth buffer of the rendered scene\n"
	"- SSAO: Shading technique that emulates better lightning and shadows.\n"
	"- Color Correction: Alterates the color balance and more of the game to achieve a desired effect\n"
	"- Chromatic aberration: Screen-based method that mimics lens distortions\n"
	"- Vignette: Effect that simulates the darkening like real-world camera lenses\n"
	"- Blur: Visual effect that adds a blur to objects in motion, customizable with different types of blur")
};

static obfuscated_string tool_tips_hud[] =
{
	("Feature that lets you change the color of nickname of players when they write something in the chat"),
	("Replaces the legacy vote pop-up with a better one and prevents the vote from stopping player movement")
};

static obfuscated_string tool_tips_utility[] =
{
	("- Auto Wallstrafing: Automatically moves and strafes for you near any wall\n"
	"- Strafer: Most comfy feature, automatically strafes for you while bunny hopping, just jump!"),
	("Automatically loops through your player model colors by the given delay")
};
//-----------------------------------------------------------------------------
// Functions
//-----------------------------------------------------------------------------

// Simple helper function to load an image into a OpenGL texture with common settings
bool CMenuModule::LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
	// Load from file
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
	if (image_data == NULL)
		return false;
	
	// Create a OpenGL texture identifier
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

	// Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	stbi_image_free(image_data);

	*out_texture = image_texture;
	*out_width = image_width;
	*out_height = image_height;

	return true;
}

void CMenuModule::LoadTextures()
{
	// This is a mess... but im too stupid to figure out another way | xd don't worry same for me
	std::string bPath = SvenModAPI()->GetBaseDirectory();

	std::string sint_logo = ("\\sven_internal\\images\\logo.png");
	std::string sint_logo_fPath = bPath + sint_logo;

	std::string sint_image = ("\\sven_internal\\images\\menu_image.png");
	std::string sint_image_fPath = bPath + sint_image;

	bool bLogoTextureLoaded = LoadTextureFromFile( sint_logo_fPath.c_str(), &m_hLogoTex, &m_iLogoWidth, &m_iLogoHeight );

	if ( !bLogoTextureLoaded )
	{
		Warning(("[SvenInt] Failed to load ImGui logo\n"));
	}

	bool bMenuTextureLoaded = LoadTextureFromFile( sint_image_fPath.c_str(), &m_hMenuTex, &m_iMenuTexWidth, &m_iMenuTexHeight );

	if ( ( !bMenuTextureLoaded ) || !( m_iMenuTexWidth == 130 && m_iMenuTexHeight == 248 ) )
	{
		if ( bMenuTextureLoaded )
		{
			Warning( ( "[SvenInt] Cannot load SvenInt menu image, make sure size is 130x248\n" ) );
		}
		else
		{
			Warning( ( "[SvenInt] Failed to load SvenInt menu image\n" ) );
		}

		DeleteTextures();

		m_bMenuTexLoaded = false;
	}

	m_bMenuTexLoaded = true;
}

void CMenuModule::DeleteTextures()
{
	if ( m_hLogoTex )
	{
		glDeleteTextures( 1, &m_hLogoTex );
		m_hLogoTex = 0;
	}

	if ( m_hMenuTex )
	{
		glDeleteTextures( 1, &m_hMenuTex );
		m_hMenuTex = 0;
	}
}

void CMenuModule::SelectCurrentFont()
{
	m_pMenuFontBig = pMenuFonts[g_Config.cvars.menu_font][FONT_BIG];
	m_pMenuFontSmall = pMenuFonts[g_Config.cvars.menu_font][FONT_SMALL];
	m_pMenuFontDefault = pMenuFonts[g_Config.cvars.menu_font][FONT_DEFAULT];
}

void CMenuModule::LoadFonts() // Once initialized
{
	ImGuiIO& io = ImGui::GetIO();

	const float svenint_font_sizes[3] = { 24.f, 17.f, 18.f };
	const float l4d2_font_sizes[3] = { 20.f, 13.5f, 15.f };

	// Load SvenInt font
	LoadFont( FONT_SVENINT, CoolFont, sizeof(CoolFont), svenint_font_sizes );
	
	// Load L4D2 font
	LoadFont( FONT_L4D2, L4D2Font, sizeof(L4D2Font), l4d2_font_sizes );
	
	// Select a font from config
	SelectCurrentFont();
}

void CMenuModule::LoadFont(int type, const void *pFont, int iFontSize, const float fontSizes[3])
{
	ImFontConfig bigFont, smallFont, defaultFont;

	ImGuiIO &io = ImGui::GetIO();

	bigFont.FontDataOwnedByAtlas = false;
	smallFont.FontDataOwnedByAtlas = false;
	defaultFont.FontDataOwnedByAtlas = false;

	pMenuFonts[type][FONT_BIG] = io.Fonts->AddFontFromMemoryTTF( (void *)pFont, iFontSize, fontSizes[FONT_BIG], &bigFont );
	pMenuFonts[type][FONT_SMALL] = io.Fonts->AddFontFromMemoryTTF( (void *)pFont, iFontSize, fontSizes[FONT_SMALL], &smallFont );
	pMenuFonts[type][FONT_DEFAULT] = io.Fonts->AddFontFromMemoryTTF( (void *)pFont, iFontSize, fontSizes[FONT_DEFAULT], &defaultFont );

	MergeIconsToCurrentFont();
}

void CMenuModule::MergeIconsToCurrentFont()
{
	static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };

	ImFontConfig icons_config;

	ImGuiIO &io = ImGui::GetIO();

	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.FontDataOwnedByAtlas = false;

	io.Fonts->AddFontFromMemoryTTF( (void *)fontAwesome, sizeof(fontAwesome), 18, &icons_config, icons_ranges );
}

// Restores window style
void CMenuModule::WindowStyle()
{
	m_pStyle->WindowRounding = 5;
	m_pStyle->ChildRounding = 8;
	m_pStyle->FrameRounding = 5;
	m_pStyle->GrabRounding = 5;
	m_pStyle->PopupRounding = 5;
	m_pStyle->FrameRounding = 5;
}

// This is needed to align icons and text on the buttons
static float ForIcon(int i)
{
	switch (i)
	{
	case 0:
		return 0.1;
	case 1:
		return 0.08;
	case 2:
		return 0.09;
	case 3:
		return 0.115;
	case 4:
		return 0.09;
	default:
		return 0;
	}
}

//-----------------------------------------------------------------------------
// Menu
//-----------------------------------------------------------------------------

void CMenuModule::Draw()
{
	if ( !m_bThemeLoaded )
	{
		LoadMenuTheme();
		WindowStyle();

		m_bThemeLoaded = true;
	}

	ImGui::GetIO().MouseDrawCursor = g_bMenuEnabled;
	ImGui::GetStyle().Alpha = g_Config.cvars.menu_opacity;

	if (g_bMenuEnabled)
	{
		// Main Window

		ImGui::SetNextWindowSize({ 800, 600 });
		ImGui::Begin(("Main"), 0, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

		if (g_Config.cvars.menu_rainbow[0] || g_Config.cvars.menu_rainbow[1])
		{
			g_MenuColors.Think();
		}

		ImGui::SetCursorPosY(0);

		ImGuiCustom.Columns(2, nullptr, true, g_Config.cvars.menu_rainbow[1] ? ImVec4(g_MenuColors.m_flRainbowColor[0], g_MenuColors.m_flRainbowColor[1], g_MenuColors.m_flRainbowColor[2], 255) : ImGui::GetStyleColorVec4(ImGuiCol_Separator));
		ImGui::SetColumnOffset(1, 130);

		// Left Side Column

		DrawLogo();

		DrawMainTabs(); // (Visuals, HUD, Utility, Configs, Settings)

		DrawMenuImage();

		DrawStats(); // Frames

		// Next Coloumn
	
		ImGui::NextColumn();

		ImGui::SetCursorPosX(134);
		ImGui::SetCursorPosY(6);

		// Sub Tabs Wrapper

		ImGui::BeginChild(("subtabs-wrapper"), ImVec2(662, 39), true, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);
		{
			ImGui::SetCursorPosY(8);

			ImGui::PushFont( m_pMenuFontSmall );

			switch (selectedTab)
			{
			case 0:
				DrawVisualsSubTabs();
				break;

			case 1:
				DrawHUDSubTabs();
				break;

			case 2:
				DrawUtilitySubTabs();
				break;

			case 3:
				DrawConfigsSubTabs();
				break;

			case 4:
				DrawSettingsSubTabs();
				break;
			}
		}

		ImGui::EndChild();

		ImGui::Spacing();

		ImGui::SetCursorPosX(134);
		ImGui::SetCursorPosY(50);

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0, 0, 0, 0));
		ImGui::BeginChild(("content-wrapper"), ImVec2(ImGui::GetContentRegionAvail().x + 7, ImGui::GetContentRegionAvail().y + 5), false);
		ImGui::PopStyleColor();

		ImGui::Columns(2, nullptr, false);
		ImGui::SetColumnOffset(1, 328.5);

		ImGui::SetCursorPosX(0);

		switch (selectedTab)
		{
		case 0:
			DrawVisualsTabContent();
			break;

		case 1:
			DrawHUDTabContent();
			break;

		case 2:
			DrawUtilityTabContent();
			break;

		case 3:
			DrawTabConfigsContent();
			break;

		case 4:
			DrawSettingsTabContent();
			break;
		}

		ImGui::EndChild();
	}
	ImGuiCustom.End();
}


// Left Column 

void CMenuModule::DrawLogo()
{
	ImGui::SetCursorPosY(10);
	ImGui::SetCursorPosX(9);

	ImGui::Image( (void *)(intptr_t)m_hLogoTex,
				  ImVec2( m_iLogoWidth, m_iLogoWidth ),
				  ImVec2( 0, 0 ),
				  ImVec2( 1, 1 ),
				  g_Config.cvars.menu_rainbow[ 0 ] ?
				  ImVec4( g_MenuColors.m_flRainbowColor[ 0 ], g_MenuColors.m_flRainbowColor[ 1 ], g_MenuColors.m_flRainbowColor[ 2 ], 1.f * g_Config.cvars.menu_opacity ) :
				  ImVec4( g_Config.cvars.logo_color[ 0 ], g_Config.cvars.logo_color[ 1 ], g_Config.cvars.logo_color[ 2 ], 1.f * g_Config.cvars.menu_opacity ) );

	ImGui::SameLine();

	ImGui::SetCursorPosY(12);
	ImGui::SetCursorPosX(50);

	ImGui::PushFont( m_pMenuFontBig );

	ImGui::TextUnformatted(("SvenInt"));

	ImGui::PopFont();

	ImGui::Spacing();
}

void CMenuModule::DrawMainTabs()
{

	ImGui::SetCursorPosX(4);

	ImGui::BeginChild(("tabs"), ImVec2(123, 232), true);

	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10);

	ImGui::PushFont( m_pMenuFontDefault );

	for (int i = 0; i < ARRAYSIZE(MainTabNames); i++)
	{
		std::string it = MainTabNames[i];
		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(ForIcon(i), 0.5));
		ImGui::PushStyleColor(ImGuiCol_Button, selectedTab == i ? m_pStyle->Colors[ImGuiCol_ButtonActive] : ImVec4(0, 0, 0, 0));
		ImGui::SetCursorPosX(7);
		if (ImGui::Button(it.c_str(), ImVec2(140, 40))) selectedTab = i;
		ImGui::PopStyleVar();
		ImGui::PopStyleColor(2);
	}

	ImGui::PopStyleVar();
	ImGui::PopFont();

	ImGui::EndChild();
}

void CMenuModule::DrawMenuImage()
{
	if ( m_bMenuTexLoaded )
	{
		ImGui::SetCursorPosX(0);
		ImGui::Image((void*)(intptr_t)m_hMenuTex, ImVec2(m_iMenuTexWidth, m_iMenuTexHeight), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.f, 1.f, 1.f, 1.f * g_Config.cvars.menu_opacity));
	}
	else
	{
		ImGui::Dummy(ImVec2(0.0f, ImGui::GetContentRegionAvail().y - 55 - m_pStyle->ItemSpacing.y));
	}
}

void CMenuModule::DrawStats()
{
	ImGui::SetCursorPosX(4);
	ImGui::BeginChild(("Frames"), ImVec2(123, 58), true);

	ImGui::PushFont( m_pMenuFontDefault );

	ImGui::Text(("%.3f ms/frame"), 1000.0f / ImGui::GetIO().Framerate);
	ImGui::Text(("%.1f FPS"), ImGui::GetIO().Framerate);

	ImGui::PopFont();

	ImGui::EndChild();
}


// Sub Tabs 

void CMenuModule::DrawVisualsSubTabs()
{
	for (int i = 0; i < ARRAYSIZE(VisualsSubTabs); i++)
	{
		std::string it = VisualsSubTabs[i];

		ImGui::PushStyleColor(ImGuiCol_Button, selectedSubTab0 == i ? m_pStyle->Colors[ImGuiCol_ButtonActive] : m_pStyle->Colors[ImGuiCol_Button]);
		if (ImGui::Button(it.c_str(), ImVec2(ImGui::CalcTextSize(it.c_str()).x + 10, 25)))
		{
			selectedSubTab0 = i;
		}
		ImGui::PopStyleColor();
		ImGui::SameLine();
	}
}

void CMenuModule::DrawHUDSubTabs()
{
	for (int i = 0; i < ARRAYSIZE(HUDSubTabs); i++)
	{
		std::string it = HUDSubTabs[i];

		ImGui::PushStyleColor(ImGuiCol_Button, selectedSubTab1 == i ? m_pStyle->Colors[ImGuiCol_ButtonActive] : m_pStyle->Colors[ImGuiCol_Button]);
		if (ImGui::Button(it.c_str(), ImVec2(ImGui::CalcTextSize(it.c_str()).x + 10, 25)))
		{
			selectedSubTab1 = i;
		}
		ImGui::PopStyleColor();
		ImGui::SameLine();
	}
}

void CMenuModule::DrawUtilitySubTabs()
{
	for (int i = 0; i < ARRAYSIZE(UtilitySubTabs); i++)
	{
		std::string it = UtilitySubTabs[i];

		ImGui::PushStyleColor(ImGuiCol_Button, selectedSubTab2 == i ? m_pStyle->Colors[ImGuiCol_ButtonActive] : m_pStyle->Colors[ImGuiCol_Button]);
		if (ImGui::Button(it.c_str(), ImVec2(ImGui::CalcTextSize(it.c_str()).x + 10, 25)))
		{
			selectedSubTab2 = i;
		}
		ImGui::PopStyleColor();
		ImGui::SameLine();
	}
}

void CMenuModule::DrawConfigsSubTabs()
{
	for (int i = 0; i < ARRAYSIZE(ConfigsSubTab); i++)
	{
		std::string it = ConfigsSubTab[i];

		ImGui::PushStyleColor(ImGuiCol_Button, selectedSubTab3 == i ? m_pStyle->Colors[ImGuiCol_ButtonActive] : m_pStyle->Colors[ImGuiCol_Button]);
		if (ImGui::Button(it.c_str(), ImVec2(ImGui::CalcTextSize(it.c_str()).x + 10, 25)))
		{
			selectedSubTab3 = i;
		}
		ImGui::PopStyleColor();
		ImGui::SameLine();
	}
}

void CMenuModule::DrawSettingsSubTabs()
{
	for (int i = 0; i < ARRAYSIZE(SettingsSubTab); i++)
	{
		std::string it = SettingsSubTab[i];

		ImGui::PushStyleColor(ImGuiCol_Button, selectedSubTab4 == i ? m_pStyle->Colors[ImGuiCol_ButtonActive] : m_pStyle->Colors[ImGuiCol_Button]);
		if (ImGui::Button(it.c_str(), ImVec2(ImGui::CalcTextSize(it.c_str()).x + 10, 25)))
		{
			selectedSubTab4 = i;
		}
		ImGui::PopStyleColor();
		ImGui::SameLine();
	}
}


// Sub Tabs Content

void CMenuModule::DrawVisualsTabContent()
{
	switch (selectedSubTab0)
	{
	case 0: // Render
	{
		ImGui::BeginChild(("render"), ImVec2(328, 460), true);

		ImGui::PushItemWidth(180);

		ImGui::Text(("Game"));

		g_Config.cvars.tooltips ? ImGuiCustom.ToolTip(tool_tips_visuals[0], true, 300) : void(nullptr);

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("No Shake"), &g_Config.cvars.no_shake); ImGui::SameLine();
		ImGui::Checkbox(("No Fade"), &g_Config.cvars.no_fade); ImGui::SameLine();
		ImGui::Checkbox(("Remove FOV Cap"), &g_Config.cvars.remove_fov_cap);
		ImGui::Checkbox(("Show Sound Origin"), &g_Config.cvars.show_sound_origin);

		ImGuiCustom.Spacing(8);

		ImGui::Text(("Draw Entities"));

		ImGuiCustom.Spacing(4);

		ImGui::Combo((" "), &g_Config.cvars.draw_entities, (const char**)draw_entities_items, IM_ARRAYSIZE(draw_entities_items));

		ImGuiCustom.Spacing(8);

		ImGui::Text(("Light Map"));

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Override Lightmap"), &g_Config.cvars.lightmap_override);

		ImGuiCustom.Spacing(4);

		ImGui::SliderFloat(("Lightmap Brightness"), &g_Config.cvars.lightmap_brightness, 0.0f, 1.0f);

		ImGui::Spacing();

		ImGui::ColorEdit3(("Lightmap Color"), g_Config.cvars.lightmap_color);

		ImGuiCustom.Spacing(8);

		ImGui::Text(("No Weapon Animations"));

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox( ( "Disable Idle Anims" ), &g_Config.cvars.viewmodel_disable_idle ); ImGui::SameLine();
		ImGui::Checkbox( ( "Disable Equip Anims" ), &g_Config.cvars.viewmodel_disable_equip );

		ImGuiCustom.Spacing(4);

		ImGui::Text(("Skip Frames"));

		ImGui::Spacing();

		ImGui::Checkbox( ( "Skip Frames" ), &g_Config.cvars.skip_frames ); ImGui::SameLine();
		ImGui::Checkbox( ( "Skip Sequence of Frames" ), &g_Config.cvars.skip_frames_sequence );

		ImGui::Spacing();

		ImGui::SliderInt(("Skip Frames Count"), &g_Config.cvars.skip_frames_count, 1, 60);

		ImGui::EndTabItem();

		ImGui::EndChild();


		ImGui::NextColumn();

		ImGui::SetCursorPosX(332);

		ImGui::BeginChild(("hitmarkers"), ImVec2(328.5, 460), true);

		ImGui::Text(("Hitmarkers"));

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Show Hitmarkers"), &g_Config.cvars.show_hitmarkers);

		ImGui::Spacing();

		ImGui::Checkbox(("Play Sound##hitmarkers"), &g_Config.cvars.hitmarkers_sound);

		ImGuiCustom.Spacing(4);

		ImGui::SliderInt(("Size##hitmarkers"), &g_Config.cvars.hitmarkers_size, 2, 100);
		
		ImGui::Spacing();

		ImGui::SliderFloat(("Stay Time##hitmarkers"), &g_Config.cvars.hitmarkers_stay_time, 0.01f, 3.0f);
		
		ImGuiCustom.Spacing(4);

		ImGui::Text(("Grenade Trajectory"));

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Show Grenade Trajectory"), &g_Config.cvars.show_grenade_trajectory);

		ImGuiCustom.Spacing(4);

		ImGui::ColorEdit4(("Color##grendtraj"), g_Config.cvars.grenade_trajectory_color);
		
		ImGui::Spacing();

		ImGui::ColorEdit4(("Impact Color##grendtraj"), g_Config.cvars.grenade_trajectory_impact_color);
		
		ImGuiCustom.Spacing(4);

		ImGui::Text(("AR Grenade Trajectory"));

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Show AR Grenade Trajectory"), &g_Config.cvars.show_ar_grenade_trajectory);

		ImGuiCustom.Spacing(4);

		ImGui::ColorEdit4(("Color##artraj"), g_Config.cvars.ar_grenade_trajectory_color);
		
		ImGui::Spacing();

		ImGui::ColorEdit4(("Impact Color##artraj"), g_Config.cvars.ar_grenade_trajectory_impact_color);

		ImGui::EndChild();
		break;
	}
	case 1: // ESP
	{
		ImGui::BeginChild(("esp"), ImVec2(328, 490), true);

		ImGui::PushItemWidth(180);

		ImGui::Checkbox(("Enable ESP"), &g_Config.cvars.esp); ImGui::SameLine(); 
		ImGui::Checkbox(("Debug##esp"), &g_Config.cvars.esp_debug);

		g_Config.cvars.tooltips ? ImGuiCustom.ToolTip(tool_tips_visuals[1], true, 300) : void(nullptr);

		ImGui::Checkbox(("Optimize##esp"), &g_Config.cvars.esp_optimize); ImGui::SameLine();
		ImGui::Checkbox(("Snap Lines##esp"), &g_Config.cvars.esp_snaplines); ImGui::SameLine();
		ImGui::Checkbox(("Outline Box"), &g_Config.cvars.esp_box_outline);
		ImGui::Checkbox(("Show Items"), &g_Config.cvars.esp_show_items); ImGui::SameLine();
		ImGui::Checkbox(("Ignore Unknown Entities"), &g_Config.cvars.esp_ignore_unknown_ents);
		ImGui::Checkbox(("Draw Entity Index"), &g_Config.cvars.esp_box_index); ImGui::SameLine();
		ImGui::Checkbox(("Draw Distance"), &g_Config.cvars.esp_box_distance);
		ImGui::Checkbox(("Show Only Visible Players"), &g_Config.cvars.esp_show_visible_players);
		ImGui::Checkbox(("Draw Player Health"), &g_Config.cvars.esp_box_player_health);
		ImGui::Checkbox(("Draw Player Armor"), &g_Config.cvars.esp_box_player_armor);
		ImGui::Checkbox(("Draw Nicknames"), &g_Config.cvars.esp_box_player_name);
		ImGui::Checkbox(("Draw Entity Name"), &g_Config.cvars.esp_box_entity_name); ImGui::SameLine();
		ImGui::Checkbox(("Draw Skeleton"), &g_Config.cvars.esp_skeleton);
		ImGui::Checkbox(("Draw Names of Bones"), &g_Config.cvars.esp_bones_name);

		ImGuiCustom.Spacing(4);

		ImGui::Text(("Colors"));

		ImGuiCustom.Spacing(4);

		ImGui::ColorEdit3(("Friend Player Color"), g_Config.cvars.esp_friend_player_color);

		ImGui::ColorEdit3(("Enemy Player Color"), g_Config.cvars.esp_enemy_player_color);

		ImGui::ColorEdit3(("Friend Color"), g_Config.cvars.esp_friend_color);

		ImGui::ColorEdit3(("Enemy Color"), g_Config.cvars.esp_enemy_color);

		ImGui::ColorEdit3(("Neutral Color"), g_Config.cvars.esp_neutral_color);

		ImGui::ColorEdit3(("Item Color"), g_Config.cvars.esp_item_color);

		ImGui::PopItemWidth();
		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::SetCursorPosX(332);

		ImGui::BeginChild(("esp-2"), ImVec2(328.5, 340), true);

		ImGui::PushItemWidth(150);

		ImGui::Text(("ESP Misc."));

		ImGuiCustom.Spacing(4);

		if (ImGui::Combo(("Player Style##style"), &g_Config.cvars.esp_player_style, (const char**)esp_style, IM_ARRAYSIZE(esp_style)))
		{
			if (g_Config.cvars.esp_player_style == 0)
			{
				g_Config.cvars.esp_friend_player_color[0] = 0.f;
				g_Config.cvars.esp_friend_player_color[1] = 1.f;
				g_Config.cvars.esp_friend_player_color[2] = 0.f;
			}
			else if (g_Config.cvars.esp_player_style == 1)
			{
				g_Config.cvars.esp_box_player_health = true;
				g_Config.cvars.esp_box_player_armor = true;

				g_Config.cvars.esp_show_visible_players = true;
				g_Config.cvars.esp_box_distance = false;
				g_Config.cvars.esp_box_index = true;

				g_Config.cvars.esp_friend_player_color[0] = 1.f;
				g_Config.cvars.esp_friend_player_color[1] = 1.f;
				g_Config.cvars.esp_friend_player_color[2] = 1.f;
			}
			else if (g_Config.cvars.esp_player_style == 2)
			{
				g_Config.cvars.esp_box_player_health = false;
				g_Config.cvars.esp_box_player_armor = false;

				g_Config.cvars.esp_box_distance = false;
				g_Config.cvars.esp_box_index = false;

				g_Config.cvars.esp_friend_player_color[0] = 0.6f;
				g_Config.cvars.esp_friend_player_color[1] = 0.75f;
				g_Config.cvars.esp_friend_player_color[2] = 1.f;
			}
		}

		ImGui::Spacing();

		ImGui::Combo(("Entity Style##style2"), &g_Config.cvars.esp_entity_style, (const char**)esp_style, IM_ARRAYSIZE(esp_style));

		ImGui::Spacing();

		ImGui::Combo(("Targets##esp"), &g_Config.cvars.esp_targets, (const char**)esp_process_items, IM_ARRAYSIZE(esp_process_items));

		ImGui::Spacing();

		ImGui::Combo(("Draw Distance Mode##esp"), &g_Config.cvars.esp_distance_mode, (const char**)esp_process_items, IM_ARRAYSIZE(esp_process_items));

		ImGui::Spacing();

		ImGui::Combo(("Draw Skeleton Mode##esp"), &g_Config.cvars.esp_skeleton_type, (const char**)esp_process_items, IM_ARRAYSIZE(esp_process_items));

		ImGui::Spacing();

		ImGui::Combo(("Draw Box Targets##esp"), &g_Config.cvars.esp_box_targets, (const char**)esp_process_items, IM_ARRAYSIZE(esp_process_items));

		ImGui::Spacing();

		ImGui::Combo(("Draw Box Type##esp"), &g_Config.cvars.esp_box, (const char**)esp_box_items, IM_ARRAYSIZE(esp_box_items));

		ImGuiCustom.Spacing(8);

		ImGui::SliderFloat(("Distance##esp"), &g_Config.cvars.esp_distance, 1.0f, 8192.0f);

		ImGui::Spacing();

		ImGui::SliderInt(("Box Fill Alpha##esp"), &g_Config.cvars.esp_box_fill, 0, 255);

		ImGui::EndChild();

		ImGui::PopItemWidth();
		break;
	}
	case 2: // Chams
	{
		ImGui::BeginChild(("chams"), ImVec2(328, 360), true);

		ImGui::PushItemWidth(150);

		ImGui::Checkbox(("Enable Chams"), &g_Config.cvars.chams);

		g_Config.cvars.tooltips ? ImGuiCustom.ToolTip(tool_tips_visuals[2], true, 300) : void(nullptr);

		ImGuiCustom.Spacing(8);

		ImGui::Text(("Players"));

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Chams Players Behind Wall"), &g_Config.cvars.chams_players_wall);

		ImGuiCustom.Spacing(4);

		ImGui::SliderInt(("Chams Players"), &g_Config.cvars.chams_players, 0, 3, chams_items[g_Config.cvars.chams_players]);

		ImGui::ColorEdit3(("Chams Players Color"), g_Config.cvars.chams_players_color);

		ImGui::ColorEdit3(("Chams Players Wall Color"), g_Config.cvars.chams_players_wall_color);

		ImGuiCustom.Spacing(8);

		ImGui::Text(("Entities"));

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Chams Entities Behind Wall"), &g_Config.cvars.chams_entities_wall);

		ImGuiCustom.Spacing(4);

		ImGui::SliderInt(("Chams Entities"), &g_Config.cvars.chams_entities, 0, 3, chams_items[g_Config.cvars.chams_entities]);

		ImGui::ColorEdit3(("Chams Entities Color"), g_Config.cvars.chams_entities_color);

		ImGui::ColorEdit3(("Chams Entities Wall Color"), g_Config.cvars.chams_entities_wall_color);

		ImGui::PopItemWidth();
		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::SetCursorPosX(332);

		ImGui::BeginChild(("chams-2"), ImVec2(328.5, 165), true);

		ImGui::PushItemWidth(160);

		ImGui::Text(("Items"));

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Chams Items Behind Wall"), &g_Config.cvars.chams_items_wall);

		ImGuiCustom.Spacing(4);

		ImGui::SliderInt(("Chams Items"), &g_Config.cvars.chams_items, 0, 3, chams_items[g_Config.cvars.chams_items]);

		ImGui::ColorEdit3(("Chams Items Color"), g_Config.cvars.chams_items_color);

		ImGui::ColorEdit3(("Chams Items Wall Color"), g_Config.cvars.chams_items_wall_color);

		ImGui::EndChild();

		ImGui::PopItemWidth();
		break;
	}
	case 3: // Glow (Incl. Dynamic)
	{
		ImGui::BeginChild(("glow"), ImVec2(328, 520), true);

		ImGui::PushItemWidth(180);

		ImGui::Checkbox(("Enable Glow"), &g_Config.cvars.glow);

		g_Config.cvars.tooltips ? ImGuiCustom.ToolTip(tool_tips_visuals[3], true, 300) : void(nullptr);

		ImGui::SameLine();

		ImGui::Checkbox(("Optimize Glow Behind Wall"), &g_Config.cvars.glow_optimize);

		ImGuiCustom.Spacing(8);

		ImGui::Text(("Players"));

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Glow Players Behind Wall"), &g_Config.cvars.glow_players_wall);

		ImGuiCustom.Spacing(4);

		ImGui::SliderInt(("Glow Players"), &g_Config.cvars.glow_players, 0, 3, glow_items[g_Config.cvars.glow_players]);

		ImGui::SliderInt(("Glow Players Width"), &g_Config.cvars.glow_players_width, 0, 30);

		ImGui::ColorEdit3(("Glow Players Color"), g_Config.cvars.glow_players_color);

		ImGuiCustom.Spacing(8);

		ImGui::Text(("Entities"));

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Glow Entities Behind Wall"), &g_Config.cvars.glow_entities_wall);

		ImGuiCustom.Spacing(4);

		ImGui::SliderInt(("Glow Entities"), &g_Config.cvars.glow_entities, 0, 3, glow_items[g_Config.cvars.glow_entities]);

		ImGui::SliderInt(("Glow Entities Width"), &g_Config.cvars.glow_entities_width, 0, 30);

		ImGui::ColorEdit3(("Glow Entities Color"), g_Config.cvars.glow_entities_color);

		ImGuiCustom.Spacing(8);

		ImGui::Text(("Items"));

		ImGuiCustom.Spacing(8);

		ImGui::Checkbox(("Glow Items Behind Wall"), &g_Config.cvars.glow_items_wall);

		ImGuiCustom.Spacing(8);

		ImGui::SliderInt(("Glow Items"), &g_Config.cvars.glow_items, 0, 3, glow_items[g_Config.cvars.glow_items]);

		ImGui::SliderInt(("Glow Items Width"), &g_Config.cvars.glow_items_width, 0, 30);

		ImGui::ColorEdit3(("Glow Items Color"), g_Config.cvars.glow_items_color);

		ImGui::PopItemWidth();
		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::SetCursorPosX(332);

		ImGui::BeginChild(("glow-dynamic"), ImVec2(328.5, 530), true);

		ImGui::PushItemWidth(140);

		ImGui::Text(("Dynamic Glow"));

		ImGui::SameLine();

		ImGui::Checkbox(("Dyn. Glow Attach To Targets"), &g_Config.cvars.dyn_glow_attach);

		ImGuiCustom.Spacing(4);

		ImGui::Text(("Self"));

		ImGui::SameLine();

		ImGui::Checkbox(("Dyn. Glow Self"), &g_Config.cvars.dyn_glow_self);

		ImGui::SliderFloat(("Dyn. Glow Self Radius"), &g_Config.cvars.dyn_glow_self_radius, 0.f, 4096.f);

		ImGui::SliderFloat(("Dyn. Glow Self Decay"), &g_Config.cvars.dyn_glow_self_decay, 0.f, 4096.f);

		ImGui::ColorEdit3(("Dyn. Glow Self Color"), g_Config.cvars.dyn_glow_self_color);

		ImGuiCustom.Spacing(8);

		ImGui::Text(("Players"));

		ImGui::SameLine();

		ImGui::Checkbox(("Dyn. Glow Players"), &g_Config.cvars.dyn_glow_players);

		ImGui::SliderFloat(("Dyn. Glow Players Radius"), &g_Config.cvars.dyn_glow_players_radius, 0.f, 4096.f);

		ImGui::SliderFloat(("Dyn. Glow Players Decay"), &g_Config.cvars.dyn_glow_players_decay, 0.f, 4096.f);

		ImGui::ColorEdit3(("Dyn. Glow Players Color"), g_Config.cvars.dyn_glow_players_color);

		ImGuiCustom.Spacing(8);

		ImGui::Text(("Entities"));

		ImGui::SameLine();

		ImGui::Checkbox(("Dyn. Glow Entities"), &g_Config.cvars.dyn_glow_entities);

		ImGui::SliderFloat(("Dyn. Glow Entities Radius"), &g_Config.cvars.dyn_glow_entities_radius, 0.f, 4096.f);

		ImGui::SliderFloat(("Dyn. Glow Entities Decay"), &g_Config.cvars.dyn_glow_entities_decay, 0.f, 4096.f);

		ImGui::ColorEdit3(("Dyn. Glow Entities Color"), g_Config.cvars.dyn_glow_entities_color);

		ImGuiCustom.Spacing(8);

		ImGui::Text(("Items"));

		ImGui::SameLine();

		ImGui::Checkbox(("Dyn. Glow Items"), &g_Config.cvars.dyn_glow_items);

		ImGui::SliderFloat(("Dyn. Glow Items Radius"), &g_Config.cvars.dyn_glow_items_radius, 0.f, 4096.f);

		ImGui::SliderFloat(("Dyn. Glow Items Decay"), &g_Config.cvars.dyn_glow_items_decay, 0.f, 4096.f);

		ImGui::ColorEdit3(("Dyn. Glow Items Color"), g_Config.cvars.dyn_glow_items_color);

		ImGui::EndChild();

		ImGui::PopItemWidth();
		break;
	}
	case 4: // Flashlight
	{
		ImGui::BeginChild(("esp"), ImVec2(328, 410), true);

		ImGui::PushItemWidth(150);

		ImGui::Checkbox(("Custom Flashlight"), &g_Config.cvars.custom_flashlight);

		ImGuiCustom.Spacing(4);

		ImGui::Text(("Local Player Flashlight"));

		ImGui::Spacing();

		ImGui::Checkbox(("Flashlight##localp"), &g_Config.cvars.flashlight_localplayer);

		ImGui::Spacing();

		ImGui::SliderFloat(("Flashlight Distance##localp"), &g_Config.cvars.flashlight_localplayer_flashlight_distance, 1.f, 8192.f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Flashlight Falloff Distance##localp"), &g_Config.cvars.flashlight_localplayer_falloff_distance, 1.f, 8192.f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Flashlight Radius##localp"), &g_Config.cvars.flashlight_localplayer_radius, 1.f, 1024.f);

		ImGui::Spacing();

		ImGui::ColorEdit3(("Flashlight Color##localp"), g_Config.cvars.flashlight_localplayer_color);

		ImGuiCustom.Spacing(4);

		ImGui::Text(("Other Players Flashlight"));

		ImGui::Spacing();

		ImGui::Checkbox(("Flashlight##plr"), &g_Config.cvars.flashlight_players);

		ImGui::Spacing();

		ImGui::SliderFloat(("Flashlight Distance##plr"), &g_Config.cvars.flashlight_players_flashlight_distance, 1.f, 8192.f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Flashlight Falloff Distance##plr"), &g_Config.cvars.flashlight_players_falloff_distance, 1.f, 8192.f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Flashlight Radius##plr"), &g_Config.cvars.flashlight_players_radius, 1.f, 1024.f);

		ImGui::Spacing();

		ImGui::ColorEdit3(("Flashlight Color##plr"), g_Config.cvars.flashlight_players_color);

		ImGui::PopItemWidth();

		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::SetCursorPosX(332);

		ImGui::BeginChild(("ESP2"), ImVec2(328.5, 315), true);

		ImGui::PushItemWidth(140);

		ImGui::Text(("Local Player Flashlight Lighting"));

		ImGui::Spacing();

		ImGui::Checkbox(("Flashlight Lighting##llocalp"), &g_Config.cvars.flashlight_lighting_localplayer);

		ImGui::Spacing();

		ImGui::SliderFloat(("Flashlight Lighting Distance##llocalp"), &g_Config.cvars.flashlight_lighting_localplayer_distance, 1.f, 1024.f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Flashlight Lighting Radius##llocalp"), &g_Config.cvars.flashlight_lighting_localplayer_radius, 1.f, 1024.f);

		ImGui::Spacing();

		ImGui::ColorEdit3(("Flashlight Lighting Color##llocalp"), g_Config.cvars.flashlight_lighting_localplayer_color);

		ImGuiCustom.Spacing(4);

		ImGui::Text(("Other Players Flashlight Lighting"));

		ImGui::Spacing();

		ImGui::Checkbox(("Flashlight Lighting##lplr"), &g_Config.cvars.flashlight_lighting_players);

		ImGui::Spacing();

		ImGui::SliderFloat(("Flashlight Lighting Distance##lplr"), &g_Config.cvars.flashlight_lighting_players_distance, 1.f, 1024.f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Flashlight Lighting Radius##lplr"), &g_Config.cvars.flashlight_lighting_players_radius, 1.f, 1024.f);

		ImGui::Spacing();

		ImGui::ColorEdit3(("Flashlight Lighting Color##lplr"), g_Config.cvars.flashlight_lighting_players_color);

		ImGui::PopItemWidth();
		break;
	}
	case 5: // Wallhack
	{
		ImGui::BeginChild(("wallhack"), ImVec2(328, 170), true);

		ImGui::PushItemWidth(150);

		ImGui::Checkbox(("Simple Wallhack"), &g_Config.cvars.wallhack); ImGui::SameLine();
		ImGui::Checkbox(("Lambert Wallhack"), &g_Config.cvars.wallhack_white_walls);

		ImGui::Spacing();

		ImGui::Checkbox(("Wireframe World##wh"), &g_Config.cvars.wallhack_wireframe); ImGui::SameLine();
		ImGui::Checkbox(("Wireframe Models##wh"), &g_Config.cvars.wallhack_wireframe_models);

		ImGui::Spacing();

		ImGui::Checkbox(("Negative"), &g_Config.cvars.wallhack_negative);

		ImGuiCustom.Spacing(4);

		ImGui::SliderFloat(("Wireframe Line Width"), &g_Config.cvars.wh_wireframe_width, 0.0f, 10.0f);

		ImGui::Spacing();

		ImGui::ColorEdit3(("Wireframe Color"), g_Config.cvars.wh_wireframe_color);

		ImGui::PopItemWidth();
		ImGui::EndChild();
		break;
	}
	case 6: // BSP
	{
		ImGui::BeginChild(("bsp"), ImVec2(328, 230), true);

		ImGui::PushItemWidth(250);

		ImGui::Text(("BSP Info"));

		g_Config.cvars.tooltips ? ImGuiCustom.ToolTip(tool_tips_visuals[4], true, 300) : void(nullptr);

		ImGui::Spacing();

		ImGui::Checkbox(("Wireframe Mode##bsp"), &g_Config.cvars.bsp_wireframe);
		
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Checkbox(("Show Spawns"), &g_Config.cvars.show_spawns);

		ImGui::Spacing();

		ImGui::Checkbox(("Show Walls"), &g_Config.cvars.show_walls);
		
		ImGui::Spacing();

		ImGui::Checkbox(("Show Triggers"), &g_Config.cvars.show_triggers);

		ImGui::Spacing();

		ImGui::Checkbox(("Show Triggers Info"), &g_Config.cvars.show_triggers_info);

		ImGui::Spacing();
		
		if (ImGui::BeginCombo( ( ""), ("Triggers To Show"), ImGuiComboFlags_HeightLargest))
		{
			ImGui::Checkbox(("Show Trigger Once"), &g_Config.cvars.show_trigger_once);

			ImGui::Spacing();

			ImGui::ColorEdit4(("Trigger Once Color"), g_Config.cvars.trigger_once_color);

			ImGui::Spacing();

			ImGui::Checkbox(("Show Trigger Multiple"), &g_Config.cvars.show_trigger_multiple);

			ImGui::Spacing();

			ImGui::ColorEdit4(("Trigger Multiple Color"), g_Config.cvars.trigger_multiple_color);

			ImGui::Spacing();

			ImGui::Checkbox(("Show Trigger Hurt"), &g_Config.cvars.show_trigger_hurt);

			ImGui::Spacing();

			ImGui::ColorEdit4(("Trigger Hurt Color"), g_Config.cvars.trigger_hurt_color);

			ImGui::Spacing();

			ImGui::Checkbox(("Show Trigger Hurt (Heal)"), &g_Config.cvars.show_trigger_hurt_heal);

			ImGui::Spacing();

			ImGui::ColorEdit4(("Trigger Hurt (Heal) Color"), g_Config.cvars.trigger_hurt_heal_color);

			ImGui::Spacing();

			ImGui::Checkbox(("Show Trigger Push"), &g_Config.cvars.show_trigger_push);

			ImGui::Spacing();

			ImGui::ColorEdit4(("Trigger Push Color"), g_Config.cvars.trigger_push_color);

			ImGui::Spacing();

			ImGui::Checkbox(("Show Trigger Teleport"), &g_Config.cvars.show_trigger_teleport);

			ImGui::Spacing();

			ImGui::ColorEdit4(("Trigger Teleport Color"), g_Config.cvars.trigger_teleport_color);

			ImGui::Spacing();

			ImGui::Checkbox(("Show Trigger Changelevel"), &g_Config.cvars.show_trigger_changelevel);

			ImGui::Spacing();

			ImGui::ColorEdit4(("Trigger Changelevel Color"), g_Config.cvars.trigger_changelevel_color);

			ImGui::Spacing();

			ImGui::Checkbox(("Show Anti-Rush Trigger"), &g_Config.cvars.show_trigger_antirush);

			ImGui::Spacing();

			ImGui::ColorEdit4(("Anti-Rush Trigger Color"), g_Config.cvars.trigger_antirush_color);

			ImGui::EndCombo();
		}

		ImGui::PopItemWidth();
		ImGui::EndChild();
		break;
	}
	case 7: // Models Manager
	{
		ImGui::BeginChild(("models-manager"), ImVec2(328, 380), true);

		ImGui::PushItemWidth(150);

		ImGui::Text(("Replace Models of All Players"));

		ImGuiCustom.Spacing(4);

		if (ImGui::Checkbox(("Replace Models of All Players"), &g_Config.cvars.replace_players_models))
		{
			if (SvenModAPI()->GetClientState() == CLS_ACTIVE)
				g_ModelsManager.ResetPlayersInfo();
		}
		if (ImGui::Checkbox(("Replace Model on Self"), &g_Config.cvars.replace_model_on_self))
		{
			if (SvenModAPI()->GetClientState() == CLS_ACTIVE)
				g_ModelsManager.ResetLocalPlayerInfo();
		}

		ImGui::Spacing();

		ImGui::InputText(("Model to Replace"), g_szReplacePlayerModelBuffer, IM_ARRAYSIZE(g_szReplacePlayerModelBuffer));

		if (ImGui::Button(("Change Model##mm")))
		{
			g_ReplacePlayerModel = g_szReplacePlayerModelBuffer;

			if (SvenModAPI()->GetClientState() == CLS_ACTIVE)
				g_ModelsManager.ResetPlayersInfo();
		}

		ImGuiCustom.Spacing(4);

		if (ImGui::Checkbox(("Replace All Players Models With Random Ones"), &g_Config.cvars.replace_players_models_with_randoms))
		{
			if (SvenModAPI()->GetClientState() == CLS_ACTIVE)
				g_ModelsManager.ResetPlayersInfo();
		}

		ImGui::Spacing();

		if (ImGui::Button(("Reload List of Random Models")))
		{
			g_ModelsManager.ReloadRandomModels();
		}

		ImGuiCustom.Spacing(4);

		ImGui::Text(("Replace Models of Specified Players"));

		ImGuiCustom.Spacing(4);

		if (ImGui::Checkbox(("Replace Models of Specified Players"), &g_Config.cvars.replace_specified_players_models))
		{
			if (SvenModAPI()->GetClientState() == CLS_ACTIVE)
				g_ModelsManager.ResetPlayersInfo();
		}
		if (ImGui::Checkbox(("Don't Replace Models of Specified Players"), &g_Config.cvars.dont_replace_specified_players_models))
		{
			if (SvenModAPI()->GetClientState() == CLS_ACTIVE)
				g_ModelsManager.ResetPlayersInfo();
		}

		ImGui::Spacing();

		if (ImGui::Button(("Reload List of Players##mm")))
		{
			g_ModelsManager.ReloadTargetPlayers();
		}

		ImGui::Spacing();

		if (ImGui::Button(("Reload List of Ignored Players##mm")))
		{
			g_ModelsManager.ReloadIgnoredPlayers();
		}

		ImGui::PopItemWidth();
		ImGui::EndChild();
		break;
	}
	case 8: // Shaders
	{
		ImGui::BeginChild(("shaders"), ImVec2(328, 415), true);

		ImGui::Checkbox(("Enable##shaders"), &g_Config.cvars.shaders);

		g_Config.cvars.tooltips ? ImGuiCustom.ToolTip(tool_tips_visuals[5], true, 300) : void(nullptr);

		ImGuiCustom.Spacing(4);

		ImGui::InputText(("Default Preset##shaders"), g_szShadersPresetInputText, IM_ARRAYSIZE(g_szShadersPresetInputText));

		g_sShadersPreset = g_szShadersPresetInputText;

		ImGuiCustom.Spacing(4);

		if (ImGui::Button(("Reset Shaders")))
		{
			ResetShaders();
		}
		
		ImGuiCustom.Spacing(4);

		ImGui::Text(("Depth Buffer"));

		ImGui::Spacing();

		ImGui::Checkbox(("Show Depth Buffer##shaders"), &g_Config.cvars.shaders_show_depth_buffer);

		ImGui::Spacing();

		ImGui::SliderFloat(("Z Near##depth"), &g_Config.cvars.shaders_depth_buffer_znear, 0.01f, 64.f);
		
		ImGui::Spacing();

		ImGui::SliderFloat(("Z Far##depth"), &g_Config.cvars.shaders_depth_buffer_zfar, 0.01f, 4096.f);
		
		ImGui::Spacing();

		ImGui::SliderFloat(("Brightness##depth"), &g_Config.cvars.shaders_depth_buffer_brightness, 0.f, 1.f);

		ImGui::Spacing();

		ImGui::Text(("Misc."));

		ImGui::Spacing();

		if (ImGui::BeginCombo( ( ""), ("SSAO"), ImGuiComboFlags_HeightLargest))
		{
			ImGui::Text(("Screen-Space Ambient Occlusion"));

			ImGui::Spacing();

			ImGui::Checkbox(("Enable SSAO##shader"), &g_Config.cvars.shaders_ssao);

			ImGui::Spacing();

			ImGui::Checkbox(("Only Ambient Occlusion##ssao"), &g_Config.cvars.shaders_ssao_onlyAO);

			ImGui::Spacing();

			ImGui::SliderFloat(("Z Near##ssao"), &g_Config.cvars.shaders_ssao_znear, 0.01f, 64.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Z Far##ssao"), &g_Config.cvars.shaders_ssao_zfar, 0.01f, 4096.f);

			ImGui::Spacing();

			ImGui::SliderInt(("Quality##ssao"), &g_Config.cvars.shaders_ssao_samples, 1, 64);

			ImGui::Spacing();

			ImGui::SliderFloat(("Strength##ssao"), &g_Config.cvars.shaders_ssao_strength, 0.0f, 10.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Radius##ssao"), &g_Config.cvars.shaders_ssao_radius, 0.0f, 50.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Depth Clamp##ssao"), &g_Config.cvars.shaders_ssao_aoclamp, 0.0f, 5.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Self-Shadowing Reduction##ssao"), &g_Config.cvars.shaders_ssao_diffarea, 0.0f, 5.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Gauss Bell Center##ssao"), &g_Config.cvars.shaders_ssao_gdisplace, 0.0f, 5.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Luminance Affection##ssao"), &g_Config.cvars.shaders_ssao_lumInfluence, 0.0f, 5.f);

			ImGuiCustom.Spacing(4);

			ImGui::Text(("Noise"));

			ImGui::Spacing();

			ImGui::Checkbox(("Use Noise Instead of Patterns##ssao"), &g_Config.cvars.shaders_ssao_noise);

			ImGui::Spacing();

			ImGui::SliderFloat(("Dithering Amount##ssao"), &g_Config.cvars.shaders_ssao_noiseamount, 0.0f, 0.1f, "%.5f");

			ImGuiCustom.Spacing(4);

			ImGui::Text(("Mist"));

			ImGui::Spacing();

			ImGui::Checkbox(("Enable Mist##ssao"), &g_Config.cvars.shaders_ssao_mist);

			ImGui::Spacing();

			ImGui::SliderFloat(("Start##ssao"), &g_Config.cvars.shaders_ssao_miststart, 0.f, 2048.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("End##ssao"), &g_Config.cvars.shaders_ssao_mistend, 0.01f, 4096.f);

			ImGui::Spacing();

			if (ImGui::Button(("Reset##ssao")))
			{
				g_Config.cvars.shaders_ssao_znear = 4.f;
				g_Config.cvars.shaders_ssao_zfar = 4096.f;
				g_Config.cvars.shaders_ssao_strength = 1.f;
				g_Config.cvars.shaders_ssao_samples = 32;
				g_Config.cvars.shaders_ssao_radius = 2.5f;
				g_Config.cvars.shaders_ssao_aoclamp = 0.14f;
				g_Config.cvars.shaders_ssao_noise = true;
				g_Config.cvars.shaders_ssao_noiseamount = 0.0002f;
				g_Config.cvars.shaders_ssao_diffarea = 0.3f;
				g_Config.cvars.shaders_ssao_gdisplace = 0.4f;
				g_Config.cvars.shaders_ssao_mist = false;
				g_Config.cvars.shaders_ssao_miststart = 0.f;
				g_Config.cvars.shaders_ssao_mistend = 4096.f;
				g_Config.cvars.shaders_ssao_lumInfluence = 0.7f;
			}

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		if (ImGui::BeginCombo( ( " "), ("Color Correction"), ImGuiComboFlags_HeightLargest))
		{
			ImGui::Checkbox(("Enable Color Correction##shader"), &g_Config.cvars.shaders_color_correction);

			ImGui::Spacing();

			ImGui::SliderFloat(("Grain##clrcor"), &g_Config.cvars.shaders_cc_grain, 0.0f, 512.f);

			ImGui::Spacing();

			//ImGui::SliderFloat(("Sharpness##clrcor"), &g_Config.cvars.shaders_cc_sharpness, 0.f, 100.f);
			//
			//ImGui::Spacing();

			ImGui::SliderFloat(("Gamma##clrcor"), &g_Config.cvars.shaders_cc_target_gamma, 0.01f, 4.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Monitor Gamma##clrcor"), &g_Config.cvars.shaders_cc_monitor_gamma, 0.01f, 4.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Hue Adjustment##clrcor"), &g_Config.cvars.shaders_cc_hue_offset, 0.f, 1.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Saturation##clrcor"), &g_Config.cvars.shaders_cc_saturation, 0.f, 10.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Contrast##clrcor"), &g_Config.cvars.shaders_cc_contrast, 0.f, 10.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Luminance##clrcor"), &g_Config.cvars.shaders_cc_luminance, 0.f, 10.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Bright Boost##clrcor"), &g_Config.cvars.shaders_cc_bright_boost, 0.f, 1.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Black Level##clrcor"), &g_Config.cvars.shaders_cc_black_level, 0.f, 1.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Red Channel Level##clrcor"), &g_Config.cvars.shaders_cc_R, 0.f, 1.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Green Channel Level##clrcor"), &g_Config.cvars.shaders_cc_G, 0.f, 1.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Blue Channel Level##clrcor"), &g_Config.cvars.shaders_cc_B, 0.f, 1.f);

			ImGui::Spacing();

			if (ImGui::Button(("Reset##clrcor")))
			{
				g_Config.cvars.shaders_cc_target_gamma = 2.2f;
				g_Config.cvars.shaders_cc_monitor_gamma = 2.2f;
				g_Config.cvars.shaders_cc_hue_offset = 0.f;
				g_Config.cvars.shaders_cc_saturation = 1.f;
				g_Config.cvars.shaders_cc_contrast = 1.f;
				g_Config.cvars.shaders_cc_luminance = 1.f;
				g_Config.cvars.shaders_cc_black_level = 0.f;
				g_Config.cvars.shaders_cc_bright_boost = 0.f;
				g_Config.cvars.shaders_cc_R = 1.f;
				g_Config.cvars.shaders_cc_G = 1.f;
				g_Config.cvars.shaders_cc_B = 1.f;
				g_Config.cvars.shaders_cc_grain = 0.f;
			}

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		if (ImGui::BeginCombo( ( "  "), ("Chromatic Aberration"), ImGuiComboFlags_HeightLarge))
		{
			ImGui::Checkbox(("Enable Chromatic Aberration##shaders"), &g_Config.cvars.shaders_chromatic_aberration);

			ImGui::Spacing();

			ImGui::SliderInt(("Type##aberrat"), &g_Config.cvars.shaders_chromatic_aberration_type, 0, 2, ca_types[g_Config.cvars.shaders_chromatic_aberration_type]);

			ImGui::Spacing();

			ImGui::SliderFloat(("Direction X##aberrat"), &g_Config.cvars.shaders_chromatic_aberration_dir_x, -50.f, 50.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Direction Y##aberrat"), &g_Config.cvars.shaders_chromatic_aberration_dir_y, -50.f, 50.f);

			ImGui::Spacing();

			if ( g_Config.cvars.shaders_chromatic_aberration_type == 0 )
			{
				ImGui::SliderFloat(("Shift##aberrat"), &g_Config.cvars.shaders_chromatic_aberration_shift, 0.f, 50.f);
			}
			else
			{
				float v = g_Config.cvars.shaders_chromatic_aberration_shift;

				g_Config.cvars.shaders_chromatic_aberration_shift = clamp(v, 0.f, 1.f);

				ImGui::SliderFloat(("Shift##aberrat"), &g_Config.cvars.shaders_chromatic_aberration_shift, 0.f, 1.f);
			}

			ImGui::Spacing();

			ImGui::SliderFloat(("Strength##aberrat"), &g_Config.cvars.shaders_chromatic_aberration_strength, 0.f, 10.f);

			ImGui::Spacing();

			if (ImGui::Button(("Reset##aberrat")))
			{
				g_Config.cvars.shaders_chromatic_aberration_type = 1;
				g_Config.cvars.shaders_chromatic_aberration_dir_x = 1.f;
				g_Config.cvars.shaders_chromatic_aberration_dir_y = 1.f;
				g_Config.cvars.shaders_chromatic_aberration_shift = 0.025f;
				g_Config.cvars.shaders_chromatic_aberration_strength = 1.f;
			}

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		if (ImGui::BeginCombo( ( "   "), ("Vignette"), 0))
		{
			ImGui::Checkbox(("Enable Vignette##shaders"), &g_Config.cvars.shaders_vignette);

			ImGui::Spacing();

			ImGui::SliderFloat(("Falloff##vignette"), &g_Config.cvars.shaders_vignette_falloff, 0.f, 1.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Amount##vignette"), &g_Config.cvars.shaders_vignette_amount, 0.f, 5.f);

			ImGui::Spacing();

			if (ImGui::Button(("Reset##vignette")))
			{
				g_Config.cvars.shaders_vignette_falloff = 0.5f;
				g_Config.cvars.shaders_vignette_amount = 0.4f;
			}

			ImGui::EndCombo();
		}

		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::SetCursorPosX(332);

		ImGui::BeginChild(("shaders-blur"), ImVec2(328.5, 260), true);

		ImGui::Text(("Blur"));

		ImGuiCustom.Spacing(4);

		if (ImGui::BeginCombo( ( ""), ("Menu Background Blur"), ImGuiComboFlags_HeightLarge))
		{
			ImGui::Checkbox(("Enable Menu Blur##mblur"), &g_Config.cvars.menu_blur);

			ImGui::Spacing();

			ImGui::SliderFloat(("Fade In Duration##mblur"), &g_Config.cvars.menu_blur_fadein_duration, 0.0f, 5.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Fade Out Duration##mblur"), &g_Config.cvars.menu_blur_fadeout_duration, 0.0f, 5.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Bluriness Radius##mblur"), &g_Config.cvars.menu_blur_radius, 0.0f, 150.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Bokeh Coefficient##mblur"), &g_Config.cvars.menu_blur_bokeh, 0.0f, 1.f);

			ImGui::Spacing();

			ImGui::SliderInt(("Quality##mblur"), &g_Config.cvars.menu_blur_samples, 1, 50);

			ImGui::Spacing();

			if (ImGui::Button(("Reset##mblur")))
			{
				g_Config.cvars.menu_blur_fadein_duration = 0.5f;
				g_Config.cvars.menu_blur_fadeout_duration = 0.4f;
				g_Config.cvars.menu_blur_radius = 20.f;
				g_Config.cvars.menu_blur_bokeh = 0.7f;
				g_Config.cvars.menu_blur_samples = 20;
			}

			ImGui::EndCombo();
		}

		ImGui::Spacing();
		
		if (ImGui::BeginCombo( ( " "), ("Depth of Field Blur"), ImGuiComboFlags_HeightLarge))
		{
			ImGui::Checkbox(("Enable DoF Blur##shader"), &g_Config.cvars.shaders_dof_blur);

			ImGui::Spacing();

			ImGui::SliderFloat(("Min. Range##dof"), &g_Config.cvars.shaders_dof_blur_min_range, 0.01f, 4096.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Max. Range##dof"), &g_Config.cvars.shaders_dof_blur_max_range, 0.01f, 4096.f);

			ImGui::Spacing();

			ImGui::SliderInt(("Interpolation Type"), &g_Config.cvars.shaders_dof_blur_interp_type, 0, 4, dof_interps[g_Config.cvars.shaders_dof_blur_interp_type]);

			ImGui::Spacing();

			ImGui::SliderFloat(("Bluriness Range##dof"), &g_Config.cvars.shaders_dof_blur_bluriness_range, 0.0f, 150.f);

			ImGui::Spacing();

			ImGui::SliderInt(("Quality##dof"), &g_Config.cvars.shaders_dof_blur_quality, 1, 50);

			ImGui::Spacing();

			ImGui::SliderFloat(("Bokeh Coefficient##dof"), &g_Config.cvars.shaders_dof_blur_bokeh, 0.f, 1.f);

			ImGui::Spacing();

			if (ImGui::Button(("Reset##dof")))
			{
				g_Config.cvars.shaders_dof_blur_min_range = 1024.f;
				g_Config.cvars.shaders_dof_blur_max_range = 4096.f;
				g_Config.cvars.shaders_dof_blur_interp_type = 1;
				g_Config.cvars.shaders_dof_blur_bluriness_range = 20.f;
				g_Config.cvars.shaders_dof_blur_quality = 20;
				g_Config.cvars.shaders_dof_blur_bokeh = 0.7f;
			}

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		if (ImGui::BeginCombo( ( "  "), ("Motion Blur"), 0))
		{
			ImGui::Checkbox(("Enable Motion Blur##shader"), &g_Config.cvars.shaders_motion_blur);

			ImGui::Spacing();

			ImGui::SliderFloat(("Min. Speed##motblur"), &g_Config.cvars.shaders_motion_blur_min_speed, 0.01f, 2000.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Max. Speed##motblur"), &g_Config.cvars.shaders_motion_blur_max_speed, 0.01f, 2000.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Strength##motblur"), &g_Config.cvars.shaders_motion_blur_strength, 0.0f, 50.f);

			ImGui::Spacing();

			if (ImGui::Button(("Reset##motblur")))
			{
				g_Config.cvars.shaders_motion_blur_strength = 2.f;
				g_Config.cvars.shaders_motion_blur_min_speed = 270.f;
				g_Config.cvars.shaders_motion_blur_max_speed = 700.f;
			}

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		if (ImGui::BeginCombo( ( "   "), ("Radial Blur"), 0))
		{
			ImGui::Checkbox(("Enable Radial Blur##shader"), &g_Config.cvars.shaders_radial_blur);

			ImGui::Spacing();

			ImGui::SliderFloat(("Distance##radblur"), &g_Config.cvars.shaders_radial_blur_distance, 0.0f, 10.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Strength##radblur"), &g_Config.cvars.shaders_radial_blur_strength, 0.0f, 50.f);

			ImGui::Spacing();

			if (ImGui::Button(("Reset##radblur")))
			{
				g_Config.cvars.shaders_radial_blur_distance = 1.f;
				g_Config.cvars.shaders_radial_blur_strength = 2.f;
			}

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		if (ImGui::BeginCombo( ( "    "), ("Bokeh Blur"), 0))
		{
			ImGui::Checkbox(("Enable Bokeh Blur##shader"), &g_Config.cvars.shaders_bokeh_blur);

			ImGui::Spacing();

			ImGui::SliderFloat(("Bluriness Radius##bokeh"), &g_Config.cvars.shaders_bokeh_blur_radius, 0.0f, 150.f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Bokeh Coefficient##bokeh"), &g_Config.cvars.shaders_bokeh_blur_coeff, 0.0f, 1.f);

			ImGui::Spacing();

			ImGui::SliderInt(("Quality##bokeh"), &g_Config.cvars.shaders_bokeh_blur_samples, 1, 50);

			ImGui::Spacing();

			if (ImGui::Button(("Reset##bokeh")))
			{
				g_Config.cvars.shaders_bokeh_blur_radius = 20.f;
				g_Config.cvars.shaders_bokeh_blur_coeff = 0.7f;
				g_Config.cvars.shaders_bokeh_blur_samples = 20;
			}

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		if (ImGui::BeginCombo( ( "     "), ("Gaussian Blur"), 0))
		{
			ImGui::Checkbox(("Enable Gaussian Blur##shader"), &g_Config.cvars.shaders_gaussian_blur);

			ImGui::Spacing();

			ImGui::SliderFloat(("Radius##gaussian"), &g_Config.cvars.shaders_gaussian_blur_radius, 0.0f, 150.f);

			ImGui::Spacing();

			if (ImGui::Button(("Reset##gaussian")))
			{
				g_Config.cvars.shaders_gaussian_blur_radius = 10.f;
			}

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		if (ImGui::BeginCombo( ( "      "), ("Gaussian Blur Fast"), 0))
		{
			ImGui::Checkbox(("Enable Gaussian Blur Fast##shader"), &g_Config.cvars.shaders_gaussian_blur_fast);

			ImGui::Spacing();

			ImGui::SliderFloat(("Radius##gaussianfast"), &g_Config.cvars.shaders_gaussian_blur_fast_radius, 0.0f, 15.f);

			ImGui::Spacing();

			if (ImGui::Button(("Reset##gaussianfast")))
			{
				g_Config.cvars.shaders_gaussian_blur_fast_radius = 1.f;
			}

			ImGui::EndCombo();
		}

		ImGui::EndChild();
		break;
	}
	case 9: // Misc
	{

		ImGui::BeginChild(("misc"), ImVec2(328, 410), true);

		ImGui::PushItemWidth(160);

		extern void ConCommand_ChangeSkybox(const CCommand & args);
		extern void ConCommand_ResetSkybox();

		extern const char* g_szSkyboxes[];
		extern int g_iSkyboxesSize;
		extern bool g_bMenuChangeSkybox;

		ImGui::Text(("Skybox Changer"));

		ImGuiCustom.Spacing(4);

		if (ImGui::Combo(("Skybox Name"), &g_Config.cvars.skybox, g_szSkyboxes, g_iSkyboxesSize))
		{
			g_bMenuChangeSkybox = true;

			ConCommand_ChangeSkybox(s_DummyCommand);

			g_bMenuChangeSkybox = false;
		}

		ImGuiCustom.Spacing(4);

		if (ImGui::Button(("Reset Skybox")))
		{
			ConCommand_ResetSkybox();
		}

		ImGuiCustom.Spacing(4);

		ImGui::Text(("Player's Push Direction"));

		ImGui::Spacing();

		ImGui::Checkbox(("Show Player's Push Direction"), &g_Config.cvars.show_players_push_direction);

		ImGui::Spacing();

		ImGui::SliderFloat(("Push Direction Length"), &g_Config.cvars.push_direction_length, 0.0f, 256.0f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Push Direction Width"), &g_Config.cvars.push_direction_width, 0.01f, 100.0f);

		ImGui::Spacing();

		ImGui::ColorEdit3(("Push Direction Color"), g_Config.cvars.push_direction_color);

		ImGuiCustom.Spacing(4);

		ImGui::Text(("Player's Sight Direction"));

		ImGui::Spacing();

		ImGui::Checkbox(("Show Players Sight Direction"), &g_Config.cvars.show_players_sight_direction);

		ImGui::Spacing();

		ImGui::SliderFloat(("Sight Direction Length"), &g_Config.cvars.players_sight_direction_length, 0.0f, 256.0f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Sight Direction Width"), &g_Config.cvars.players_sight_direction_width, 0.01f, 100.0f);

		ImGui::Spacing();

		ImGui::ColorEdit3(("Sight Direction Color"), g_Config.cvars.players_sight_direction_color);

		ImGui::PopItemWidth();
		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::SetCursorPosX(332);

		ImGui::BeginChild(("misc2"), ImVec2(328.5, 235), true);

		ImGui::PushItemWidth(160);

		ImGui::Text(("Fog"));

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Enable Fog"), &g_Config.cvars.fog);

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Fog Skybox"), &g_Config.cvars.fog_skybox);

		ImGui::SameLine();

		ImGui::Checkbox(("Disable Water Fog"), &g_Config.cvars.remove_water_fog);

		ImGuiCustom.Spacing(4);

		ImGui::SliderFloat(("Fog Start"), &g_Config.cvars.fog_start, 0.0f, 10000.0f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Fog End"), &g_Config.cvars.fog_end, 0.0f, 10000.0f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Density"), &g_Config.cvars.fog_density, 0.0f, 10.0f);

		ImGui::Spacing();

		ImGui::ColorEdit3(("Color"), g_Config.cvars.fog_color);

		ImGui::EndChild();
		ImGui::PopItemWidth();
		break;
	}
	}
}

void CMenuModule::DrawHUDTabContent()
{
	switch (selectedSubTab1)
	{
	case 0: // General (HUD Color Remap, Crosshair)
	{
		ImGui::BeginChild(("general"), ImVec2(328, 420), true);

		ImGui::Text(("Custom HUD Color"));

		ImGui::Spacing();

		ImGui::Checkbox(("Change HUD Color"), &g_Config.cvars.remap_hud_color);

		ImGui::Spacing();

		ImGui::ColorEdit3(("HUD Color"), g_Config.cvars.hud_color);

		ImGuiCustom.Spacing(8);

		ImGui::Text(("Crosshair"));

		ImGui::PushItemWidth(130);

		ImGui::Spacing();

		ImGui::Checkbox(("Draw Crosshair"), &g_Config.cvars.draw_crosshair);

		ImGui::Spacing();

		ImGui::Checkbox(("Draw Crosshair Dot"), &g_Config.cvars.draw_crosshair_dot);

		ImGui::Spacing();

		ImGui::Checkbox(("Draw Crosshair Outline"), &g_Config.cvars.draw_crosshair_outline);

		ImGuiCustom.Spacing(4);

		ImGui::SliderInt(("Crosshair Size"), &g_Config.cvars.crosshair_size, 1, 50);

		ImGui::Spacing();

		ImGui::SliderInt(("Crosshair Gap"), &g_Config.cvars.crosshair_gap, 0, 50);

		ImGui::Spacing();

		ImGui::SliderInt(("Crosshair Thickness"), &g_Config.cvars.crosshair_thickness, 1, 50);

		ImGui::Spacing();

		ImGui::SliderInt(("Crosshair Outline Thickness"), &g_Config.cvars.crosshair_outline_thickness, 1, 50);

		ImGuiCustom.Spacing(4);

		ImGui::PopItemWidth();

		ImGui::PushItemWidth(160);

		ImGui::ColorEdit4(("Crosshair Color"), g_Config.cvars.crosshair_color);

		ImGui::Spacing();

		ImGui::ColorEdit4(("Crosshair Outline Color"), g_Config.cvars.crosshair_outline_color);

		ImGui::PopItemWidth();

		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::SetCursorPosX(332);

		ImGui::BeginChild(("ESP2"), ImVec2(328.5, 195), true);

		ImGui::PushItemWidth(160);

		ImGui::Text(("Grenade's Timer"));

		ImGui::Spacing();

		ImGui::Checkbox(("Show Grenade's Timer"), &g_Config.cvars.grenade_timer);

		ImGui::Spacing();

		ImGui::ColorEdit3(("Timer Color##nade_t"), g_Config.cvars.grenade_timer_color);

		ImGui::Spacing();

		ImGui::ColorEdit3(("Explosion Time Color##nade_t"), g_Config.cvars.grenade_explosive_time_color);

		ImGui::Spacing();

		ImGui::SliderFloat(("Width Fraction##nade_t"), &g_Config.cvars.grenade_timer_width_frac, 0.f, 1.f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Height Fraction##nade_t"), &g_Config.cvars.grenade_timer_height_frac, 0.f, 1.f);

		ImGui::EndChild();
		ImGui::PopItemWidth();
		break;
	}
	case 1: // Speedometer
	{
		ImGui::BeginChild(("speedometer"), ImVec2(328, 470), true);

		ImGui::PushItemWidth(120);

		ImGui::Checkbox(("Show Speedometer"), &g_Config.cvars.show_speed);

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Show Jump's Speed"), &g_Config.cvars.show_jumpspeed);

		ImGui::Spacing();

		ImGui::Checkbox(("Store Vertical Speed"), &g_Config.cvars.show_vertical_speed);

		ImGuiCustom.Spacing(4);

		ImGui::SliderFloat(("Jump's Speed: Fade Duration"), &g_Config.cvars.jumpspeed_fade_duration, 0.1f, 2.0f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Speed Width Fraction"), &g_Config.cvars.speed_width_fraction, 0.0f, 1.0f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Speed Height Fraction"), &g_Config.cvars.speed_height_fraction, 0.0f, 1.0f);

		ImGui::PopItemWidth();
		ImGuiCustom.Spacing(4);
		ImGui::PushItemWidth(200);

		ImGui::ColorEdit3(("Speed Color"), g_Config.cvars.speed_color);

		ImGui::Spacing();

		if (ImGui::Button(("Reset Color of Speedometer")))
		{
			g_Config.cvars.speed_color[0] = 100.f / 255.f;
			g_Config.cvars.speed_color[1] = 130.f / 255.f;
			g_Config.cvars.speed_color[2] = 200.f / 255.f;
		}

		ImGui::PopItemWidth();
		ImGui::PushItemWidth(120);

		ImGuiCustom.Spacing(8);

		ImGui::Text(("Legacy Speedometer"));

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Show Speedometer (Legacy)"), &g_Config.cvars.show_speed_legacy);

		ImGui::Spacing();

		ImGui::Checkbox(("Store Vertical Speed (Legacy)"), &g_Config.cvars.show_vertical_speed_legacy);

		ImGui::Spacing();

		ImGui::SliderFloat(("Speed Width Fraction (Legacy)"), &g_Config.cvars.speed_width_fraction_legacy, 0.0f, 1.0f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Speed Height Fraction (Legacy)"), &g_Config.cvars.speed_height_fraction_legacy, 0.0f, 1.0f);

		ImGui::PopItemWidth();
		ImGuiCustom.Spacing(4);
		ImGui::PushItemWidth(170);

		ImGui::ColorEdit3(("Speed Color (Legacy)"), g_Config.cvars.speed_color_legacy);

		ImGui::PopItemWidth();
		ImGui::EndChild();
		break;
	}
	case 2: // Radar
	{
		ImGui::BeginChild(("radar"), ImVec2(328, 265), true);

		ImGui::PushItemWidth(180);

		ImGui::Checkbox(("Enable Radar"), &g_Config.cvars.radar);

		ImGui::Spacing();

		ImGui::Checkbox(("Show Player Name##radar"), &g_Config.cvars.radar_show_player_name);

		ImGui::Spacing();

		ImGui::Checkbox(("Show Entity Name##radar"), &g_Config.cvars.radar_show_entity_name);

		ImGuiCustom.Spacing(4);

		ImGui::SliderInt(("Type##rdr"), &g_Config.cvars.radar_type, 0, 1, radar_type[g_Config.cvars.radar_type]);

		ImGui::Spacing();

		ImGui::SliderInt(("Size##rdr"), &g_Config.cvars.radar_size, 1, 1000);

		ImGui::Spacing();

		ImGui::SliderFloat(("Distance##rdr"), &g_Config.cvars.radar_distance, 1.f, 16384.f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Width Fraction##rdr"), &g_Config.cvars.radar_width_frac, 0.f, 1.f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Height Fraction##rdr"), &g_Config.cvars.radar_height_frac, 0.f, 1.f);

		ImGui::PopItemWidth();
		ImGui::EndChild();
		break;
	}
	case 3: // Chat Colors
	{
		ImGui::BeginChild(("chat-colors"), ImVec2(328, 525), true);

		extern void ConCommand_ChatColorsLoadPlayers();

		ImGui::Checkbox(("Enable Chat Colors"), &g_Config.cvars.enable_chat_colors);

		g_Config.cvars.tooltips ? ImGuiCustom.ToolTip(tool_tips_hud[0], true, 300) : void(nullptr);

		ImGuiCustom.Spacing(4);

		if (ImGui::Button(("Load Players List")))
		{
			ConCommand_ChatColorsLoadPlayers();
		}

		ImGui::Spacing();

		if (ImGui::Button(("Reset Default Player Color")))
		{
			g_Config.cvars.player_name_color[0] = 0.6f;
			g_Config.cvars.player_name_color[1] = 0.75f;
			g_Config.cvars.player_name_color[2] = 1.0f;
		}

		ImGuiCustom.Spacing(4);

		ImGui::PushItemWidth(170);

		ImGui::ColorEdit3(("Default Player Color"), g_Config.cvars.player_name_color);

		ImGui::PopItemWidth();

		ImGuiCustom.Spacing(4);

		ImGui::PushItemWidth(150);

		ImGui::Text(("Rainbow Names"));

		ImGuiCustom.Spacing(4);

		ImGui::SliderFloat(("Rainbow Update Delay"), &g_Config.cvars.chat_rainbow_update_delay, 0.0f, 0.5f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Rainbow Hue Delta"), &g_Config.cvars.chat_rainbow_hue_delta, 0.0f, 0.5f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Rainbow Saturation"), &g_Config.cvars.chat_rainbow_saturation, 0.0f, 1.0f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Rainbow Lightness"), &g_Config.cvars.chat_rainbow_lightness, 0.0f, 1.0f);

		ImGuiCustom.Spacing(8);

		ImGui::PopItemWidth();
		ImGui::PushItemWidth(185);

		ImGui::Text(("Custom Colors"));

		ImGuiCustom.Spacing(4);

		ImGui::ColorEdit3(("Custom Color #1"), g_Config.cvars.chat_color_one);

		ImGui::Spacing();

		ImGui::ColorEdit3(("Custom Color #2"), g_Config.cvars.chat_color_two);

		ImGui::Spacing();

		ImGui::ColorEdit3(("Custom Color #3"), g_Config.cvars.chat_color_three);

		ImGui::Spacing();

		ImGui::ColorEdit3(("Custom Color #4"), g_Config.cvars.chat_color_four);

		ImGui::Spacing();

		ImGui::ColorEdit3(("Custom Color #5"), g_Config.cvars.chat_color_five);

		ImGui::Spacing();

		ImGui::ColorEdit3(("Custom Color #6"), g_Config.cvars.chat_color_six);

		ImGui::PopItemWidth();
		ImGui::EndChild();
		break;
	}
	case 4: // Custom Vote Popup 
	{
		ImGui::BeginChild(("custom-vote-popup"), ImVec2(328, 325), true);

		ImGui::PushItemWidth(180);

		ImGui::Checkbox(("Enable Custom Vote Popup"), &g_Config.cvars.vote_popup);

		g_Config.cvars.tooltips ? ImGuiCustom.ToolTip(tool_tips_hud[1], true, 300) : void(nullptr);

		ImGui::Spacing();

		ImGui::InputInt(("Yes Key##votepopup"), &g_Config.cvars.vote_popup_yes_key);

		ImGui::Spacing();

		ImGui::InputInt(("No Key##votepopup"), &g_Config.cvars.vote_popup_no_key);

		ImGui::Spacing();

		ImGui::SliderInt(("Width Size##cvp"), &g_Config.cvars.vote_popup_width_size, 0, 1000);

		ImGui::Spacing();

		ImGui::SliderInt(("Height Size##cvp"), &g_Config.cvars.vote_popup_height_size, 0, 1000);

		ImGui::Spacing();

		ImGui::SliderInt(("Width Border Pixels##cvp"), &g_Config.cvars.vote_popup_w_border_pix, 0, 100);

		ImGui::Spacing();

		ImGui::SliderInt(("Height Border Pixels##cvp"), &g_Config.cvars.vote_popup_h_border_pix, 0, 100);

		ImGui::Spacing();

		ImGui::SliderFloat(("Width Fraction##cvp"), &g_Config.cvars.vote_popup_width_frac, 0.0f, 1.0f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Height Fraction##cvp"), &g_Config.cvars.vote_popup_height_frac, 0.0f, 1.0f);

		ImGuiCustom.Spacing(4);

		if (ImGui::Button(("Restore Default Values")))
		{
			g_Config.cvars.vote_popup_width_size = 250;
			g_Config.cvars.vote_popup_height_size = 125;
			g_Config.cvars.vote_popup_w_border_pix = 12;
			g_Config.cvars.vote_popup_h_border_pix = 7;
			g_Config.cvars.vote_popup_width_frac = 0.015f;
			g_Config.cvars.vote_popup_height_frac = 0.37f;
		}

		ImGui::PopItemWidth();
		ImGui::EndChild();
		break;
	}
	}
}

void CMenuModule::DrawUtilityTabContent()
{
	switch (selectedSubTab2)
	{
	case 0: // Player (Generals (Sink,Freeze,Autojump, etc..), Spinner, Aimbot, No Recoil)
	{
		ImGui::BeginChild(("player"), ImVec2(328, 435), true);

		extern void ConCommand_AutoSelfSink();
		extern void ConCommand_Freeze();
		extern void ConCommand_Freeze2();
		extern void ConCommand_DropEmptyWeapon();

		ImGui::Text(("General"));

		g_Config.cvars.tooltips ? ImGuiCustom.ToolTip(tool_tips_utility[0], true, 300) : void(nullptr);

		ImGui::Spacing();

		if (ImGui::Button(("Selfsink")))
			ConCommand_AutoSelfSink();

		ImGui::SameLine();

		if (ImGui::Button(("Drop Empty Weapon")))
			ConCommand_DropEmptyWeapon();

		ImGui::Spacing();

		if (ImGui::Button(("Freeze")))
			ConCommand_Freeze();

		ImGui::SameLine();

		if (ImGui::Button(("Freeze #2")))
			ConCommand_Freeze2();

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Autojump"), &g_Config.cvars.autojump);

		ImGui::SameLine();

		ImGui::Checkbox(("Jumpbug"), &g_Config.cvars.jumpbug);

		ImGui::SameLine();

		ImGui::Checkbox(("Edgejump"), &g_Config.cvars.edgejump);

		ImGui::Spacing();

		ImGui::Checkbox(("Doubleduck"), &g_Config.cvars.ducktap);

		ImGui::SameLine();

		ImGui::Checkbox(("Fastrun"), &g_Config.cvars.fastrun);

		ImGui::Spacing();

		ImGui::Checkbox(("Auto Ceil-Clipping"), &g_Config.cvars.auto_ceil_clipping); ImGui::SameLine();
		ImGui::Checkbox(("Tertiary Attack Glitch"), &g_Config.cvars.tertiary_attack_glitch);

		ImGui::Spacing();

		ImGui::Checkbox(("Rotate Dead Body"), &g_Config.cvars.rotate_dead_body); ImGui::SameLine();
		ImGui::Checkbox(("Quake Guns"), &g_Config.cvars.quake_guns);

		ImGui::Spacing();

		ImGui::Checkbox(("Revert Pitch"), &g_Config.cvars.revert_pitch); ImGui::SameLine();
		ImGui::Checkbox(("Revert Yaw"), &g_Config.cvars.revert_yaw);

		ImGuiCustom.Spacing(8);

		ImGui::Text(("Auto Wallstrafing"));

		ImGui::Spacing();

		ImGui::Checkbox(("Enable Auto Wallstrafing"), &g_Config.cvars.auto_wallstrafing);

		ImGuiCustom.Spacing(4);

		ImGui::PushItemWidth(170);

		ImGui::SliderFloat(("Angle (~6.5 is perfect)"), &g_Config.cvars.wallstrafing_angle, 0.f, 45.f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Max Distance to Wall"), &g_Config.cvars.wallstrafing_dist, 1.f, 128.f);

		ImGui::PopItemWidth();

		ImGuiCustom.Spacing(8);

		if (ImGui::BeginCombo( ( ""), ("Spinner & View Angles"), ImGuiComboFlags_HeightLarge))
		{
			ImGui::Text(("Spinner"));

			ImGui::Spacing();

			ImGui::Checkbox(("Inclined Rotation"), &g_Config.cvars.spin_pitch_angle);

			ImGui::SliderFloat(("Inclined Rotation: Angle"), &g_Config.cvars.spin_pitch_rotation_angle, -10.0f, 10.0f);

			ImGui::Spacing();

			ImGui::Checkbox(("Spin Yaw"), &g_Config.cvars.spin_yaw_angle);

			ImGui::SliderFloat(("Spin Yaw: Angle"), &g_Config.cvars.spin_yaw_rotation_angle, -10.0f, 10.0f);

			ImGuiCustom.Spacing(8);

			ImGui::Text(("Lock View Angles"));

			ImGui::Spacing();

			ImGui::Checkbox(("Lock Pitch"), &g_Config.cvars.lock_pitch);

			ImGui::SliderFloat(("Lock Pitch: Angle"), &g_Config.cvars.lock_pitch_angle, -179.999f, 180.0f);

			ImGui::Spacing();

			ImGui::Checkbox(("Lock Yaw"), &g_Config.cvars.lock_yaw);

			ImGui::SliderFloat(("Lock Yaw: Angle"), &g_Config.cvars.lock_yaw_angle, 0.0f, 360.0f);

			ImGui::EndCombo();
		}

		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::SetCursorPosX(332);

		ImGui::BeginChild(("player2"), ImVec2(328.5, 525), true);

		ImGui::Text(("Strafer"));

		ImGui::Spacing();

		ImGui::Checkbox(("Enable Strafer"), &g_Config.cvars.strafe);

		ImGui::Spacing();

		ImGui::Checkbox(("Ignore Ground"), &g_Config.cvars.strafe_ignore_ground);

		ImGuiCustom.Spacing(4);

		ImGui::Combo(("Strafe Direction"), &g_Config.cvars.strafe_dir, (const char**)strafe_dir_items, IM_ARRAYSIZE(strafe_dir_items));

		ImGui::Spacing();

		ImGui::Combo(("Strafe Type"), &g_Config.cvars.strafe_type, (const char**)strafe_type_items, IM_ARRAYSIZE(strafe_type_items));

		ImGuiCustom.Spacing(8);

		ImGui::Text(("Aimbot"));

		ImGui::Spacing();

		ImGui::Checkbox(("Aimbot"), &g_Config.cvars.aimbot);

		ImGui::SameLine();

		ImGui::Checkbox(("Silent Aimbot"), &g_Config.cvars.silent_aimbot);

		ImGui::SameLine();

		ImGui::Checkbox(("Ragebot"), &g_Config.cvars.ragebot);

		ImGuiCustom.Spacing(4);

		if (ImGui::BeginCombo( ( ""), ("Aim Type"), 0))
		{
			ImGui::Checkbox(("Aim to Hitboxes"), &g_Config.cvars.aimbot_aim_hitboxes);
			ImGui::Checkbox(("Aim to Head"), &g_Config.cvars.aimbot_aim_head);
			ImGui::Checkbox(("Aim to Neck"), &g_Config.cvars.aimbot_aim_neck);
			ImGui::Checkbox(("Aim to Chest"), &g_Config.cvars.aimbot_aim_chest);
			ImGui::Checkbox(("Aim to Unknown Entities"), &g_Config.cvars.aimbot_aim_unknown_ents);

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		ImGui::Checkbox(("Ignore Glass"), &g_Config.cvars.aimbot_ignore_glass);
		ImGui::Checkbox(("Ignore Studio Models"), &g_Config.cvars.aimbot_ignore_blockers);

		ImGui::Spacing();

		ImGui::Checkbox( ( "Change Angles Back##aim" ), &g_Config.cvars.aimbot_change_angles_back );

		ImGui::Spacing();

		ImGui::PushItemWidth(180);

		ImGui::Checkbox(("Consider FOV"), &g_Config.cvars.aimbot_consider_fov);

		ImGui::SliderFloat(("Aimbot FOV"), &g_Config.cvars.aimbot_fov, 0.0f, 180.0f);

		ImGui::SliderFloat(("Aimbot Distance"), &g_Config.cvars.aimbot_distance, 0.0f, 8192.0f);

		ImGui::PopItemWidth();

		ImGuiCustom.Spacing(8);

		ImGui::Text(("Recoil & Spread"));

		ImGui::Spacing();

		ImGui::Checkbox(("No Recoil"), &g_Config.cvars.no_recoil);

		ImGui::Checkbox(("No Recoil [Visual]"), &g_Config.cvars.no_recoil_visual);

		ImGui::EndChild();
		break;
	}
	case 1: // Color Pulsator 
	{
		extern void ConCommand_ResetColors();
		extern void ConCommand_SyncColors();

		ImGui::BeginChild(("color-pulsator"), ImVec2(328, 200), true);

		ImGui::Checkbox(("Enable Pulsator"), &g_Config.cvars.color_pulsator);

		g_Config.cvars.tooltips ? ImGuiCustom.ToolTip(tool_tips_utility[1], true, 300) : void(nullptr);

		ImGui::Spacing();

		ImGui::Checkbox(("Change Top Color"), &g_Config.cvars.color_pulsator_top);

		ImGui::Spacing();

		ImGui::Checkbox(("Change Bottom Color"), &g_Config.cvars.color_pulsator_bottom);

		ImGuiCustom.Spacing(8);

		ImGui::Text(("Change Color Delay"));

		ImGuiCustom.Spacing(4);

		ImGui::SliderFloat( ( "  "), &g_Config.cvars.color_pulsator_delay, 0.1f, 2.5f);

		ImGui::Spacing();

		if (ImGui::Button(("Reset Colors")))
			ConCommand_ResetColors();

		ImGui::SameLine();

		if (ImGui::Button(("Sync. Colors")))
			ConCommand_SyncColors();

		ImGui::EndChild();
		break;
	}
	case 2: // Fake Lag 
	{
		ImGui::BeginChild(("fake-lag"), ImVec2(328, 210), true);

		ImGui::Checkbox(("Enable Fake Lag"), &g_Config.cvars.fakelag);

		ImGui::Spacing();

		ImGui::Checkbox(("Adaptive Ex Interp"), &g_Config.cvars.fakelag_adaptive_ex_interp);

		ImGuiCustom.Spacing(4);

		ImGui::SliderInt(("Limit"), &g_Config.cvars.fakelag_limit, 0, 256);

		ImGui::Spacing();

		ImGui::SliderFloat(("Variance"), &g_Config.cvars.fakelag_variance, 0.0f, 100.0f);

		ImGui::Spacing();

		ImGui::Combo(("Fake Lag Type"), &g_Config.cvars.fakelag_type, (const char**)fakelag_type_items, IM_ARRAYSIZE(fakelag_type_items));

		ImGui::Spacing();

		ImGui::Combo(("Fake Move Type"), &g_Config.cvars.fakelag_move, (const char**)fakelag_move_items, IM_ARRAYSIZE(fakelag_move_items));

		ImGui::EndChild();
		break;
	}
	case 3: // Anti-AFK
	{
		ImGui::BeginChild(("anti-afk"), ImVec2(328, 240), true);

		ImGui::Combo(("Mode"), &g_Config.cvars.antiafk, (const char**)antiafk_items, IM_ARRAYSIZE(antiafk_items));

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Anti-AFK Rotate Camera"), &g_Config.cvars.antiafk_rotate_camera);

		ImGui::Spacing();

		ImGui::Checkbox(("Anti-AFK Stay Within Range"), &g_Config.cvars.antiafk_stay_within_range);

		ImGui::Spacing();

		ImGui::Checkbox(("Anti-AFK Reset Stay Position"), &g_Config.cvars.antiafk_reset_stay_pos);

		ImGuiCustom.Spacing(4);

		ImGui::PushItemWidth(150);

		ImGui::SliderFloat(("Rotation Angle"), &g_Config.cvars.antiafk_rotation_angle, -7.0f, 7.0f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Stay Within Radius"), &g_Config.cvars.antiafk_stay_radius, 10.0f, 500.0f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Stay Within Spread Angle"), &g_Config.cvars.antiafk_stay_radius_offset_angle, 0.0f, 89.0f);

		ImGui::PopItemWidth();

		ImGui::EndChild();
		break;
	}
	case 4: // Spammer
	{
		ImGui::BeginChild(("spammer"), ImVec2(328, 270), true);

		extern void ConCommand_PrintSpamKeyWords(void);
		extern void ConCommand_PrintSpamTasks(void);

		ImGui::Text(("Key Spammer"));

		ImGui::Spacing();

		ImGui::Checkbox(("Hold Mode"), &g_Config.cvars.keyspam_hold_mode);

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Spam E"), &g_Config.cvars.keyspam_e); ImGui::SameLine();
		ImGui::Checkbox(("Spam Q"), &g_Config.cvars.keyspam_q);

		ImGui::Spacing();

		ImGui::Checkbox(("Spam W"), &g_Config.cvars.keyspam_w); ImGui::SameLine();
		ImGui::Checkbox(("Spam S"), &g_Config.cvars.keyspam_s);

		ImGui::Spacing();

		ImGui::Checkbox(("Spam CTRL"), &g_Config.cvars.keyspam_ctrl);

		ImGuiCustom.Spacing(8);

		ImGui::Text(("Message Spammer"));

		ImGuiCustom.Spacing(4);

		if (ImGui::Button(("Show Spam Tasks")))
			ConCommand_PrintSpamTasks();

		ImGui::Spacing();

		if (ImGui::Button(("Show Spam Keywords")))
			ConCommand_PrintSpamKeyWords();

		ImGui::EndChild();
		break;
	}
	case 5: // Speedrun Tools
	{
		ImGui::BeginChild(("speedrun-tools"), ImVec2(328, 420), true);

		ImGui::Text(("Timer"));

		ImGui::Spacing();

		ImGui::Checkbox(("Enable Timer##st"), &g_Config.cvars.st_timer);

		ImGuiCustom.Spacing(4);

		ImGui::SliderFloat(("Width Fraction##st"), &g_Config.cvars.st_timer_width_frac, 0.f, 1.f);

		ImGui::Spacing();

		ImGui::SliderFloat(("Height Fraction##st"), &g_Config.cvars.st_timer_height_frac, 0.f, 1.f);

		ImGuiCustom.Spacing(4);

		ImGui::ColorEdit3(("Timer Color##st"), g_Config.cvars.st_timer_color);

		ImGuiCustom.Spacing(4);

		ImGui::Text(("Player Hulls"));

		ImGui::Spacing();

		ImGui::Checkbox(("Show Hulls of Players##st"), &g_Config.cvars.st_player_hulls); 
		ImGui::Checkbox(("Show Server's Hulls of Players##st"), &g_Config.cvars.st_server_player_hulls);
		
		ImGui::Spacing();

		ImGui::Checkbox(("Show on Local Player##st_hulls"), &g_Config.cvars.st_player_hulls_show_local_player); 

		ImGuiCustom.Spacing(4);

		ImGui::ColorEdit4(("Hull Color##st"), g_Config.cvars.st_player_hulls_color);

		ImGui::Spacing();

		ImGui::ColorEdit4(("Hull Dead Color##st"), g_Config.cvars.st_player_hulls_dead_color);

		ImGuiCustom.Spacing( 4 );

		ImGui::Checkbox( ( "Wireframe Mode##st_hulls" ), &g_Config.cvars.st_player_hulls_show_wireframe );

		ImGuiCustom.Spacing( 2 );

		ImGui::SliderFloat( ( "Wireframe Width##st_hulls" ), &g_Config.cvars.st_player_hulls_wireframe_width, 0.f, 15.f );

		ImGui::EndChild();


		ImGui::NextColumn();

		ImGui::SetCursorPosX(332);

		ImGui::BeginChild(("speedrun-tools2"), ImVec2(328.5, 375), true);

		ImGui::Text(("Tools"));

		ImGui::Spacing();

		ImGui::ColorEdit3(("HUD Color##st"), g_Config.cvars.st_hud_color);

		ImGui::Spacing();

		if (ImGui::BeginCombo( ( ""), ("View Angles"), 0))
		{
			ImGui::Checkbox(("Show View Angles##st"), &g_Config.cvars.st_show_view_angles);

			ImGui::SliderFloat(("Width Fraction##st_va"), &g_Config.cvars.st_show_view_angles_width_frac, 0.0f, 1.0f);
			ImGui::SliderFloat(("Height Fraction##st_va"), &g_Config.cvars.st_show_view_angles_height_frac, 0.0f, 1.0f);

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		if (ImGui::BeginCombo( ( " "), ("Position"), 0))
		{
			ImGui::Checkbox(("Show Position##st"), &g_Config.cvars.st_show_pos); ImGui::SameLine();

			ImGui::Checkbox(("Use View Origin##st_pos"), &g_Config.cvars.st_show_pos_view_origin);

			ImGui::SliderFloat(("Width Fraction##st_pos"), &g_Config.cvars.st_show_pos_width_frac, 0.0f, 1.0f);
			ImGui::SliderFloat(("Height Fraction##st_pos"), &g_Config.cvars.st_show_pos_height_frac, 0.0f, 1.0f);

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		if (ImGui::BeginCombo( ( "  "), ("Velocity"), 0))
		{
			ImGui::Checkbox(("Show Velocity##st"), &g_Config.cvars.st_show_velocity);

			ImGui::SliderFloat(("Width Fraction##st_vel"), &g_Config.cvars.st_show_velocity_width_frac, 0.0f, 1.0f);
			ImGui::SliderFloat(("Height Fraction##st_vel"), &g_Config.cvars.st_show_velocity_height_frac, 0.0f, 1.0f);

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		if (ImGui::BeginCombo( ( "   "), ("Gauss Boost"), 0))
		{
			ImGui::Checkbox(("Show Gauss Boost Info##st"), &g_Config.cvars.st_show_gauss_boost_info);

			ImGui::SliderFloat(("Width Fraction##st_gaussboost"), &g_Config.cvars.st_show_gauss_boost_info_width_frac, 0.0f, 1.0f);
			ImGui::SliderFloat(("Height Fraction##st_gaussboost"), &g_Config.cvars.st_show_gauss_boost_info_height_frac, 0.0f, 1.0f);

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		if (ImGui::BeginCombo( ( "    "), ("Selfgauss"), 0))
		{
			ImGui::Checkbox(("Show Selfgauss Info##st"), &g_Config.cvars.st_show_selfgauss_info);

			ImGui::SliderFloat(("Width Fraction##st_selfgauss"), &g_Config.cvars.st_show_selfgauss_width_frac, 0.0f, 1.0f);
			ImGui::SliderFloat(("Height Fraction##st_selfgauss"), &g_Config.cvars.st_show_selfgauss_height_frac, 0.0f, 1.0f);

			ImGui::EndCombo();
		}

		ImGui::Spacing();

		if (ImGui::BeginCombo( ( "     "), ("Entity"), 0))
		{
			ImGui::Checkbox(("Show Entity Info##st"), &g_Config.cvars.st_show_entity_info); ImGui::SameLine();
			ImGui::Checkbox(("Check Players##st_entinfo"), &g_Config.cvars.st_show_entity_info_check_players);

			ImGui::SliderFloat(("Width Fraction##st_ent"), &g_Config.cvars.st_show_entity_info_width_frac, 0.0f, 1.0f);
			ImGui::SliderFloat(("Height Fraction##st_ent"), &g_Config.cvars.st_show_entity_info_height_frac, 0.0f, 1.0f);
			ImGui::EndCombo();
		}
		
		ImGui::Spacing();

		if (ImGui::BeginCombo( ( "      "), ("Revive"), 0))
		{
			ImGui::Checkbox(("Show Revive Info##st"), &g_Config.cvars.st_show_revive_info); ImGui::SameLine();
			ImGui::Checkbox(("Show With Any Weapon##st_revive"), &g_Config.cvars.st_show_revive_info_any_weapon);

			ImGuiCustom.Spacing( 4 );

			ImGui::SliderFloat(("Width Fraction##st_revive"), &g_Config.cvars.st_show_revive_info_width_frac, 0.0f, 1.0f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Height Fraction##st_revive"), &g_Config.cvars.st_show_revive_info_height_frac, 0.0f, 1.0f);

			ImGuiCustom.Spacing( 4 );

			ImGui::ColorEdit4( ( "Marker Color##stst_revive" ), g_Config.cvars.st_show_revive_info_color );

			ImGui::Spacing();

			ImGui::ColorEdit4( ( "No Ammo Marker Color##stst_revive" ), g_Config.cvars.st_show_revive_info_no_ammo_color );

			ImGui::EndCombo();
		}
		
		ImGui::Spacing();

		if (ImGui::BeginCombo( ( "       "), ("Revive Boost"), ImGuiComboFlags_HeightLargest ))
		{
			ImGui::Checkbox(("Show Revive Boost Info##st"), &g_Config.cvars.st_show_revive_boost_info); ImGui::SameLine();
			ImGui::Checkbox( ( "Show With Any Weapon##st_revive_boost" ), &g_Config.cvars.st_show_revive_boost_any_weapon );

			ImGuiCustom.Spacing( 4 );

			ImGui::SliderFloat(("Width Fraction##st_revive_boost"), &g_Config.cvars.st_show_revive_boost_info_width_frac, 0.0f, 1.0f);

			ImGui::Spacing();

			ImGui::SliderFloat(("Height Fraction##st_revive_boost"), &g_Config.cvars.st_show_revive_boost_info_height_frac, 0.0f, 1.0f);

			ImGuiCustom.Spacing( 4 );

			ImGui::Text( ( "Revive Target Hull" ) );

			ImGui::Spacing();

			ImGui::Checkbox( ( "Wireframe Hull##st_revive_boost" ), &g_Config.cvars.st_show_revive_boost_info_wireframe_hull );

			ImGui::Spacing();

			ImGui::SliderFloat( ( "Wireframe Hull Width##st_revive_boost" ), &g_Config.cvars.st_show_revive_boost_info_wireframe_hull_width, 0.0f, 10.0f );

			ImGui::Spacing();

			ImGui::ColorEdit4( ( "Hull Color##st_revive_boost" ), g_Config.cvars.st_show_revive_boost_info_hull_color );

			ImGuiCustom.Spacing( 4 );

			ImGui::Text( ( "Direction Boost" ) );

			ImGui::Spacing();

			ImGui::Checkbox( ( "Wireframe Direction Box##st_revive_boost" ), &g_Config.cvars.st_show_revive_boost_info_wireframe_direction_box );

			ImGui::Spacing();

			ImGui::Combo( ( "Draw Direction Type##st_revive_boost" ),
						  &g_Config.cvars.st_show_revive_boost_info_direction_type,
						  (const char **)revive_boost_direction_type,
						  IM_ARRAYSIZE( revive_boost_direction_type ) );

			ImGui::Spacing();

			ImGui::SliderFloat( ( "Direction Width##st_revive_boost" ), &g_Config.cvars.st_show_revive_boost_info_direction_line_width, 0.0f, 10.0f );

			ImGui::Spacing();

			ImGui::SliderFloat( ( "Direction Length##st_revive_boost" ), &g_Config.cvars.st_show_revive_boost_info_direction_length, 0.0f, 4096.0f );
			
			ImGui::Spacing();

			ImGui::SliderFloat( ( "Direction Box Extent##st_revive_boost" ), &g_Config.cvars.st_show_revive_boost_info_direction_box_extent, 0.0f, 12.0f );

			ImGui::Spacing();

			ImGui::ColorEdit4( ( "Direction Color##st_revive_boost" ), g_Config.cvars.st_show_revive_boost_info_direction_color );

			ImGui::Spacing();

			ImGuiCustom.Spacing( 4 );

			ImGui::Text( ( "Prediction" ) );

			ImGui::Spacing();

			ImGui::Checkbox( ( "Predict Trajectory##st_revive_boost" ), &g_Config.cvars.st_show_revive_boost_predict_trajectory );

			ImGui::Spacing();

			ImGui::ColorEdit4( ( "Trajectory Color##st_revive_boost" ), g_Config.cvars.st_show_revive_boost_predict_trajectory_color );

			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::Checkbox( ( "Predict Collision##st_revive_boost" ), &g_Config.cvars.st_show_revive_boost_predict_collision );

			ImGui::Spacing();

			ImGui::SliderFloat( ( "Collision Hull Width##st_revive_boost" ), &g_Config.cvars.st_show_revive_boost_predict_collision_width, 0.0f, 10.0f );

			ImGui::Spacing();

			ImGui::ColorEdit4( ( "Collision Hull Color##st_revive_boost" ), g_Config.cvars.st_show_revive_boost_predict_collision_color );

			ImGui::EndCombo();
		}
		
		ImGui::Spacing();

		if (ImGui::BeginCombo( ( "        "), ("Revive / Unstuck Area"), ImGuiComboFlags_HeightLargest ))
		{
			ImGui::Checkbox(("Show Revive / Unstuck Area Info##st"), &g_Config.cvars.st_show_revive_area_info);

			ImGui::Spacing();

			ImGui::Checkbox(("Show on Local Player##st_revive_area"), &g_Config.cvars.st_show_revive_area_local_player);

			ImGuiCustom.Spacing( 4 );

			ImGui::Text( ( "Small Hull Visualization" ) );

			ImGui::Spacing();

			ImGui::Checkbox( ( "Draw##st_revive_area_small" ), &g_Config.cvars.st_show_revive_area_draw_small_hull );

			ImGui::Spacing();

			ImGui::SliderFloat( ( "Hull Width##st_revive_area_small" ), &g_Config.cvars.st_show_revive_area_small_hull_width, 0.0f, 10.0f );

			ImGui::Spacing();

			ImGui::ColorEdit4( ( "Hull Color##st_revive_area_small" ), g_Config.cvars.st_show_revive_area_small_hull_color );
			
			ImGuiCustom.Spacing( 4 );

			ImGui::Text( ( "Medium Hull Visualization" ) );

			ImGui::Spacing();

			ImGui::Checkbox( ( "Draw##st_revive_area_medium" ), &g_Config.cvars.st_show_revive_area_draw_medium_hull );

			ImGui::Spacing();

			ImGui::SliderFloat( ( "Hull Width##st_revive_area_medium" ), &g_Config.cvars.st_show_revive_area_medium_hull_width, 0.0f, 10.0f );

			ImGui::Spacing();

			ImGui::ColorEdit4( ( "Hull Color##st_revive_area_medium" ), g_Config.cvars.st_show_revive_area_medium_hull_color );
			
			ImGuiCustom.Spacing( 4 );

			ImGui::Text( ( "Large Hull Visualization" ) );

			ImGui::Spacing();

			ImGui::Checkbox( ( "Draw##st_revive_area_large" ), &g_Config.cvars.st_show_revive_area_draw_large_hull );

			ImGui::Spacing();

			ImGui::SliderFloat( ( "Hull Width##st_revive_area_large" ), &g_Config.cvars.st_show_revive_area_large_hull_width, 0.0f, 10.0f );

			ImGui::Spacing();

			ImGui::ColorEdit4( ( "Hull Color##st_revive_area_large" ), g_Config.cvars.st_show_revive_area_large_hull_color );

			ImGui::EndCombo();
		}
		
		ImGui::Spacing();

		if (ImGui::BeginCombo( ( "         "), ("Landing Prediction"), ImGuiComboFlags_HeightLargest ))
		{
			ImGui::Checkbox(("Show Land Point##st"), &g_Config.cvars.st_show_land_point );

			ImGui::Spacing();

			ImGui::SliderInt( ( "Max Prediction Landings##st_land_point" ), &g_Config.cvars.st_show_land_point_max_points, 1, 5 );

			ImGuiCustom.Spacing( 4 );

			ImGui::Text( ( "Hull Visualization" ) );

			ImGui::Spacing();

			ImGui::Checkbox( ( "Draw##st_land_point" ), &g_Config.cvars.st_show_land_point_draw_hull );
			
			ImGui::Spacing();

			ImGui::Checkbox( ( "Wireframe##st_land_point" ), &g_Config.cvars.st_show_land_point_draw_hull_wireframe );

			ImGui::Spacing();

			ImGui::SliderFloat( ( "Hull Width##st_land_point" ), &g_Config.cvars.st_show_land_point_draw_hull_width, 0.0f, 10.0f );

			ImGui::Spacing();

			ImGui::ColorEdit4( ( "Hull Color##st_land_point" ), g_Config.cvars.st_show_land_point_draw_hull_color );
			
			ImGuiCustom.Spacing( 4 );

			ImGui::Text( ( "Land Point Visualization" ) );

			ImGui::Spacing();

			ImGui::Checkbox( ( "Draw##st_land_point_exact" ), &g_Config.cvars.st_show_land_point_draw_exact_point );
			
			ImGui::Spacing();

			ImGui::ColorEdit4( ( "Land Point Color##st_land_point_exact" ), g_Config.cvars.st_show_land_point_draw_exact_point_color );
			
			ImGui::EndCombo();
		}

		ImGui::EndChild();
		break;
	}
	case 6: // Misc
	{
		ImGui::BeginChild(("misc"), ImVec2(328, 315), true);

		extern void ConCommand_CamHack(void);
		extern void ConCommand_CamHackResetRoll(void);
		extern void ConCommand_CamHackReset(void);

		ImGui::Text(("Camera Hack"));

		ImGui::Spacing();

		if (ImGui::Button(("Toggle Cam Hack")))
			ConCommand_CamHack();

		ImGui::Spacing();

		if (ImGui::Button(("Reset Roll Axis")))
			ConCommand_CamHackResetRoll();

		if (ImGui::Button(("Reset Cam Hack")))
			ConCommand_CamHackReset();

		ImGui::Spacing();

		ImGui::PushItemWidth(160);

		ImGui::Text(("Speed Factor"));

		ImGui::Spacing();

		ImGui::SliderFloat(("CamHack: Speed Factor"), &g_Config.cvars.camhack_speed_factor, 0.0f, 15.0f);

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Hide HUD"), &g_Config.cvars.camhack_hide_hud);

		ImGui::Checkbox(("Show Model"), &g_Config.cvars.camhack_show_model);

		ImGuiCustom.Spacing(4);

		ImGui::PopItemWidth();

		ImGui::PushItemWidth(200);

		if (ImGui::BeginCombo( ( ""), ("First-Person Roaming"), 0))
		{
			ImGui::Checkbox(("Enable First-Person Roaming"), &g_Config.cvars.fp_roaming);

			ImGui::Spacing();

			ImGui::Checkbox(("Draw Crosshair in Roaming"), &g_Config.cvars.fp_roaming_draw_crosshair);

			ImGui::Spacing();

			ImGui::Checkbox(("Lerp First-Person View"), &g_Config.cvars.fp_roaming_lerp);

			ImGuiCustom.Spacing(4);

			ImGui::Text(("Lerp Value"));

			ImGui::Spacing();

			ImGui::SliderFloat(("FP Roaming: Lerp Value"), &g_Config.cvars.fp_roaming_lerp_value, 0.001f, 1.0f);

			ImGui::EndCombo();
		}

		ImGuiCustom.Spacing(4);

		if (ImGui::BeginCombo( ( " "), ("Enhanced Thirdperson"), ImGuiComboFlags_HeightLargest))
		{
			extern void ConCommand_ThirdPerson_ResetPosition();
			extern void ConCommand_ThirdPerson_ResetAngles();

			extern ConVar sc_thirdperson;
			extern ConVar sc_thirdperson_edit_mode;
			extern ConVar sc_thirdperson_hidehud;
			extern ConVar sc_thirdperson_ignore_pitch;
			extern ConVar sc_thirdperson_ignore_yaw;

			bool bThirdPerson = sc_thirdperson.GetBool();
			bool bThirdPersonEditMode = sc_thirdperson_edit_mode.GetBool();
			bool bThirdPersonHideHud = sc_thirdperson_hidehud.GetBool();
			bool bThirdPersonPitch = sc_thirdperson_ignore_pitch.GetBool();
			bool bThirdPersonYaw = sc_thirdperson_ignore_yaw.GetBool();

			if (ImGui::Button(("Reset Position")))
				ConCommand_ThirdPerson_ResetPosition();

			ImGui::SameLine();

			if (ImGui::Button(("Reset Angles")))
				ConCommand_ThirdPerson_ResetAngles();

			ImGui::Spacing();

			if (ImGui::Button(("Reset Roll Angle")))
				g_ThirdPerson.ResetRollAxis();

			ImGuiCustom.Spacing(4);

			if (ImGui::Checkbox(("Enhanced Thirdperson Mode"), &bThirdPerson))
			{
				sc_thirdperson.SetValue(g_Config.cvars.thirdperson = !sc_thirdperson.GetBool());
			}

			ImGui::Spacing();

			if (ImGui::Checkbox(("Enable Edit Mode##thirdperson"), &bThirdPersonEditMode))
			{
				sc_thirdperson_edit_mode.SetValue(g_Config.cvars.thirdperson_edit_mode = !sc_thirdperson_edit_mode.GetBool());
			}

			ImGui::Spacing();

			if (ImGui::Checkbox(("Hide HUD##thirdperson"), &bThirdPersonHideHud))
			{
				sc_thirdperson_hidehud.SetValue(g_Config.cvars.thirdperson_hidehud = !sc_thirdperson_hidehud.GetBool());
			}

			ImGui::Spacing();

			if (ImGui::Checkbox(("Ignore Pitch Angle##thirdperson"), &bThirdPersonPitch))
			{
				sc_thirdperson_ignore_pitch.SetValue(g_Config.cvars.thirdperson_ignore_pitch = !sc_thirdperson_ignore_pitch.GetBool());
			}

			ImGui::SameLine();

			if (ImGui::Checkbox(("Ignore Yaw Angle##thirdperson"), &bThirdPersonYaw))
			{
				sc_thirdperson_ignore_yaw.SetValue(g_Config.cvars.thirdperson_ignore_yaw = !sc_thirdperson_ignore_yaw.GetBool());
			}

			ImGuiCustom.Spacing(4);

			ImGui::Checkbox(("Clip to Wall##thirdperson"), &g_Config.cvars.thirdperson_clip_to_wall);

			ImGui::Spacing();

			ImGui::Combo(("Trace Type##thirdperson"), &g_Config.cvars.thirdperson_trace_type, (const char**)trace_type, IM_ARRAYSIZE(trace_type));

			ImGuiCustom.Spacing(4);

			ImGui::Text(("Camera Position"));
			ImGui::InputFloat3(("##thirdperson_origin"), g_Config.cvars.thirdperson_origin);
			ImGui::DragFloat3(("##thirdperson_origin2"), g_Config.cvars.thirdperson_origin, 0.1f, -4096.f, 4096.f);

			ImGuiCustom.Spacing(4);

			ImGui::Text(("Camera Angles"));
			ImGui::InputFloat3(("##thirdperson_angles"), g_Config.cvars.thirdperson_angles);
			ImGui::DragFloat3(("##thirdperson_angles2"), g_Config.cvars.thirdperson_angles, 0.1f, -180.f, 180.f);

			ImGui::EndCombo();
		}

		ImGui::PopItemWidth();
		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::SetCursorPosX(332);

		ImGui::BeginChild(("player2"), ImVec2(328.5, 315), true);

		ImGui::Text(("One Tick Exploit"));

		ImGui::Checkbox(("One Tick Exploit"), &g_Config.cvars.one_tick_exploit);

		ImGui::Spacing();

		ImGui::Text(("Lag Interval"));
		ImGui::SliderInt(("##one_tick_exploit_lag_interval"), &g_Config.cvars.one_tick_exploit_lag_interval, 1, 256);

		ImGui::Text(("Speedhack"));
		ImGui::SliderFloat(("##one_tick_exploit_speedhack"), &g_Config.cvars.one_tick_exploit_speedhack, 0.01f, 100000.0f);

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Fast Crowbar"), &g_Config.cvars.fast_crowbar); ImGui::SameLine();
		ImGui::Checkbox(("Fast Crowbar [Auto Freeze]"), &g_Config.cvars.fast_crowbar2);
		ImGui::Checkbox(("Fast Medkit"), &g_Config.cvars.fast_medkit);

		ImGuiCustom.Spacing(4);

		extern bool g_bDupeWeapon;
		extern bool g_bSpamKill;

		ImGui::Checkbox(("Dupe Weapon"), &g_bDupeWeapon);
		ImGui::Checkbox(("Spam Kill"), &g_bSpamKill);
		ImGui::Checkbox(("Weapon Configs"), &g_Config.cvars.weapon_configs);

		ImGui::EndChild();
		break;
	}
	}
}

void CMenuModule::DrawTabConfigsContent()
{
	switch (selectedSubTab3)
	case 0: // List
	{
		ImGui::BeginChild(("configs"), ImVec2(328, 315), true);

		ImGui::Text(("List of Configs"));

		ImGui::Spacing();

		if (ImGui::BeginListBox(("##configs_list"), ImVec2(-FLT_MIN, 8 * ImGui::GetTextLineHeightWithSpacing())))
		{
			for (size_t i = 0; i < g_Config.configs.size(); i++)
			{
				bool bSelected = ( g_Config.current_config.compare( g_Config.configs[i] ) == 0 );

				if ( ImGui::Selectable( g_Config.configs[i].c_str(), bSelected ) )
				{
					g_Config.current_config = g_Config.configs[i];

					strncpy( g_szCurrentConfigInputText, g_Config.current_config.c_str(), sizeof(g_szCurrentConfigInputText) );
					g_szCurrentConfigInputText[ M_ARRAYSIZE(g_szCurrentConfigInputText) - 1 ] = '\0';
				}

				if ( bSelected )
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndListBox();
		}

		ImGui::Spacing();

		if (ImGui::Button(("Load")))
		{
			g_Config.Load();

			LoadMenuTheme();
			WindowStyle();
		}

		ImGui::SameLine();

		if (ImGui::Button(("Save")))
			g_Config.Save();

		ImGui::SameLine();

		if (ImGui::Button(("New")))
			g_Config.New();

		ImGui::SameLine();

		if (ImGui::Button(("Delete")))
			g_Config.Remove();

		ImGuiCustom.Spacing(4);

		ImGui::InputText(("##renamecfg"), g_szCurrentConfigInputText, IM_ARRAYSIZE(g_szCurrentConfigInputText));

		ImGui::SameLine();

		if (ImGui::Button(("Rename")))
			g_Config.Rename();

		ImGuiCustom.Spacing( 8 );

		ImGui::InputText( ( "Autoexec CFG" ), g_szAutoExecConfigText, IM_ARRAYSIZE( g_szAutoExecConfigText ) );

		ImGui::EndChild();
		

		ImGui::NextColumn();

		ImGui::SetCursorPosX(332);

		ImGui::BeginChild(("shader_configs"), ImVec2(328.5, 280), true);

		ImGui::Text(("List of Shader Configs"));

		ImGui::Spacing();

		if (ImGui::BeginListBox(("##shader_configs_list"), ImVec2(-FLT_MIN, 8 * ImGui::GetTextLineHeightWithSpacing())))
		{
			for (size_t i = 0; i < g_ShadersConfig.configs.size(); i++)
			{
				bool bSelected = ( g_ShadersConfig.current_config.compare( g_ShadersConfig.configs[i] ) == 0 );

				if ( ImGui::Selectable( g_ShadersConfig.configs[i].c_str(), bSelected ) )
				{
					g_ShadersConfig.current_config = g_ShadersConfig.configs[i];

					strncpy( g_szCurrentShaderConfigInputText, g_ShadersConfig.current_config.c_str(), sizeof(g_szCurrentShaderConfigInputText) );
					g_szCurrentShaderConfigInputText[ M_ARRAYSIZE(g_szCurrentShaderConfigInputText) - 1 ] = '\0';
				}

				if ( bSelected )
				{
					ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndListBox();
		}

		ImGui::Spacing();

		if (ImGui::Button(("Load##shader")))
		{
			g_ShadersConfig.Load();
		}

		ImGui::SameLine();

		if (ImGui::Button(("Save##shader")))
			g_ShadersConfig.Save();

		ImGui::SameLine();

		if (ImGui::Button(("New##shader")))
			g_ShadersConfig.New();

		ImGui::SameLine();

		if (ImGui::Button(("Delete##shader")))
			g_ShadersConfig.Remove();

		ImGuiCustom.Spacing(4);

		ImGui::InputText(("##renamecfgshader"), g_szCurrentShaderConfigInputText, IM_ARRAYSIZE(g_szCurrentShaderConfigInputText));

		ImGui::SameLine();

		if (ImGui::Button(("Rename##shader")))
			g_ShadersConfig.Rename();

		ImGui::EndChild();
		break;
	}
}

void CMenuModule::DrawSettingsTabContent()
{
	switch (selectedSubTab4)
	{
	case 0: // Menu
	{
		ImGui::BeginChild(("menu"), ImVec2(328, 410), true);

		ImGui::Text(("Toggle Key"));

		ImGuiCustom.Spacing(4);

		if (ImGui::Button(("Use Insert")))
			g_Config.cvars.toggle_button = 0x2D;

		ImGui::SameLine();

		if (ImGui::Button(("Use Delete")))
			g_Config.cvars.toggle_button = 0x2E;

		ImGui::Spacing();

		if (ImGui::Button(("Use Home")))
			g_Config.cvars.toggle_button = 0x24;

		ImGui::SameLine();

		if (ImGui::Button(("Use End")))
			g_Config.cvars.toggle_button = 0x23;

		ImGuiCustom.Spacing(8);

		ImGui::Text(("Style"));

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Show Tooltips"), &g_Config.cvars.tooltips); 
		
		ImGuiCustom.ToolTip(("Displays (?) a tool tip for most uncommon features."), true, 125);

		ImGuiCustom.Spacing(4);

		if ( ImGui::Button( ( "Reload Textures" ) ) )
		{
			DeleteTextures();
			LoadTextures();
		}
		
		ImGuiCustom.Spacing(4);

		ImGui::PushItemWidth(150);

		if (ImGui::Combo(("Theme"), &g_Config.cvars.menu_theme, (const char**)theme_items, IM_ARRAYSIZE(theme_items)))
		{
			LoadSavedStyle();
			LoadMenuTheme();
			WindowStyle();
		}

		if (g_Config.cvars.menu_theme == 16)
		{
			ImGui::PushItemWidth(100);
			ImGui::SameLine();
			if (ImGui::BeginCombo( ( ""), ("Customize"), ImGuiComboFlags_HeightLargest))
			{
				if ( ImGui::Button(("Reset##custom_style")) )
				{
					g_Config.cvars.WindowBgU32 = 3858759680;
					g_Config.cvars.BorderU32 = 0;
					g_Config.cvars.ButtonU32 = 4283054648;
					g_Config.cvars.ButtonActiveU32 = 4292125797;
					g_Config.cvars.ButtonHoveredU32 = 4294097778;
					g_Config.cvars.FrameBgU32 = 4283054648;
					g_Config.cvars.FrameBgActiveU32 = 4294097778;
					g_Config.cvars.FrameBgHoveredU32 = 4289104990;
					g_Config.cvars.TextU32 = 4294967295;
					g_Config.cvars.ChildBgU32 = 4281147937;
					g_Config.cvars.CheckMarkU32 = 4294967295;
					g_Config.cvars.SliderGrabU32 = 4292125797;
					g_Config.cvars.SliderGrabActiveU32 = 4292125797;
					g_Config.cvars.HeaderU32 = 4292125797;
					g_Config.cvars.HeaderHoveredU32 = 4294097778;
					g_Config.cvars.HeaderActiveU32 = 4292125797;
					g_Config.cvars.ResizeGripActiveU32 = 4282528276;
					g_Config.cvars.SeparatorActiveU32 = 2353148666;
					g_Config.cvars.TitleBgActiveU32 = 4282528276;
					g_Config.cvars.SeparatorU32 = 4286930991;
				}

				ImGuiCustom.Spacing(4);

				ImGuiCustom.ColorPickerU32(("Window Color##1"), &g_Config.cvars.WindowBgU32, ImGuiColorEditFlags_AlphaBar);
				ImGuiCustom.ColorPickerU32(("Border Color##1"), &g_Config.cvars.BorderU32, ImGuiColorEditFlags_AlphaBar);
				ImGuiCustom.ColorPickerU32(("Button Color##1"), &g_Config.cvars.ButtonU32, ImGuiColorEditFlags_AlphaBar);
				ImGuiCustom.ColorPickerU32(("ButtonActive Color##1"), &g_Config.cvars.ButtonActiveU32, ImGuiColorEditFlags_AlphaBar);
				ImGuiCustom.ColorPickerU32(("ButtonHovered Color##1"), &g_Config.cvars.ButtonHoveredU32, ImGuiColorEditFlags_AlphaBar);
				ImGuiCustom.ColorPickerU32(("FrameBg Color##1"), &g_Config.cvars.FrameBgU32, ImGuiColorEditFlags_AlphaBar);
				ImGuiCustom.ColorPickerU32(("FrameBgActive Color##1"), &g_Config.cvars.FrameBgActiveU32, ImGuiColorEditFlags_AlphaBar);
				ImGuiCustom.ColorPickerU32(("FrameBgHovered Color##1"), &g_Config.cvars.FrameBgHoveredU32, ImGuiColorEditFlags_AlphaBar);
				ImGuiCustom.ColorPickerU32(("Text Color##1"), &g_Config.cvars.TextU32, ImGuiColorEditFlags_AlphaBar);
				ImGuiCustom.ColorPickerU32(("ChildBg Color##1"), &g_Config.cvars.ChildBgU32, ImGuiColorEditFlags_AlphaBar);
				ImGuiCustom.ColorPickerU32(("CheckMark Color##1"), &g_Config.cvars.CheckMarkU32, ImGuiColorEditFlags_AlphaBar);
				ImGuiCustom.ColorPickerU32(("SliderGrab Color##1"), &g_Config.cvars.SliderGrabU32, ImGuiColorEditFlags_AlphaBar);
				ImGuiCustom.ColorPickerU32(("SliderGrabActive Color##1"), &g_Config.cvars.SliderGrabActiveU32, ImGuiColorEditFlags_AlphaBar);
				ImGuiCustom.ColorPickerU32(("Header Color##1"), &g_Config.cvars.HeaderU32, ImGuiColorEditFlags_AlphaBar);
				ImGuiCustom.ColorPickerU32(("HeaderHovered Color##1"), &g_Config.cvars.HeaderHoveredU32, ImGuiColorEditFlags_AlphaBar);
				ImGuiCustom.ColorPickerU32(("HeaderActive Color##1"), &g_Config.cvars.HeaderActiveU32, ImGuiColorEditFlags_AlphaBar);
				ImGuiCustom.ColorPickerU32(("ResizeGripActive Color##1"), &g_Config.cvars.ResizeGripActiveU32, ImGuiColorEditFlags_AlphaBar);
				ImGuiCustom.ColorPickerU32(("Separator Color##1"), &g_Config.cvars.SeparatorU32, ImGuiColorEditFlags_AlphaBar);
				ImGuiCustom.ColorPickerU32(("SeparatorActive Color##1"), &g_Config.cvars.SeparatorActiveU32, ImGuiColorEditFlags_AlphaBar);
				ImGuiCustom.ColorPickerU32(("TitleBgActive Color##1"), &g_Config.cvars.TitleBgActiveU32, ImGuiColorEditFlags_AlphaBar);

				StyleColors_Custom(); // Not very optimal but should be fine

				ImGui::EndCombo();
			}

			ImGui::PopItemWidth();
		}

		ImGui::PopItemWidth();

		ImGui::PushItemWidth(150);

		ImGui::Spacing();

		if (ImGui::Combo(("Font"), &g_Config.cvars.menu_font, (const char**)font_items, IM_ARRAYSIZE(font_items)))
		{
			SelectCurrentFont();
		}

		ImGui::PopItemWidth();

		ImGuiCustom.Spacing(4);

		ImGui::SliderFloat(("Opacity"), &g_Config.cvars.menu_opacity, 0.1f, 1.0f);

		ImGuiCustom.Spacing(8);

		ImGui::Text(("SvenInt Logo Color"));

		ImGui::Spacing();

		ImGui::ColorEdit3( ( ""), g_Config.cvars.logo_color);

		ImGuiCustom.Spacing(8);

		if (ImGui::BeginCombo( ( " "), ("Rainbow Colors"), ImGuiComboFlags_HeightLargest))
		{
			ImGui::Checkbox(("Rainbow Logo"), &g_Config.cvars.menu_rainbow[0]);
			ImGui::Checkbox(("Rainbow Separator"), &g_Config.cvars.menu_rainbow[1]);

			ImGuiCustom.Spacing(4);

			ImGui::SliderFloat(("Rainbow Speed"), &g_Config.cvars.menu_rainbow_update_delay, 0.01, 0.150);

			ImGui::Spacing();

			ImGui::SliderFloat(("Rainbow Hue Delta"), &g_Config.cvars.menu_rainbow_hue_delta, 0.005, 0.100);
			ImGui::SliderFloat(("Rainbow Saturation"), &g_Config.cvars.menu_rainbow_saturation, 0.01, 1);
			ImGui::SliderFloat(("Rainbow Lightness"), &g_Config.cvars.menu_rainbow_lightness, 0.01, 1);

			ImGui::Spacing();

			if (ImGui::Button(("Restore Defaults")))
			{
				g_Config.cvars.menu_rainbow_update_delay = 0.050f;
				g_Config.cvars.menu_rainbow_hue_delta = 0.005f;
				g_Config.cvars.menu_rainbow_saturation = 0.8f;
				g_Config.cvars.menu_rainbow_lightness = 0.5f;
			}

			ImGui::EndCombo();
		}

		ImGui::EndChild();
		break;
	}
	case 1: // Game
	{
		ImGui::BeginChild(("game"), ImVec2(328, 100), true);

		ImGui::Text(("Maps"));

		ImGuiCustom.Spacing(4);

		ImGui::Checkbox(("Save Soundcache"), &g_Config.cvars.save_soundcache);

		ImGui::Spacing();

		ImGui::Checkbox(("Ignore Different Map Versions"), &g_Config.cvars.ignore_different_map_versions);

		ImGui::EndChild();
		break;
	}
	}
}

//-----------------------------------------------------------------------------
// Resets all shaders
//-----------------------------------------------------------------------------

void CMenuModule::ResetShaders()
{
	// bruh
	g_Config.cvars.menu_blur_fadein_duration = 0.5f;
	g_Config.cvars.menu_blur_fadeout_duration = 0.4f;
	g_Config.cvars.menu_blur_radius = 20.f;
	g_Config.cvars.menu_blur_bokeh = 0.7f;
	g_Config.cvars.menu_blur_samples = 20;

	g_Config.cvars.shaders_depth_buffer_znear = 4.f;
	g_Config.cvars.shaders_depth_buffer_zfar = 4096.f;
	g_Config.cvars.shaders_depth_buffer_brightness = 1.f;

	g_Config.cvars.shaders_ssao_znear = 4.f;
	g_Config.cvars.shaders_ssao_zfar = 4096.f;
	g_Config.cvars.shaders_ssao_strength = 1.f;
	g_Config.cvars.shaders_ssao_samples = 32;
	g_Config.cvars.shaders_ssao_radius = 2.5f;
	g_Config.cvars.shaders_ssao_aoclamp = 0.14f;
	g_Config.cvars.shaders_ssao_noise = true;
	g_Config.cvars.shaders_ssao_noiseamount = 0.0002f;
	g_Config.cvars.shaders_ssao_diffarea = 0.3f;
	g_Config.cvars.shaders_ssao_gdisplace = 0.4f;
	g_Config.cvars.shaders_ssao_mist = false;
	g_Config.cvars.shaders_ssao_miststart = 0.f;
	g_Config.cvars.shaders_ssao_mistend = 4096.f;
	g_Config.cvars.shaders_ssao_lumInfluence = 0.7f;

	g_Config.cvars.shaders_cc_target_gamma = 2.2f;
	g_Config.cvars.shaders_cc_monitor_gamma = 2.2f;
	g_Config.cvars.shaders_cc_hue_offset = 0.f;
	g_Config.cvars.shaders_cc_saturation = 1.f;
	g_Config.cvars.shaders_cc_contrast = 1.f;
	g_Config.cvars.shaders_cc_luminance = 1.f;
	g_Config.cvars.shaders_cc_black_level = 0.f;
	g_Config.cvars.shaders_cc_bright_boost = 0.f;
	g_Config.cvars.shaders_cc_R = 1.f;
	g_Config.cvars.shaders_cc_G = 1.f;
	g_Config.cvars.shaders_cc_B = 1.f;
	g_Config.cvars.shaders_cc_grain = 0.f;
	//float shaders_cc_sharpness = 0.f;

	g_Config.cvars.shaders_chromatic_aberration_type = 1;
	g_Config.cvars.shaders_chromatic_aberration_dir_x = 1.f;
	g_Config.cvars.shaders_chromatic_aberration_dir_y = 1.f;
	g_Config.cvars.shaders_chromatic_aberration_shift = 0.025f;
	g_Config.cvars.shaders_chromatic_aberration_strength = 1.f;

	g_Config.cvars.shaders_dof_blur_min_range = 1024.f;
	g_Config.cvars.shaders_dof_blur_max_range = 4096.f;
	g_Config.cvars.shaders_dof_blur_interp_type = 1;
	g_Config.cvars.shaders_dof_blur_bluriness_range = 20.f;
	g_Config.cvars.shaders_dof_blur_quality = 20;
	g_Config.cvars.shaders_dof_blur_bokeh = 0.7f;

	g_Config.cvars.shaders_motion_blur_strength = 2.f;
	g_Config.cvars.shaders_motion_blur_min_speed = 270.f;
	g_Config.cvars.shaders_motion_blur_max_speed = 700.f;

	g_Config.cvars.shaders_radial_blur_distance = 1.f;
	g_Config.cvars.shaders_radial_blur_strength = 2.f;

	g_Config.cvars.shaders_bokeh_blur_radius = 20.f;
	g_Config.cvars.shaders_bokeh_blur_coeff = 0.7f;
	g_Config.cvars.shaders_bokeh_blur_samples = 20;

	g_Config.cvars.shaders_gaussian_blur_radius = 10.f;

	g_Config.cvars.shaders_gaussian_blur_fast_radius = 1.f;

	g_Config.cvars.shaders_vignette_falloff = 0.5f;
	g_Config.cvars.shaders_vignette_amount = 0.4f;
}

//-----------------------------------------------------------------------------
// Hooks
//-----------------------------------------------------------------------------

LRESULT CALLBACK HOOKED_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if ( uMsg == WM_KEYDOWN && wParam == g_Config.cvars.toggle_button )
	{
		g_bMenuEnabled = !g_bMenuEnabled;

		if ( g_bMenuEnabled )
		{
			extern void OnMenuOpen();

			g_iMenuState = 1;
			g_flMenuCloseTime = -1.f;
			g_flMenuOpenTime = g_pEngineFuncs->Sys_FloatTime();

			OnMenuOpen();
		}
		else
		{
			extern void OnMenuClose();

			g_bMenuClosed = true;

			g_iMenuState = 2;
			g_flMenuOpenTime = -1.f;
			g_flMenuCloseTime = g_pEngineFuncs->Sys_FloatTime();

			OnMenuClose();
		}

		return 0;
	}

	if ( g_bMenuEnabled )
	{
		ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
	}

	return CallWindowProc(hGameWndProc, hWnd, uMsg, wParam, lParam);
}

DECLARE_FUNC(BOOL, APIENTRY, HOOKED_wglSwapBuffers, HDC hdc)
{
	static bool bImGuiInitialized = false;

	if ( !bImGuiInitialized )
	{
		hGameWnd = WindowFromDC(hdc);
		hGameWndProc = (WNDPROC)SetWindowLong(hGameWnd, GWL_WNDPROC, (LONG)HOOKED_WndProc);
		SetTimer( hGameWnd, 1, 500, NULL );

		ImGui::CreateContext();
		ImGui_ImplWin32_Init(hGameWnd);

	#if IMGUI_USE_GL3
		ImGui_ImplOpenGL3_Init();
	#else
		ImGui_ImplOpenGL2_Init();
	#endif

		InitImGuiStyles();
		ImGui::StyleColorsDark();
		SaveCurrentStyle();

		ImGuiIO &io = ImGui::GetIO();
		io.IniFilename = NULL;
		io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

		g_MenuModule.m_pStyle = &ImGui::GetStyle();

		g_MenuModule.LoadFonts();
		g_MenuModule.LoadTextures();

		bImGuiInitialized = true;
	}

	bool bMenuEnabled = g_bMenuEnabled;

#if IMGUI_USE_GL3
	ImGui_ImplOpenGL3_NewFrame();
#else
	ImGui_ImplOpenGL2_NewFrame();
#endif

	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	g_MenuModule.Draw();

	ImGui::Render();

#if IMGUI_USE_GL3
	ImGui_ImplOpenGL3_RenderDrawData( ImGui::GetDrawData() );
#else
	ImGui_ImplOpenGL2_RenderDrawData( ImGui::GetDrawData() );
#endif

	if ( bMenuEnabled && !g_bMenuEnabled )
	{
		extern void OnMenuClose();

		g_bMenuClosed = true;
		OnMenuClose();
	}

	return ORIG_wglSwapBuffers(hdc);
}

DECLARE_FUNC(BOOL, WINAPI, HOOKED_SetCursorPos, int X, int Y)
{
	if ( g_bMenuEnabled )
		return FALSE;

	return ORIG_SetCursorPos(X, Y);
}

//-----------------------------------------------------------------------------
// Menu feature impl
//-----------------------------------------------------------------------------

CMenuModule::CMenuModule()
{
	m_pStyle = NULL;

	m_pMenuFontDefault = NULL;
	m_pMenuFontBig = NULL;
	m_pMenuFontSmall = NULL;

	m_pfnwglSwapBuffers = NULL;
	m_pfnSetCursorPos = NULL;

	m_hwglSwapBuffers = 0;
	m_hSetCursorPos = 0;

	m_iLogoWidth = 0;
	m_iLogoHeight = 0;
	m_hLogoTex = 0;

	m_iMenuTexWidth = 0;
	m_iMenuTexHeight = 0;
	m_hMenuTex = 0;

	m_bMenuTexLoaded = false;

	m_bThemeLoaded = false;
	m_bMenuSettings = false;
	m_bMenuConfig = false;
	m_bMenuAim = false;
	m_bMenuVisuals = false;
	m_bMenuHud = false;
	m_bMenuUtility = false;
}

bool CMenuModule::Load()
{
	m_pfnwglSwapBuffers = Sys_GetProcAddress(Sys_GetModuleHandle("opengl32.dll"), "wglSwapBuffers");
	m_pfnSetCursorPos = Sys_GetProcAddress(Sys_GetModuleHandle("user32.dll"), "SetCursorPos");

	if ( !m_pfnwglSwapBuffers )
	{
		Warning(("Couldn't find function \"wglSwapBuffers\"\n"));
		return false;
	}

	if ( !m_pfnSetCursorPos )
	{
		Warning(("Couldn't find function \"SetCursorPos\"\n"));
		return false;
	}

	if ( *(unsigned char *)m_pfnSetCursorPos == 0xE9 ) // JMP opcode, hooked by gameoverlayrenderer.dll
	{
		m_pfnSetCursorPos = MemoryUtils()->CalcAbsoluteAddress(m_pfnSetCursorPos);
	}

	if ( *(unsigned char *)m_pfnwglSwapBuffers == 0xE9 )
	{
		m_pfnwglSwapBuffers = MemoryUtils()->CalcAbsoluteAddress(m_pfnwglSwapBuffers);
	}

	return true;
}

void CMenuModule::PostLoad()
{
	m_hwglSwapBuffers = DetoursAPI()->DetourFunction( m_pfnwglSwapBuffers, HOOKED_wglSwapBuffers, GET_FUNC_PTR(ORIG_wglSwapBuffers) );
	m_hSetCursorPos = DetoursAPI()->DetourFunction( m_pfnSetCursorPos, HOOKED_SetCursorPos, GET_FUNC_PTR(ORIG_SetCursorPos) );
}

void CMenuModule::Unload()
{
	if ( hGameWnd && hGameWndProc )
	{
		SetWindowLong(hGameWnd, GWL_WNDPROC, (LONG)hGameWndProc);
	}

	DeleteTextures();

	DetoursAPI()->RemoveDetour( m_hwglSwapBuffers );
	DetoursAPI()->RemoveDetour( m_hSetCursorPos );
}
