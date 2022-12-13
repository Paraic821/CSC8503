#pragma once
#include "GameObject.h"

namespace NCL {
	namespace CSC8503 {
		class Destructible : public GameObject {
		public:
			Destructible();
			~Destructible();

			virtual void OnCollisionBegin(GameObject* otherObject) {
				//std::cout << "OnCollisionBegin event occured!\n";
			}

			virtual void OnCollisionEnd(GameObject* otherObject) {
				//std::cout << "OnCollisionEnd event occured!\n";
			}

		protected:

		};
	}
}