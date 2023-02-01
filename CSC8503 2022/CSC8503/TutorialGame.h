#pragma once
#include "GameTechRenderer.h"
#ifdef USEVULKAN
#include "GameTechVulkanRenderer.h"
#endif
#include "PhysicsSystem.h"

#include "StateGameObject.h"

#include "PushdownMachine.h"
#include "PushdownState.h"

#include "NavigationGrid.h"
#include "NavigationMesh.h"
#include "NavigationPath.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

namespace NCL {
	namespace CSC8503 {
		class TutorialGame		{
		public:
			TutorialGame();
			~TutorialGame();

			virtual void UpdateGame(float dt);

			void InitWorld();
			void InitMaze();
			void ClearWorld();
			bool GetRemainingDestructibles();
			float GetScore();

		protected:
			void InitialiseAssets();

			void InitCamera();
			void UpdateKeys();
			void InitCapsuleTest();

			/*
			These are some of the world/object creation functions I created when testing the functionality
			in the module. Feel free to mess around with them to see different objects being created in different
			test scenarios (constraints, collision types, and so on). 
			*/
			void InitGameExamples();
			void HangingConstraint(Vector3 position);
			void BridgeConstraintTest();

			void InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius);
			void InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing);
			void InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims);

			void InitDefaultFloor();

			bool PlayerLeftClick();
			bool SelectObject();
			void MoveSelectedObject();
			void DebugObjectMovement();
			void LockedObjectMovement();

			void TestPathfinding();
			void DisplayPathfinding();
			void GetPathToDest(Vector3 destination);
			Vector3 ChoosePosition();

			bool CheckCanSeePlayer(Vector3 newPos);
			bool CheckEnemyBounds();
			void EnemyRespawn();

			GameObject* ChooseBonus();
			float randomNumber(float a, float b);

			void RunBehaviourTree(float dt);
			void FarmerBehaviourTree();

			void DestroyObjects();

			GameObject* AddFloorToWorld(const Vector3& position, ObjectType t = Default);
			GameObject* AddSphereToWorld(const Vector3& position, float radius, float inverseMass = 10.0f, ObjectType t = Default, bool hollow = false);
			GameObject* AddCapsuleToWorld(const Vector3& position, float radius, float halfHeight, float inverseMass = 10.0f, ObjectType t = Default);
			GameObject* AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass = 10.0f, ObjectType t = Default);
			GameObject* AddOBBToWorld(const Vector3& position, Vector3 orientation, Vector3 dimensions, float inverseMass = 10.0f, ObjectType t = Default);

			GameObject* AddPlayerToWorld(const Vector3& position);
			GameObject* AddEnemyToWorld(const Vector3& position);
			GameObject* AddBonusToWorld(const Vector3& position);

			void AddMaze();
			void AddWalls();
			void AddObstacles();
			void AddBoundaries();

			StateGameObject* AddStateObjectToWorld(const Vector3& position, ObjectType t = Default);
			StateGameObject* testStateObject;

#ifdef USEVULKAN
			GameTechVulkanRenderer*	renderer;
#else
			GameTechRenderer* renderer;
#endif
			PhysicsSystem*		physics;
			GameWorld*			world;

			bool useGravity;
			bool inSelectionMode;

			float		forceMagnitude;
			float		pitch = 0.0f;
			float		yaw = 0.0f;
			float		playerRange = 10.0f;
			float		playerForce = 3.0f;

			GameObject* selectionObject = nullptr;

			MeshGeometry*	capsuleMesh = nullptr;
			MeshGeometry*	cubeMesh	= nullptr;
			MeshGeometry*	sphereMesh	= nullptr;

			TextureBase*	basicTex	= nullptr;
			ShaderBase*		basicShader = nullptr;

			//Coursework Meshes
			MeshGeometry*	charMesh	= nullptr;
			MeshGeometry*	enemyMesh	= nullptr;
			MeshGeometry*	bonusMesh	= nullptr;
			MeshGeometry*	gooseMesh	= nullptr;

			vector <Vector3> testNodes;

			//Coursework Additional functionality	
			GameObject* lockedObject	= nullptr;
			Vector3 lockedOffset		= Vector3(0, 1.5f, 7);
			void LockCameraToObject(GameObject* o) {
				lockedObject = o;
			}

			GameObject* objClosest = nullptr;
			GameObject* player = nullptr;
			GameObject* enemy = nullptr;

			NavigationMesh navMesh;
			NavigationMesh testNavMesh;

			std::vector<Vector3> enemyPath;
			Vector3 enemyDest = Vector3(0,0,0);
			Vector3 enemyNextPos = Vector3(0, 0, 0);
			BehaviourSequence* enemyRootSequence = nullptr;
			BehaviourState enemyState = Ongoing;


			std::vector<GameObject*> bonusObjects;
			GameObject* chosenBonus = nullptr;

			PushdownMachine* machine;
			PushdownState* previousState = nullptr;
			GameTimer* timer = nullptr;
		};
	}
}

