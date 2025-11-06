// SvenInt (c) Sw1ft
// r_bsp.cpp

#include "stdafx.h"
#include "r_bsp.h"
#include "r_drawing.h"
#include "r_draw_context.h"

#include <istream>
#include <streambuf>
#include <algorithm>

#include <gl/GL.h>

using namespace Globals;

//-----------------------------------------------------------------------------
// Structs
//-----------------------------------------------------------------------------

struct membuf : std::streambuf
{
	membuf( unsigned char *begin, int len )
	{
		this->setg( (char *)begin, (char *)begin, (char *)begin + len );
	}
};

//-----------------------------------------------------------------------------
// Vars
//-----------------------------------------------------------------------------

EXPOSE_FEATURE_SINGLETON( CBsp, bsp, "Render", "BSP" );

//-----------------------------------------------------------------------------
// CDrawTransModel
//-----------------------------------------------------------------------------

class CDrawTransModel : public IDrawContext
{
public:
	CDrawTransModel( int modelindex );

	virtual void	Draw( void );
	virtual bool	ShouldStopDraw( void );

	virtual const Vector &GetDrawOrigin( void ) const;

private:
	int m_modelindex;
	Vector m_vecMidPoint;
};

CDrawTransModel::CDrawTransModel( int modelindex )
{
	if ( THIS_FEATURE()->GetData() == NULL )
		return;

	m_modelindex = modelindex;

	dheader_t *header = (dheader_t *)THIS_FEATURE()->GetData();

	lump_t *lump_models = &header->lumps[ LUMP_MODELS ];

	dmodel_t *models = (dmodel_t *)( THIS_FEATURE()->GetData() + lump_models->fileofs );
	int models_count = lump_models->filelen / sizeof( dmodel_t );

	dmodel_t *model = &models[ m_modelindex ];

	Vector vecOrigin = *(Vector *)model->origin;
	Vector vecMins = *(Vector *)model->mins;
	Vector vecMaxs = *(Vector *)model->maxs;

	m_vecMidPoint = vecOrigin + vecMins;
	m_vecMidPoint += ( ( vecOrigin + vecMaxs ) - ( vecOrigin + vecMins ) ) * 0.5f;
}

void CDrawTransModel::Draw( void )
{
	if ( THIS_FEATURE()->GetData() == NULL )
		return;

	glEnable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	glDisable( GL_TEXTURE_2D );

	dheader_t *header = (dheader_t *)THIS_FEATURE()->GetData();

	lump_t *lump_models = &header->lumps[ LUMP_MODELS ];
	lump_t *lump_nodes = &header->lumps[ LUMP_NODES ];
	lump_t *lump_faces = &header->lumps[ LUMP_FACES ];
	lump_t *lump_edges = &header->lumps[ LUMP_EDGES ];
	lump_t *lump_surfedges = &header->lumps[ LUMP_SURFEDGES ];
	lump_t *lump_vertexes = &header->lumps[ LUMP_VERTEXES ];

	dmodel_t *models = (dmodel_t *)( THIS_FEATURE()->GetData() + lump_models->fileofs );
	int models_count = lump_models->filelen / sizeof( dmodel_t );

	dnode_t *nodes = (dnode_t *)( THIS_FEATURE()->GetData() + lump_nodes->fileofs );
	int nodes_count = lump_nodes->filelen / sizeof( dnode_t );

	dface_t *faces = (dface_t *)( THIS_FEATURE()->GetData() + lump_faces->fileofs );
	int faces_count = lump_faces->filelen / sizeof( dface_t );

	dedge_t *edges = (dedge_t *)( THIS_FEATURE()->GetData() + lump_edges->fileofs );
	int edges_count = lump_edges->filelen / sizeof( dedge_t );

	uint32_t *surfedges = (uint32_t *)( THIS_FEATURE()->GetData() + lump_surfedges->fileofs );
	int surfedges_count = lump_surfedges->filelen / sizeof( uint32_t );

	dvertex_t *vertexes = (dvertex_t *)( THIS_FEATURE()->GetData() + lump_vertexes->fileofs );
	int vertexes_count = lump_vertexes->filelen / sizeof( dvertex_t );

	dmodel_t *model = &models[ m_modelindex ];

	glColor4f( 1.f, 1.f, 1.f, 0.6f );

	for ( int i = 0; i < model->numfaces; i++ )
	{
		int facenum = model->firstface + i;
		dface_t *face = &faces[ facenum ];

		glBegin( GL_TRIANGLE_FAN );
		//glBegin(GL_TRIANGLES);

		for ( int j = 0; j < face->numedges; j++ )
		{
			int32_t edgenum = surfedges[ face->firstedge + j ];
			dedge_t *edge = &edges[ abs( edgenum ) ];

			int edgevertexnum = ( edgenum >= 0 ? 1 : 0 );
			int vertexnum = edge->v[ edgevertexnum ];

			Vector vertex = *(Vector *)&vertexes[ vertexnum ];

			//Features::drawcontext->DrawPoint(vertex, { 255, 255, 255, 255 }, 24.f, 10.f);
			glVertex3f( vertex.x, vertex.y, vertex.z );
		}

		glEnd();
	}

	glEnable( GL_TEXTURE_2D );

	glDisable( GL_BLEND );
	glDisable( GL_ALPHA_TEST );
}

bool CDrawTransModel::ShouldStopDraw( void )
{
	return false;
}

const Vector &CDrawTransModel::GetDrawOrigin( void ) const
{
	return m_vecMidPoint;
}

//-----------------------------------------------------------------------------
// Utilities
//-----------------------------------------------------------------------------

static inline std::string &rtrim( std::string &s, const char *t )
{
	s.erase( s.find_last_not_of( t ) + 1 );
	return s;
}

static inline std::string &ltrim( std::string &s, const char *t )
{
	s.erase( 0, s.find_first_not_of( t ) );
	return s;
}

static inline std::string &trim( std::string &s, const char *t )
{
	return ltrim( rtrim( s, t ), t );
}

//-----------------------------------------------------------------------------
// ConVars / ConCommands
//-----------------------------------------------------------------------------

CON_COMMAND( sc_bsp_draw_model, "Draw a specified BSP model by their index" )
{
	if ( args.ArgC() > 1 )
	{
		if ( THIS_FEATURE()->GetData() == NULL )
			return;

		int modelindex = atoi( args[ 1 ] );

		Features::drawcontext->AddDrawContext( new CDrawTransModel( modelindex ), 5.f );

		model_t *pModel = enginestudio->GetModelByIndex( modelindex );

		if (pModel != NULL)
		{
			Msg( "[MODEL] type: %d\n", pModel->type );
			Msg( "[MODEL] numframes: %d\n", pModel->numframes );
			Msg( "[MODEL] synctype: %d\n", pModel->synctype );
			Msg( "[MODEL] flags: %d\n", pModel->flags );
			Msg( "[MODEL] radius: %d\n", pModel->radius );
			Msg( "[MODEL] firstmodelsurface: %d\n", pModel->firstmodelsurface );
			Msg( "[MODEL] nummodelsurfaces: %d\n", pModel->nummodelsurfaces );
			Msg( "[MODEL] numsubmodels: %d\n", pModel->numsubmodels );
			Msg( "[MODEL] numplanes: %d\n", pModel->numplanes );
			Msg( "[MODEL] numleafs: %d\n", pModel->numleafs );
			Msg( "[MODEL] numvertexes: %d\n", pModel->numvertexes );
			Msg( "[MODEL] numedges: %d\n", pModel->numedges );
			Msg( "[MODEL] numnodes: %d\n", pModel->numnodes );
			Msg( "[MODEL] numtexinfo: %d\n", pModel->numtexinfo );
			Msg( "[MODEL] numsurfaces: %d\n", pModel->numsurfaces );
			Msg( "[MODEL] numsurfedges: %d\n", pModel->numsurfedges );
			Msg( "[MODEL] numclipnodes: %d\n", pModel->numclipnodes );
			Msg( "[MODEL] nummarksurfaces: %d\n", pModel->nummarksurfaces );
			Msg( "[MODEL] numtextures: %d\n", pModel->numtextures );

			for ( int i = 0; i < MAX_MAP_HULLS; i++ )
			{
				Msg( "[MODEL HULLS] hulls[%d].firstclipnode: %d\n", i, pModel->hulls[ i ].firstclipnode );
				Msg( "[MODEL HULLS] hulls[%d].lastclipnode: %d\n", i, pModel->hulls[ i ].lastclipnode );
				Msg( "[MODEL HULLS] hulls[%d].clip_mins: %.2f %.2f %.2f\n", i, VectorExpand( pModel->hulls[ i ].clip_mins ) );
				Msg( "[MODEL HULLS] hulls[%d].clip_maxs: %.2f %.2f %.2f\n\n", i, VectorExpand( pModel->hulls[ i ].clip_maxs ) );
			}
		}

		dheader_t *header = (dheader_t *)THIS_FEATURE()->GetData();

		lump_t *lump_models = &header->lumps[ LUMP_MODELS ];
		lump_t *lump_nodes = &header->lumps[ LUMP_NODES ];
		lump_t *lump_faces = &header->lumps[ LUMP_FACES ];
		lump_t *lump_edges = &header->lumps[ LUMP_EDGES ];
		lump_t *lump_surfedges = &header->lumps[ LUMP_SURFEDGES ];
		lump_t *lump_vertexes = &header->lumps[ LUMP_VERTEXES ];

		dmodel_t *models = (dmodel_t *)( THIS_FEATURE()->GetData() + lump_models->fileofs );
		int models_count = lump_models->filelen / sizeof( dmodel_t );

		dnode_t *nodes = (dnode_t *)( THIS_FEATURE()->GetData() + lump_nodes->fileofs );
		int nodes_count = lump_nodes->filelen / sizeof( dnode_t );

		dface_t *faces = (dface_t *)( THIS_FEATURE()->GetData() + lump_faces->fileofs );
		int faces_count = lump_faces->filelen / sizeof( dface_t );

		dedge_t *edges = (dedge_t *)( THIS_FEATURE()->GetData() + lump_edges->fileofs );
		int edges_count = lump_edges->filelen / sizeof( dedge_t );

		uint32_t *surfedges = (uint32_t *)( THIS_FEATURE()->GetData() + lump_surfedges->fileofs );
		int surfedges_count = lump_surfedges->filelen / sizeof( uint32_t );

		dvertex_t *vertexes = (dvertex_t *)( THIS_FEATURE()->GetData() + lump_vertexes->fileofs );
		int vertexes_count = lump_vertexes->filelen / sizeof( dvertex_t );

		dedge_t *pivot_edge = &edges[ 0 ];

		// dump
		Msg( "Model Index: %d\n", modelindex );

		dmodel_t *model = &models[ modelindex ];

		Msg( "Number of faces #%d\n", model->numfaces );

		Msg( "origin: %.3f %.3f %.3f\n", VectorExpand( *(Vector *)model->origin ) );
		Msg( "mins: %.3f %.3f %.3f\n", VectorExpand( *(Vector *)model->mins ) );
		Msg( "maxs: %.3f %.3f %.3f\n", VectorExpand( *(Vector *)model->maxs ) );
		Msg( "headnode: %d %d %d %d\n", model->headnode[ 0 ], model->headnode[ 1 ], model->headnode[ 2 ], model->headnode[ 3 ] );
		Msg( "visleafs: %d\n", model->visleafs );
		Msg( "firstface: %d\n", model->firstface );
		Msg( "numfaces: %d\n", model->numfaces );

		//for (int n = 0; n < MAX_MAP_HULLS; n++)
		//{
		//	int nodenum = model->headnode[n];
		//	if (nodenum == 0)
		//		continue;

		//	bspnode_t *node = &nodes[nodenum];

		//	Msg("nodenum: %d\n", nodenum);
		//	Msg("planenum: %d\n", node->planenum);
		//	Msg("children[0]: %d\n", node->children[0]);
		//	Msg("children[1]: %d\n", node->children[1]);
		//	Msg("mins: %d %d %d\n", node->mins[0], node->mins[1], node->mins[2]);
		//	Msg("maxs: %d %d %d\n", node->maxs[0], node->maxs[1], node->maxs[2]);
		//	Msg("firstface: %d\n", node->firstface);
		//	Msg("numfaces: %d\n", node->numfaces);

		//	continue;

		//	for (int i = 0; i < node->numfaces; i++)
		//	{
		//		int facenum = node->firstface + i;
		//		bspface_t *face = &faces[facenum];

		//		Msg("Face #%d\n", facenum);
		//		Msg("Face side #%d\n", face->side);

		//		Msg("Number of sides #%d\n", face->numedges);

		//		for (int j = 0; j < face->numedges; j++)
		//		{
		//			int edgenum = face->firstedge + j;
		//			bspedge_t *edge = &edges[abs(edgenum)];

		//			int edgevertexnum = (edgenum >= 0 ? 1 : 0);
		//			int vertexnum = edge->v[edgevertexnum];

		//			Vector vertex = *(Vector *)&vertexes[vertexnum];
		//			////Vector vertex = *(Vector *)&vertexes[pivot_edge->v[edgevertexnum]] - *(Vector *)&vertexes[vertexnum];

		//			Msg("Edge vertex[%d] - %d (%.3f %.3f %.3f)\n", edgevertexnum, vertexnum, VectorExpand(vertex));

		//			Debug()->DrawPoint(vertex, { 255, 255, 255, 255 }, 24.f, 10.f);

		//			////Vector vertex1 = *(Vector *)&vertexes[edge->v[0]] - *(Vector *)&vertexes[pivot_edge->v[0]];
		//			////Vector vertex2 = *(Vector *)&vertexes[edge->v[1]] - *(Vector *)&vertexes[pivot_edge->v[1]];

		//			////Msg("Edge #%d\n", edgenum);
		//			////Msg("Edge vertex[0] - %d (%.3f %.3f %.3f)\n", edge->v[0], VectorExpand(vertex1));
		//			////Msg("Edge vertex[1] - %d (%.3f %.3f %.3f)\n", edge->v[1], VectorExpand(vertex2));

		//			////if ( edge->v[0] && edge->v[1] )
		//			////{
		//			////	bspvertex_t *vertex1 = &vertexes[edge->v[0]];
		//			////	bspvertex_t *vertex2 = &vertexes[edge->v[1]];

		//			////	Debug()->DrawPoint( vertex1->point, { 255, 255, 255, 255 }, 24.f, 10.f );
		//			////	Debug()->DrawPoint( vertex2->point, { 255, 255, 255, 255 }, 24.f, 10.f );
		//			////}
		//		}

		//		Msg("\n");
		//	}
		//}

		for ( int i = 0; i < model->numfaces; i++ )
		{
			int facenum = model->firstface + i;
			dface_t *face = &faces[ facenum ];

			Msg( "Face #%d\n", facenum );
			Msg( "Face side #%d\n", face->side );

			Msg( "Number of edges #%d\n", face->numedges );

			for ( int j = 0; j < face->numedges; j++ )
			{
				int32_t edgenum = surfedges[ face->firstedge + j ];
				//int edgenum = face->firstedge + j;
				dedge_t *edge = &edges[ abs( edgenum ) ];

				int edgevertexnum = ( edgenum >= 0 ? 1 : 0 );
				int vertexnum = edge->v[ edgevertexnum ];

				Vector vertex = *(Vector *)&vertexes[ vertexnum ];
				////Vector vertex = *(Vector *)&vertexes[pivot_edge->v[edgevertexnum]] - *(Vector *)&vertexes[vertexnum];

				Msg( "Edge (%d) > vertex[%d] - %d (%.3f %.3f %.3f)\n", edgenum, edgevertexnum, vertexnum, VectorExpand( vertex ) );

				//Features::drawcontext->DrawPoint( vertex, { 255, 255, 255, 255 }, 24.f, 10.f );
				Features::drawcontext->DrawBox( vertex, Vector( -2, -2, -2 ), Vector( 2, 2, 2 ), { 255, 255, 255, 127 }, 10.f );

				////Vector vertex1 = *(Vector *)&vertexes[edge->v[0]] - *(Vector *)&vertexes[pivot_edge->v[0]];
				////Vector vertex2 = *(Vector *)&vertexes[edge->v[1]] - *(Vector *)&vertexes[pivot_edge->v[1]];

				////Msg("Edge #%d\n", edgenum);
				////Msg("Edge vertex[0] - %d (%.3f %.3f %.3f)\n", edge->v[0], VectorExpand(vertex1));
				////Msg("Edge vertex[1] - %d (%.3f %.3f %.3f)\n", edge->v[1], VectorExpand(vertex2));

				////if ( edge->v[0] && edge->v[1] )
				////{
				////	bspvertex_t *vertex1 = &vertexes[edge->v[0]];
				////	bspvertex_t *vertex2 = &vertexes[edge->v[1]];

				////	Debug()->DrawPoint( vertex1->point, { 255, 255, 255, 255 }, 24.f, 10.f );
				////	Debug()->DrawPoint( vertex2->point, { 255, 255, 255, 255 }, 24.f, 10.f );
				////}
			}

			Msg( "\n" );
		}
	}
	else
	{
		for ( int i = 0; i < MAX_MAP_MODELS; i++ )
		{
			model_t *pModel = enginestudio->GetModelByIndex( i );
			if ( pModel == NULL )
				continue;

			Msg( "Modelname: %s (%d)\n", pModel->name, i );
		}
	}
}

//-----------------------------------------------------------------------------
// Draw utilities
//-----------------------------------------------------------------------------

float CBsp::GetPulsatingAlpha( float a, float time, float speed )
{
	float s = sinf( speed * time );
	s *= 0.05f;
	s += a;
	s = Q_max( s, 0.0f );
	s = Q_min( s, 1.0f );
	return s;
}

void CBsp::DrawTrianglesBox( const Vector &vecTriggerOrigin, const Vector &vecTriggerMins, const Vector &vecTriggerMaxs, float r, float g, float b, float a, float width, bool wireframe )
{
	if ( wireframe )
	{
		DrawBox( vecTriggerOrigin, vecTriggerMins, vecTriggerMaxs, r, g, b, a, width, true );
		return;
	}

	Vector vecPoints[ 8 ];

	Vector vecMins = vecTriggerMins;
	Vector vecMaxs = vecTriggerMaxs;

	VectorAdd( vecMins, vecTriggerOrigin, vecMins );
	VectorAdd( vecMaxs, vecTriggerOrigin, vecMaxs );

	// Build points of box
	vecPoints[ 0 ].x = vecMins.x;
	vecPoints[ 0 ].y = vecMins.y;
	vecPoints[ 0 ].z = vecMins.z;

	vecPoints[ 1 ].x = vecMins.x;
	vecPoints[ 1 ].y = vecMaxs.y;
	vecPoints[ 1 ].z = vecMins.z;

	vecPoints[ 2 ].x = vecMaxs.x;
	vecPoints[ 2 ].y = vecMaxs.y;
	vecPoints[ 2 ].z = vecMins.z;

	vecPoints[ 3 ].x = vecMaxs.x;
	vecPoints[ 3 ].y = vecMins.y;
	vecPoints[ 3 ].z = vecMins.z;

	vecPoints[ 4 ].x = vecMins.x;
	vecPoints[ 4 ].y = vecMins.y;
	vecPoints[ 4 ].z = vecMaxs.z;

	vecPoints[ 5 ].x = vecMins.x;
	vecPoints[ 5 ].y = vecMaxs.y;
	vecPoints[ 5 ].z = vecMaxs.z;

	vecPoints[ 6 ].x = vecMaxs.x;
	vecPoints[ 6 ].y = vecMaxs.y;
	vecPoints[ 6 ].z = vecMaxs.z;

	vecPoints[ 7 ].x = vecMaxs.x;
	vecPoints[ 7 ].y = vecMins.y;
	vecPoints[ 7 ].z = vecMaxs.z;

	float flPulsateOffset = m_pTriggerPulsateOffset->GetFloat();
	float flPulsateSpeed = m_pTriggerPulsateSpeed->GetFloat();

	float orig_alpha = a;
	int nSurface = 0;
	float flSurfaceOffset = 0.f;
	const float flTime = m_pTriggerPulsate->GetBool() ? cl_enginefuncs->GetClientTime() : 0.f;

	for ( int i = 0; i < 4; i++ )
	{
		int j = ( i + 1 ) % 4;

		flSurfaceOffset = (float)nSurface * float( M_PI ) / flPulsateOffset;
		a = GetPulsatingAlpha( orig_alpha, flTime + flSurfaceOffset, flPulsateSpeed );

		cl_enginefuncs->pTriAPI->Color4f( r, g, b, a );
		cl_enginefuncs->pTriAPI->Begin( TRI_POLYGON );
			cl_enginefuncs->pTriAPI->Vertex3f( VectorExpand( vecPoints[ i ] ) );
			cl_enginefuncs->pTriAPI->Vertex3f( VectorExpand( vecPoints[ j ] ) );
			cl_enginefuncs->pTriAPI->Vertex3f( VectorExpand( vecPoints[ j + 4 ] ) );
			cl_enginefuncs->pTriAPI->Vertex3f( VectorExpand( vecPoints[ i + 4 ] ) );
		cl_enginefuncs->pTriAPI->End();

		nSurface++;
	}

	// Bottom
	flSurfaceOffset = (float)nSurface * float( M_PI ) / flPulsateOffset;
	a = GetPulsatingAlpha( orig_alpha, flTime + flSurfaceOffset, flPulsateSpeed );

	cl_enginefuncs->pTriAPI->Color4f( r, g, b, a );
	cl_enginefuncs->pTriAPI->Begin( TRI_POLYGON );
		cl_enginefuncs->pTriAPI->Vertex3f( VectorExpand( vecPoints[ 0 ] ) );
		cl_enginefuncs->pTriAPI->Vertex3f( VectorExpand( vecPoints[ 1 ] ) );
		cl_enginefuncs->pTriAPI->Vertex3f( VectorExpand( vecPoints[ 2 ] ) );
		cl_enginefuncs->pTriAPI->Vertex3f( VectorExpand( vecPoints[ 3 ] ) );
	cl_enginefuncs->pTriAPI->End();

	nSurface++;

	// Top
	flSurfaceOffset = (float)nSurface * float( M_PI ) / flPulsateOffset;
	a = GetPulsatingAlpha( orig_alpha, flTime + flSurfaceOffset, flPulsateSpeed );

	cl_enginefuncs->pTriAPI->Color4f( r, g, b, a );
	cl_enginefuncs->pTriAPI->Begin( TRI_POLYGON );
		cl_enginefuncs->pTriAPI->Vertex3f( VectorExpand( vecPoints[ 4 ] ) );
		cl_enginefuncs->pTriAPI->Vertex3f( VectorExpand( vecPoints[ 5 ] ) );
		cl_enginefuncs->pTriAPI->Vertex3f( VectorExpand( vecPoints[ 6 ] ) );
		cl_enginefuncs->pTriAPI->Vertex3f( VectorExpand( vecPoints[ 7 ] ) );
	cl_enginefuncs->pTriAPI->End();
}

//-----------------------------------------------------------------------------
// Draw primitives
//-----------------------------------------------------------------------------

void CBsp::DrawTriangles( void )
{
	glDisable( GL_TEXTURE_2D );

	if ( m_pShowTriggers->GetBool() )
	{
		cl_enginefuncs->pTriAPI->RenderMode( kRenderTransAdd );
		cl_enginefuncs->pTriAPI->CullFace( TRI_NONE );

		for ( const CBspTriggerEntity &trigger : m_triggers )
		{
			float r, g, b, a;
			bool bDraw = true;

			switch ( trigger.iType )
			{
			case TRIGGER_ONCE:
				if ( m_pShowTriggerOnce->GetBool() )
				{
					r = m_pTriggerOnceColor->GetColor()[ 0 ];
					g = m_pTriggerOnceColor->GetColor()[ 1 ];
					b = m_pTriggerOnceColor->GetColor()[ 2 ];
					a = m_pTriggerOnceColor->GetColor()[ 3 ];
				}
				else
				{
					bDraw = false;
				}

				break;

			case TRIGGER_MULTIPLE:
				if ( m_pShowTriggerMultiple->GetBool() )
				{
					r = m_pTriggerMultipleColor->GetColor()[ 0 ];
					g = m_pTriggerMultipleColor->GetColor()[ 1 ];
					b = m_pTriggerMultipleColor->GetColor()[ 2 ];
					a = m_pTriggerMultipleColor->GetColor()[ 3 ];
				}
				else
				{
					bDraw = false;
				}

				break;

			case TRIGGER_HURT:
				if ( m_pShowTriggerHurt->GetBool() )
				{
					r = m_pTriggerHurtColor->GetColor()[ 0 ];
					g = m_pTriggerHurtColor->GetColor()[ 1 ];
					b = m_pTriggerHurtColor->GetColor()[ 2 ];
					a = m_pTriggerHurtColor->GetColor()[ 3 ];
				}
				else
				{
					bDraw = false;
				}

				break;

			case TRIGGER_HURT_HEAL:
				if ( m_pShowTriggerHurtHeal->GetBool() )
				{
					r = m_pTriggerHurtHealColor->GetColor()[ 0 ];
					g = m_pTriggerHurtHealColor->GetColor()[ 1 ];
					b = m_pTriggerHurtHealColor->GetColor()[ 2 ];
					a = m_pTriggerHurtHealColor->GetColor()[ 3 ];
				}
				else
				{
					bDraw = false;
				}

				break;

			case TRIGGER_PUSH:
				if ( m_pShowTriggerPush->GetBool() )
				{
					r = m_pTriggerPushColor->GetColor()[ 0 ];
					g = m_pTriggerPushColor->GetColor()[ 1 ];
					b = m_pTriggerPushColor->GetColor()[ 2 ];
					a = m_pTriggerPushColor->GetColor()[ 3 ];
				}
				else
				{
					bDraw = false;
				}

				break;

			case TRIGGER_TELEPORT:
				if ( m_pShowTriggerTeleport->GetBool() )
				{
					r = m_pTriggerTeleportColor->GetColor()[ 0 ];
					g = m_pTriggerTeleportColor->GetColor()[ 1 ];
					b = m_pTriggerTeleportColor->GetColor()[ 2 ];
					a = m_pTriggerTeleportColor->GetColor()[ 3 ];
				}
				else
				{
					bDraw = false;
				}

				break;

			case TRIGGER_CHANGELEVEL:
				if ( m_pShowTriggerChangelevel->GetBool() )
				{
					r = m_pTriggerChangelevelColor->GetColor()[ 0 ];
					g = m_pTriggerChangelevelColor->GetColor()[ 1 ];
					b = m_pTriggerChangelevelColor->GetColor()[ 2 ];
					a = m_pTriggerChangelevelColor->GetColor()[ 3 ];
				}
				else
				{
					bDraw = false;
				}

				break;

			case TRIGGER_ANTIRUSH:
				if ( m_pShowTriggerAntirush->GetBool() )
				{
					r = m_pTriggerAntirushColor->GetColor()[ 0 ];
					g = m_pTriggerAntirushColor->GetColor()[ 1 ];
					b = m_pTriggerAntirushColor->GetColor()[ 2 ];
					a = m_pTriggerAntirushColor->GetColor()[ 3 ];
				}
				else
				{
					bDraw = false;
				}

				break;

			default:
				r = 1.f;
				g = 1.f;
				b = 1.f;
				a = 0.5f;
				break;
			}

			if ( bDraw )
			{
				DrawTrianglesBox( trigger.vecOrigin, trigger.vecMins, trigger.vecMaxs, r, g, b, a, 4.f, m_pWireframe->GetBool() );

				if ( trigger.iType == TRIGGER_PUSH )
				{
					Vector vecEnd;
					float dist = ( trigger.vecMaxs - trigger.vecMins ).Length();

					VectorMA( trigger.vecMidPoint, dist * 0.75f, trigger.vecDirection, vecEnd );

					Features::drawcontext->DrawLine( trigger.vecMidPoint, vecEnd, r, g, b, a, 10.f );
				}
			}
		}
	}

	if ( m_pShowWalls->GetBool() )
	{
		for ( const CBspFuncWall &funcWall : m_funcWalls )
		{
			DrawTrianglesBox( funcWall.vecOrigin, funcWall.vecMins, funcWall.vecMaxs, 0.f, 1.f, 1.f, 32.f / 255.f, 4.f, m_pWireframe->GetBool() );
		}
	}

	glEnable( GL_TEXTURE_2D );

	cl_enginefuncs->pTriAPI->RenderMode( kRenderNormal );
}

//-----------------------------------------------------------------------------
// VGui paint
//-----------------------------------------------------------------------------

void CBsp::Draw( void )
{
	if ( m_pShowSpawns->GetBool() )
	{
		for ( const CBspMonsterSpawn &monster : m_monsterSpawns )
		{
			Vector2D vecScreen;
			Vector vecOrigin = monster.vecOrigin;

			if ( !UTIL_WorldToScreen( vecOrigin, vecScreen ) )
				continue;

			pmtrace_t trace;

			Vector vecStart = localplayer->GetEyePosition();

			cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_POINT );
			cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecStart, vecOrigin, PM_WORLD_ONLY, -1, &trace );

			if ( trace.fraction != 1.f )
				continue;

			Features::drawing->DrawString( Features::drawing->GetFontESP(),
										   (int)vecScreen.x, (int)vecScreen.y,
										   255, 255, 255, 255,
										   FONT_ALIGN_CENTER,
										   monster.szClassname );
		}
	}

	if ( m_pShowTriggers->GetBool() && m_pShowTriggersInfo->GetBool() )
	{
		for ( const CBspTriggerEntity &trigger : m_triggers )
		{
			Color clr;
			bool bDraw = true;

			switch ( trigger.iType )
			{
			case TRIGGER_ONCE:
			case TRIGGER_MULTIPLE:
				bDraw = false;
				break;

			case TRIGGER_HURT:
				if ( m_pShowTriggerHurt->GetBool() )
				{
					clr.SetColor( m_pTriggerHurtColor->GetColor()[ 0 ],
								  m_pTriggerHurtColor->GetColor()[ 1 ],
								  m_pTriggerHurtColor->GetColor()[ 2 ],
								  1.f );
				}
				else
				{
					bDraw = false;
				}

				break;

			case TRIGGER_HURT_HEAL:
				if ( m_pShowTriggerHurtHeal->GetBool() )
				{
					clr.SetColor( m_pTriggerHurtHealColor->GetColor()[ 0 ],
								  m_pTriggerHurtHealColor->GetColor()[ 1 ],
								  m_pTriggerHurtHealColor->GetColor()[ 2 ],
								  1.f );
				}
				else
				{
					bDraw = false;
				}

				break;

			case TRIGGER_PUSH:
				if ( m_pShowTriggerPush->GetBool() )
				{
					clr.SetColor( m_pTriggerPushColor->GetColor()[ 0 ],
								  m_pTriggerPushColor->GetColor()[ 1 ],
								  m_pTriggerPushColor->GetColor()[ 2 ],
								  1.f );
				}
				else
				{
					bDraw = false;
				}

				break;

			case TRIGGER_TELEPORT:
			case TRIGGER_CHANGELEVEL:
				break;

			case TRIGGER_ANTIRUSH:
				if ( m_pShowTriggerAntirush->GetBool() )
				{
					clr.SetColor( m_pTriggerAntirushColor->GetColor()[ 0 ],
								  m_pTriggerAntirushColor->GetColor()[ 1 ],
								  m_pTriggerAntirushColor->GetColor()[ 2 ],
								  1.f );
				}
				else
				{
					bDraw = false;
				}

				break;
			}

			if ( bDraw )
			{
				Vector2D vecScreen;
				Vector vecOrigin = trigger.vecMidPoint;

				if ( !UTIL_WorldToScreen( vecOrigin, vecScreen ) )
					continue;

				pmtrace_t trace;

				Vector vecStart = localplayer->GetEyePosition();

				cl_enginefuncs->pEventAPI->EV_SetTraceHull( PM_HULL_POINT );
				cl_enginefuncs->pEventAPI->EV_PlayerTrace( vecStart, vecOrigin, PM_WORLD_ONLY, -1, &trace );

				if ( trace.fraction != 1.f )
				{
					continue;
				}

				switch ( trigger.iType )
				{
				case TRIGGER_ONCE:
				case TRIGGER_MULTIPLE:
					break;

				case TRIGGER_HURT:
					Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
													(int)vecScreen.x, (int)vecScreen.y,
													clr.r, clr.g, clr.b, 255,
													FONT_ALIGN_CENTER,
													"Damage: %d", trigger.iDamage);
					break;

				case TRIGGER_HURT_HEAL:
					Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
													(int)vecScreen.x, (int)vecScreen.y,
													clr.r, clr.g, clr.b, 255,
													FONT_ALIGN_CENTER,
													"Heal: %d", abs( trigger.iDamage ) );
					break;

				case TRIGGER_PUSH:
					Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
													(int)vecScreen.x, (int)vecScreen.y,
													clr.r, clr.g, clr.b, 255,
													FONT_ALIGN_CENTER,
													"Push Speed: %d", trigger.iSpeed );
					break;

				case TRIGGER_TELEPORT:
					Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
													(int)vecScreen.x, (int)vecScreen.y,
													clr.r, clr.g, clr.b, 255,
													FONT_ALIGN_CENTER,
													"Model: %d", trigger.iModel );
					break;

				case TRIGGER_CHANGELEVEL:
					break;

				case TRIGGER_ANTIRUSH:
					Features::drawing->DrawStringF( Features::drawing->GetFontESP(),
													(int)vecScreen.x, (int)vecScreen.y,
													clr.r, clr.g, clr.b, 255,
													FONT_ALIGN_CENTER,
													"Percentage: %.1f %%", trigger.flPercentage * 100.f );
					break;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Load and parse a BSP map
//-----------------------------------------------------------------------------

void CBsp::LoadBSP( void )
{
	DeleteBspData();

	m_monsterSpawns.clear();
	m_triggers.clear();
	m_ents.clear();

	char mapname[ MAX_PATH ];

	strncpy( mapname, cl_enginefuncs->pfnGetLevelName(), MAX_PATH );
	mapname[ MAX_PATH - 1 ] = 0;

	FileHandle_t hFile = filesystem->Open( mapname, "r", "GAME" );
	if ( !hFile )
		hFile = filesystem->Open( mapname, "r", "GAMEDOWNLOAD" );

	if ( !hFile )
		return;

	int file_len = filesystem->Size( hFile );
	m_pBSP = new uint8_t[ file_len ];

	if ( !m_pBSP )
	{
		PrintWarning( "Failed to allocate memory for a BSP file\n" );
		return;
	}

	filesystem->Read( m_pBSP, file_len, hFile );
	filesystem->Close( hFile );

	dheader_t *header = (dheader_t *)m_pBSP;

	lump_t *lump_entities = &header->lumps[ LUMP_ENTITIES ];
	lump_t *lump_models = &header->lumps[ LUMP_MODELS ];

	dmodel_t *models = (dmodel_t *)( m_pBSP + lump_models->fileofs );
	int models_count = lump_models->filelen / sizeof( dmodel_t );

	if ( !LoadEntsFromBSP( m_pBSP, lump_entities ) )
		return;

	const std::string sClassname = "classname";
	const std::string sMonsterType = "monstertype";
	const std::string sModel = "model";
	const std::string sOrigin = "origin";
	const std::string sAngles = "angles";
	const std::string sMinHullSize = "minhullsize";
	const std::string sMaxHullSize = "maxhullsize";
	const std::string sPercentage = "m_flPercentage";
	const std::string sSpeed = "speed";
	const std::string sDamage = "damage";
	const std::string sDmg = "dmg";

	for ( const CBspEntityKeyValues &keyvalues : m_ents )
	{
		auto found_classname = keyvalues.find( sClassname );
		if ( found_classname == keyvalues.end() )
			continue;

		bool bEntSpawn = false;
		bool bFuncWall = false;

	#define INVALID_BSP_TRIGGER (EBspTriggerType)(-1)
		EBspTriggerType trigger_type = INVALID_BSP_TRIGGER;

		const std::string &classname = keyvalues.at( sClassname );

		if ( !classname.rfind( "monster_", 0 ) )
		{
			auto stringEndsWith = []( std::string const &str, std::string const &suffix )
			{
				if ( str.length() < suffix.length() )
					return false;

				return std::equal( suffix.rbegin(), suffix.rend(), str.rbegin() );
			};

			if ( !stringEndsWith( classname, "_dead" ) )
			{
				bEntSpawn = true;
			}
		}
		else if ( classname == "monstermaker" || classname == "squadmaker" || classname == "env_xenmaker" || classname == "info_player_deathmatch" )
		{
			bEntSpawn = true;
		}
		else if ( classname == "func_wall" )
		{
			bFuncWall = true;
		}
		else if ( classname == "trigger_once" )
		{
			trigger_type = TRIGGER_ONCE;
		}
		else if ( classname == "trigger_multiple" )
		{
			trigger_type = TRIGGER_MULTIPLE;
		}
		else if ( classname == "trigger_hurt" )
		{
			trigger_type = TRIGGER_HURT;
		}
		else if ( classname == "trigger_push" )
		{
			trigger_type = TRIGGER_PUSH;
		}
		else if ( classname == "trigger_teleport" )
		{
			trigger_type = TRIGGER_TELEPORT;
		}
		else if ( classname == "trigger_changelevel" )
		{
			trigger_type = TRIGGER_CHANGELEVEL;
		}
		else if ( classname == "trigger_once_mp" )
		{
			trigger_type = TRIGGER_ANTIRUSH;
		}

		if ( trigger_type != INVALID_BSP_TRIGGER )
		{
			CBspTriggerEntity trigger;
			ZeroMemory( &trigger, sizeof( CBspTriggerEntity ) );

			trigger.iType = trigger_type;

			if ( trigger_type != TRIGGER_ANTIRUSH )
			{
				auto found_model = keyvalues.find( sModel );
				auto found_origin = keyvalues.find( sOrigin );
				auto found_angles = keyvalues.find( sAngles );

				if ( trigger_type == TRIGGER_HURT )
				{
					auto found_dmg = keyvalues.find( sDmg );
					auto found_damage = keyvalues.find( sDamage );

					if ( found_damage != keyvalues.end() )
					{
						trigger.iDamage = atoi( keyvalues.at( sDamage ).c_str() );

						if ( trigger.iDamage < 0 )
						{
							trigger.iType = TRIGGER_HURT_HEAL;
						}
					}
					else if ( found_dmg != keyvalues.end() )
					{
						trigger.iDamage = atoi( keyvalues.at( sDmg ).c_str() );

						if ( trigger.iDamage < 0 )
						{
							trigger.iType = TRIGGER_HURT_HEAL;
						}
					}
				}
				else if ( trigger_type == TRIGGER_PUSH )
				{
					auto found_speed = keyvalues.find( sSpeed );

					if ( found_speed != keyvalues.end() )
					{
						trigger.iSpeed = atoi( keyvalues.at( sSpeed ).c_str() );
					}
				}

				if ( found_model != keyvalues.end() )
				{
					if ( keyvalues.at( sModel )[ 0 ] == '*' )
					{
						int iModelIndex = atoi( keyvalues.at( sModel ).c_str() + 1 );

						if ( iModelIndex > 0 && iModelIndex < models_count )
						{
							dmodel_t *model = &models[ iModelIndex ];

							trigger.iModel = iModelIndex;

							trigger.vecOrigin = model->origin;
							trigger.vecMins = model->mins;
							trigger.vecMaxs = model->maxs;
						}
					}
				}

				if ( found_origin != keyvalues.end() )
				{
					float x = 0.f;
					float y = 0.f;
					float z = 0.f;

					int nParamsRead = sscanf( keyvalues.at( sOrigin ).c_str(), "%f %f %f", &x, &y, &z );

					if ( nParamsRead >= 3 )
					{
						trigger.vecOrigin.x += x;
						trigger.vecOrigin.y += y;
						trigger.vecOrigin.z += z;
					}
					else if ( nParamsRead == 2 )
					{
						trigger.vecOrigin.x += x;
						trigger.vecOrigin.y += y;
					}
					else if ( nParamsRead == 1 )
					{
						trigger.vecOrigin.x += x;
					}
				}

				if ( found_angles != keyvalues.end() )
				{
					Vector vecAngles;

					int nParamsRead = sscanf( keyvalues.at( sAngles ).c_str(), "%f %f %f", &vecAngles.x, &vecAngles.y, &vecAngles.z );

					if ( nParamsRead != 0 )
					{
						AngleVectors( vecAngles, &trigger.vecDirection, NULL, NULL );
					}
				}
			}
			else // TRIGGER_ANTIRUSH
			{
				auto found_mins = keyvalues.find( sMinHullSize );
				auto found_maxs = keyvalues.find( sMaxHullSize );

				if ( found_mins == keyvalues.end() || found_maxs == keyvalues.end() ) // not found
					continue;

				float x = 0.f;
				float y = 0.f;
				float z = 0.f;

				// Mins
				int nParamsRead = sscanf( keyvalues.at( sMinHullSize ).c_str(), "%f %f %f", &x, &y, &z );

				if ( nParamsRead >= 3 )
				{
					trigger.vecOrigin.x = x;
					trigger.vecOrigin.y = y;
					trigger.vecOrigin.z = z;
				}
				else if ( nParamsRead == 2 )
				{
					trigger.vecOrigin.x = x;
					trigger.vecOrigin.y = y;
				}
				else if ( nParamsRead == 1 )
				{
					trigger.vecOrigin.x = x;
				}

				// Maxs
				nParamsRead = sscanf( keyvalues.at( sMaxHullSize ).c_str(), "%f %f %f", &x, &y, &z );

				if ( nParamsRead >= 3 )
				{
					trigger.vecMaxs.x = x - trigger.vecOrigin.x;
					trigger.vecMaxs.y = y - trigger.vecOrigin.y;
					trigger.vecMaxs.z = z - trigger.vecOrigin.z;
				}
				else if ( nParamsRead == 2 )
				{
					trigger.vecMaxs.x = x - trigger.vecOrigin.x;
					trigger.vecMaxs.y = y - trigger.vecOrigin.y;
				}
				else if ( nParamsRead == 1 )
				{
					trigger.vecMaxs.x = x - trigger.vecOrigin.x;
				}

				auto found_percentage = keyvalues.find( sPercentage );

				if ( found_percentage != keyvalues.end() )
				{
					trigger.flPercentage = (float)atof( keyvalues.at( sPercentage ).c_str() );
				}
			}

			trigger.vecMidPoint = trigger.vecOrigin + trigger.vecMins;
			trigger.vecMidPoint += ( ( trigger.vecOrigin + trigger.vecMaxs ) - ( trigger.vecOrigin + trigger.vecMins ) ) * 0.5f;

			m_triggers.push_back( trigger );
		}
		else if ( bEntSpawn )
		{
			CBspMonsterSpawn monster;
			ZeroMemory( &monster, sizeof( CBspMonsterSpawn ) );

			auto found_origin = keyvalues.find( sOrigin );

			if ( found_origin != keyvalues.end() )
			{
				float x = 0.f;
				float y = 0.f;
				float z = 0.f;

				int nParamsRead = sscanf( keyvalues.at( sOrigin ).c_str(), "%f %f %f", &x, &y, &z );

				if ( nParamsRead >= 3 )
				{
					monster.vecOrigin.x = x;
					monster.vecOrigin.y = y;
					monster.vecOrigin.z = z;
				}
				else if ( nParamsRead == 2 )
				{
					monster.vecOrigin.x = x;
					monster.vecOrigin.y = y;
				}
				else if ( nParamsRead == 1 )
				{
					monster.vecOrigin.x = x;
				}

				//auto found_monstertype = keyvalues.find(sOrigin);

				if ( classname == "monstermaker" || classname == "squadmaker" || classname == "env_xenmaker" )
				{
					auto found_monstertype = keyvalues.find( sMonsterType );

					if ( found_origin == keyvalues.end() )
						continue;

					strncpy_s( monster.szClassname, keyvalues.at( sMonsterType ).c_str(), keyvalues.at( sMonsterType ).length() );
				}
				else
				{
					strncpy_s( monster.szClassname, classname.c_str(), classname.length() );
				}

				m_monsterSpawns.push_back( monster );
			}
		}
		else if ( bFuncWall )
		{
			CBspFuncWall funcWall;
			ZeroMemory( &funcWall, sizeof( CBspFuncWall ) );

			auto found_model = keyvalues.find( sModel );
			auto found_origin = keyvalues.find( sOrigin );

			if ( found_model != keyvalues.end() )
			{
				if ( keyvalues.at( sModel )[ 0 ] == '*' )
				{
					int iModelIndex = atoi( keyvalues.at( sModel ).c_str() + 1 );

					if ( iModelIndex > 0 && iModelIndex < models_count )
					{
						dmodel_t *model = &models[ iModelIndex ];

						funcWall.vecOrigin = model->origin;
						funcWall.vecMins = model->mins;
						funcWall.vecMaxs = model->maxs;
					}
				}
			}

			if ( found_origin != keyvalues.end() )
			{
				float x = 0.f;
				float y = 0.f;
				float z = 0.f;

				int nParamsRead = sscanf( keyvalues.at( sOrigin ).c_str(), "%f %f %f", &x, &y, &z );

				if ( nParamsRead >= 3 )
				{
					funcWall.vecOrigin.x += x;
					funcWall.vecOrigin.y += y;
					funcWall.vecOrigin.z += z;
				}
				else if ( nParamsRead == 2 )
				{
					funcWall.vecOrigin.x += x;
					funcWall.vecOrigin.y += y;
				}
				else if ( nParamsRead == 1 )
				{
					funcWall.vecOrigin.x += x;
				}
			}

			m_funcWalls.push_back( funcWall );
		}
	}

	m_ents.clear();
}

//-----------------------------------------------------------------------------
// LoadEntsFromBSP
//-----------------------------------------------------------------------------

bool CBsp::LoadEntsFromBSP( unsigned char *bsp, lump_t *lump_entities )
{
	static const char *trim_chars = " \t\n\r\f\v";

	membuf sbuf( bsp + lump_entities->fileofs, lump_entities->filelen );
	FILE *file = fopen( SVENINT_FOLDER_NAME "/last_entmap.ent", "w" );

	std::istream buffer( &sbuf );
	std::string sLine;

	int nLine = 0;
	int mode = 0;

	while ( std::getline( buffer, sLine ) )
	{
		nLine++;

		sLine = trim( sLine, trim_chars );

		if ( sLine.empty() || !sLine[ 0 ] )
			continue;

		if ( file != NULL )
			fprintf( file, "%s\n", sLine.c_str() );

		if ( sLine[ 0 ] == '/' )
			continue;

		if ( mode == 0 )
		{
			if ( sLine[ 0 ] == '{' )
			{
				mode = 1;

				m_ents.push_back( CBspEntityKeyValues() );
			}
			else
			{
				Warning( "[LoadEntsFromBsp] Expected start of keyvalues at line %d\n", nLine );

				if ( file != NULL )
					fclose( file );

				return false;
			}
		}
		else if ( mode == 1 )
		{
			if ( sLine[ 0 ] == '}' )
			{
				mode = 0;
			}
			else if ( sLine[ 0 ] == '{' )
			{
				Warning( "[LoadEntsFromBsp] Expected end of keyvalues at line %d\n", nLine );

				if ( file != NULL )
					fclose( file );

				return false;
			}
			else
			{
				std::string sKey, sValue;

				char *buffer = (char *)( sLine.c_str() );
				char *key = buffer + 1;
				char *value = NULL;

				if ( *buffer != '\"' )
				{
					Warning( "[LoadEntsFromBsp] Expected start of key at line %d\n", nLine );

					if ( file != NULL )
						fclose( file );

					return false;
				}

				if ( *key == '\"' )
					continue;

				buffer++;

				while ( *buffer )
				{
					if ( *buffer == '\"' )
					{
						char ch = *buffer;
						*buffer = 0;

						sKey = key;

						*buffer = ch;

						buffer++;
						break;
					}

					buffer++;
				}

				while ( *buffer )
				{
					if ( *buffer == '\"' )
					{
						buffer++;
						value = buffer;
						break;
					}

					buffer++;
				}

				if ( value != NULL && *value != '\"' )
				{
					while ( *buffer )
					{
						if ( *buffer == '\"' )
						{
							char ch = *buffer;
							*buffer = 0;

							sValue = value;

							*buffer = ch;

							break;
						}

						buffer++;
					}
				}
				else
				{
					sValue = "";
				}

				m_ents.back()[ sKey ] = sValue;
			}
		}
	}

	if ( file != NULL )
		fclose( file );

	return true;
}

//-----------------------------------------------------------------------------
// DeleteBspData
//-----------------------------------------------------------------------------

void CBsp::DeleteBspData( void )
{
	if ( m_pBSP != NULL )
	{
		delete[] m_pBSP;
		m_pBSP = NULL;
	}
}

//-----------------------------------------------------------------------------
// Hook event
//-----------------------------------------------------------------------------

EHookResult CBsp::OnEvent( CHookEvent *pEvent, bool bPostCall )
{
	if ( pEvent->GetType() == kHUD_OnClientDisconnect_HookEvent )
	{
		DeleteBspData();
	}
	else if ( pEvent->GetType() == kVGuiClientPanelPaint_HookEvent )
	{
		Draw();
	}
	else if ( pEvent->GetType() == kHUD_DrawTransparentTriangles_HookEvent )
	{
		DrawTriangles();
	}
	else if ( pEvent->GetType() == kServerInfo_HookEvent )
	{
		LoadBSP();
	}

	return kHookContinue;
}

//-----------------------------------------------------------------------------
// Init feature
//-----------------------------------------------------------------------------

CBsp::CBsp( const char *pszCategoryName, const char *pszName ) : CBaseFeature( pszCategoryName, pszName )
{
	m_pWireframe = NULL;
	m_pShowSpawns = NULL;
	m_pShowWalls = NULL;
	m_pShowTriggers = NULL;
	m_pShowTriggersInfo = NULL;
	m_pShowTriggerOnce = NULL;
	m_pShowTriggerMultiple = NULL;
	m_pShowTriggerHurt = NULL;
	m_pShowTriggerHurtHeal = NULL;
	m_pShowTriggerPush = NULL;
	m_pShowTriggerTeleport = NULL;
	m_pShowTriggerChangelevel = NULL;
	m_pShowTriggerAntirush = NULL;

	m_pTriggerPulsate = NULL;
	m_pTriggerPulsateOffset = NULL;
	m_pTriggerPulsateSpeed = NULL;

	m_pTriggerOnceColor = NULL;
	m_pTriggerMultipleColor = NULL;
	m_pTriggerHurtColor = NULL;
	m_pTriggerHurtHealColor = NULL;
	m_pTriggerPushColor = NULL;
	m_pTriggerTeleportColor = NULL;
	m_pTriggerChangelevelColor = NULL;
	m_pTriggerAntirushColor = NULL;

	m_pBSP = NULL;
}

//-----------------------------------------------------------------------------
// Enable feature: called from cmds or menu, register event listeners etc..
//-----------------------------------------------------------------------------

void CBsp::OnEnable( void )
{
	hookevents->RegisterListener( this, kHUD_OnClientDisconnect_HookEvent );
	hookevents->RegisterListener( this, kVGuiClientPanelPaint_HookEvent );
	hookevents->RegisterListener( this, kHUD_DrawTransparentTriangles_HookEvent );
	hookevents->RegisterListener( this, kServerInfo_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Disable feature: revert OnEnable routine
//-----------------------------------------------------------------------------

void CBsp::OnDisable( void )
{
	hookevents->UnregisterListener( this, kHUD_OnClientDisconnect_HookEvent );
	hookevents->UnregisterListener( this, kVGuiClientPanelPaint_HookEvent );
	hookevents->UnregisterListener( this, kHUD_DrawTransparentTriangles_HookEvent );
	hookevents->UnregisterListener( this, kServerInfo_HookEvent, kHookPostCall );
}

//-----------------------------------------------------------------------------
// Load feature: create menu (config) parameters, scan for signatures etc...
//-----------------------------------------------------------------------------

bool CBsp::Load( void )
{
	Modules::menu->BindFeature( this );
	Modules::menu->AddElementResetButton( this, "Reset" );

	m_pWireframe = Modules::menu->AddParamBool( this, "Wireframe", NULL, false );
	m_pShowSpawns = Modules::menu->AddParamBool( this, "ShowSpawns", NULL, false );
	m_pShowWalls = Modules::menu->AddParamBool( this, "ShowWalls", NULL, false );
	m_pShowTriggers = Modules::menu->AddParamBool( this, "ShowTriggers", NULL, false );
	m_pShowTriggersInfo = Modules::menu->AddParamBool( this, "ShowTriggersInfo", NULL, false );
	m_pShowTriggerOnce = Modules::menu->AddParamBool( this, "ShowTriggerOnce", NULL, true );
	m_pShowTriggerMultiple = Modules::menu->AddParamBool( this, "ShowTriggerMultiple", NULL, true );
	m_pShowTriggerHurt = Modules::menu->AddParamBool( this, "ShowTriggerHurt", NULL, true );
	m_pShowTriggerHurtHeal = Modules::menu->AddParamBool( this, "ShowTriggerHurtHeal", NULL, true );
	m_pShowTriggerPush = Modules::menu->AddParamBool( this, "ShowTriggerPush", NULL, true );
	m_pShowTriggerTeleport = Modules::menu->AddParamBool( this, "ShowTriggerTeleport", NULL, true );
	m_pShowTriggerChangelevel = Modules::menu->AddParamBool( this, "ShowTriggerChangelevel", NULL, true );
	m_pShowTriggerAntirush = Modules::menu->AddParamBool( this, "ShowTriggerAntirush", NULL, true );

	Modules::menu->AddElementSeparator( this );

	m_pTriggerPulsate = Modules::menu->AddParamBool( this, "TriggerPulsate", NULL, true );
	m_pTriggerPulsateOffset = Modules::menu->AddParamFloat( this, "TriggerPulsateOffset", NULL, 5.f, 1.f, 10.f );
	m_pTriggerPulsateSpeed = Modules::menu->AddParamFloat( this, "TriggerPulsateSpeed", NULL, 8.f, 0.f, 15.f );

	Modules::menu->AddElementSeparator( this );

	m_pTriggerOnceColor = Modules::menu->AddParamColorRGBA( this, "TriggerOnceColor", NULL, Color( 255, 255, 0, 32 ) );
	m_pTriggerMultipleColor = Modules::menu->AddParamColorRGBA( this, "TriggerMultipleColor", NULL, Color( 255, 255, 0, 32 ) );
	m_pTriggerHurtColor = Modules::menu->AddParamColorRGBA( this, "TriggerHurtColor", NULL, Color( 255, 0, 0, 32 ) );
	m_pTriggerHurtHealColor = Modules::menu->AddParamColorRGBA( this, "TriggerHurtHealColor", NULL, Color( 0, 255, 0, 32 ) );
	m_pTriggerPushColor = Modules::menu->AddParamColorRGBA( this, "TriggerPushColor", NULL, Color( 255, 127, 0, 32 ) );
	m_pTriggerTeleportColor = Modules::menu->AddParamColorRGBA( this, "TriggerTeleportColor", NULL, Color( int( 255.f * 0.9084f ), 0, int( 255.f * 0.9084f ), 32 ) );
	m_pTriggerChangelevelColor = Modules::menu->AddParamColorRGBA( this, "TriggerChangelevelColor", NULL, Color( 255, 255, 255, 32 ) );
	m_pTriggerAntirushColor = Modules::menu->AddParamColorRGBA( this, "TriggerAntirushColor", NULL, Color( 0, 63, 255, 32 ) );

	return true;
}

//-----------------------------------------------------------------------------
// Post load feature: register cvars/commands, attach hooks etc...
//-----------------------------------------------------------------------------

void CBsp::PostLoad( void )
{
	cvar->RegisterConCommand( &EXPAND_CON_COMMAND( sc_bsp_draw_model ) );
}

//-----------------------------------------------------------------------------
// Unload feature: revert PostLoad routine
//-----------------------------------------------------------------------------

void CBsp::Unload( void )
{
	DeleteBspData();
	cvar->UnregisterConCommand( &EXPAND_CON_COMMAND( sc_bsp_draw_model ) );
}