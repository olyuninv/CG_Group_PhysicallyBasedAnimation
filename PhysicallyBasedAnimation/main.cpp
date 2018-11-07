#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp> 
#include <glm/gtc/constants.hpp> 

#include "ParticleSimulation.h"

const int MAX_NUM_PARTICLES = 10000;
const int NUM_COLORS = 5;

particle particles[MAX_NUM_PARTICLES];
float speed = 20.0;

int main(int argc, char** argv) {
	
	int num_particles = 1000;

	for (int i=0; i<num_particles; i++)
	{
		particles[i].mass = 1.0;
		particles[i].color = i%NUM_COLORS;
		for (int j = 0; j < 3; j++)
		{
			particles[i].position[j] = 2.0 * ((float)rand() / RAND_MAX) - 1.0;
			particles[i].velocity[j] = speed * 2.0 * ((float)rand() / RAND_MAX) - 1.0;
		}
		particles[i].position[3] = 1.0;
		particles[i].velocity[3] = 0.0;
	}
}











