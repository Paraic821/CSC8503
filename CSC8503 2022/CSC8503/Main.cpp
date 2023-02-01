#include "Window.h"

#include "Debug.h"

#include "StateMachine.h"
#include "StateTransition.h"
#include "State.h"

#include "GameServer.h"
#include "GameClient.h"

#include "NavigationGrid.h"
#include "NavigationMesh.h"

#include "TutorialGame.h"
#include "NetworkedGame.h"
#include "NetworkBase.h"
#include "NetworkObject.h"
#include "NetworkState.h"

#include "PushdownMachine.h"

#include "PushdownState.h"

#include "BehaviourNode.h"
#include "BehaviourSelector.h"
#include "BehaviourSequence.h"
#include "BehaviourAction.h"

using namespace NCL;
using namespace CSC8503;


#include <chrono>
#include <thread>
#include <sstream>

//class PauseScreen : public PushdownState {
//	PushdownResult OnUpdate(float dt, PushdownState * *newState) override {
//		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::U)) {
//			return PushdownResult::Pop;
//		}
//		return PushdownResult::NoChange;
//	}
//
//	void OnAwake() override {
//		std::cout << "Press U to unpause game!\n";
//	}
//};
//
//class GameScreen : public PushdownState {
//	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
//		pauseReminder -= dt;
//		if (pauseReminder < 0) {
//			std::cout << "Coins mined:" << coinsMined << "\n";
//			std::cout << "Press P to pause game , or F1 to return to main menu !\n";
//			pauseReminder += 1.0f;
//		}
//		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::P)) {
//			*newState = new PauseScreen();
//			return PushdownResult::Push;
//		}
//		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F1)) {
//			std::cout << "Returning to main menu!\n";
//			return PushdownResult::Pop;
//		}
//		if (rand() % 7 == 0) {
//			coinsMined++;
//		}
//		return PushdownResult::NoChange;
//	};
//	void OnAwake() override {
//		std::cout << "Preparing to mine coins!\n";
//	}
//protected:
//	int coinsMined = 0;
//	float pauseReminder = 1;
//};
//
//class IntroScreen : public PushdownState {
//	PushdownResult OnUpdate(float dt, PushdownState * *newState) override {
//		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::SPACE)) {
//			* newState = new GameScreen();
//			return PushdownResult::Push;
//		}
//		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE)) {
//			return PushdownResult::Pop;
//		}
//		return PushdownResult::NoChange;
//	};
//
//	void OnAwake() override {
//		std::cout << "Welcome to a really awesome game!\n";
//		std::cout << "Press Space To Begin or escape to quit!\n";
//	}
//};
//
//void TestPushdownAutomata(Window* w) {
//	PushdownMachine machine(new IntroScreen());
//	while (w->UpdateWindow()) {
//		float dt = w->GetTimer()->GetTimeDeltaSeconds();
//		if (!machine.Update(dt)) {
//			return;
//		}
//	}
//}

class TestPacketReceiver : public PacketReceiver {
public:
	TestPacketReceiver(string name) {
		this->name = name;
	}
	
	void ReceivePacket(int type, GamePacket * payload, int source) {
		if (type == String_Message) {
			StringPacket * realPacket = (StringPacket*)payload;
			string msg = realPacket->GetStringFromData();
			std::cout << name << " received message : " << msg << std::endl;
		}
	}
protected:
	string name;
};


class PauseScreen : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::U)) {
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	}

	void OnAwake() override {
		Debug::Print("Press U to unpause game!\n", Vector3(5, 15, 0));
		StateID = 1;
	}
};

class GameScreen : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		timer -= dt;

		//std::cout << "Time remaining:" << std::to_string(timer) << "\n";
		Debug::Print("Time remaining:" + std::to_string(timer) + "\n", Vector3(5, 20, 0));
		//Debug::Print("Press P to pause game, or F5 to return to main menu !\n", Vector3(5, 10, 0));
		Debug::Print("Press F5 to return to main menu\n", Vector3(5, 25, 0));

		//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::P)) {
		//	*newState = new PauseScreen();
		//	return PushdownResult::Push;
		//}
		if (timer <= 0) {
			std::cout << "Out of time! Try again.\n";
			return PushdownResult::Pop;
		}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F5)) {
			std::cout << "Returning to main menu!\n";
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	};
	void OnAwake() override {
		std::cout << "Timer started!\n";
		StateID = 2;
	}
protected:
	float timer = 180;
	float pauseReminder = 1;
};

class MultiPlayer : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		timer -= dt;

		//std::cout << "Time remaining:" << std::to_string(timer) << "\n";
		Debug::Print("Time remaining:" + std::to_string(timer) + "\n", Vector3(5, 20, 0));
		//Debug::Print("Press P to pause game, or F5 to return to main menu !\n", Vector3(5, 10, 0));
		Debug::Print("Press F5 to return to main menu\n", Vector3(5, 25, 0));

		//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::P)) {
		//	*newState = new PauseScreen();
		//	return PushdownResult::Push;
		//}
		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::F5)) {
			std::cout << "Returning to main menu!\n";
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	};
	void OnAwake() override {
		std::cout << "Timer started!\n";
		StateID = 3;
	}
protected:
	float timer = 180;
	float pauseReminder = 1;
};

class IntroScreen : public PushdownState {
	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		Debug::Print("Press F1 To Play SinglePlayer", Vector3(10, 40, 0));
		Debug::Print("or F2 to play MultiPlayer\n", Vector3(10, 50, 0));
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
			*newState = new GameScreen();
			return PushdownResult::Push;
		}
		else if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
			*newState = new MultiPlayer();
			return PushdownResult::Push;
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE)) {
			return PushdownResult::Pop;
		}
		return PushdownResult::NoChange;
	};

	void OnAwake() override {
		//std::cout << "Welcome to a really awesome game!\n";
		//std::cout << "Press qwerty To Begin or escape to quit!\n";
		StateID = 4;
	}
};

void TestStateMachine() {
	StateMachine* testMachine = new StateMachine();
	int data = 0;
		
	State* A = new State([&](float dt)->void{	std::cout << "I’m in state A!\n";
												data++;		});
	State* B = new State([&](float dt)->void{	std::cout << "I’m in state B!\n";
												data--;		});

	StateTransition* stateAB = new StateTransition(A, B, [&](void)->bool{ return data > 10; });
	StateTransition* stateBA = new StateTransition(B, A, [&](void)->bool{ return data < 0; });

	testMachine->AddState(A);
	testMachine->AddState(B);
	testMachine->AddTransition(stateAB);
	testMachine->AddTransition(stateBA);
	
	for(int i = 0; i < 100; ++i) {
		testMachine->Update(1.0f);
	}
}

void TestPushdownAutomata(Window* w) {
	PushdownMachine machine(new IntroScreen());
	while (w->UpdateWindow()) {
		float dt = w->GetTimer()->GetTimeDeltaSeconds();
		if (!machine.Update(dt)) {
			return;
		}
	}
}

void TestNetworking() {
	NetworkBase::Initialise();
	
	TestPacketReceiver serverReceiver("Server");
	TestPacketReceiver clientReceiver("Client");
	
	int port = NetworkBase::GetDefaultPort();
	
	GameServer* server = new GameServer(port, 1);
	GameClient* client = new GameClient();
	
	server->RegisterPacketHandler(String_Message, &serverReceiver);
	client->RegisterPacketHandler(String_Message, &clientReceiver);
	
	bool canConnect = client->Connect(127, 0, 0, 1, port);
	
	for (int i = 0; i < 100; ++i) {
		server->SendGlobalPacket(StringPacket("Server says hello!" + std::to_string(i)));
		client->SendPacket(StringPacket("Client says hello!" + std::to_string(i)));
		
		server->UpdateServer();
		client->UpdateClient();
		
		std::this_thread::sleep_for(std::chrono::milliseconds(10));		
	}	
	NetworkBase::Destroy();
}


/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead. 

This time, we've added some extra functionality to the window class - we can
hide or show the 

*/
int main() {
	Window*w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);
	//TestPushdownAutomata(w);
	TestNetworking();

	if (!w->HasInitialised()) {
		return -1;
	}	

	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);

	PushdownMachine machine(new IntroScreen());
	PushdownState* previousState = machine.GetCurrentState();

	TutorialGame* g = new TutorialGame();
	w->GetTimer()->GetTimeDeltaSeconds(); //Clear the timer so we don't get a larget first dt!
	while (w->UpdateWindow() && !Window::GetKeyboard()->KeyDown(KeyboardKeys::ESCAPE)) {
		float dt = w->GetTimer()->GetTimeDeltaSeconds();

		machine.Update(dt);

		if (dt > 0.1f) {
			std::cout << "Skipping large time delta" << std::endl;
			continue; //must have hit a breakpoint or something to have a 1 second frame time!
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::PRIOR)) {
			w->ShowConsole(true);
		}
		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NEXT)) {
			w->ShowConsole(false);
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::T)) {
			w->SetWindowPosition(0, 0);
		}

		w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt));

		g->UpdateGame(dt);

		if (previousState && previousState->GetStateID() == 2 && g->GetRemainingDestructibles() == 0) {
			machine.SetNewState(new IntroScreen());
			std::cout << "Congratulations! Finished with a score of:" << g->GetScore() << std::endl;
		}

		if (machine.GetCurrentState() != previousState) {
			previousState = machine.GetCurrentState();
			//if (typeid(previousState) == typeid(GameScreen)) {
			if (previousState->GetStateID() == 2) {
				g->InitWorld();
			}
			//else if (typeid(previousState) == typeid(MultiPlayer)) {
			else if (previousState->GetStateID() == 3) {
				g->InitMaze();
			}
			//else if (typeid(previousState) == typeid(IntroScreen)) {
			else if (previousState->GetStateID() == 4) {
				g->ClearWorld();
			}
		}
	}
	Window::DestroyGameWindow();
}