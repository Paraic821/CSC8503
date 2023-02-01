#pragma once
#include "NavigationMap.h"
#include "Plane.h"
#include <string>
#include <vector>
namespace NCL {
	namespace CSC8503 {
		class NavigationMesh : public NavigationMap	{
		public:
			NavigationMesh();
			NavigationMesh(const std::string&filename);
			~NavigationMesh();

			bool FindVertPath(const Vector3& from, const Vector3& to, NavigationPath& outPath);
			bool FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) override;
			void SetStringPulling(bool s) {
				stringPulling = s;
			}
			void DrawNavMesh();
		
		protected:
			struct CentrePoint {
				int index = -1;
				int adjacentIndexA = -1;
				int adjacentIndexB = -1;

				CentrePoint() {	}
				CentrePoint(int vertIndex, int adjacentIndex1, int adjacentIndex2) {
					index = vertIndex;
					adjacentIndexA = adjacentIndex1;
					adjacentIndexB = adjacentIndex2;
				}
			};

			struct NavTri {
				Plane   triPlane;
				Vector3 centroid;
				CentrePoint centrePoints[3];
				//CentrePoint centrePointA;
				//CentrePoint centrePointB;
				//CentrePoint centrePointC;
				float	area;
				NavTri* neighbours[3];

				int indices[3];
				float f = 0;
				float g = 0;
				NavTri* parent = nullptr;
				int index = 0;

				NavTri() {
					area = 0.0f;
					neighbours[0] = nullptr;
					neighbours[1] = nullptr;
					neighbours[2] = nullptr;

					indices[0] = -1;
					indices[1] = -1;
					indices[2] = -1;
				}
			};

			struct NavVert {
				Vector3 pos;
				int index = -1;
				int adjacentTriangleIndex1 = -1;
				int adjacentTriangleIndex2 = -1;
				float f = 0;
				float g = 0;
				NavTri* tri = nullptr;
				NavVert* parent = nullptr;

				//NavVert(Vector3 v, NavTri* triangle = nullptr, int vertIndex = -1, int adjacentIndex1 = -1, int adjacentIndex2 = -1) {
				NavVert(Vector3 v, NavTri* triangle = nullptr, int vertIndex = -1, int adjacentTriIndex1 = -1, int adjacentTriIndex2 = -1) { 
					pos = v; 
					tri = triangle;
					index = vertIndex;
					adjacentTriangleIndex1 = adjacentTriIndex1;
					adjacentTriangleIndex2 = adjacentTriIndex2;
				}
			};

			NavTri* GetTriForPosition(const Vector3& pos);


			int CentrePointInList(Vector3 centrePoint);
			//bool VertInAdjacentList(NavVert vert, std::vector<NavVert> vertList);
			bool VertInList(NavVert* vert, std::vector<NavVert*> vertList);
			//std::vector<NavVert> GetTriVertices(NavVert* currentVert);
			std::vector<NavVert*> GetTriVertices(NavVert* currentVert);
			//std::vector<NavVert> GetAdjacentVertices(NavVert* currentVert);
			std::vector<NavVert*> GetAdjacentVertices(NavVert* currentVert);
			NavVert* RemoveBestVert(std::vector<NavVert*>& list) const;
			NavTri* RemoveBestNode(std::vector<NavTri*>& list) const;
			bool FoundEndTri(NavVert* currentVert, NavTri* endTri);
			bool NodeInList(NavTri* n, std::vector<NavTri*>& list) const;
			NavigationPath StringPull(std::vector<NavTri*> path, Vector3 startPoint, Vector3 endPoint);
			bool IsPointToTheLeft(Vector3 a, Vector3 b, Vector3 c);
			std::vector<NavTri*> GetTriPath(std::vector<NavVert*> path);
			std::vector<NavTri*> InvertPath(std::vector<NavTri*> path);
			Vector3 FindMidPoint(Vector3 a, Vector3 b);

			NavTri* Copy(const NavTri* tri) {
				//NavTri 
				return nullptr;
			}

			std::vector<NavTri>		allTris;
			std::vector<NavVert>	allNavVerts;
			std::vector<Vector3>	allVerts;

			bool stringPulling = true;
			int numOfVertices = 0;
			int numOfTriangles = 0;
		};
	}
}