#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm\common.hpp>
#include <glm\geometric.hpp>
#include <math.h>
#include <iostream>
#include <deque>

bool show_test_window = false;
const int widthParticles = 14;
const int depthParticles = 18;

#define NPARTICLES 252
#define PI 3.141592654

using v3 = glm::vec3;
struct particle {
	v3 pos;
	v3 vel;
};

float arrayParticles[NPARTICLES * 3];
particle arrayStructParticles[widthParticles][depthParticles];
particle initialData[widthParticles][depthParticles];

namespace ClothMesh {
	extern void setupClothMesh();
	extern void cleanupClothMesh();
	extern void updateClothMesh(float* array_data);
	extern void drawClothMesh();
}

void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);

	// Do your GUI code here....
	{	
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate
		
	}
	// .........................
	
	ImGui::End();

	// Example code -- ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	if(show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}

void arrayStructToArray() {
	for (int i = 0; i < NPARTICLES; ++i) {
		for (int j = 0; j < 3; ++j) {
			switch (j) {
			case 0:
				arrayParticles[i * 3 + j] = arrayStructParticles[i%widthParticles][i / widthParticles].pos.x;
				break;
			case 1:
				arrayParticles[i * 3 + j] = arrayStructParticles[i%widthParticles][i / widthParticles].pos.y;
				break;
			case 2:
				arrayParticles[i * 3 + j] = arrayStructParticles[i%widthParticles][i / widthParticles].pos.z;
				break;
			}
		}
	}
}

void initialDarticles() {
	float localZ = -1;
	float XPosition = -5;
	float YPosition = 3;
	float ZPosition = -5;
	float gridExtentX = 10.f/static_cast<float>(widthParticles-1);
	float gridExtentZ = 10.f/static_cast<float>(depthParticles-1);

	for (int i = 0; i < NPARTICLES; ++i) {
		for (int j = 0; j < 3; ++j) {
			switch (j) {
			case 0:
				initialData[i%widthParticles][i/widthParticles].pos.x=arrayStructParticles[i%widthParticles][i/widthParticles].pos.x = XPosition + (i % widthParticles)*gridExtentX;
				if (i % 14 == 0)
					++localZ;
				break;
			case 1:
				initialData[i%widthParticles][i / widthParticles].pos.y = arrayStructParticles[i%widthParticles][i / widthParticles].pos.y = YPosition;
				break;
			case 2:
				initialData[i%widthParticles][i / widthParticles].pos.z = arrayStructParticles[i%widthParticles][i / widthParticles].pos.z = ZPosition + (localZ)*gridExtentZ;
				break;
			}
		}
	}

	arrayStructToArray();
}

void PhysicsInit() {
	// Do your initialization code here...
	// ...................................
	
	ClothMesh::setupClothMesh();
	initialDarticles();
}

static float currentTime = 0;
void PhysicsUpdate(float dt) {
	currentTime += dt;

	struct wave {
		float lambda = 5;
		float kWave = 2 * PI / lambda;
		float A = 0.25; //amplitud de la ona
		v3 K = { 1, 0, 0.4 }; //direcció de la ona
		float w = 4; //velocitat de la ona
		float phi = 0;
	};

	std::deque<wave> myWaveDeque;
	wave one;
	wave two;
	two.K = {-0.4, 0, 1};
	myWaveDeque.push_back(one);
	myWaveDeque.push_back(two);

	for (int i = 0; i < NPARTICLES; ++i) {
		arrayStructParticles[i%widthParticles][i / widthParticles].pos = initialData[i%widthParticles][i / widthParticles].pos;
		v3 xSum(0);
		float ySum=0;
		for (int j = 0; j < myWaveDeque.size(); ++j) {
			xSum+= (myWaveDeque.at(j).K / myWaveDeque.at(j).kWave)*myWaveDeque.at(j).A*
				sin(glm::dot(myWaveDeque.at(j).K, initialData[i%widthParticles][i / widthParticles].pos) - myWaveDeque.at(j).w * currentTime + myWaveDeque.at(j).phi);

			ySum+= myWaveDeque.at(j).A *
				cos(glm::dot(myWaveDeque.at(j).K, initialData[i%widthParticles][i / widthParticles].pos) - myWaveDeque.at(j).w * currentTime + myWaveDeque.at(j).phi);
			
		}
		//x
		arrayStructParticles[i%widthParticles][i / widthParticles].pos = initialData[i%widthParticles][i / widthParticles].pos - xSum;

		//y
		arrayStructParticles[i%widthParticles][i / widthParticles].pos.y += ySum;
	}

	arrayStructToArray();
	ClothMesh::updateClothMesh(arrayParticles);
	ClothMesh::drawClothMesh();
}

void PhysicsCleanup() {
	// Do your cleanup code here...
	// ............................
	ClothMesh::cleanupClothMesh();
}