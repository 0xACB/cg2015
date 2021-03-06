#include "world.hpp"

World::World() {
	//window = _window;
}

World::~World(){}

void World::load(const char* path, GLdouble x, GLdouble y, GLdouble z, GLfloat opacity) {
	Object *obj = new Object();
	obj->path = path;
	obj->load();
	obj->type = "";
	objects.push_back(obj);

	obj->x = x;
	obj->y = y;
	obj->z = z;
	obj->opacity = opacity;
}

void World::loadSpheres() {
  srand(time(NULL));
  for (GLfloat x = -5; x < 5; x += 2) {
    for (GLfloat z = -5; z < 5; z += 2) {
      load(new Sphere(x * 6, 30, z * 6, 2, -1, -1, -1), "sphere");
    }
  }
}

void World::load(Object *obj, string type) {
	obj->load();
	obj->type = type;
	objects.push_back(obj);

	if (type == "sphere") {
		spheres.push_back((Sphere*) obj);
	}
}

void World::explode(GLfloat x, GLfloat y, GLfloat z) {
	for (int i = 0; i < 400; i++) {
		Particle *particle = new Particle(x, y, z, randomFloat(-0.1, 0.1), randomFloat(0, 0.12), randomFloat(-0.1, 0.1));
		explosion.push_back(particle);
	}
}

void World::update() {
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)) {
		if (canAddSphere) {
			Sphere *newSphere = new Sphere(camera.x, camera.y, camera.z, 1.0f, -1, -1, -1);

			float speedNormal = sqrt(pow(camera.lastCamMovementXComponent, 2) +
			 											   pow(camera.lastCamMovementYComponent, 2) +
															 pow(camera.lastCamMovementZComponent, 2));

			newSphere->vx = camera.lastCamMovementXComponent / speedNormal;
			newSphere->vy = camera.lastCamMovementYComponent / speedNormal;
			newSphere->vz = camera.lastCamMovementZComponent / speedNormal;

			load(newSphere, "sphere");
			canAddSphere = false;

			//explode(camera.x, camera.y, camera.z);
		 }
	} else {
		canAddSphere = true;
	}

	vector<int> particlesToRemove;
	for (int i = 0; i < (int) explosion.size(); i++) {
		explosion[i]->update();

		if (explosion[i]->color[3] <= 0.0f) {
			particlesToRemove.push_back(i);
		}
	}

	/* Clean up explosion particles */
	for (int i = (int) particlesToRemove.size() - 1; i >= 0; i--) {
		explosion.erase(explosion.begin() + particlesToRemove[i]);
	}

	for (int i = 0; i < (int) objects.size(); i++) {
		objects[i]->update();
	}

	vector<int> toRemove;
	vector<Sphere*> toAdd;
	vector<bool> canCollide(spheres.size(), true);

	for (int i = 0; i < (int) spheres.size(); i++) {
		for (int u = i + 1; u < (int) spheres.size(); u++) {
			//printf("comparing balls %d and %d\n", i, u);

			if (spheres[i]->isColliding(spheres[u]) && canCollide[i] && canCollide[u]) {
				//printf("colliding balls %d and %d\n", i, u);

				GLfloat newColor[4] = {(spheres[i]->color[0] + spheres[i]->color[0]) / 2,
					(spheres[i]->color[1] + spheres[i]->color[1]) / 2,
					(spheres[i]->color[2] + spheres[i]->color[2]) / 2,
					1.0f};

				toRemove.push_back(i);
				toRemove.push_back(u);

				GLfloat newX = (spheres[i]->x + spheres[u]->x) / 2;
				GLfloat newY = (spheres[i]->y + spheres[u]->y) / 2;
				GLfloat newZ = (spheres[i]->z + spheres[u]->z) / 2;

				Sphere *newSphere = new Sphere(newX, newY, newZ, (spheres[i]->radius + spheres[u]->radius) / 2 * 1.2, -1, -1, -1);
				newSphere->vx = spheres[i]->vx + spheres[u]->vx;
				newSphere->vy = spheres[i]->vy + spheres[u]->vy;
				newSphere->vz = spheres[i]->vz + spheres[u]->vz;

				playBubbleSound(newX, newY, newZ);

				toAdd.push_back(newSphere);
				canCollide[i] = false;
				canCollide[u] = false;
				memcpy(newSphere->color, newColor, sizeof(newColor));
				explode(newX, newY + 3.0f, newZ);
			}
		}

		if (fabs(spheres[i]->x) >= cubeSide ||
		 	  spheres[i]->y >= cubeHeight ||
				spheres[i]->y <= sea.seaLevel ||
				fabs(spheres[i]->z) >= cubeSide) {
			spheres[i]->vx = -spheres[i]->vx;
			spheres[i]->vy = -spheres[i]->vy;
			spheres[i]->vz = -spheres[i]->vz;
		}
	}

	sort(toAdd.begin(), toAdd.end());
	toAdd.erase(unique(toAdd.begin(), toAdd.end()), toAdd.end());

	for (int i = toRemove.size() - 1; i >= 0; i--) {
		spheres.erase(spheres.begin() + toRemove[i]);

		for (int u = (int) objects.size() - 1; u >= 0; u--) {
			//printf("u = %d, objects.size = %d; i = %d, toRemove.size = %d\n", u, (int) objects.size(), i, (int) toRemove.size());

			if (spheres[toRemove[i]] == objects[u]) {
				objects.erase(objects.begin() + u);
			}
		}
	}

	for (int i = 0; i < (int) toAdd.size(); i++) {
		load(toAdd[i], "sphere");
	}
}

void World::render() {
	for (int i = 0; i < (signed) objects.size(); i++) {
		if (objects[i]->type != "sphere") {
			glPushMatrix();
			glTranslatef(objects[i]->x, objects[i]->y, objects[i]->z);
			objects[i]->render();
			glPopMatrix();
		}
	}

	for (int i = 0; i < (int) spheres.size(); i++) {
		glPushMatrix();
		glTranslatef(spheres[i]->x, spheres[i]->y, spheres[i]->z);
		spheres[i]->render();
		glPopMatrix();
	}

	for (int i = 0; i < (int) explosion.size(); i++) {
		glPushMatrix();
		glTranslatef(explosion[i]->x, explosion[i]->y, explosion[i]->z);
		explosion[i]->render();
		glPopMatrix();
	}
}

void World::loadMusic() {
  if (backgroundMusic.openFromFile("sound/background-music.ogg")) {
    backgroundMusic.play();
    backgroundMusic.setLoop(true);
  }

  bubbleSound.openFromFile("sound/bubble.ogg");
}

int clampVolume(float volume) {
	if (volume > 100) {
		return 100;
	}

	if (volume < 0) {
		return 20;
	}

	return (int)volume;
}

void World::playBubbleSound(GLfloat x, GLfloat y, GLfloat z) {
	float distance = sqrt(pow(x-camera.x, 2)+pow(y-camera.y, 2)+pow(z-camera.z, 2));
	float pitch = randomFloat(1.0, 2.0);
	bubbleSound.setPitch(pitch);
	bubbleSound.setVolume(clampVolume(100-distance));
  bubbleSound.play();
}
