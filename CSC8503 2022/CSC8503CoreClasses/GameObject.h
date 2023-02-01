#pragma once
#include "Transform.h"
#include "CollisionVolume.h"
#include "PhysicsObject.h"
#include "RenderObject.h"

using std::vector;

namespace NCL::CSC8503 {
	class NetworkObject;

	enum ObjectType { Points, Destructible, Player, Enemy, Default	};

	class GameObject	{
	public:
		GameObject(ObjectType t = Default, std::string name = "");
		~GameObject();

		void SetBoundingVolume(CollisionVolume* vol) {
			boundingVolume = vol;
		}

		const CollisionVolume* GetBoundingVolume() const {
			return boundingVolume;
		}

		bool IsActive() const {
			return isActive;
		}

		void SetIsActive(bool a) {
			isActive = a;
		}

		Transform& GetTransform() {
			return transform;
		}

		RenderObject* GetRenderObject() const {
			return renderObject;
		}

		PhysicsObject* GetPhysicsObject() const {
			return physicsObject;
		}

		NetworkObject* GetNetworkObject() const {
			return networkObject;
		}

		void SetRenderObject(RenderObject* newObject) {
			renderObject = newObject;
			SetColour();
		}

		void SetPhysicsObject(PhysicsObject* newObject) {
			physicsObject = newObject;
		}

		bool GetIsProjectile() {
			return physicsObject->GetIsProjectile();
		}

		const std::string& GetName() const {
			return name;
		}

		ObjectType GetObjectType() {
			return objType;
		}

		void SetObjectType(ObjectType t) {
			objType = t;
		}

		float GetPoints() {
			return points;
		}

		void SetPoints(float p) {
			points = p;
		}

		virtual void OnCollisionBegin(GameObject* otherObject) {
			//std::cout << "OnCollisionBegin event occured!\n";
			switch (objType) {
				case Destructible:
					if (otherObject->GetIsProjectile())
						isActive = false;
					break;
				case Enemy:
					if (otherObject->GetObjectType() == Points && otherObject->IsActive()) {
						renderObject->SetColour(Vector4(0, 0, 1, 1));
					}
				case Player: 
					if (otherObject->GetObjectType() == Points && otherObject->IsActive()) {
						points++;
						otherObject->isActive = false;
					}
					break;
			}
		}

		virtual void OnCollisionEnd(GameObject* otherObject) {
			//std::cout << "OnCollisionEnd event occured!\n";
		}

		bool GetBroadphaseAABB(Vector3&outsize) const;

		void UpdateBroadphaseAABB();

		void SetWorldID(int newID) {
			worldID = newID;
		}

		int		GetWorldID() const {
			return worldID;
		}

	protected:
		Transform			transform;

		CollisionVolume*	boundingVolume;
		PhysicsObject*		physicsObject;
		RenderObject*		renderObject;
		NetworkObject*		networkObject;

		ObjectType objType;

		void SetColour();

		float		points = 0;
		bool		isActive;
		int			worldID;
		std::string	name;

		Vector3 broadphaseAABB;
	};
}

