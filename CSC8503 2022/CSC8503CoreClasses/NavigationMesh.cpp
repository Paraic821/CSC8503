#include "NavigationMesh.h"
#include "Assets.h"
#include "Maths.h"
#include <fstream>
#include "Debug.h"
using namespace NCL;
using namespace CSC8503;
using namespace std;

NavigationMesh::NavigationMesh()
{
}

NavigationMesh::NavigationMesh(const std::string&filename)
{
	ifstream file(Assets::DATADIR + filename);

	int numVertices = 0;
	int numIndices	= 0;

	file >> numVertices;
	file >> numIndices;

	for (int i = 0; i < numVertices; ++i) {
		Vector3 vert;
		file >> vert.x;
		file >> vert.y;
		file >> vert.z;

		allVerts.emplace_back(vert);
		allNavVerts.emplace_back(NavVert(vert, nullptr, i));
	}

	allTris.resize(numIndices / 3);

	for (int i = 0; i < allTris.size(); ++i) {
		NavTri* tri = &allTris[i];
		file >> tri->indices[0];
		file >> tri->indices[1];
		file >> tri->indices[2];

		tri->index = i;

		tri->centroid = allVerts[tri->indices[0]] +
			allVerts[tri->indices[1]] +
			allVerts[tri->indices[2]];

		tri->centroid = allTris[i].centroid / 3.0f;

		tri->triPlane = Plane::PlaneFromTri(allVerts[tri->indices[0]],
			allVerts[tri->indices[1]],
			allVerts[tri->indices[2]]);

		tri->area = Maths::CrossAreaOfTri(allVerts[tri->indices[0]], allVerts[tri->indices[1]], allVerts[tri->indices[2]]);

		allNavVerts.emplace_back(NavVert(tri->centroid, tri));
	}

	numOfVertices = numVertices;
	numOfTriangles = allTris.size();
	int centrePointIndex = numOfVertices + numOfTriangles;
	int existingIndex = -1;
	Vector3 centrePoint;
	for (int i = 0; i < allTris.size(); ++i) {
		NavTri* tri = &allTris[i];
		for (int j = 0; j < 3; ++j) {
			int index = 0;
			file >> index;
			if (index != -1) {
				tri->neighbours[j] = &allTris[index];
			}
		}

		centrePoint = FindMidPoint(allVerts[tri->indices[0]], allVerts[tri->indices[1]]);
		existingIndex = CentrePointInList(centrePoint);
		if (existingIndex < 0) {
			allVerts.emplace_back(centrePoint);
			allNavVerts.emplace_back(NavVert(centrePoint, nullptr, centrePointIndex, tri->index));
			tri->centrePoints[0] = CentrePoint(centrePointIndex, tri->indices[0], tri->indices[1]);
			centrePointIndex++;
		}
		else {
			tri->centrePoints[0] = CentrePoint(existingIndex, tri->indices[0], tri->indices[1]);
			allNavVerts[existingIndex].adjacentTriangleIndex2 = tri->index;
		}

		centrePoint = FindMidPoint(allVerts[tri->indices[1]], allVerts[tri->indices[2]]);
		existingIndex = CentrePointInList(centrePoint);
		if (existingIndex < 0) {
			allVerts.emplace_back(centrePoint);
			allNavVerts.emplace_back(NavVert(centrePoint, nullptr, centrePointIndex, tri->index));
			tri->centrePoints[1] = CentrePoint(centrePointIndex, tri->indices[1], tri->indices[2]);
			centrePointIndex++;
		}
		else{
			tri->centrePoints[1] = CentrePoint(existingIndex, tri->indices[1], tri->indices[2]);
			allNavVerts[existingIndex].adjacentTriangleIndex2 = tri->index;
		}

		centrePoint = FindMidPoint(allVerts[tri->indices[2]], allVerts[tri->indices[0]]);
		existingIndex = CentrePointInList(centrePoint);
		if (existingIndex < 0) {
			allVerts.emplace_back(centrePoint);
			allNavVerts.emplace_back(NavVert(centrePoint, nullptr, centrePointIndex, tri->index));
			tri->centrePoints[2] = CentrePoint(centrePointIndex, tri->indices[2], tri->indices[0]);
			centrePointIndex++;
		}
		else{
			tri->centrePoints[2] = CentrePoint(existingIndex, tri->indices[2], tri->indices[0]);
			allNavVerts[existingIndex].adjacentTriangleIndex2 = tri->index;
		}
	}
}

NavigationMesh::~NavigationMesh()
{
}

Vector3 NavigationMesh::FindMidPoint(Vector3 a, Vector3 b) {
	Vector3 midpoint;

	midpoint.x = (a.x + b.x) / 2;
	midpoint.y = (a.y + b.y) / 2;
	midpoint.z = (a.z + b.z) / 2;

	return midpoint;
}

bool NavigationMesh::FindPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) {
	NavTri* startNode		= GetTriForPosition(from);
	NavTri* endNode			= GetTriForPosition(to);

	std::vector<NavTri*>	openList;
	std::vector<NavTri*>	closedList;
	std::vector<NavTri*>	path;

	openList.push_back(startNode);

	startNode->f			= 0;
	startNode->g			= 0;
	startNode->parent		= nullptr;
	NavTri* currentBestNode = nullptr;

	while (!openList.empty()) {
		currentBestNode = RemoveBestNode(openList);

		if (currentBestNode == endNode) {			//we've found the path!
			NavTri* node = endNode;
			while (node != nullptr) {
				path.emplace_back(node);

				if(!stringPulling)
					outPath.PushWaypoint(node->centroid);

				node = node->parent;
			}

			if(stringPulling)
				outPath = StringPull(path, from, to);

			return true;
		}
		else {
			for (int i = 0; i < 3; ++i) {
				NavTri* neighbour = currentBestNode->neighbours[i];
				if (!neighbour) { //might not be connected...
					continue;
				}
				bool inClosed = NodeInList(neighbour, closedList);
				if (inClosed) {
					continue; //already discarded this neighbour...
				}

				//float h = Heuristic(neighbour, endNode);
				float h = (neighbour->centroid - endNode->centroid).Length();
				float cost = currentBestNode->parent ? (currentBestNode->parent->centroid - currentBestNode->centroid).Length() : 0;
				//float g = currentBestNode->g + currentBestNode->costs[i];
				//float g = currentBestNode->g + cost;
				float g = currentBestNode->g + (currentBestNode->centroid - neighbour->centroid).Length();
				//float g = currentBestNode->g;
				float f = h + g;

				bool inOpen = NodeInList(neighbour, openList);

				if (inOpen && f < neighbour->f) {
					int test = 1;
					test++;
				}

				if (!inOpen) { //first time we've seen this neighbour
					openList.emplace_back(neighbour);
				}
				if (!inOpen || f < neighbour->f) {//might be a better route to this neighbour
					neighbour->parent = currentBestNode;
					neighbour->f = f;
					neighbour->g = g;
				}
			}
			closedList.emplace_back(currentBestNode);
		}
	}


	return false;
}

bool NavigationMesh::FindVertPath(const Vector3& from, const Vector3& to, NavigationPath& outPath) {
	NavTri* startTri = GetTriForPosition(from);
	NavTri* endTri = GetTriForPosition(to);

	//NavVert* startVert = &NavVert(from, startTri);
	//NavVert* endVert = &NavVert(to, endTri);

	NavVert start = NavVert(from, startTri);
	NavVert end = NavVert(to, endTri);
	NavVert* startVert = &start;
	NavVert* endVert = &end;

	std::vector<NavVert*>	openList;
	std::vector<NavVert*>	closedList;
	std::vector<NavVert*>	path;
	std::vector<NavTri*>	triPath;
	std::vector<NavVert*>	adjacentVertices;

	openList.push_back(startVert);

	startVert->f = 0;
	startVert->g = 0;
	startVert->parent = nullptr;
	NavVert* currentBestVert = nullptr;

	while (!openList.empty()) {
		currentBestVert = RemoveBestVert(openList);

		if(FoundEndTri(currentBestVert, endTri)){
			NavVert* vert = endVert;
			vert->parent = currentBestVert;
			while (vert != nullptr) {
				path.emplace_back(vert);

				if (!stringPulling)
					outPath.PushWaypoint(vert->pos);

				vert = vert->parent;
			}

			if (stringPulling) {
				triPath = GetTriPath(path);
				outPath = StringPull(triPath, from, to);
			}

			return true;
		}
		else {
			adjacentVertices =  currentBestVert->tri ? GetTriVertices(currentBestVert) : GetAdjacentVertices(currentBestVert);
			for (NavVert* adjacentVertex : adjacentVertices) {
				bool inClosed = VertInList(adjacentVertex, closedList);
				if (inClosed) {
					continue; //already discarded this neighbour...
				}

				float h = (endVert->pos - adjacentVertex->pos).Length();
				float g = currentBestVert->g + (currentBestVert->pos - adjacentVertex->pos).Length();
				float f = h + g;

				bool inOpen = VertInList(adjacentVertex, openList);

				if (!inOpen) { //first time we've seen this neighbour
					openList.emplace_back(adjacentVertex);
				}
				if (!inOpen || f < adjacentVertex->f) {//might be a better route to this neighbour
					adjacentVertex->parent = currentBestVert;
					adjacentVertex->f = f;
					adjacentVertex->g = g;
				}
			}
			closedList.emplace_back(currentBestVert);
		}
	}
	return false;
}

//std::vector<NavigationMesh::NavVert*> NavigationMesh::GetTriVertices(NavigationMesh::NavVert* currentVert) {
//	std::vector<NavVert*> triVerts;
//
//	NavVert vert1 = NavVert(allVerts[currentVert->tri->indices[0]], nullptr, currentVert->tri->indices[0]);
//	NavVert vert2 = NavVert(allVerts[currentVert->tri->indices[1]], nullptr, currentVert->tri->indices[1]);
//	NavVert vert3 = NavVert(allVerts[currentVert->tri->indices[2]], nullptr, currentVert->tri->indices[2]);
//	NavVert* triVert1 = &vert1;
//	NavVert* triVert2 = &vert2;
//	NavVert* triVert3 = &vert3;
//	triVerts.push_back(triVert1);
//	triVerts.push_back(triVert2);
//	triVerts.push_back(triVert3);
//
//	return triVerts;
//}

std::vector<NavigationMesh::NavVert*> NavigationMesh::GetTriVertices(NavigationMesh::NavVert* currentVert) {
	std::vector<NavVert*> triVerts;
	//triVerts.push_back(&allNavVerts[currentVert->tri->indices[0]]);
	//triVerts.push_back(&allNavVerts[currentVert->tri->indices[1]]);
	//triVerts.push_back(&allNavVerts[currentVert->tri->indices[2]]);

	if (currentVert->tri->neighbours[0]) triVerts.push_back(&allNavVerts[numOfVertices + currentVert->tri->neighbours[0]->index]);
	if (currentVert->tri->neighbours[1]) triVerts.push_back(&allNavVerts[numOfVertices + currentVert->tri->neighbours[1]->index]);
	if (currentVert->tri->neighbours[2]) triVerts.push_back(&allNavVerts[numOfVertices + currentVert->tri->neighbours[2]->index]);

	triVerts.push_back(&allNavVerts[currentVert->tri->centrePoints[0].index]);
	triVerts.push_back(&allNavVerts[currentVert->tri->centrePoints[1].index]);
	triVerts.push_back(&allNavVerts[currentVert->tri->centrePoints[2].index]);

	return triVerts;
}

//std::vector<NavigationMesh::NavVert*> NavigationMesh::GetAdjacentVertices(NavigationMesh::NavVert* currentVert) {
//	int index = currentVert->index;
//	std::vector<NavVert*> adjacentVerts;
//
//	for (NavTri tri : allTris) {
//		for (int triIndex : tri.indices) {
//			if (triIndex == index) {
//				NavVert vert1 = NavVert(allVerts[tri.indices[0]], nullptr, tri.indices[0]);
//				NavVert vert2 = NavVert(allVerts[tri.indices[1]], nullptr, tri.indices[1]);
//				NavVert vert3 = NavVert(allVerts[tri.indices[2]], nullptr, tri.indices[2]);
//				NavVert* triVert1 = &vert1;
//				NavVert* triVert2 = &vert2;
//				NavVert* triVert3 = &vert3;
//				if (tri.indices[0] != index && !VertInList(triVert1, adjacentVerts)) adjacentVerts.push_back(triVert1);
//				if (tri.indices[1] != index && !VertInList(triVert2, adjacentVerts)) adjacentVerts.push_back(triVert2);
//				if (tri.indices[2] != index && !VertInList(triVert3, adjacentVerts)) adjacentVerts.push_back(triVert3);
//			}
//		}
//	}
//	return adjacentVerts;
//}

std::vector<NavigationMesh::NavVert*> NavigationMesh::GetAdjacentVertices(NavigationMesh::NavVert* currentVert) {
	int index = currentVert->index;
	std::vector<NavVert*> adjacentVerts;

	for (NavTri tri : allTris) {
		//for (int triIndex : tri.indices) {
		for(int i = 0; i < 3; i++){
			if (tri.indices[i] == index || tri.centrePoints[i].index == index) {
				//Push pack vertices of triangle
				//if (tri.indices[0] != index && !VertInList(&allNavVerts[tri.indices[0]], adjacentVerts)) adjacentVerts.push_back(&allNavVerts[tri.indices[0]]);
				//if (tri.indices[1] != index && !VertInList(&allNavVerts[tri.indices[1]], adjacentVerts)) adjacentVerts.push_back(&allNavVerts[tri.indices[1]]);
				//if (tri.indices[2] != index && !VertInList(&allNavVerts[tri.indices[2]], adjacentVerts)) adjacentVerts.push_back(&allNavVerts[tri.indices[2]]);

				//Push back centroid of triangle
				adjacentVerts.push_back(&allNavVerts[numOfVertices + tri.index]);

				//Push back centre points of triangle
				//if (tri.centrePoints[0].adjacentIndexA == index || tri.centrePoints[0].adjacentIndexB == index) 
				//	adjacentVerts.push_back(&allNavVerts[tri.centrePoints[0].index]);
				//if (tri.centrePoints[1].adjacentIndexA == index || tri.centrePoints[1].adjacentIndexB == index)
				//	adjacentVerts.push_back(&allNavVerts[tri.centrePoints[1].index]);
				//if (tri.centrePoints[2].adjacentIndexA == index || tri.centrePoints[2].adjacentIndexB == index)
				//	adjacentVerts.push_back(&allNavVerts[tri.centrePoints[2].index]);
				if (tri.centrePoints[0].index != index && !VertInList(&allNavVerts[tri.centrePoints[0].index], adjacentVerts)) 
					adjacentVerts.push_back(&allNavVerts[tri.centrePoints[0].index]);
				if (tri.centrePoints[1].index != index && !VertInList(&allNavVerts[tri.centrePoints[1].index], adjacentVerts))
					adjacentVerts.push_back(&allNavVerts[tri.centrePoints[1].index]);
				if (tri.centrePoints[2].index != index && !VertInList(&allNavVerts[tri.centrePoints[2].index], adjacentVerts))
					adjacentVerts.push_back(&allNavVerts[tri.centrePoints[2].index]);
			}
		}
	}
	return adjacentVerts;
}

//bool NavigationMesh::VertInAdjacentList(NavVert vert, std::vector<NavVert> vertList) {
//	for (NavVert v : vertList) {
//		if (v.pos == vert.pos) return true;
//	}
//	return false;
//}

bool NavigationMesh::VertInList(NavVert* vert, std::vector<NavVert*> vertList) {
	for (NavVert* v : vertList) {
		if (v->pos == vert->pos) return true;
	}
	return false;
}

int NavigationMesh::CentrePointInList(Vector3 centrePoint) {
	for (NavVert v : allNavVerts) {
		if (v.pos == centrePoint) return v.index;
	}
	return -1;
}

bool NavigationMesh::FoundEndTri(NavVert* currentVert, NavTri* endTri) {
	for (int i = 0; i < 3; i++) {
		if (currentVert->pos == allVerts[endTri->indices[i]] 
			|| currentVert->pos == allVerts[endTri->centrePoints[i].index - numOfTriangles]
			|| currentVert->tri == endTri) return true;
	}
	return false;
}

/*
If you have triangles on top of triangles in a full 3D environment, you'll need to change this slightly,
as it is currently ignoring height. You might find tri/plane raycasting is handy.
*/

NavigationMesh::NavTri* NavigationMesh::GetTriForPosition(const Vector3& pos) {
	for (NavTri& t : allTris) {
		Vector3 planePoint = t.triPlane.ProjectPointOntoPlane(pos);

		float ta = Maths::CrossAreaOfTri(allVerts[t.indices[0]], allVerts[t.indices[1]], planePoint);
		float tb = Maths::CrossAreaOfTri(allVerts[t.indices[1]], allVerts[t.indices[2]], planePoint);
		float tc = Maths::CrossAreaOfTri(allVerts[t.indices[2]], allVerts[t.indices[0]], planePoint);

		float areaSum = ta + tb + tc;

		if (abs(areaSum - t.area)  > 0.01f) { //floating points are annoying! Are we more or less inside the triangle?
			continue;
		}
		return &t;
	}
	return nullptr;
}


NavigationMesh::NavTri* NavigationMesh::RemoveBestNode(std::vector<NavTri*>& list) const{
	std::vector<NavTri*>::iterator bestI = list.begin();

	NavTri* bestNode = *list.begin();

	for (auto i = list.begin(); i != list.end(); ++i) {
		if ((*i)->f < bestNode->f) {
			bestNode = (*i);
			bestI = i;
		}
	}
	list.erase(bestI);

	return bestNode;
}


NavigationMesh::NavVert* NavigationMesh::RemoveBestVert(std::vector<NavVert*>& list) const {
	std::vector<NavVert*>::iterator bestI = list.begin();

	NavVert* bestVert = *list.begin();

	for (auto i = list.begin(); i != list.end(); ++i) {
		if ((*i)->f < bestVert->f) {
			bestVert = (*i);
			bestI = i;
		}
	}
	list.erase(bestI);

	return bestVert;
}

bool NavigationMesh::NodeInList(NavTri* n, std::vector<NavTri*>& list) const {
	std::vector<NavTri*>::iterator i = std::find(list.begin(), list.end(), n);
	return i == list.end() ? false : true;
}

NavigationPath NavigationMesh::StringPull(std::vector<NavTri*> path, Vector3 startPoint, Vector3 endPoint) {
	NavigationPath outpath;
	outpath.PushWaypoint(startPoint);

	std::vector<NavTri*> realPath = InvertPath(path);

	std::vector<Vector3> leftVertices;
	std::vector<Vector3> rightVertices;

	Vector3 apex = startPoint;
	int left = 0;
	int right = 0;

	//Initialise portal vertices
	for (int i = 0; i < realPath.size() - 1; i++) {
		Vector3 currentTriVerts[3];
		Vector3 nextTriVerts[3];
		Vector3 sharedVerts[2];

		currentTriVerts[0] = allVerts[realPath[i]->indices[0]];
		currentTriVerts[1] = allVerts[realPath[i]->indices[1]];
		currentTriVerts[2] = allVerts[realPath[i]->indices[2]];

		nextTriVerts[0] = allVerts[realPath[i + 1]->indices[0]];
		nextTriVerts[1] = allVerts[realPath[i + 1]->indices[1]];
		nextTriVerts[2] = allVerts[realPath[i + 1]->indices[2]];

		int n = 0;
		for (int j = 0; j < 3; j++) {
			if (currentTriVerts[j] == nextTriVerts[0]) { sharedVerts[n] = currentTriVerts[j]; n++; }
			else if (currentTriVerts[j] == nextTriVerts[1]) { sharedVerts[n] = currentTriVerts[j]; n++; }
			else if (currentTriVerts[j] == nextTriVerts[2]) { sharedVerts[n] = currentTriVerts[j]; n++; }
		}

		if (IsPointToTheLeft(realPath[i]->centroid, (sharedVerts[0] + sharedVerts[1]) / 2, sharedVerts[0])) {
			leftVertices.emplace_back(sharedVerts[0]);
			rightVertices.emplace_back(sharedVerts[1]);
		}
		else {
			leftVertices.emplace_back(sharedVerts[1]);
			rightVertices.emplace_back(sharedVerts[0]);
		}
	}

	leftVertices.emplace_back(endPoint);
	rightVertices.emplace_back(endPoint);

	// Step through channel.
	for (int i = 1; i <= realPath.size() - 1; i++) {
		// If new left vertex is different, process.
		if (leftVertices[i] != leftVertices[left] && i > left)
		{
			// If new side does not widen funnel, update.
			// Checks if leftVertices[i] is to the left of the line apex -> leftVertices[left]
			if (!IsPointToTheLeft(apex, leftVertices[left], leftVertices[i]))
			{
				// If new side crosses other side, update apex.
				if (!IsPointToTheLeft(apex, rightVertices[right], leftVertices[i]))
				{
					int next = right;

					// Find next vertex.
					for (int j = next; j <= realPath.size(); j++)
					{
						if (rightVertices[j] != rightVertices[next])
						{
							next = j;
							break;
						}
					}

					outpath.PushWaypoint(rightVertices[right]);
					apex = rightVertices[right];
					i = right;
					right = next;
				}
				else
				{
					left = i;
				}
			}
		}

		// If new right vertex is different, process.
		if (rightVertices[i] != rightVertices[right] && i > right)
		{
			// If new side does not widen funnel, update.
			if (IsPointToTheLeft(apex, rightVertices[right], rightVertices[i]))
			{
				// If new side crosses other side, update apex.
				if (IsPointToTheLeft(apex, leftVertices[left], rightVertices[i]))
				{
					int next = left;

					// Find next vertex.
					for (int j = next; j <= realPath.size(); j++)
					{
						if (leftVertices[j] != leftVertices[next])
						{
							next = j;
							break;
						}
					}

					outpath.PushWaypoint(leftVertices[left]);
					apex = leftVertices[left];
					i = right;
					left = next;
				}
				else
				{
					right = i;
				}
			}
		}
	}
	outpath.PushWaypoint(endPoint);
	return outpath;
}

/*
	Returns whether point c is to the left of line ab
*/
bool NavigationMesh::IsPointToTheLeft(Vector3 a, Vector3 b, Vector3 c) {
	//return ((b.x - a.x) * (c.z - a.z) - (b.z - a.z) * (c.x - a.x)) > 0;
	return ((b.x - a.x) * (c.z - a.z) - (b.z - a.z) * (c.x - a.x)) < 0;
}

std::vector<NavigationMesh::NavTri*> NavigationMesh::InvertPath(std::vector<NavTri*> path) {
	vector<NavTri*> realPath;

	for (int i = path.size() - 1; i > 0; i--) {
		realPath.emplace_back(path[i]);
	}
	return realPath;
}

std::vector<NavigationMesh::NavTri*> NavigationMesh::GetTriPath(std::vector<NavigationMesh::NavVert*> path) {
	std::vector<NavTri*> triPath;

	for (NavVert* vert : path) {
		if (vert->tri) {
			if (!NodeInList(vert->tri, triPath)) triPath.emplace_back(vert->tri);
		}
		else {
			if (!NodeInList(&allTris[vert->adjacentTriangleIndex1], triPath)) triPath.emplace_back(&allTris[vert->adjacentTriangleIndex1]);
			if (!NodeInList(&allTris[vert->adjacentTriangleIndex2], triPath)) triPath.emplace_back(&allTris[vert->adjacentTriangleIndex2]);
		}
	}

	return triPath;
}

void NavigationMesh::DrawNavMesh() {
	for (NavTri tri : allTris) {
		Debug::DrawLine(allVerts[tri.indices[0]], allVerts[tri.indices[1]], Vector4(0, 0, 1, 1));
		Debug::DrawLine(allVerts[tri.indices[1]], allVerts[tri.indices[2]], Vector4(0, 0, 1, 1));
		Debug::DrawLine(allVerts[tri.indices[0]], allVerts[tri.indices[2]], Vector4(0, 0, 1, 1));
	}
}