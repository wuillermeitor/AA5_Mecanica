#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <glm\common.hpp>
#include <math.h>

bool show_test_window = false;
const int widthParticles = 14;
const int depthParticles = 18;

#define NPARTICLES 252

using v3 = glm::vec3;
struct particle {
	v3 pos;
	v3 vel;
};

float arrayParticles[NPARTICLES * 3];
particle arrayStructParticles[NPARTICLES];

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

void initialDarticles() {
	float localZ = -5.5;
	float XPosition = -3.5;
	float YPosition = 9.5;
	float ZPosition = -2.5;

	for (int i = 0; i < NPARTICLES; ++i) {
		for (int j = 0; j < 3; ++j) {
			switch (j) {
			case 0:
				arrayParticles[i * 3 + j] = XPosition + (i % 14);
				arrayStructParticles[i].pos.x = XPosition + (i % 14);
				if (i % 14 == 0)
					++localZ;
				break;
			case 1:
				arrayParticles[i * 3 + j] = YPosition;
				arrayStructParticles[i].pos.y = YPosition;
				break;
			case 2:
				arrayParticles[i * 3 + j] = ZPosition + (localZ);
				arrayStructParticles[i].pos.z = ZPosition + (localZ);
				break;
			}
		}
	}
}

void PhysicsInit() {
	// Do your initialization code here...
	// ...................................
	
	ClothMesh::setupClothMesh();
	initialDarticles();
}

void PhysicsUpdate(float dt) {
	ClothMesh::updateClothMesh(arrayParticles);
	ClothMesh::drawClothMesh();
}

void PhysicsCleanup() {
	// Do your cleanup code here...
	// ............................
	ClothMesh::cleanupClothMesh();
}