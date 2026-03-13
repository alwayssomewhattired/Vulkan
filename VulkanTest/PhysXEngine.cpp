#include "PhysXEngine.h"

PhysXEngine::PhysXEngine() {

	foundation = PxCreateFoundation(PX_PHYSICS_VERSION, allocator, errorCallback);
	physics = PxCreatePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale());

	// | scene creation

	PxSceneDesc sceneDesc(physics->getTolerancesScale());
	sceneDesc.gravity = PxVec3(0.0f, -9.81f, 0.0f);

	dispatcher = PxDefaultCpuDispatcherCreate(std::thread::hardware_concurrency());

	sceneDesc.cpuDispatcher = dispatcher;
	sceneDesc.filterShader = PxDefaultSimulationFilterShader;

	scene = physics->createScene(sceneDesc);


	// | collision object creation

	PxMaterial* material = physics->createMaterial(0.5f, 0.5f, 0.6f);

	PxRigidStatic* ground = PxCreatePlane(*physics, PxPlane(0, 1, 0, 0), *material);

	scene->addActor(*ground);

	// | player collider
	
	PxTransform transform(PxVec3(0, 5, 0));

	PxRigidDynamic* player = physics->createRigidDynamic(transform);

	PxShape* shape = physics->createShape(PxCapsuleGeometry(0.5f, 1.0f), *material);

	player->attachShape(*shape);

	scene->addActor(*player);
}

void PhysXEngine::stepPhysics(float deltaTime) {

	if (deltaTime > 0) {
		// | cap at ~30fps
		scene->simulate(std::min(deltaTime, 0.033f));

		scene->fetchResults(true);
	}
}