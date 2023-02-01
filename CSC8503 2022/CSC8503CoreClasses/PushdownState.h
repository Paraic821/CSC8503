#pragma once

namespace NCL {
	namespace CSC8503 {
		class PushdownState
		{
		public:
			enum PushdownResult {
				Push, Pop, NoChange
			}; 
			PushdownState() {}
			//PushdownState(NCL::CSC8503::TutorialGame* g)  {
			//	game = g;
			//}
			virtual ~PushdownState() {}

			virtual PushdownResult OnUpdate(float dt, PushdownState** pushFunc) = 0;
			virtual void OnAwake() {}
			virtual void OnSleep() {}


			//TutorialGame* GetGame() {
			//	return game;
			//}

			int GetStateID() {
				return StateID;
			}
		protected:
			//NCL::CSC8503::TutorialGame* game = nullptr;
			int StateID = 0;
		};
	}
}