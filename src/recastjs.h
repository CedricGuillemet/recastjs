#pragma once

class dtNavMeshQuery;
class dtNavMesh;
class MeshLoader;
class NavMesh;
struct rcConfig;

class NavMesh
{
public:
	NavMesh() : m_navQuery(0)
	, m_navMesh(0)
	, mMesh(0)
	{
	}
	void Build(const float* positions, const int positionCount, const int* indices, const int indexCount, const rcConfig& config);
	//NavPath* ComputePath( vec_t start, vec_t end ) const;
	//bool ComputePath( vec_t start, vec_t end, NavPath *dest ) const;
	//bool RandomPositionAround(const vec_t& position, float radius, vec_t& res ) const;
	//bool ClosestPoint(const vec_t& position, vec_t& res) const;
	//bool MoveAlong(const vec_t& positionStart, const vec_t& positionEnd, vec_t& res) const;
protected:

	dtNavMeshQuery* m_navQuery;
	dtNavMesh* m_navMesh;
	MeshLoader *mMesh;
};