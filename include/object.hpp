#ifndef OBJECT_HPP
#define OBJECT_HPP

#ifdef __APPLE__
	#include <OpenGL/gl.h>
#else
	#include <GL/glew.h>
#endif

#include <stdio.h>
#include <string.h>
#include <string>
#include <vector>

#include "structs.hpp"
#include "texture.hpp"

#define MAXLEN 100
#define TEXTURE_PATH "obj/textures/"
#define TEXTURE_EXTENSION ".bmp"
#define SLASH "/"

using namespace std;

class Object {
	public:
		Object();
		~Object();
		string type;
		const char* path;
		vector <point> verts;
		vector <point> normals;
		vector <point> uvs;
		vector <face> faces;
		GLuint texture;
		bool load();
		bool uvEnabled;
		virtual void update();
		virtual void render();
		GLdouble x, y, z;
		GLfloat opacity = 1.0;
};

#endif
