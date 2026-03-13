#pragma once
#include "PxPhysicsAPI.h"

#include <thread>
#include <algorithm>

using namespace physx;

class PhysXEngine {
public:

	PhysXEngine();

	void stepPhysics(float deltatTime);


private:

	PxDefaultAllocator allocator;
	PxDefaultErrorCallback errorCallback;

	PxFoundation* foundation = nullptr;
	PxPhysics* physics = nullptr;
	PxScene* scene;

	PxDefaultCpuDispatcher* dispatcher = nullptr;

};