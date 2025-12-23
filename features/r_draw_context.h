// SvenInt (c) Sw1ft
// r_draw_context.h

#ifndef SINT_FEATURE_DRAW_CONTEXT_H
#define SINT_FEATURE_DRAW_CONTEXT_H

#ifdef _WIN32
#pragma once
#endif

#include "base_feature.h"
#include "game/hook_events.h"

//-----------------------------------------------------------------------------
// Purpose: draw interface
//-----------------------------------------------------------------------------

class IDrawContext
{
public:
	virtual			~IDrawContext() {}

	virtual void	Draw( void ) = 0;
	virtual bool	ShouldStopDraw( void ) = 0;

	virtual const Vector &GetDrawOrigin( void ) const = 0;

	virtual bool	CheckPVS( void ) { return true; }
};

//-----------------------------------------------------------------------------
// Draw box, no depth buffer
//-----------------------------------------------------------------------------

class CDrawBoxNoDepthBuffer : public IDrawContext
{
public:
	CDrawBoxNoDepthBuffer( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, const Color &color );
	virtual ~CDrawBoxNoDepthBuffer( void ) {}

	virtual void Draw( void ) override;
	virtual bool ShouldStopDraw( void ) override { return false; };

	virtual const Vector &GetDrawOrigin( void ) const override { return m_vecDrawOrigin; };

private:
	Vector m_vecDrawOrigin;
	Vector m_vecOrigin;

	Vector m_vecMins;
	Vector m_vecMaxs;

	Color m_color;
};

//-----------------------------------------------------------------------------
// Draw wireframe box
//-----------------------------------------------------------------------------

class CWireframeBox : public IDrawContext
{
public:
	CWireframeBox( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, const Color &color, float width, bool bIgnoreDepthBuffer );
	virtual ~CWireframeBox( void ) {}

	virtual void Draw( void ) override;
	virtual bool ShouldStopDraw( void ) override { return false; };

	virtual const Vector &GetDrawOrigin( void ) const override { return m_vecDrawOrigin; };

	virtual bool CheckPVS( void ) override;

private:
	Vector m_vecDrawOrigin;
	Vector m_vecOrigin;

	Vector m_vecMins;
	Vector m_vecMaxs;

	Color m_color;

	float m_flWidth;
	bool m_bIgnoreDepthBuffer;
};

//-----------------------------------------------------------------------------
// Draw wireframe rotated box
//-----------------------------------------------------------------------------

class CWireframeBoxAngles : public IDrawContext
{
public:
	CWireframeBoxAngles( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, const Vector &vAngles, const Color &color, float width, bool bIgnoreDepthBuffer );
	virtual ~CWireframeBoxAngles( void ) {}

	virtual void Draw( void ) override;
	virtual bool ShouldStopDraw( void ) override { return false; };

	virtual const Vector &GetDrawOrigin( void ) const override { return m_vecDrawOrigin; };

	virtual bool CheckPVS( void ) override;

private:
	Vector m_vecDrawOrigin;
	Vector m_vecOrigin;
	Vector m_vecAngles;

	Vector m_vecMins;
	Vector m_vecMaxs;

	Color m_color;

	float m_flWidth;
	bool m_bIgnoreDepthBuffer;
};

//-----------------------------------------------------------------------------
// Draw linear trajectory
//-----------------------------------------------------------------------------

class CDrawTrajectory : public IDrawContext
{
public:
	CDrawTrajectory( const Color &lineColor, const Color &impactColor, float flWidth = 2.f );
	virtual ~CDrawTrajectory();

	virtual void Draw() override;
	virtual bool ShouldStopDraw() override;

	virtual const Vector &GetDrawOrigin() const override;

public:
	void AddLine( const Vector &start, const Vector &end );
	void AddImpact( const Vector &impact );

private:
	std::vector<Vector> m_trajectoryLines;
	std::vector<Vector> m_impacts;

	Color m_lineColor;
	Color m_impactColor;

	float m_flWidth;
};

//-----------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------

void DrawBox( const Vector &vecOrigin, const Vector &vecMins, const Vector &vecMaxs, float r, float g, float b, float alpha, float width, bool wireframe, float duration = 0.f );
void DrawBoxAngles( const Vector &vecOrigin, const Vector &vecMins, const Vector &vecMaxs, const Vector &vecAngles, float r, float g, float b, float alpha, float width, bool wireframe, float duration = 0.f );

//-----------------------------------------------------------------------------
// Draw context internal struct
//-----------------------------------------------------------------------------

struct draw_context_t
{
	inline bool operator <( const draw_context_t &draw_context ) const
	{
		return ( flDistanceSqr > draw_context.flDistanceSqr );
	}

	IDrawContext *pDrawContext;
	float flDuration;
	float flDistanceSqr;
};

//-----------------------------------------------------------------------------
// Draw context feature
//-----------------------------------------------------------------------------

class CDrawContext final : public CBaseFeature, IHookEventListener
{
public:
	CDrawContext( const char *pszCategoryName, const char *pszName );

	virtual bool Load( void ) override;
	virtual void PostLoad( void ) override;
	virtual void Unload( void ) override;

	virtual EHookResult OnEvent( CHookEvent *pEvent, bool bPostCall ) override;

public:
	void SetRenderOrigin( const Vector &vecOrigin );

	void AddDrawContext( IDrawContext *pContext, float duration = 0.f );

	void DrawPoint( const Vector &vPoint, const Color &color, float size = 24.f, float duration = 0.f );
	void DrawLine( const Vector &vStart, const Vector &vEnd, const Color &color, float width = 2.f, float duration = 0.f );
	void DrawBox( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, const Color &color, float duration = 0.f );
	void DrawBoxAngles( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, const Vector &vAngles, const Color &color, float duration = 0.f );
	void DrawClear( void );

	inline void DrawPoint( const Vector &vPoint, float r, float g, float b, float alpha, float size = 24.f, float duration = 0.f )
	{
		DrawPoint( vPoint, { r, g, b, alpha }, size, duration );
	}
	inline void DrawLine( const Vector &vStart, const Vector &vEnd, float r, float g, float b, float alpha, float width = 2.f, float duration = 0.f )
	{
		DrawLine( vStart, vEnd, { r, g, b, alpha }, width, duration );
	}
	inline void DrawBox( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, float r, float g, float b, float alpha, float duration = 0.f )
	{
		DrawBox( vOrigin, vMins, vMaxs, { r, g, b, alpha }, duration );
	}
	inline void DrawBoxAngles( const Vector &vOrigin, const Vector &vMins, const Vector &vMaxs, const Vector &vAngles, float r, float g, float b, float alpha, float duration = 0.f )
	{
		DrawBoxAngles( vOrigin, vMins, vMaxs, vAngles, { r, g, b, alpha }, duration );
	}

private:
	std::vector<draw_context_t> m_vDrawContext;
	Vector						m_vecRenderOrigin;
};

EXTERN_FEATURE( CDrawContext, drawcontext );

#endif // SINT_FEATURE_DRAW_CONTEXT_H