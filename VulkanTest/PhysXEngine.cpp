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
	
	PxTransform transform(PxVec3(0, 1.5, 6));

	player = physics->createRigidDynamic(transform);

	PxShape* playerShape = physics->createShape(PxCapsuleGeometry(0.5f, 1.0f), *material);

	PxFilterData filterData;
	filterData.word0 = CollisionGroups::PLAYER;
	filterData.word1 = CollisionGroups::WORLD;
	playerShape->setSimulationFilterData(filterData);

	player->attachShape(*playerShape);
	PxRigidBodyExt::updateMassAndInertia(*player, 10.0f);

	scene->addActor(*player);

	playerShape->release();

	// | house collider

	//PxTransform houseTransform;

	//house = physics->createRigidStatic(houseTransform);

	//PxShape* houseShape = physics->createShape();

	//PxFilterData houseFilterData;
	//houseFilterData.word0 = CollisionGroups::PLAYER;
	//houseFilterData.word1 = 0;
	//houseShape->setSimulationFilterData(houseFilterData);

	//house->attachShape(*houseShape);

	//// | impassable object collider
	//glm::mat4& model = classReference.modelMatrix().model;
	//glm::vec3 pos = glm::vec3(model[3]);
	//PxMaterial* computerMaterial = physics->createMaterial(
	//	0.5f, // static friction
	//	0.5f, // dynamic friction
	//	0.5f  // bounciness
	//);

	//PxBoxGeometry computerGeom(0.5f, 0.5f, 0.5f);

	//PxTransform computerTransform(PxVec3(pos.x, pos.y, pos.z);

	//computer = physics->createRigidStatic(computerTransform);

	//PxShape* computerShape = physics->createShape(computerGeom, *computerMaterial);

	//PxFilterData computerFilterData;
	//computerFilterData.word0 = CollisionGroups::WORLD;
	//computerFilterData.word1 = CollisionGroups::PLAYER;
	//computerShape->setSimulationFilterData(computerFilterData);

	//computer->attachShape(*computerShape);
	//computerShape->release();

	//scene->addActor(*computer);
}

void PhysXEngine::stepPhysics(float deltaTime) {

	if (deltaTime > 0) {
		// | cap at ~30fps
		scene->simulate(std::min(deltaTime, 0.033f));

		scene->fetchResults(true);
		
	}
}

void PhysXEngine::processKeyboard(glm::vec3 cameraOrientation) {

	PxVec3 vel = player->getLinearVelocity();

	if (glm::length(cameraOrientation) > 0.0f) {
		cameraOrientation.y = 0;
		cameraOrientation = glm::normalize(cameraOrientation);

		float speed = 5.0f;

		vel.x = cameraOrientation.x * speed;
		vel.z = cameraOrientation.z * speed;
	}
	else {
		vel.x = 0;
		vel.z = 0;
	}

	player->setLinearVelocity(vel);
}

// | camera
void PhysXEngine::readTransforms(glm::vec3& cameraPosition) {

	PxTransform pose = player->getGlobalPose();
	PxVec3 position = pose.p;
	PxQuat rotation = pose.q;

	glm::vec3 pos(
		pose.p.x,
		pose.p.y,
		pose.p.z
	);

	// | offset for eye-level height
	cameraPosition = pos + glm::vec3(0, 0.8f, 0);;

	glm::quat rot(
		pose.q.w,
		pose.q.x,
		pose.q.y,
		pose.q.z
	);


	//glm::mat4 view =
	//	glm::inverse(
	//		glm::translate(glm::mat4(1.0f), pos) * glm::mat4_cast(rot));
}


void PhysXEngine::boxCollider(ItemInterface& classReference) {
	std::cout << "floating\n";
	// | impassable object collider
	glm::mat4& model = classReference.modelMatrix().model;
	glm::vec3 pos = glm::vec3(model[3]);

	// | scale extraction
	glm::vec3 scale;
	scale.x = glm::length(glm::vec3(model[0]));
	scale.y = glm::length(glm::vec3(model[1]));
	scale.z = glm::length(glm::vec3(model[2]));

	material = physics->createMaterial(
		0.5f, // static friction
		0.5f, // dynamic friction
		0.5f  // bounciness
	);

	std::cout << scale.x << "\n";
	std::cout << scale.y << "\n";
	std::cout << scale.z << "\n";

	PxBoxGeometry geom(0.5f, 0.5f, 0.5f);

	// | rotation extraction
	glm::quat rot = glm::quat_cast(model);
	rot = glm::normalize(rot);
	PxQuat q(rot.x, rot.y, rot.z, rot.w);
	PxTransform transform(PxVec3(pos.x, pos.y, pos.z), q);

	//PxTransform transform(PxVec3(pos.x, pos.y, pos.z));

	auto rigid = classReference.collisionBody();
	rigid = physics->createRigidStatic(transform);

	PxShape* rigidShape = physics->createShape(geom, *material);

	PxFilterData filterData;
	filterData.word0 = CollisionGroups::WORLD;
	filterData.word1 = CollisionGroups::PLAYER;
	rigidShape->setSimulationFilterData(filterData);

	rigid->attachShape(*rigidShape);
	rigidShape->release();

	scene->addActor(*rigid);

}


PhysXEngine::~PhysXEngine() {
	scene->release();
	dispatcher->release();
	physics->release();
	foundation->release();
}