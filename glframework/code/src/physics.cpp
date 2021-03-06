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
float liquidDensity = 0.997;
#define NPARTICLES 252
#define PI 3.141592654


using v3 = glm::vec3;
struct particle {
	v3 pos;
	v3 vel;
};

const v3 GRAVITY(0, -9.81, 0);

float arrayParticles[NPARTICLES * 3];
particle arrayStructParticles[widthParticles][depthParticles];
particle initialData[widthParticles][depthParticles];

namespace ClothMesh {
	extern void setupClothMesh();
	extern void cleanupClothMesh();
	extern void updateClothMesh(float* array_data);
	extern void drawClothMesh();
}

namespace Sphere {
	extern void setupSphere(glm::vec3 pos = glm::vec3(0.f, 1.f, 0.f), float radius = 1.f);
	extern void cleanupSphere();
	extern void updateSphere(glm::vec3 pos, float radius = 1.f);
	extern void drawSphere();
	v3 pos;
	float rad;
	v3 vel;
	v3 acc;
	float mass;
	float density;
	float Cd = 0.47;
}

void resetSphere();

void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);



	// Do your GUI code here....
	{	
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);//FrameRate
		
	}
	// .........................


	ImGui::SliderFloat("Sphere Mass", &Sphere::mass, 0.01, 10);
	if (ImGui::Button("Reset")) {
		resetSphere();
	}
	
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
	Sphere::pos = { 0, 7, 0 };
	Sphere::rad = 1;
	Sphere::vel = {0, 0, 0};
	Sphere::acc = GRAVITY;
	Sphere::mass = 2;
	Sphere::density = Sphere::mass/(4.f / 3.f * PI * glm::pow(Sphere::rad, 3));


	ClothMesh::setupClothMesh();
	Sphere::setupSphere(Sphere::pos, Sphere::rad);
	initialDarticles();
}

float getRandomFloatInRange(int min, int max) {
	return min + rand() % (max - min);
}

static float currentTime = 0;

void resetSphere() {
	Sphere::pos = {getRandomFloatInRange(-3, 3), getRandomFloatInRange(6, 8), getRandomFloatInRange(-3, 3)};
	Sphere::vel = { 0, 0, 0 };
	Sphere::acc = GRAVITY;
	currentTime = 0;
}


void addForce(v3 force) {
	Sphere::acc += force/Sphere::mass;
}

void dragForce(std::pair<v3, float> part) {
	float aux = glm::clamp(part.second, 0.f, Sphere::rad);
	float areaC= PI*glm::pow(Sphere::rad* (1-(aux / Sphere::rad)), 2);
	if (part.first.y > Sphere::pos.y) {
		areaC = PI*glm::pow(Sphere::rad, 2);
	}

	float Fdrag = -0.5f*Sphere::density*Sphere::Cd*glm::abs(areaC) * glm::abs(Sphere::vel.y)*Sphere::vel.y;
	addForce(v3{0, Fdrag, 0});
}

void buoyancy(std::pair<v3, float> part) {
	if (part.second < Sphere::rad || Sphere::pos.y - Sphere::rad < part.first.y) {
		float distLowSphere = glm::distance(Sphere::pos - v3{ 0, Sphere::rad, 0 }, part.first);
		float volSub;
		if (distLowSphere > Sphere::rad * 2)
			volSub = (4.f / 3.f)*PI*glm::pow(Sphere::rad, 3);
		else
			volSub = ((PI*glm::pow(distLowSphere, 2)) / 3.f)*(3.f * Sphere::rad - distLowSphere);
		
		float fx = (liquidDensity*glm::abs(GRAVITY.x)*volSub);
		float fy = (liquidDensity*glm::abs(GRAVITY.y)*volSub);
		float fz = (liquidDensity*glm::abs(GRAVITY.z)*volSub);

		addForce(v3{fx, fy, fz});
	}
}

void PhysicsUpdate(float dt) {
	

	//UPDATE DE LA OLA
	struct wave {
		float lambda = 5;
		float kWave = 2 * PI / lambda;
		float A = 0.25; //amplitud de la ona
		v3 K = { 1, 0, 0.4 }; //direcci� de la ona
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

	//COLISI�N ESFERA CON OLA
	std::pair<v3, float> shortestD = std::make_pair(v3{-1, -1, -1}, INT_MAX);
	for (int i = 0; i < NPARTICLES; ++i) {
		float aux=glm::distance(Sphere::pos, arrayStructParticles[i%widthParticles][i / widthParticles].pos);
		if (aux < shortestD.second) {
			shortestD.first = arrayStructParticles[i%widthParticles][i / widthParticles].pos;
			shortestD.second = aux;
		}
	}

	buoyancy(shortestD);
	dragForce(shortestD);
	
	//MOVIMIENTO ESFERA 
	Sphere::pos += dt*Sphere::vel;
	Sphere::vel += dt*Sphere::acc;

	//DRAW ESFERA Y CLOTH
	ClothMesh::drawClothMesh();
	Sphere::updateSphere(Sphere::pos, Sphere::rad);
	Sphere::drawSphere();

	Sphere::acc = GRAVITY;

	currentTime += dt;
	if (currentTime >= 15) {
		resetSphere();
	}

}

void PhysicsCleanup() {
	// Do your cleanup code here...
	// ............................
	ClothMesh::cleanupClothMesh();
}