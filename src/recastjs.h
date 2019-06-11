#pragma once
#include "../recastnavigation/Detour/Include/DetourNavMesh.h"
#include <vector>
class dtNavMeshQuery;
class dtNavMesh;
class MeshLoader;
class NavMesh;
struct rcConfig;


struct Vec3 
{
	Vec3() {}
 	float x() { return mx; }
  	float y() { return my; }
  	float z() { return mz; }

    float mx,my,mz;
};

struct Triangle 
{
	Triangle(){}
	const Vec3& GetPoint(long n)
    {
        if (n < 2)
        {
            return mPoint[n];
        }
        return mPoint[2];
    }
    Vec3 mPoint[3];
};

struct DebugNavMesh 
{
	int TriangleCount() { return int(mTriangles.size()); }
	const Triangle& GetTriangle(int n)
    {
        if (n < int(mTriangles.size()))
        {
            return mTriangles[n];
        }
        return mTriangles.back();
    }
    std::vector<Triangle> mTriangles;
};

class NavMesh
{
public:
	NavMesh() : m_navQuery(0)
	, m_navMesh(0)
	//, mMesh(0)
	{
	}
	void Build(const float* positions, const int positionCount, const int* indices, const int indexCount, const rcConfig& config);
	//NavPath* ComputePath( vec_t start, vec_t end ) const;
	//bool ComputePath( vec_t start, vec_t end, NavPath *dest ) const;
	//bool RandomPositionAround(const vec_t& position, float radius, vec_t& res ) const;
	//bool ClosestPoint(const vec_t& position, vec_t& res) const;
	//bool MoveAlong(const vec_t& positionStart, const vec_t& positionEnd, vec_t& res) const;

    DebugNavMesh& GetDebugNavMesh();
protected:

	dtNavMeshQuery* m_navQuery;
	dtNavMesh* m_navMesh;
	//MeshLoader *mMesh;
    DebugNavMesh mDebugNavMesh;

    void NavMeshPoly(const dtNavMesh& mesh, dtPolyRef ref);
    void NavMeshPolysWithFlags(const dtNavMesh& mesh, const unsigned short polyFlags);
};
