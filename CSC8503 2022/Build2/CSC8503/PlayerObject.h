#pragma once
#include "GameObject.h"
#include "Destructible.h"
#include <vector>
#include <typeinfo>

namespace NCL {
	namespace CSC8503 {
		class PlayerObject : public GameObject {
		public:
			PlayerObject();
			~PlayerObject();

			std::vector<GameObject*> GetObjectsToBeDestroyed() {
				return toBeDestroyed;
			}

			void SetObjectsToBeDestoryed(std::vector<GameObject*> objects) {
				toBeDestroyed = objects;
			}

			virtual void OnCollisionBegin(GameObject* otherObject) {
				//std::cout << "OnCollisionBegin event occured!\n";
				if (typeid(otherObject) == typeid(Destructible)) {
					toBeDestroyed.push_back(otherObject);
					points++;
				}
			}

			virtual void OnCollisionEnd(GameObject* otherObject) {
				//std::cout << "OnCollisionEnd event occured!\n";
			}

		protected:
			std::vector<GameObject*> toBeDestroyed;
			float points = 0;
		};
	}
}