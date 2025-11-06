// SvenInt (c) Sw1ft
// menu.h

#ifndef SINT_MENU_MODULE_H
#define SINT_MENU_MODULE_H

#ifdef _WIN32
#pragma once
#endif

#include "config.h"

#define IMGUI_USE_SDL ( 0 )
#define IMGUI_USE_GL3 ( 1 )

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------

class CMenuElementButton;
class CConfigProperty;
class CBaseFeature;

//-----------------------------------------------------------------------------
// Customizable theme
//-----------------------------------------------------------------------------

struct menu_custom_theme_vars
{
	CConfigProperty *WindowBg;
	CConfigProperty *Border;
	CConfigProperty *Button;
	CConfigProperty *ButtonActive;
	CConfigProperty *ButtonHovered;
	CConfigProperty *FrameBg;
	CConfigProperty *FrameBgActive;
	CConfigProperty *FrameBgHovered;
	CConfigProperty *Text;
	CConfigProperty *ChildBg;
	CConfigProperty *CheckMark;
	CConfigProperty *SliderGrab;
	CConfigProperty *SliderGrabActive;
	CConfigProperty *Header;
	CConfigProperty *HeaderHovered;
	CConfigProperty *HeaderActive;
	CConfigProperty *ResizeGripActive;
	CConfigProperty *SeparatorActive;
	CConfigProperty *TitleBgActive;
	CConfigProperty *Separator;
};

//-----------------------------------------------------------------------------
// Menu elements
//-----------------------------------------------------------------------------

typedef enum
{
	kMenuElementCheckbox = 0,
	kMenuElementIntegerSlider,
	kMenuElementFloatSlider,
	kMenuElementColorPickerRGB,
	kMenuElementColorPickerRGBA,
	kMenuElementVectorDrag,
	kMenuElementList,
	kMenuElementInputText,
	kMenuElementSeparator,
	kMenuElementButton,
	kMenuElementResetButton,
	kMenuElementSameLine,
} EMenuElement;

// Button press callback
class IMenuElementButtonCallback
{
public:
	virtual ~IMenuElementButtonCallback() {}
	virtual void OnButtonPressed( CMenuElementButton *pButton ) = 0;
};

class CBaseMenuElement
{
	friend class CMenuModule;

public:
	CBaseMenuElement( int type, const char *pszName, CBaseFeature *pFeature, CConfigProperty *pCfgProperty );

	inline int				GetType( void ) const { return m_type; }
	inline bool				HasValue( void ) const { return m_bHasValue; }
	inline const char		*GetName( void ) const { return m_pszName; }
	inline const char		*GetInternalName( void ) const { return m_sInternalName.c_str(); }
	inline CBaseFeature		*GetFeature( void ) const { return m_pFeature; }
	inline CConfigProperty	*GetCfgProperty( void ) const { return m_pCfgProperty; }
	inline ConVar			*GetConVar( void ) const { return m_pConVar; }
	void					Revert( void );

protected:
	inline void				SetConVar( ConVar *pConVar ) { if ( !pConVar->IsCommand() ) m_pConVar = pConVar; }
	void					OnValueChanged( void );

protected:
	bool m_bHasValue;

private:
	unsigned char m_type;
	const char *m_pszName;
	CBaseFeature *m_pFeature;
	CConfigProperty *m_pCfgProperty;
	ConVar *m_pConVar;
	std::string m_sInternalName;
};

class CMenuValueBool : public CBaseMenuElement
{
public:
	CMenuValueBool( const char *pszName, CBaseFeature *pFeature, CConfigProperty *pCfgProperty ) :
		CBaseMenuElement( kMenuElementCheckbox, pszName, pFeature, pCfgProperty )
	{
		m_bHasValue = true;
	}

	bool GetBool( void );
};

class CMenuValueInteger : public CBaseMenuElement
{
public:
	CMenuValueInteger( const char *pszName, int iMin, int iMax, CBaseFeature *pFeature, CConfigProperty *pCfgProperty ) :
		CBaseMenuElement( kMenuElementIntegerSlider, pszName, pFeature, pCfgProperty )
	{
		m_bHasValue = true;
		m_iMin = iMin;
		m_iMax = iMax;
	}

	int GetInt( void );

	inline int GetMin( void ) const { return m_iMin; }
	inline int GetMax( void ) const { return m_iMax; }

private:
	int m_iMin;
	int m_iMax;
};

class CMenuValueFloat : public CBaseMenuElement
{
public:
	CMenuValueFloat( const char *pszName, float flMin, float flMax, CBaseFeature *pFeature, CConfigProperty *pCfgProperty ) :
		CBaseMenuElement( kMenuElementFloatSlider, pszName, pFeature, pCfgProperty )
	{
		m_bHasValue = true;
		m_flMin = flMin;
		m_flMax = flMax;
	}

	float GetFloat( void );

	inline float GetMin( void ) const { return m_flMin; }
	inline float GetMax( void ) const { return m_flMax; }

private:
	float m_flMin;
	float m_flMax;
};

class CMenuValueColorRGB : public CBaseMenuElement
{
public:
	CMenuValueColorRGB( const char *pszName, CBaseFeature *pFeature, CConfigProperty *pCfgProperty ) :
		CBaseMenuElement( kMenuElementColorPickerRGB, pszName, pFeature, pCfgProperty )
	{
		m_bHasValue = true;
	}

	Color GetColor32( void );
	float *GetColor( void );
};

class CMenuValueColorRGBA : public CBaseMenuElement
{
public:
	CMenuValueColorRGBA( const char *pszName, CBaseFeature *pFeature, CConfigProperty *pCfgProperty ) :
		CBaseMenuElement( kMenuElementColorPickerRGBA, pszName, pFeature, pCfgProperty )
	{
		m_bHasValue = true;
	}

	Color GetColor32( void );
	float *GetColor( void );
};

class CMenuValueVector : public CBaseMenuElement
{
public:
	CMenuValueVector( const char *pszName, float flStep, float flMin, float flMax, CBaseFeature *pFeature, CConfigProperty *pCfgProperty ) :
		CBaseMenuElement( kMenuElementVectorDrag, pszName, pFeature, pCfgProperty )
	{
		m_bHasValue = true;
		m_flStep = flStep;
		m_flMin = flMin;
		m_flMax = flMax;
	}

	Vector &GetVector( void );

	inline float GetStep( void ) const { return m_flStep; }
	inline float GetMin( void ) const { return m_flMin; }
	inline float GetMax( void ) const { return m_flMax; }

private:
	float m_flStep;
	float m_flMin;
	float m_flMax;
};

class CMenuValueList : public CBaseMenuElement
{
public:
	CMenuValueList( const char *pszName, const char *pszItems, CBaseFeature *pFeature, CConfigProperty *pCfgProperty ) :
		CBaseMenuElement( kMenuElementList, pszName, pFeature, pCfgProperty )
	{
		m_bHasValue = true;
		m_pszItems = pszItems;
	}

	int GetInt( void );

	inline const char *GetItems( void ) { return m_pszItems; }

private:
	const char *m_pszItems;
};

class CMenuValueText : public CBaseMenuElement
{
public:
	CMenuValueText( const char *pszName, CBaseFeature *pFeature, CConfigProperty *pCfgProperty ) :
		CBaseMenuElement( kMenuElementInputText, pszName, pFeature, pCfgProperty )
	{
		m_bHasValue = true;
	}

	const char *GetCString( void );
	int GetSize( void );
};

class CMenuElementSeparator : public CBaseMenuElement
{
public:
	CMenuElementSeparator( const char *pszName, CBaseFeature *pFeature ) :
		CBaseMenuElement( kMenuElementSeparator, pszName, pFeature, NULL )
	{
	}
};

class CMenuElementButton : public CBaseMenuElement
{
public:
	CMenuElementButton( const char *pszName, IMenuElementButtonCallback *pCallback, CBaseFeature *pFeature ) :
		CBaseMenuElement( kMenuElementButton, pszName, pFeature, NULL )
	{
		m_pCallback = pCallback;
	}

	inline IMenuElementButtonCallback *GetCallback( void ) { return m_pCallback; }

private:
	IMenuElementButtonCallback *m_pCallback;
};

class CMenuElementResetButton : public CBaseMenuElement
{
public:
	CMenuElementResetButton( const char *pszName, CBaseFeature *pFeature ) :
		CBaseMenuElement( kMenuElementResetButton, pszName, pFeature, NULL )
	{
	}
};

class CMenuElementSameLine : public CBaseMenuElement
{
public:
	CMenuElementSameLine( const char *pszName, CBaseFeature *pFeature ) :
		CBaseMenuElement( kMenuElementSameLine, pszName, pFeature, NULL )
	{
	}
};

//-----------------------------------------------------------------------------
// Feature description
//-----------------------------------------------------------------------------

class CMenuFeature
{
public:
	CMenuFeature();
	CMenuFeature( CBaseFeature *pFeature, bool bToggleable, bool bShaderFeature );

	bool							m_bToggleable;
	bool							m_bPopupOpened;
	bool							m_bShaderFeature;
	CBaseFeature					*m_pFeature;
	CConfigProperty					*m_pCfgEnabled;
	std::string						m_sInternalName;
	std::vector<CBaseMenuElement *> m_elements;
};

//-----------------------------------------------------------------------------
// Menu (feature) category
//-----------------------------------------------------------------------------

class CMenuCategory
{
public:
	const char					*m_pszName;
	std::vector<CMenuFeature>	m_features;
};

//-----------------------------------------------------------------------------
// Menu module
//-----------------------------------------------------------------------------

class CMenuModule : public IConfigListener
{
public:
	CMenuModule();

	bool			Init( void );
	void			Shutdown( void );

public:
	// IConfigListener
	virtual void OnConfigLoad( const char *pszFilename, bool bShaderConfig ) override;
	virtual void OnConfigSave( const char *pszFilename, bool bShaderConfig ) override;

public:
	// Bind feature with customizable parameters
	// Any created parameter is added to the config to be saved and loaded
	bool				BindFeature( CBaseFeature *pFeature, bool bToggleable = true );
	bool				BindShaderFeature( CBaseFeature *pFeature, bool bToggleable = true );

	// Bind a ConVar to a menu item to synchronize any changes to the ConVar as well as the menu item's value
	// Binded ConVar, when its value was changed, is always clamped to defined min/max bound value(s)
	bool				BindConVar( CBaseMenuElement *pMenuElement, ConVar *pConvar );

	// Checkbox
	CMenuValueBool		*AddParamBool( CBaseFeature *pFeature,
									   const char *pszCfgPropName,
									   const char *pszDescription,
									   bool bDefaultValue );

	// Slider integer
	CMenuValueInteger	*AddParamInteger( CBaseFeature *pFeature,
										  const char *pszCfgPropName,
										  const char *pszDescription,
										  int iDefaultValue,
										  int iMin,
										  int iMax );

	// Slider float
	CMenuValueFloat		*AddParamFloat( CBaseFeature *pFeature,
										const char *pszCfgPropName,
										const char *pszDescription,
										float flDefaultValue,
										float flMin,
										float flMax );

	// Color edit 3 
	CMenuValueColorRGB	*AddParamColorRGB( CBaseFeature *pFeature,
										   const char *pszCfgPropName,
										   const char *pszDescription,
										   Color clrDefault );

	// Color edit 4
	CMenuValueColorRGBA	*AddParamColorRGBA( CBaseFeature *pFeature,
											const char *pszCfgPropName,
											const char *pszDescription,
											Color clrDefault );
	
	// Drag float3
	CMenuValueVector	*AddParamVector( CBaseFeature *pFeature,
										 const char *pszCfgPropName,
										 const char *pszDescription,
										 Vector vecDefault,
										 float flStep,
										 float flMin,
										 float flMax );
	
	// Combo
	CMenuValueList		*AddParamList( CBaseFeature *pFeature,
									   const char *pszCfgPropName,
									   const char *pszDescription,
									   int iDefaultValue,
									   const char *pszItems );
	
	// Input text
	CMenuValueText		*AddParamText( CBaseFeature *pFeature,
									   const char *pszCfgPropName,
									   const char *pszDescription,
									   const char *pszDefaultValue = NULL );
	
	// Just a horizontal line separator
	void				AddElementSeparator( CBaseFeature *pFeature, const char *pszTextSeparator = NULL );

	// Pressable button that calls a given callback
	CMenuElementButton *AddElementButton( CBaseFeature *pFeature, IMenuElementButtonCallback *pCallback, const char *pszDescription );

	// Resets the whole config properties of a binded feature
	void				AddElementResetButton( CBaseFeature *pFeature, const char *pszDescription );

	// Next added element will stay on that current line
	void				AddElementSameLine( CBaseFeature *pFeature );

public:
#if IMGUI_USE_SDL
	bool			SDL_PollEvent( SDL_Event *event );
	void			SDL_GL_SwapWindow( SDL_Window *window );
#else
	bool			WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );
	void			wglSwapBuffers( HDC hdc );
#endif

	inline void		SetOpen( bool state ) { m_bOpened = state; }
	inline bool		IsOpen( void ) const { return m_bOpened; }
	inline float	GetOpenTime( void ) const { return m_flOpenTime; }
	inline float	GetCloseTime( void ) const { return m_flCloseTime; }
	inline WNDPROC	GetGameWindowProc( void ) const { return m_hGameWndProc; }

	inline Vector	&GetFrozenCameraAngles( void ) { return m_va; }
	void			KeepCameraFrozen( void );

	inline menu_custom_theme_vars *GetCustomThemeVars( void ) { return &m_CustomThemeVars; }

	void			OnVidInit( void );
	void			OnCvarChange( cvar_t *pCvar, const char *pszOldValue, float flOldValue );

private:
	void			PreInit( void );
	void			RemoveEmptyCategories( void );
	void			InitConfigProperties( void );
	void			OnOpen( void );
	void			OnClose( void );
	void			Draw( void );
	void			DrawFeatures( CMenuCategory &category );
	void			DrawFeatureElements( CMenuFeature &feature );
	void			DrawPrivateCategories( void );
	void			DrawConfigs( void );
	void			DrawShaderConfigs( void );
	void			DrawCustomizableTheme( void );
	void			DrawCustomizableRainbowColor( void );
	void			DrawMiscInfo( void );

	void			UpdateRainbow( void );
	void			UpdateConfigs( void );
	void			UpdateShaderConfigs( void );
	void			BuildConfigsFullPath( void );

	struct ImFont	*GetDefaultFont( void );

	void			InitFont( void );
	void			WindowStyle( void );

	inline void		AddCategory( const char *pszName ) { m_categories.push_back( { pszName, std::vector<CMenuFeature>() } ); }

private:
	typedef struct MenuConVarBind_s
	{
		cvar_t *cvar;
		ConVar *convar;
		CBaseMenuElement *menuElement;
	} MenuConVarBind_t;

	class CLookupFunctor
	{
	public:
		CLookupFunctor() {}

		bool operator()( const MenuConVarBind_t &a, const MenuConVarBind_t &b ) const
		{
			const char *pszLValue = NULL, *pszRValue = NULL;

			if ( a.cvar != NULL )
			{
				pszLValue = a.cvar->name;
			}
			else if ( a.convar != NULL )
			{
				pszLValue = a.convar->GetName();
			}

			if ( b.cvar != NULL )
			{
				pszRValue = b.cvar->name;
			}
			else if ( b.convar != NULL )
			{
				pszRValue = b.convar->GetName();
			}

			if ( pszLValue != NULL && pszRValue != NULL )
				return !stricmp( pszLValue, pszRValue );

			return false;
		}

		unsigned int operator()( const MenuConVarBind_t &item ) const
		{
			if ( item.cvar != NULL )
			{
				return HashStringCaseless( item.cvar->name );
			}
			else if ( item.convar != NULL )
			{
				return HashStringCaseless( item.convar->GetName() );
			}

			return 0;
		}
	};

private:
	bool			m_bOpened;
	bool			m_bThemeLoaded;
	bool			m_bImGuiInitialized;
	float			m_flOpenTime;
	float			m_flCloseTime;

	CConfigProperty *m_pCfgMenuKey;
	CConfigProperty *m_pCfgMenuHide;
	CConfigProperty *m_pCfgMenuOpacity;
	CConfigProperty *m_pCfgMenuFont;
	CConfigProperty *m_pCfgMenuTheme;
	CConfigProperty *m_pCfgAutoexecGameConfig;
	CConfigProperty *m_pCfgAutoexecShaderConfig;

	CConfigProperty *m_pCfgMenuRainbowEnable;
	CConfigProperty *m_pCfgMenuRainbowUpdate;
	CConfigProperty *m_pCfgMenuRainbowHue;
	CConfigProperty *m_pCfgMenuRainbowSaturation;
	CConfigProperty *m_pCfgMenuRainbowLightness;
	CConfigProperty *m_pCfgMenuRainbowAlpha;

	menu_custom_theme_vars m_CustomThemeVars;

	float			m_flRainbowColor[ 3 ];
	float			m_flRainbowDelta;
	float			m_flRainbowUpdateTime;

	struct ImFont	*m_pFont;
	struct ImFont	*m_pFontL4D;

	Vector			m_va;

	CLookupFunctor m_Functor;
	CHash<MenuConVarBind_t, CLookupFunctor &, CLookupFunctor &> m_MenuConVarBinds;

	std::vector<CMenuCategory> m_categories;

	std::vector<std::string> m_Configs;
	std::vector<std::string> m_ShaderConfigs;

	std::string m_sSelectedConfig;
	std::string m_sSelectedShadersConfig;
	std::string m_sConfigsFullPath;
	std::string m_sShaderConfigsFullPath;

#if !IMGUI_USE_SDL
	HWND			m_hGameWnd;
	WNDPROC			m_hGameWndProc;
#endif
};

namespace Modules { extern CMenuModule *menu; }

#endif // SINT_MENU_MODULE_H
