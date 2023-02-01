#pragma once
using namespace NCL::Maths;

namespace NCL {
	class CollisionVolume;
	
	namespace CSC8503 {
		class Transform;

		class PhysicsObject	{
		public:
			PhysicsObject(Transform* parentTransform, const CollisionVolume* parentVolume, float elastic = 0.66f, float lDamping = 0.4f, float aDamping = 0.4f);
			~PhysicsObject();

			Vector3 GetLinearVelocity() const {
				return linearVelocity;
			}

			Vector3 GetAngularVelocity() const {
				return angularVelocity;
			}

			Vector3 GetTorque() const {
				return torque;
			}

			Vector3 GetForce() const {
				return force;
			}

			void SetInverseMass(float invMass) {
				inverseMass = invMass;
			}

			float GetInverseMass() const {
				return inverseMass;
			}

			float GetLinearDamping() {
				return linearDamping;
			}

			void SetLinearDamping(float lDamping) {
				linearDamping = lDamping;
			}

			float GetAngularDamping() {
				return angularDamping;
			}

			void SetAngularDamping(float aDamping) {
				angularDamping = aDamping;
			}

			float GetCoefficientOfRestitution() {
				return elasticity;
			}

			void SetCoefficientOfRestitution(float e) {
				elasticity = e;
			}

			void ApplyAngularImpulse(const Vector3& force);
			void ApplyLinearImpulse(const Vector3& force);
			
			void AddForce(const Vector3& force);

			void AddForceAtPosition(const Vector3& force, const Vector3& position);

			void AddTorque(const Vector3& torque);


			void ClearForces();

			void SetLinearVelocity(const Vector3& v) {
				linearVelocity = v;
			}

			void SetAngularVelocity(const Vector3& v) {
				angularVelocity = v;
			}

			bool GetIsProjectile() {
				return isProjectile;
			}

			void SetIsProjectile(bool p) {
				isProjectile = p;
			}

			void InitCubeInertia();
			void InitSphereInertia(bool hollow = false);
			void InitCapsuleInertia();

			void UpdateInertiaTensor();

			Matrix3 GetInertiaTensor() const {
				return inverseInteriaTensor;
			}

		protected:
			const CollisionVolume* volume;
			Transform*		transform;

			bool isProjectile = false;

			float inverseMass;
			float elasticity;
			float friction;

			//linear stuff
			Vector3 linearVelocity;
			Vector3 force;

			float	linearDamping = 0.4f;
			float	angularDamping = 0.4f;
			
			//angular stuff
			Vector3 angularVelocity;
			Vector3 torque;
			Vector3 inverseInertia;
			Matrix3 inverseInteriaTensor;
		};
	}
}

