#include "recastjs.h"
#include "Recast.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshBuilder.h"
#include "DetourNavMesh.h"
#include "DetourNavMeshQuery.h"

#include <stdio.h>
#include <vector>
#include <float.h>
#include <algorithm>
#include <math.h>

void Log(const char* str)
{
    printf("%s", str);
}

struct vec_t
{
    vec_t() {}
    vec_t(float v) : x(v), y(v), z(v) {}
    vec_t(float x, float y, float z) : x(x), y(y), z(z) {}
    void isMinOf(const vec_t& v)
    {
        x = std::min(x, v.x);
        y = std::min(y, v.y);
        z = std::min(z, v.z);
    }
    void isMaxOf(const vec_t& v)
    {
        x = std::max(x, v.x);
        y = std::max(y, v.y);
        z = std::max(z, v.z);
    }
    float operator [](int index) {
        return ((float*)&x)[index];
    }
    float x, y, z;
};

void NavMesh::Build(const float* positions, const int positionCount, const int* indices, const int indexCount, const rcConfig& config)
{
	std::vector<vec_t> mTriangles;
	const float* pv = &positions[0];
	const int* t = &indices[0];

	vec_t bbMin(FLT_MAX);
	vec_t bbMax(-FLT_MAX);
	mTriangles.resize(indexCount);
	for (unsigned int i = 0; i<indexCount; i++)
	{
		int ind = (*t++) * 3;
		vec_t v( -pv[ind], pv[ind+2], pv[ind+1] );
		bbMin.isMinOf( v );
		bbMax.isMaxOf( v );
		mTriangles[i] = v;
	}

    bool m_keepInterResults = false;
    float m_agentHeight;
	float m_agentRadius;
	float m_agentMaxClimb;
    m_agentHeight = 0.5f;
	m_agentRadius = 0.5f;
	m_agentMaxClimb = 0.3f;

	// Init build configuration from GUI
	/*rcConfig m_cfg;	
	float m_cellSize;
	float m_cellHeight;
	float m_agentHeight;
	float m_agentRadius;
	float m_agentMaxClimb;
	float m_agentMaxSlope;
	float m_regionMinSize;
	float m_regionMergeSize;
	bool m_monotonePartitioning;
	float m_edgeMaxLen;
	float m_edgeMaxError;
	float m_vertsPerPoly;
	float m_detailSampleDist;
	float m_detailSampleMaxError;

	

	m_cellSize = 0.1f;
	m_cellHeight = 0.1f;
	m_agentHeight = 0.5f;
	m_agentRadius = mRadius;
	m_agentMaxClimb = 0.3f;
	m_agentMaxSlope = 90.0f;
	m_regionMinSize = 8;
	m_regionMergeSize = 20;
	m_monotonePartitioning = false;
	m_edgeMaxLen = 12.0f;
	m_edgeMaxError = 1.3f;
	m_vertsPerPoly = 6.0f;
	m_detailSampleDist = 6.0f;
	m_detailSampleMaxError = 1.0f;


	memset(&m_cfg, 0, sizeof(m_cfg));
	m_cfg.cs = m_cellSize;
	m_cfg.ch = m_cellHeight;
	m_cfg.walkableSlopeAngle = m_agentMaxSlope;
	m_cfg.walkableHeight = (int)ceilf(m_agentHeight / m_cfg.ch);
	m_cfg.walkableClimb = (int)floorf(m_agentMaxClimb / m_cfg.ch);
	m_cfg.walkableRadius = (int)ceilf(m_agentRadius / m_cfg.cs);
	m_cfg.maxEdgeLen = (int)(m_edgeMaxLen / m_cellSize);
	m_cfg.maxSimplificationError = m_edgeMaxError;
	m_cfg.minRegionArea = (int)rcSqr(m_regionMinSize);		// Note: area = size*size
	m_cfg.mergeRegionArea = (int)rcSqr(m_regionMergeSize);	// Note: area = size*size
	m_cfg.maxVertsPerPoly = (int)m_vertsPerPoly;
	m_cfg.detailSampleDist = m_detailSampleDist < 0.9f ? 0 : m_cellSize * m_detailSampleDist;
	m_cfg.detailSampleMaxError = m_cellHeight * m_detailSampleMaxError;
	
	// Set the area where the navigation will be build.
	// Here the bounds of the input mesh are used, but the
	// area could be specified by an user defined box, etc.
	//float bmin[3] = {-20.f, 0.f, -20.f};
	//float bmax[3] = { 20.f, 1.f,  20.f};
	rcVcopy(m_cfg.bmin, &bbMin.x);
	rcVcopy(m_cfg.bmax, &bbMax.x);
    */
    rcConfig m_cfg = config;
	rcCalcGridSize(m_cfg.bmin, m_cfg.bmax, m_cfg.cs, &m_cfg.width, &m_cfg.height);

	rcContext ctx;
	rcContext* m_ctx = &ctx;

	// Reset build times gathering.
	//m_ctx->resetTimers();

	// Start the build process.	
	//m_ctx->startTimer(RC_TIMER_TOTAL);
	
	//m_ctx->log(RC_LOG_PROGRESS, "Building navigation:");
	//m_ctx->log(RC_LOG_PROGRESS, " - %d x %d cells", m_cfg.width, m_cfg.height);
	//m_ctx->log(RC_LOG_PROGRESS, " - %.1fK verts, %.1fK tris", nverts/1000.0f, ntris/1000.0f);
	
	//
	// Step 2. Rasterize input polygon soup.
	//
	rcHeightfield* m_solid;
	// Allocate voxel heightfield where we rasterize our input data to.
	m_solid = rcAllocHeightfield();
	if (!m_solid)
	{
		Log("buildNavigation: Out of memory 'solid'.");
		return ;
	}
	if (!rcCreateHeightfield(m_ctx, *m_solid, m_cfg.width, m_cfg.height, m_cfg.bmin, m_cfg.bmax, m_cfg.cs, m_cfg.ch))
	{
		Log("buildNavigation: Could not create solid heightfield.");
		return ;
	}
	
		float *verts = new float[mTriangles.size()*3];
	int nverts = mTriangles.size();
	for (unsigned int i =0;i<mTriangles.size();i++)
	{
		verts[i*3+0] = mTriangles[i].x;
		verts[i*3+1] = mTriangles[i].y;
		verts[i*3+2] = mTriangles[i].z;
	}
	int ntris = mTriangles.size()/3;
	int *tris = new int[mTriangles.size()];
	for (unsigned int i = 0;i<mTriangles.size();i++)
		tris[i] = mTriangles.size()-i-1;


	//InputGeom* m_geom;
	// Allocate array that can hold triangle area types.
	// If you have multiple meshes you need to process, allocate
	// and array which can hold the max number of triangles you need to process.
	unsigned char*m_triareas = (unsigned char*)malloc(ntris);
	
	
	// Find triangles which are walkable based on their slope and rasterize them.
	// If your input data is multiple meshes, you can transform them here, calculate
	// the are type for each of the meshes and rasterize them.
	memset(m_triareas, RC_WALKABLE_AREA, ntris*sizeof(unsigned char));



	//rcMarkWalkableTriangles(m_ctx, m_cfg.walkableSlopeAngle, verts, nverts, tris, ntris, m_triareas);
	rcRasterizeTriangles(m_ctx, verts, nverts, tris, m_triareas, ntris, *m_solid, m_cfg.walkableClimb);

	free(m_triareas);
	/*
	if (!m_keepInterResults)
	{
		delete [] m_triareas;
		m_triareas = 0;
	}
	*/
	//
	// Step 3. Filter walkables surfaces.
	//
	
	// Once all geoemtry is rasterized, we do initial pass of filtering to
	// remove unwanted overhangs caused by the conservative rasterization
	// as well as filter spans where the character cannot possibly stand.
	
	rcFilterLowHangingWalkableObstacles(m_ctx, m_cfg.walkableClimb, *m_solid);
	rcFilterLedgeSpans(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid);
	rcFilterWalkableLowHeightSpans(m_ctx, m_cfg.walkableHeight, *m_solid);
	

	//
	// Step 4. Partition walkable surface to simple regions.
	// 

	// Compact the heightfield so that it is faster to handle from now on.
	// This will result more cache coherent data as well as the neighbours
	// between walkable cells will be calculated.
	rcCompactHeightfield* m_chf;
	m_chf = rcAllocCompactHeightfield();
	if (!m_chf)
	{
		Log("buildNavigation: Out of memory 'chf'.");
		return ;
	}
	
	if (!rcBuildCompactHeightfield(m_ctx, m_cfg.walkableHeight, m_cfg.walkableClimb, *m_solid, *m_chf))
	{
		Log("buildNavigation: Could not build compact data.");
		return ;
	}
	
	if (!m_keepInterResults)
	{
		rcFreeHeightField(m_solid);
		m_solid = 0;
	}
		
	// Erode the walkable area by agent radius.
	if (!rcErodeWalkableArea(m_ctx, m_cfg.walkableRadius, *m_chf))
	{
		Log("buildNavigation: Could not erode.");
		return ;
	}

	// (Optional) Mark areas.
	//class InputGeom* m_geom;
	/*
	const ConvexVolume* vols = m_geom->getConvexVolumes();
	for (int i  = 0; i < m_geom->getConvexVolumeCount(); ++i)
		rcMarkConvexPolyArea(m_ctx, vols[i].verts, vols[i].nverts, vols[i].hmin, vols[i].hmax, (unsigned char)vols[i].area, *m_chf);
	*/
	/*
	if (m_monotonePartitioning)
	{
		// Partition the walkable surface into simple regions without holes.
		// Monotone partitioning does not need distancefield.
		if (!rcBuildRegionsMonotone(m_ctx, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
		{
			//m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build regions.");
			return false;
		}
	}
	else
	*/
	{
		// Prepare for region partitioning, by calculating Distance field along the walkable surface.
		if (!rcBuildDistanceField(m_ctx, *m_chf))
		{
			Log("buildNavigation: Could not build Distance field.");
			return ;
		}

		// Partition the walkable surface into simple regions without holes.
		if (!rcBuildRegions(m_ctx, *m_chf, 0, m_cfg.minRegionArea, m_cfg.mergeRegionArea))
		{
			Log("buildNavigation: Could not build regions.");
			return ;
		}
	}
	
	//
	// Step 5. Trace and simplify region contours.
	//
	
	// Create contours.
	rcContourSet* m_cset;
	m_cset = rcAllocContourSet();
	if (!m_cset)
	{
		Log("buildNavigation: Out of memory 'cset'.");
		return ;
	}
	if (!rcBuildContours(m_ctx, *m_chf, m_cfg.maxSimplificationError, m_cfg.maxEdgeLen, *m_cset))
	{
		Log("buildNavigation: Could not create contours.");
		return ;
	}
	
	//
	// Step 6. Build polygons mesh from contours.
	//
	
	// Build polygon navmesh from the contours.
	rcPolyMesh* m_pmesh;
	m_pmesh = rcAllocPolyMesh();
	if (!m_pmesh)
	{
		Log("buildNavigation: Out of memory 'pmesh'.");
		return ;
	}
	if (!rcBuildPolyMesh(m_ctx, *m_cset, m_cfg.maxVertsPerPoly, *m_pmesh))
	{
		Log("buildNavigation: Could not triangulate contours.");
		return ;
	}
	
	//
	// Step 7. Create detail mesh which allows to access approximate height on each polygon.
	//
	rcPolyMeshDetail* m_dmesh;
	m_dmesh = rcAllocPolyMeshDetail();
	if (!m_dmesh)
	{
		Log("buildNavigation: Out of memory 'pmdtl'.");
		return ;
	}

	if (!rcBuildPolyMeshDetail(m_ctx, *m_pmesh, *m_chf, m_cfg.detailSampleDist, m_cfg.detailSampleMaxError, *m_dmesh))
	{
		//m_ctx->log(RC_LOG_ERROR, "buildNavigation: Could not build detail mesh.");
		return ;
	}

	if (!m_keepInterResults)
	{
		rcFreeCompactHeightfield(m_chf);
		m_chf = 0;
		rcFreeContourSet(m_cset);
		m_cset = 0;
	}
	/*dtNavMeshQuery* */m_navQuery = dtAllocNavMeshQuery();
	//dtNavMesh* m_navMesh;
	//m_navQuery = dtAllocNavMeshQuery();

	//m_pmesh->
	
	// At this point the navigation mesh data is ready, you can access it from m_pmesh.
	// See duDebugDrawPolyMesh or dtCreateNavMeshData as examples how to access the data.
	

	//
	// (Optional) Step 8. Create Detour data from Recast poly mesh.
	//
	
	// The GUI may allow more max points per polygon than Detour can handle.
	// Only build the detour navmesh if we do not exceed the limit.
	if (m_cfg.maxVertsPerPoly <= DT_VERTS_PER_POLYGON)
	{
		unsigned char* navData = 0;
		int navDataSize = 0;

		// Update poly flags from areas.
		
		for (int i = 0; i < m_pmesh->npolys; ++i)
		{
			if (m_pmesh->areas[i] == RC_WALKABLE_AREA)
				m_pmesh->areas[i] = 0;//SAMPLE_POLYAREA_GROUND;
				
			if (m_pmesh->areas[i] == 0/*SAMPLE_POLYAREA_GROUND ||
				m_pmesh->areas[i] == SAMPLE_POLYAREA_GRASS ||
				m_pmesh->areas[i] == SAMPLE_POLYAREA_ROAD*/)
			{
				m_pmesh->flags[i] = 1/*SAMPLE_POLYFLAGS_WALK*/;
			}
			/*
			else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_WATER)
			{
				m_pmesh->flags[i] = SAMPLE_POLYFLAGS_SWIM;
			}
			else if (m_pmesh->areas[i] == SAMPLE_POLYAREA_DOOR)
			{
				m_pmesh->flags[i] = SAMPLE_POLYFLAGS_WALK | SAMPLE_POLYFLAGS_DOOR;
			}
		*/	
		}
		

		dtNavMeshCreateParams params;
		memset(&params, 0, sizeof(params));
		params.verts = m_pmesh->verts;
		params.vertCount = m_pmesh->nverts;
		params.polys = m_pmesh->polys;
		params.polyAreas = m_pmesh->areas;
		params.polyFlags = m_pmesh->flags;
		params.polyCount = m_pmesh->npolys;
		params.nvp = m_pmesh->nvp;
		params.detailMeshes = m_dmesh->meshes;
		params.detailVerts = m_dmesh->verts;
		params.detailVertsCount = m_dmesh->nverts;
		params.detailTris = m_dmesh->tris;
		params.detailTriCount = m_dmesh->ntris;
		params.offMeshConVerts = 0;//m_geom->getOffMeshConnectionVerts();
		params.offMeshConRad = 0;//m_geom->getOffMeshConnectionRads();
		params.offMeshConDir = 0;//m_geom->getOffMeshConnectionDirs();
		params.offMeshConAreas = 0;//m_geom->getOffMeshConnectionAreas();
		params.offMeshConFlags = 0;//m_geom->getOffMeshConnectionFlags();
		params.offMeshConUserID = 0;//m_geom->getOffMeshConnectionId();
		params.offMeshConCount = 0;//m_geom->getOffMeshConnectionCount();
		params.walkableHeight = m_agentHeight;
		params.walkableRadius = m_agentRadius;
		params.walkableClimb = m_agentMaxClimb;
		rcVcopy(params.bmin, m_pmesh->bmin);
		rcVcopy(params.bmax, m_pmesh->bmax);
		params.cs = m_cfg.cs;
		params.ch = m_cfg.ch;
		params.buildBvTree = true;
		
		if (!dtCreateNavMeshData(&params, &navData, &navDataSize))
		{
			Log("Could not build Detour navmesh.");
			return ;
		}
		
		m_navMesh = dtAllocNavMesh();
		if (!m_navMesh)
		{
			dtFree(navData);
			Log("Could not create Detour navmesh");
			return ;
		}
		
		dtStatus status;
		
		status = m_navMesh->init(navData, navDataSize, DT_TILE_FREE_DATA);
		if (dtStatusFailed(status))
		{
			dtFree(navData);
			Log("Could not init Detour navmesh");
			return ;
		}
		
		status = m_navQuery->init(m_navMesh, 2048);
		if (dtStatusFailed(status))
		{
			Log("Could not init Detour navmesh query");
			return ;
		}
	}
}