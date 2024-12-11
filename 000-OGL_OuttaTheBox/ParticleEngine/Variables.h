#pragma once

#include<Windows.h>
#include<stdio.h>
#include<gl\glew.h>
#include<gl/GL.h>
#include "vmath.h"

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"User32.lib")
#pragma comment(lib,"Gdi32.lib")
#pragma comment(lib,"Kernel32.lib")

enum
{
	PARTICLE_GROUP_SIZE = 1024,
	PARTICLE_GROUP_COUNT = 8192,
	PARTICLE_COUNT = (PARTICLE_GROUP_SIZE * PARTICLE_GROUP_COUNT),
	MAX_ATTRACTORS = 64
};

GLint   dt_location;

// Posisition and velocity buffers
static union
{
	struct
	{
		GLuint position_buffer;
		GLuint velocity_buffer;
	};
	GLuint buffers[2];
};

// TBOs
static union
{
	struct
	{
		GLuint position_tbo;
		GLuint velocity_tbo;
	};
	GLuint tbos[2];
};

// Attractor UBO
GLuint  attractor_buffer;

// Mass of the attractors
float attractor_masses[MAX_ATTRACTORS];

LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
LARGE_INTEGER Frequency;

