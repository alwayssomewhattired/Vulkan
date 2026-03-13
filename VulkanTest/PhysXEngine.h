#pragma once

#include "Camera.h"
#include "items/ItemInterface.h"

#include "PxPhysicsAPI.h"
#include "glm_config.h"

#include <thread>
#include <algorithm>

using namespace physx;

class PhysXEngine {
public:

	PhysXEngine();
	~PhysXEngine();

	void stepPhysics(float deltatTime);
	void readTransforms(glm::vec3& position);
	void processKeyboard(glm::vec3 cameraOrientation);
	void boxCollider(ItemInterface& classReference);


private:

	PxDefaultAllocator allocator;
	PxDefaultErrorCallback errorCallback;

	PxFoundation* foundation = nullptr;
	PxPhysics* physics = nullptr;
	PxScene* scene;

	PxRigidDynamic* player;

	PxDefaultCpuDispatcher* dispatcher = nullptr;

	struct CollisionGroups
	{
		static const PxU32 PLAYER = (1 << 0);
		static const PxU32 WORLD = (1 << 1);
		static const PxU32 VISUAL = (1 << 2);
	};

};