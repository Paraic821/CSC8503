#include "TutorialGame.h"
#include "GameWorld.h"
#include "PhysicsObject.h"
#include "RenderObject.h"
#include "TextureLoader.h"

#include "PositionConstraint.h"
#include "OrientationConstraint.h"
#include "StateGameObject.h"
#include <algorithm>

#include "Debug.h";

using namespace NCL;
using namespace CSC8503;

TutorialGame::TutorialGame()	{
	world		= new GameWorld();
#ifdef USEVULKAN
	renderer	= new GameTechVulkanRenderer(*world);
#else 
	renderer = new GameTechRenderer(*world);
#endif

	physics		= new PhysicsSystem(*world);

	forceMagnitude	= 10.0f;
	useGravity		= true;
	inSelectionMode = false;
	testStateObject = nullptr;

	InitialiseAssets();
	
	//machine = new PushdownMachine(new IntroScreen());
	//timer = new GameTimer();
}

//void TestPushdownAutomata(Window* w) {
//	PushdownMachine machine(new IntroScreen());
//	while (w->UpdateWindow()) {
//		float dt = w->GetTimer()->GetTimeDeltaSeconds();
//		if (!machine.Update(dt)) {
//			return;
//		}
//	}
//}

/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	cubeMesh	= renderer->LoadMesh("cube.msh");
	sphereMesh	= renderer->LoadMesh("sphere.msh");
	charMesh	= renderer->LoadMesh("goat.msh");
	enemyMesh	= renderer->LoadMesh("Keeper.msh");
	bonusMesh	= renderer->LoadMesh("apple.msh");
	capsuleMesh = renderer->LoadMesh("capsule.msh");
	gooseMesh = renderer->LoadMesh("goose.msh");

	basicTex	= renderer->LoadTexture("checkerboard.png");
	basicShader = renderer->LoadShader("scene.vert", "scene.frag");

	InitCamera();
	//InitWorld();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMesh;
	delete enemyMesh;
	delete bonusMesh;
	delete gooseMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;

	delete timer;
	delete machine;
	delete previousState;
}

void TutorialGame::UpdateGame(float dt) {
	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}
	if (testStateObject) {
		testStateObject->Update(dt);
	}
	if (lockedObject != nullptr) {
		//Update the mouse by how much
		pitch -= (Window::GetMouse()->GetRelativePosition().y);
		yaw -= (Window::GetMouse()->GetRelativePosition().x);

		//Bounds check the pitch, to be between straight up and straight down ;)
		pitch = min(pitch, 20.0f);
		pitch = max(pitch, -10.0f);

		if (yaw < 0) {
			yaw += 360.0f;
		}
		if (yaw > 360.0f) {
			yaw -= 360.0f;
		}

		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 offset = lockedObject->GetTransform().GetOrientation() * lockedOffset;
		offset = Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), yaw) * offset;
		Vector3 camPos = objPos + offset;
		//camPos =  * camPos;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0,1,0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetPitch(pitch);
		world->GetMainCamera()->SetYaw(angles.y);
	}

	UpdateKeys();

	//if (useGravity) {
	//	Debug::Print("(G)ravity on", Vector2(5, 95), Debug::RED);
	//}
	//else {
	//	Debug::Print("(G)ravity off", Vector2(5, 95), Debug::RED);
	//}
	//Debug::Print("Linear Damping:" + std::to_string(physics->GetLinearDamping()), Vector2(5, 100));
	//Debug::Print("x", Vector2(screenSize.x / 2, screenSize.y / 2));
	Debug::Print(".", Vector2(50, 50));

	RayCollision closestCollision;
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::K) && selectionObject) {
		Vector3 rayPos;
		Vector3 rayDir;

		rayDir = selectionObject->GetTransform().GetOrientation() * Vector3(0, 0, -1);

		rayPos = selectionObject->GetTransform().GetPosition();

		Ray r = Ray(rayPos, rayDir);

		if (world->Raycast(r, closestCollision, true, selectionObject)) {
			if (objClosest) {
				objClosest->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
			}
			objClosest = (GameObject*)closestCollision.node;

			objClosest->GetRenderObject()->SetColour(Vector4(1, 0, 1, 1));
		}
	}

	Debug::DrawLine(Vector3(), Vector3(0, 100, 0), Vector4(1, 0, 0, 1));
	Debug::DrawLine(Vector3(1,0,1), Vector3(1, 100, 1), Vector4(1, 0, 0, 1));

	Debug::DrawLine(Vector3(0, 0, 400), Vector3(0, 100, 400), Vector4(0, 1, 0, 1));
	Debug::DrawLine(Vector3(1, 0, 399), Vector3(1, 100, 399), Vector4(0, 1, 0, 1));

	Debug::DrawLine(Vector3(400, 0, 0), Vector3(400, 100, 0), Vector4(0, 0, 1, 1));
	Debug::DrawLine(Vector3(399, 0, 1), Vector3(399, 100, 1), Vector4(0, 0, 1, 1));

	Debug::DrawLine(Vector3(399, 0, 399), Vector3(399, 100, 399), Vector4(1, 1, 1, 1));
	Debug::DrawLine(Vector3(400, 0, 400), Vector3(400, 100, 400), Vector4(1, 1, 1, 1));

	if (enemy && enemyRootSequence) {
		RunBehaviourTree(dt);
		DisplayPathfinding();
	}
	if (enemy && !CheckEnemyBounds()) {
		EnemyRespawn();
	}

	SelectObject();
	MoveSelectedObject();

	DisplayPathfinding();

	//DestroyObjects();

	world->UpdateWorld(dt);
	renderer->Update(dt);
	physics->Update(dt);

	renderer->Render();
	Debug::UpdateRenderables(dt);

	//timer->Tick();
	//float delta = timer->GetTimeDeltaSeconds();
	//machine->Update(delta);
}

void TutorialGame::InitCapsuleTest() {
	world->ClearAndErase();
	physics->Clear();
	lockedObject = nullptr;

	AddCapsuleToWorld(Vector3(7.5f, 8.0f, 17.5f), 2.0f, 4.0f, 200.0f);
	AddCubeToWorld(Vector3(7.5f, 8.0f, 10.0f), Vector3(1, 1, 1), 2.0f);
	//AddSphereToWorld(Vector3(7.5f, 8.0f, 17.4f), 2.0f);

	//InitGameExamples();
	//InitDefaultFloor();
}

void TutorialGame::UpdateKeys() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F3)) {
		InitMaze();
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F4)) {
		InitCapsuleTest();
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::P)) {
		lockedObject = lockedObject ? nullptr : player;
	}

	//if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F3)) {
	//	BridgeConstraintTest();
	//}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}
	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}

	if (lockedObject) {
		LockedObjectMovement();
		Debug::Print("LEFT CLICK TO HEADBUTT", Vector3(5, 5, 0));
		Debug::Print("Destructibles remaining: " + std::to_string(world->GetDestructibleCount()), Vector3(5, 10, 0));
		Debug::Print("Score: " + std::to_string(player->GetPoints()), Vector3(5, 15, 0));
	}
	else {
		DebugObjectMovement();
	}
	//if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PLUS)) {
	//	physics->SetLinearDamping(	physics->GetLinearDamping() + 0.1f	);
	//}
	//if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::MINUS)) {
	//	physics->SetLinearDamping(physics->GetLinearDamping() - 0.1f);
	//}
}

void TutorialGame::TestPathfinding() {
	//NavigationGrid grid("TestGrid1.txt");
	//NavigationPath outPath;

	//Vector3 startPos(80, 0, 10);
	//Vector3 endPos(80, 0, 80);

	//bool found = grid.FindPath(startPos, endPos, outPath);
	//Vector3 pos;
	//while (outPath.PopWaypoint(pos)) {
	//	testNodes.push_back(pos);
	//}
	testNodes.clear();
	testNavMesh = NavigationMesh("basicNavMesh.navmesh");
	NavigationMesh navMesh("basicNavMesh.navmesh");
	NavigationPath outPath;

	Vector3 startPos(200, 0, 50);
	Vector3 endPos(225, 0, 200);
	AddCubeToWorld(startPos, Vector3(1, 1, 1), 0.0f);
	AddCubeToWorld(endPos, Vector3(1, 1, 1), 0.0f);

	navMesh.SetStringPulling(true);
	//bool found = navMesh.FindPath(startPos, endPos, outPath);
	bool found = navMesh.FindVertPath(startPos, endPos, outPath);
	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		testNodes.push_back(pos);
	}
}

void TutorialGame::DisplayPathfinding() {
	for (int i = 1; i < enemyPath.size(); ++i) {
		Vector3 a = enemyPath[i - 1];
		Vector3 b = enemyPath[i];
		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
	}

	for (int i = 1; i < testNodes.size(); ++i) {
		Vector3 a = testNodes[i - 1];
		Vector3 b = testNodes[i];
		Debug::DrawLine(a, b, Vector4(0, 1, 0, 1));
	}

	try {
		//testNavMesh.DrawNavMesh();
	}
	catch (int a) {}
}

void TutorialGame::RunBehaviourTree(float dt) {
	if (enemyState == Ongoing) {
		enemyState = enemyRootSequence->Execute(1.0f);
	}
	if (enemyState == Success) {
		//std::cout << "What a successful adventure!\n";
		enemyRootSequence->Reset();
		enemyState = enemyRootSequence->Execute(1.0f);
	}
	else if (enemyState == Failure) {
		//std::cout << "What a waste of time!\n";
		enemyRootSequence->Reset();
		enemyState = enemyRootSequence->Execute(1.0f);
	}
	enemy->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(0, 0, 0));
}

void TestBehaviourTree() {
	float behaviourTimer;
	float distanceToTarget;
	//GameObject* farmer = enemy;

	BehaviourAction* findKey = new BehaviourAction("Find Key",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Looking for a key!\n";
				behaviourTimer = rand() % 100;
				state = Ongoing;
			}
			else if (state == Ongoing) {
				behaviourTimer -= dt;
				if (behaviourTimer <= 0.0f) {
					std::cout << "Found a key!\n";
					return Success;
				}
			}
			return state; // will be ’ongoing ’ until success
		});

	BehaviourAction* goToRoom = new BehaviourAction("Go To Room",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Going to the loot room!\n";
				state = Ongoing;
			}
			else if (state == Ongoing) {
				distanceToTarget -= dt;
				if (distanceToTarget <= 0.0f) {
					std::cout << "Reached room!\n";
					return Success;
				}
			}
			return state; // will be ’ongoing ’ until success
		});

	BehaviourAction* openDoor = new BehaviourAction("Open Door",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Opening Door!\n";
				return Success;
			}
			return state;
		});

	BehaviourAction* lookForTreasure = new BehaviourAction("Look For Treasure",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Looking for treasure!\n";
				return Ongoing;
			}
			else if (state == Ongoing) {
				bool found = rand() % 2;
				if (found) {
					std::cout << "I found some treasure!\n";
					return Success;
				}
				std::cout << "No treasure in here...\n";
				return Failure;
			}
			return state;
		});

	BehaviourAction* lookForItems = new BehaviourAction("Look For Items",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				std::cout << "Looking for items!\n";
				return Ongoing;
			}
			else if (state == Ongoing) {
				bool found = rand() % 2;
				if (found) {
					std::cout << "I found some items!\n";
					return Success;
				}
				std::cout << "No items in here...\n";
				return Failure;
			}
			return state;
		});

	BehaviourSequence* sequence = new BehaviourSequence("Room Sequence");
	sequence->AddChild(findKey);
	sequence->AddChild(goToRoom);
	sequence->AddChild(openDoor);

	BehaviourSelector* selection = new BehaviourSelector("Loot Selection");
	selection->AddChild(lookForTreasure);
	selection->AddChild(lookForItems);

	BehaviourSequence* rootSequence = new BehaviourSequence("Root Sequence");
	rootSequence->AddChild(sequence);
	rootSequence->AddChild(selection);

	for (int i = 0; i < 5; ++i) {
		rootSequence->Reset();
		behaviourTimer = 0.0f;
		distanceToTarget = rand() % 250;
		BehaviourState state = Ongoing;
		std::cout << "We’re going on an adventure!\n";
		while (state == Ongoing) {
			state = rootSequence->Execute(1.0f); // fake dt
		}
		if (state == Success) {
			std::cout << "What a successful adventure!\n";
		}
		else if (state == Failure) {
			std::cout << "What a waste of time!\n";
		}
	}
	std::cout << "All done!\n";
}

void TutorialGame::GetPathToDest(Vector3 destination) {
	navMesh = NavigationMesh("basicNavMesh.navmesh");
	NavigationPath outPath;

	Vector3 startPos = enemy->GetTransform().GetPosition();
	startPos.y = 0;
	enemyDest = destination;

	navMesh.SetStringPulling(true);
	bool found = navMesh.FindPath(startPos, destination, outPath);

	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		enemyPath.push_back(pos);
	}
}

bool TutorialGame::CheckCanSeePlayer(Vector3 newPos) {
	Vector3 playerPos = player->GetTransform().GetPosition();
	Vector3 rayDir = playerPos - newPos;

	//If player further than 35units away, this is considered a safe position
	if (rayDir.Length() >= 50.0f)
		return false;

	Ray ray = Ray(newPos, rayDir.Normalised());
	RayCollision closestCollision;
	if (world->Raycast(ray, closestCollision, true, enemy)) {
		//Only check closest collision. If wall obstructing player, then safe to move
		GameObject* collidedObject = (GameObject*)closestCollision.node;

		if(collidedObject->GetObjectType() == Player || collidedObject == player)
			return true;
	}
	return false;
}

void TutorialGame::FarmerBehaviourTree() {
	float timer;
	float timerEnd;

	BehaviourAction* wander = new BehaviourAction("Wander",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				/*std::cout << "Choosing position!\n";
				GetPathToDest(Vector3(300, 0, 200));
				enemyNextPos = enemyPath[0];*/
				//enemyDest = Vector3(300, 0, 200);
				ChoosePosition();
				return Ongoing;
			}
			else if (state == Ongoing) {
				Vector3 enemyPos = enemy->GetTransform().GetPosition();
				enemyPos.y = 0;
				Vector3 enemyTestDest = enemyDest;
				enemyTestDest.y = 0;
				if ((enemyPos - enemyDest).Length() < 0.1f) {
					return Success;
				}
				//if ((enemyNextPos - enemyPos).Length() < 0.1f) {
				//	bool foundCurrent = false;
				//	for (int i = 0; i < enemyPath.size(); i++) {
				//		if (foundCurrent) {
				//			enemyNextPos = enemyPath[i];
				//			break;
				//		}
				//		if (enemyPath[i] == enemyNextPos)
				//			foundCurrent = true;
				//	}
				//}
				//else {
				//	Vector3 forceDir = enemyNextPos - enemyPos;
				//	enemy->GetPhysicsObject()->AddForce(forceDir.Normalised() * 10.0f);
				//}

				//Check to see if new position will be too close to player
				if (CheckCanSeePlayer(enemyDest))
					ChoosePosition();
				else
					enemy->GetTransform().SetPosition(enemyDest);

			}
			return state;
		});

	BehaviourAction* idle = new BehaviourAction("Idle",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				return Ongoing;
			}
			else if (state == Ongoing) {
				int r = rand() % 5000;
				//std::cout << r << "\n";

				if (r < 5) {
					return Success;
				}
			}
			return state;
		});

	BehaviourAction* collectPoints = new BehaviourAction("Collect Points",
		[&](float dt, BehaviourState state)->BehaviourState {
			if (state == Initialise) {
				ChooseBonus();
				return Ongoing;
			}
			else if (state == Ongoing) {
				if (chosenBonus == nullptr)
					return Failure;
				
				enemy->GetTransform().SetPosition(chosenBonus->GetTransform().GetPosition());
				return Success;
			}
			return state;
		});

	//BehaviourSequence* sequence = new BehaviourSequence("Run Sequence");
	//sequence->AddChild(runFromPlayer);
	//sequence->AddChild(idle);

	BehaviourSequence* wanderSequence = new BehaviourSequence("Wander Sequence");
	wanderSequence->AddChild(wander);
	wanderSequence->AddChild(idle);
	wanderSequence->AddChild(collectPoints);

	BehaviourSelector* selection = new BehaviourSelector("Wander Selection");
	selection->AddChild(wanderSequence);
	selection->AddChild(collectPoints);

	enemyRootSequence = new BehaviourSequence("Root Sequence");
	//rootSequence->AddChild(sequence);
	enemyRootSequence->AddChild(selection);
}

float TutorialGame::randomNumber(float a, float b) {
	float random = ((float)rand()) / (float)RAND_MAX;
	float diff = b - a;
	float r = random * diff;
	return a + r;
}

GameObject* TutorialGame::ChooseBonus() {
	//float index = randomNumber(0, bonusObjects.size());
	int index = rand() % bonusObjects.size();

	chosenBonus = index == 0 ? nullptr : bonusObjects[index];
	return chosenBonus;
}

Vector3 TutorialGame::ChoosePosition() {
	enemyDest = Vector3(randomNumber(5, 395), 0, randomNumber(5, 395));
	return enemyDest;
}


bool TutorialGame::CheckEnemyBounds() {
	Vector3 enemyPos = enemy->GetTransform().GetPosition();

	return	enemyPos.y > -1 && enemyPos.y < 50 &&
			enemyPos.x >= 0 && enemyPos.x <= 400 &&
			enemyPos.z >= 0 && enemyPos.z <= 400;
}

void TutorialGame::EnemyRespawn() {
	enemy->GetTransform().SetPosition(Vector3(125, 5, 75));
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();


	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::W)) {
		lockedObject->GetPhysicsObject()->AddForce(fwdAxis * 30.0f);
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::A)) {
		lockedObject->GetPhysicsObject()->AddForce(-rightAxis * 10.0f);
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::S)) {
		lockedObject->GetPhysicsObject()->AddForce(-fwdAxis * 10.0f);
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::D)) {
		lockedObject->GetPhysicsObject()->AddForce(rightAxis * 10.0f);
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::Q)) {
		lockedObject->GetPhysicsObject()->AddTorque(Vector3(0, 2, 0));
	}
	if (Window::GetKeyboard()->KeyDown(KeyboardKeys::E)) {
		lockedObject->GetPhysicsObject()->AddTorque(Vector3(0, -2, 0));
	}

	if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
		PlayerLeftClick();
	}

	//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
	//	lockedObject->GetPhysicsObject()->AddForce(-fwdAxis);
	//}

	//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NEXT)) {
	//	lockedObject->GetPhysicsObject()->AddForce(Vector3(0,-10,0));
	//}
}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}
}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	ClearWorld();

	TestPathfinding();

	//Markers
	//AddCubeToWorld(Vector3(10.0f, 1.0f, 10.0f), Vector3(1.0f, 1.0f, 1.0f), 0);
	//AddCubeToWorld(Vector3(50.0f, 1.0f, 50.0f), Vector3(1.0f, 1.0f, 1.0f), 0);
	//AddCubeToWorld(Vector3(50.0f, 1.0f, 200.0f), Vector3(1.0f, 1.0f, 1.0f), 0);
	//AddCubeToWorld(Vector3(100.0f, 1.0f, 100.0f), Vector3(1.0f, 1.0f, 1.0f), 0);
	//AddCubeToWorld(Vector3(100.0f, 1.0f, 200.0f), Vector3(1.0f, 1.0f, 1.0f), 0);
	//AddCubeToWorld(Vector3(150.0f, 1.0f, 150.0f), Vector3(1.0f, 1.0f, 1.0f), 0);
	//AddCubeToWorld(Vector3(150.0f, 1.0f, 200.0f), Vector3(1.0f, 1.0f, 1.0f), 0);
	//AddCubeToWorld(Vector3(200.0f, 1.0f, 200.0f), Vector3(1.0f, 1.0f, 1.0f), 0);

	AddWalls();
	AddObstacles();
	AddBoundaries();
	InitGameExamples();
	InitDefaultFloor();

	FarmerBehaviourTree();
}

void TutorialGame::ClearWorld() {
	if(enemyRootSequence)
		enemyRootSequence->Reset();

	world->ClearAndErase();
	physics->Clear();
	testStateObject = nullptr;
	player = nullptr;
	enemy = nullptr;
	lockedObject = nullptr;
	testNodes.clear();

	
	physics->UseGravity(true);
}

void TutorialGame::InitMaze() {
	ClearWorld();

	InitDefaultFloor();
	AddBoundaries();
	AddMaze();

	player = AddPlayerToWorld(Vector3(100.0f, 2.0f, 200.0f));
	LockCameraToObject(player);

	testNavMesh = NavigationMesh("mazeMesh.navmesh");
	NavigationMesh navMesh("mazeMesh.navmesh");
	NavigationPath outPath;

	//Vector3 startPos(100, 0, 200);
	//Vector3 endPos(300, 0, 200);
	Vector3 startPos(160, 0, 160);
	Vector3 endPos(240, 0, 240);
	AddCubeToWorld(startPos, Vector3(1, 1, 1), 0.0f);
	AddCubeToWorld(endPos, Vector3(1, 1, 1), 0.0f);

	navMesh.SetStringPulling(true);
	//bool found = navMesh.FindPath(startPos, endPos, outPath);
	bool found = navMesh.FindVertPath(startPos, endPos, outPath);
	Vector3 pos;
	while (outPath.PopWaypoint(pos)) {
		testNodes.push_back(pos);
	}
}

void TutorialGame::AddWalls() {
	//Centre Room
	AddCubeToWorld(Vector3(200.0f, 5.0f, 175.0f), Vector3(50.0f, 5.0f, 1.0f), 0);
	AddCubeToWorld(Vector3(200.0f, 5.0f, 225.0f), Vector3(50.0f, 5.0f, 1.0f), 0);
	AddCubeToWorld(Vector3(150.0f, 5.0f, 185.0f), Vector3(1.0f, 5.0f, 10.0f), 0);
	AddCubeToWorld(Vector3(150.0f, 5.0f, 215.0f), Vector3(1.0f, 5.0f, 10.0f), 0);
	AddCubeToWorld(Vector3(250.0f, 5.0f, 185.0f), Vector3(1.0f, 5.0f, 10.0f), 0);
	AddCubeToWorld(Vector3(250.0f, 5.0f, 215.0f), Vector3(1.0f, 5.0f, 10.0f), 0);

	AddCubeToWorld(Vector3(75.0f, 5.0f, 300.0f), Vector3(50.0f, 5.0f, 1.0f), 0);


	AddCubeToWorld(Vector3(250.0f, 5.0f, 100.0f), Vector3(1.0f, 5.0f, 50.0f), 0);
}

void TutorialGame::AddObstacles() {
	//Newton's Cradles
	HangingConstraint(Vector3(60.0f, 50.0f, 60.0f));
	HangingConstraint(Vector3(60.0f, 50.0f, 80.0f));
	HangingConstraint(Vector3(60.0f, 50.0f, 100.0f));
	AddCubeToWorld(Vector3(60.0f, 0.0f, 45.0f), Vector3(2.0f, 15.0f, 2.0f), 0);
	AddSphereToWorld(Vector3(60.0f, 20.0f, 45.0f), 4.0f, 100.0f, Destructible);

	HangingConstraint(Vector3(120.0f, 50.0f, 120.0f));
	HangingConstraint(Vector3(100.0f, 50.0f, 120.0f));
	HangingConstraint(Vector3(80.0f, 50.0f, 120.0f));
	AddCubeToWorld(Vector3(135.0f, 0.0f, 120.0f), Vector3(2.0f, 15.0f, 2.0f), 0);
	AddSphereToWorld(Vector3(135.0f, 20.0f, 120.0f), 4.0f, 100.0f, Points);

	//Platform
	AddOBBToWorld(Vector3(60.0f, 0.0f, 164.0f), Vector3(-63.0f, 0.0f, 0.0f), Vector3(9.0f, 40.0f, 1.0f), 0);
	AddCubeToWorld(Vector3(60.0f, 18.0f, 120.0f), Vector3(9.0f, 1.0f, 9.0f), 0);

	//Platform Obstacle
	testStateObject = AddStateObjectToWorld(Vector3(60.0f, 22.0f, 120.0f));

	//Cubes
	AddCubeToWorld(Vector3(195, 1, 95), Vector3(1, 1, 1), 5.0f);
	AddCubeToWorld(Vector3(195, 1, 100), Vector3(1, 1, 1), 5.0f);
	AddCubeToWorld(Vector3(195, 1, 105), Vector3(1, 1, 1), 5.0f);
	AddCubeToWorld(Vector3(200, 1, 95), Vector3(1, 1, 1), 5.0f);
	AddCubeToWorld(Vector3(200, 1, 100), Vector3(1, 1, 1), 5.0f, Destructible);
	AddCubeToWorld(Vector3(200, 1, 105), Vector3(1, 1, 1), 5.0f);
	AddCubeToWorld(Vector3(205, 1, 95), Vector3(1, 1, 1), 5.0f);
	AddCubeToWorld(Vector3(205, 1, 100), Vector3(1, 1, 1), 5.0f);
	AddCubeToWorld(Vector3(205, 1, 105), Vector3(1, 1, 1), 5.0f);

	//Capsules
	AddCapsuleToWorld(Vector3(50.0f, 4.0f, 250.0f), 2.0f, 4.0f, 200.0f);
	AddCapsuleToWorld(Vector3(60.0f, 4.0f, 250.0f), 2.0f, 4.0f, 200.0f, Destructible);
	AddCapsuleToWorld(Vector3(60.0f, 4.0f, 260.0f), 2.0f, 4.0f, 200.0f);
	AddCapsuleToWorld(Vector3(80.0f, 4.0f, 250.0f), 2.0f, 4.0f, 200.0f);
	AddCapsuleToWorld(Vector3(80.0f, 4.0f, 260.0f), 2.0f, 4.0f, 200.0f, Destructible);
	AddSphereToWorld(Vector3(70.0f, 2.0f, 250.0f), 2.0f, 200.0f, Destructible);
	AddSphereToWorld(Vector3(70.0f, 2.0f, 260.0f), 2.0f, 200.0f);
	AddCubeToWorld(Vector3(50.0f, 2.0f, 240.0f), Vector3(2, 2, 2), 200.0f);
	AddCubeToWorld(Vector3(60.0f, 2.0f, 240.0f), Vector3(2, 2, 2), 200.0f);
	AddCubeToWorld(Vector3(70.0f, 2.0f, 240.0f), Vector3(2, 2, 2), 200.0f);
	AddCubeToWorld(Vector3(80.0f, 2.0f, 240.0f), Vector3(2, 2, 2), 200.0f);
	GameObject* capsule = AddCapsuleToWorld(Vector3(50.0f, 2.0f, 260.0f), 2.0f, 4.0f, 200.0f, Destructible);
	capsule->GetTransform().SetOrientation(Quaternion::EulerAnglesToQuaternion(0, 0, 90.0f));

	bonusObjects.emplace_back(AddCapsuleToWorld(Vector3(75.0f, 4.0f, 320.0f), 2.0f, 4.0f, 200.0f, Points));
	bonusObjects.emplace_back(AddCapsuleToWorld(Vector3(120.0f, 4.0f, 260.0f), 2.0f, 4.0f, 200.0f, Points));
	bonusObjects.emplace_back(AddCapsuleToWorld(Vector3(150.0f, 4.0f, 260.0f), 2.0f, 4.0f, 200.0f, Points));
	bonusObjects.emplace_back(AddCapsuleToWorld(Vector3(180.0f, 4.0f, 260.0f), 2.0f, 4.0f, 200.0f, Points));
	bonusObjects.emplace_back(AddCapsuleToWorld(Vector3(210.0f, 4.0f, 260.0f), 2.0f, 4.0f, 200.0f, Points));
	bonusObjects.emplace_back(AddCapsuleToWorld(Vector3(240.0f, 4.0f, 260.0f), 2.0f, 4.0f, 200.0f, Points));
	bonusObjects.emplace_back(AddCapsuleToWorld(Vector3(270.0f, 4.0f, 260.0f), 2.0f, 4.0f, 200.0f, Points));
	bonusObjects.emplace_back(AddCapsuleToWorld(Vector3(300.0f, 4.0f, 260.0f), 2.0f, 4.0f, 200.0f, Points));

	//Spheres
	AddSphereToWorld(Vector3(297.5f, 1.0f, 197.5f), 2.0f, 10.0f, Points);
	AddSphereToWorld(Vector3(302.5f, 1.0f, 197.5f), 2.0f);
	AddSphereToWorld(Vector3(302.5f, 1.0f, 202.5f), 2.0f, 10.0f, Points);
	AddSphereToWorld(Vector3(297.5f, 1.0f, 202.5f), 2.0f);

	AddSphereToWorld(Vector3(297.5f, 5.0f, 197.5f), 2.0f);
	AddSphereToWorld(Vector3(302.5f, 5.0f, 197.5f), 2.0f, 10.0f, Points);
	AddSphereToWorld(Vector3(302.5f, 5.0f, 202.5f), 2.0f);
	AddSphereToWorld(Vector3(297.5f, 5.0f, 202.5f), 2.0f, 10.0f, Points);

	AddSphereToWorld(Vector3(297.5f, 9.0f, 197.5f), 2.0f, 10.0f, Points);
	AddSphereToWorld(Vector3(302.5f, 9.0f, 197.5f), 2.0f);
	AddSphereToWorld(Vector3(302.5f, 9.0f, 202.5f), 2.0f, 10.0f, Points);
	AddSphereToWorld(Vector3(297.5f, 9.0f, 202.5f), 2.0f);

}

void TutorialGame::AddBoundaries() {
	//Boundaries
	(AddCubeToWorld(Vector3(0.5f, 5.0f, 200.0f), Vector3(0.5f, 5.0f, 200.0f), 0))->GetRenderObject()->SetColour(Vector4(1, 1, 1, 0.4));
	(AddCubeToWorld(Vector3(200.0f, 5.0f, 0.5f), Vector3(200.0f, 5.0f, 0.5f), 0))->GetRenderObject()->SetColour(Vector4(1, 1, 1, 0.4));
	(AddCubeToWorld(Vector3(200.0f, 5.0f, 399.5f), Vector3(200.0f, 5.0f, 0.5f), 0))->GetRenderObject()->SetColour(Vector4(1, 1, 1, 0.4));
	(AddCubeToWorld(Vector3(399.5, 5.0f, 200.0f), Vector3(0.5f, 5.0f, 200.0f), 0))->GetRenderObject()->SetColour(Vector4(1, 1, 1, 0.4));
}

void TutorialGame::AddMaze() {
	AddCubeToWorld(Vector3(200.0f, 5.0f, 150.0f), Vector3(50.0f, 5.0f, 1.0f), 0);
	AddCubeToWorld(Vector3(200.0f, 5.0f, 250.0f), Vector3(50.0f, 5.0f, 1.0f), 0);
	AddCubeToWorld(Vector3(250.0f, 5.0f, 170.0f), Vector3(1.0f, 5.0f, 20.0f), 0);
	AddCubeToWorld(Vector3(250.0f, 5.0f, 230.0f), Vector3(1.0f, 5.0f, 20.0f), 0);
	AddCubeToWorld(Vector3(150.0f, 5.0f, 170.0f), Vector3(1.0f, 5.0f, 20.0f), 0);
	AddCubeToWorld(Vector3(150.0f, 5.0f, 230.0f), Vector3(1.0f, 5.0f, 20.0f), 0);

	AddCubeToWorld(Vector3(170.0f, 5.0f, 200.0f), Vector3(1.0f, 5.0f, 20.0f), 0);
	AddCubeToWorld(Vector3(230.0f, 5.0f, 200.0f), Vector3(1.0f, 5.0f, 20.0f), 0);

	AddCubeToWorld(Vector3(170.0f, 5.0f, 170.0f), Vector3(20.0f, 5.0f, 1.0f), 0);
	AddCubeToWorld(Vector3(230.0f, 5.0f, 230.0f), Vector3(20.0f, 5.0f, 1.0f), 0);
	AddCubeToWorld(Vector3(170.0f, 5.0f, 230.0f), Vector3(20.0f, 5.0f, 1.0f), 0);
	AddCubeToWorld(Vector3(230.0f, 5.0f, 170.0f), Vector3(20.0f, 5.0f, 1.0f), 0);

	AddCubeToWorld(Vector3(200.0f, 5.0f, 210.0f), Vector3(10.0f, 5.0f, 1.0f), 0);
	AddCubeToWorld(Vector3(200.0f, 5.0f, 190.0f), Vector3(10.0f, 5.0f, 1.0f), 0);

	AddCapsuleToWorld(Vector3(160.0f, 4.0f, 160.0f), 2.0f, 4.0f, 200.0f, Points);
	AddCapsuleToWorld(Vector3(160.0f, 4.0f, 240.0f), 2.0f, 4.0f, 200.0f, Points);
	AddCapsuleToWorld(Vector3(240.0f, 4.0f, 160.0f), 2.0f, 4.0f, 200.0f, Points);
	AddCapsuleToWorld(Vector3(240.0f, 4.0f, 240.0f), 2.0f, 4.0f, 200.0f, Points);

	AddCapsuleToWorld(Vector3(180.0f, 4.0f, 200.0f), 2.0f, 4.0f, 200.0f, Points);
	AddCapsuleToWorld(Vector3(220.0f, 4.0f, 200.0f), 2.0f, 4.0f, 200.0f, Points);
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Vector3& position, ObjectType t) {
	GameObject* floor = new GameObject(t);

	Vector3 floorSize = Vector3(200, 2, 200);
	AABBVolume* volume = new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);

	return floor;
}

StateGameObject* TutorialGame::AddStateObjectToWorld(const Vector3& position, ObjectType t) {
	StateGameObject* obstacle = new StateGameObject(t);

	Vector3 capsuleSize = Vector3(2 * 2, 4, 2 * 2);

	CapsuleVolume* volume = new CapsuleVolume(4.0f, 2.0f);
	obstacle->SetBoundingVolume((CollisionVolume*)volume);
	obstacle->GetTransform()
		.SetScale(capsuleSize)
		.SetPosition(position);

	obstacle->SetRenderObject(new RenderObject(&obstacle->GetTransform(), capsuleMesh, basicTex, basicShader));
	obstacle->SetPhysicsObject(new PhysicsObject(&obstacle->GetTransform(), obstacle->GetBoundingVolume()));

	obstacle->GetPhysicsObject()->SetInverseMass(0.001f);
	obstacle->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(obstacle);

	return obstacle;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Vector3& position, float radius, float inverseMass, ObjectType t, bool hollow) {
	GameObject* sphere = new GameObject(t);

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	sphere->GetPhysicsObject()->InitSphereInertia(hollow);

	world->AddGameObject(sphere);

	return sphere;
}

GameObject* TutorialGame::AddCapsuleToWorld(const Vector3& position, float radius, float halfHeight, float inverseMass, ObjectType t) {
	GameObject* capsule = new GameObject(t);

	Vector3 capsuleSize = Vector3(radius * 2, halfHeight, radius * 2);
	//Vector3 capsuleSize = Vector3(1.0f, 1.0f, 1.0f);
	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform()
		.SetScale(capsuleSize)
		.SetPosition(position);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCapsuleInertia();

	world->AddGameObject(capsule);

	return capsule;
}

GameObject* TutorialGame::AddCubeToWorld(const Vector3& position, Vector3 dimensions, float inverseMass, ObjectType t) {
	GameObject* cube = new GameObject(t);

	AABBVolume* volume = new AABBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddOBBToWorld(const Vector3& position, Vector3 orientation, Vector3 dimensions, float inverseMass, ObjectType t) {
	GameObject* cube = new GameObject(t);

	OBBVolume* volume = new OBBVolume(dimensions);
	cube->SetBoundingVolume((CollisionVolume*)volume);

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2)
		.SetOrientation(Quaternion::EulerAnglesToQuaternion(orientation.x, orientation.y, orientation.z));

	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(cube);

	return cube;
}

GameObject* TutorialGame::AddPlayerToWorld(const Vector3& position) {
	float meshSize		= 1.0f;
	float inverseMass	= 0.5f;

	GameObject* character = new GameObject(Player);
	SphereVolume* volume  = new SphereVolume(1.0f);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), charMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume(), 0.66f, 0.99f, 0.99f));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddEnemyToWorld(const Vector3& position) {
	float meshSize		= 3.0f;
	float inverseMass	= 0.5f;

	GameObject* character = new GameObject(Enemy);

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.9f, 0.3f) * meshSize);
	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	character->SetRenderObject(new RenderObject(&character->GetTransform(), enemyMesh, nullptr, basicShader));
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	return character;
}

GameObject* TutorialGame::AddBonusToWorld(const Vector3& position) {
	GameObject* apple = new GameObject(Points);

	SphereVolume* volume = new SphereVolume(0.5f);
	apple->SetBoundingVolume((CollisionVolume*)volume);
	apple->GetTransform()
		.SetScale(Vector3(2, 2, 2))
		.SetPosition(position);

	apple->SetRenderObject(new RenderObject(&apple->GetTransform(), bonusMesh, nullptr, basicShader));
	apple->SetPhysicsObject(new PhysicsObject(&apple->GetTransform(), apple->GetBoundingVolume()));

	apple->GetPhysicsObject()->SetInverseMass(1.0f);
	apple->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(apple);

	return apple;
}

void TutorialGame::BridgeConstraintTest() {
	Vector3 cubeSize = Vector3(8, 8, 8);
	
	float invCubeMass = 5; // how heavy the middle pieces are
	int numLinks = 10;
	float maxDistance = 30; // constraint distance
	float cubeDistance = 20; // distance between links
	
	Vector3 startPos = Vector3(50, 50, 50);
	
	GameObject* start = AddCubeToWorld(startPos + Vector3(0, 0, 0), cubeSize, 0);
	GameObject* end = AddCubeToWorld(startPos + Vector3((numLinks + 2) * cubeDistance, 0, 0), cubeSize, 0);
	
	GameObject* previous = start;
	
	for(int i = 0; i < numLinks; ++i) {
		GameObject * block = AddCubeToWorld(startPos + Vector3((i + 1) * cubeDistance, 0, 0), cubeSize, invCubeMass);
		PositionConstraint * constraint = new PositionConstraint(previous, block, maxDistance);
		world->AddConstraint(constraint);
		previous = block;
	}
	PositionConstraint * constraint = new PositionConstraint(previous,
	end, maxDistance);
	world->AddConstraint(constraint);
}

void TutorialGame::HangingConstraint(Vector3 position) {
	Vector3 cubeSize = Vector3(4, 4, 4);

	float distance = 30.0f;

	GameObject* cube = AddCubeToWorld(position, cubeSize, 0);
	//This is a hollow sphere, so it has different inertia tensor to regular sphere
	GameObject* sphere = AddSphereToWorld(Vector3(position.x, position.y - distance, position.z), 4, 500.0f, Default, true);

	PositionConstraint* constraint = new PositionConstraint(cube, sphere, distance);
	world->AddConstraint(constraint);
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(200, -2, 200));
}

void TutorialGame::InitGameExamples() {
	player = AddPlayerToWorld(Vector3(100, 5, 100));
	LockCameraToObject(player);

	enemy = AddEnemyToWorld(Vector3(125, 0, 100));
	//AddBonusToWorld(Vector3(10, 5, 0));
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims);
			}
			else {
				AddSphereToWorld(position, sphereRadius);
			}
		}
	}

	AddSphereToWorld(Vector3(10, 20, 20), 2.0f, 40.0f, Default, true);
	AddSphereToWorld(Vector3(15, 20, 20), 2.0f, 40.0f);
	AddCubeToWorld(Vector3(20, 20, 20), Vector3(1, 3, 1), 40.0f);
	AddCapsuleToWorld(Vector3(25, 20, 20), 1.0f, 2.0f, 40.0f);
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

/*
Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::R)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (!lockedObject) {
		if (inSelectionMode) {
			//Debug::Print("Press R to change to camera mode!", Vector2(5, 85));

			if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
				if (selectionObject) {	//set colour to deselected;
					selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
					selectionObject = nullptr;
				}

				Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

				RayCollision closestCollision;
				if (world->Raycast(ray, closestCollision, true)) {
					selectionObject = (GameObject*)closestCollision.node;

					selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
					return true;
				}
				else {
					return false;
				}
			}
			if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
				if (selectionObject) {
					if (lockedObject == selectionObject) {
						lockedObject = nullptr;
					}
					else {
						lockedObject = selectionObject;
					}
				}
			}
		}
		else {
			//Debug::Print("Press R to change to select mode!", Vector2(5, 85));
		}
	}
	return false;
}

bool TutorialGame::PlayerLeftClick() {
	if (selectionObject) {
		selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
		selectionObject = nullptr;
	}

	Ray ray = CollisionDetection::BuildRayFromCenterScreen(*world->GetMainCamera());

	RayCollision closestCollision;
	if (world->Raycast(ray, closestCollision, true, player)) {
		//selectionObject = (GameObject*)closestCollision.node;
		//selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));

		GameObject* collidedObject = (GameObject*)closestCollision.node;

		if ((closestCollision.collidedAt - player->GetTransform().GetPosition()).Length() < playerRange){
			collidedObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * playerForce, closestCollision.collidedAt);

			if (collidedObject->GetObjectType() == Destructible)
				collidedObject->SetIsActive(false);

			return true;
		}
		else
			return false;
	}
	else {
		return false;
	}
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/

void TutorialGame::MoveSelectedObject() {
	if(selectionObject)
		Debug::Print("Click Force:" + std::to_string(forceMagnitude), Vector2(5, 90));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject) {
		return;//we haven't selected anything!
	}
	//Push the selected object!
	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}
	//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
	//	selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 10, 0));
	//}
	//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
	//	selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
	//}
	//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
	//	selectionObject->GetPhysicsObject()->AddForce(Vector3(10, 0, 0));
	//}
	//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
	//	selectionObject->GetPhysicsObject()->AddForce(Vector3(-10, 0, 0));
	//}
}

bool TutorialGame::GetRemainingDestructibles() {
	return world->GetDestructibleCount();
}

float TutorialGame::GetScore() {
	if (player)
		return player->GetPoints();
	else
		return 0;
}

