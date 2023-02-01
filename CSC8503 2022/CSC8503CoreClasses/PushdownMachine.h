#pragma once
#include "PushdownState.h"

namespace NCL {
	namespace CSC8503 {
		class PushdownMachine
		{
		public:
			PushdownMachine(PushdownState* initialState);
			~PushdownMachine();

			bool Update(float dt);
			PushdownState* GetCurrentState() {
				return activeState;
			}

			void SetNewState(PushdownState* s) {
				newState = s;
				tempResult = PushdownState::PushdownResult::Pop;
			}

		protected:
			PushdownState* activeState;
			PushdownState* initialState;

			std::stack<PushdownState*> stateStack;
			PushdownState* newState = nullptr;
			PushdownState::PushdownResult tempResult = PushdownState::PushdownResult::NoChange;
		};
	}
}

