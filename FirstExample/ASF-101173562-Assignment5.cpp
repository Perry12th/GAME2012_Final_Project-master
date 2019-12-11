//*************************************************************************** 
// ASF-101173562-Assignment5.cpp by Flavio Jorge Araujo Silva (C) 2018 All Rights Reserved. 
// 
// Assignment 5 submission. 
// 
// Description: 
//   Click run to see the results. 
//***************************************************************************** 

using namespace std;

#include <iostream>
#include "stdlib.h"
#include "time.h"
#include "vgl.h"
#include "LoadShaders.h"
#include "glm\glm.hpp"
#include "glm\gtc\matrix_transform.hpp"
#include "SoilLib/SOIL.h"
#include "GeometryGenerator.h"

#define X_AXIS glm::vec3(1,0,0)
#define Y_AXIS glm::vec3(0,1,0)
#define Z_AXIS glm::vec3(0,0,1)
#define XY_AXIS glm::vec3(1,1,0)
#define YZ_AXIS glm::vec3(0,1,1)
#define XZ_AXIS glm::vec3(1,0,1)

// Lighting Strucs

// Light variables
struct Light
{
	glm::vec3 ambientColour;
	GLfloat ambientStrength;
	glm::vec3 diffuseColour;
	GLfloat diffuseStrength;
	Light(glm::vec3 aCol, GLfloat aStr, glm::vec3 dCol, GLfloat dStr)
	{
		ambientColour = aCol;
		ambientStrength = aStr;
		diffuseColour = dCol;
		diffuseStrength = dStr;
	}
};

struct DirectionalLight : public Light
{
	glm::vec3 direction;
	DirectionalLight(glm::vec3 dir, glm::vec3 aCol, GLfloat aStr, glm::vec3 dCol, GLfloat dStr)
		: Light(aCol, aStr, dCol, dStr)
	{
		direction = dir;
	}
};

struct PointLight : public Light
{
	glm::vec3 position;
	GLfloat falloffStart, falloffEnd;
	PointLight(glm::vec3 pos, GLfloat fallS, GLfloat fallE,
		glm::vec3 aCol, GLfloat aStr, glm::vec3 dCol, GLfloat dStr)
		: Light(aCol, aStr, dCol, dStr)
	{
		position = pos;
		falloffStart = fallS;
		falloffEnd = fallE;
	}
};

struct Material
{
	GLfloat speculatStrength;
	GLfloat shininess;
};

// IDs
GLuint program, vao, ibo, points_vbo, colours_vbo, tex_vbo, normal_vbo, modelID, projID, viewID;
GLuint leather_tex, cone_tex, cylinder_tex, diamond_tex, disc_tex, hexagon_tex, plane_tex, pyramid_tex;
float xpos = 0.0f, ypos = 0.0f, zpos = 10.0f, scrollSpd = 1.0f;

// Camera and transform variables.
glm::vec3 position, frontVec, worldUp, upVec, rightVec; // Set by function.
GLfloat pitch, yaw,
moveSpeed = 0.1f,
turnSpeed = 1.0f,
rotAngle = 0.0f, 
rotAngle2 = 0.0f;

// Mouse variables.
bool mouseFirst = true, mouseClicked = false;
int lastX, lastY;

 // Light variables.
PointLight pLight(glm::vec3(1.0f, 1.0f, 1.0f),			   // Position 
						1.0f, 10.0f,				       // Falloff start and end range
						glm::vec3(1.0f, 1.0f, 1.0f),	   // Ambient colour
						0.2f,							   // Ambient strength
						glm::vec3(1.0f, 1.0f, 1.0f),	   // Diffuse colour
						1.0f);							   // Diffuse strength

PointLight pCrystalLight(glm::vec3(0.0f, 2.0f, -0.8f),			   // Position 
								1.0f, 10.0f,				       // Falloff start and end range
								glm::vec3(1.0f, 1.0f, 1.0f),	   // Ambient colour
								0.2f,							   // Ambient strength
								glm::vec3(1.0f, 1.0f, 1.0f),	   // Diffuse colour
								1.0f);							   // Diffuse strength

PointLight pCrystalBackLight(glm::vec3(0.0f, 3.0f, -1.5f),			   // Position 
									1.0f, 10.0f,				       // Falloff start and end range
									glm::vec3(1.0f, 1.0f, 1.0f),	   // Ambient colour
									0.2f,							   // Ambient strength
									glm::vec3(1.0f, 1.0f, 1.0f),	   // Diffuse colour
									1.0f);							   // Diffuse strength

Material mat = { 5.0f, 32 }; // Alternate way to construct an object
// Matrices.
glm::mat4 model, view, projection;

// Testing v3
GeometryGenrator g;
//---------------------------------------------------------------------
//
// resetView
//
void resetView()
{
	position = glm::vec3(0.0f, 0.5f, 5.0f);
	frontVec = glm::vec3(0.0f, 0.0f, -1.0f);
	worldUp = glm::vec3(0, 1, 0);
	pitch = -10.0f;
	yaw = -90.0f;
}

void bindTexture(GLuint &text_id, GLint tex_num, GLint width, GLint height, unsigned char* image)
{
	text_id = 0;
	glGenTextures(1, &text_id);
	glBindTexture(GL_TEXTURE_2D, text_id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glUniform1i(glGetUniformLocation(program, "texture0"), tex_num);

}

void init(void)
{
	//Specifying the name of vertex and fragment shaders.
	ShaderInfo shaders[] = {
		{ GL_VERTEX_SHADER, "triangles.vert" },
		{ GL_FRAGMENT_SHADER, "triangles.frag" },
		{ GL_NONE, NULL }
	};
	
	//Loading and compiling shaders
	program = LoadShaders(shaders);
	glUseProgram(program);	//My Pipeline is set up
	resetView();


	modelID = glGetUniformLocation(program, "model");
	projID = glGetUniformLocation(program, "projection");
	viewID = glGetUniformLocation(program, "view");

	// Setting material values.
	glUniform1f(glGetUniformLocation(program, "specularStrength"), mat.speculatStrength);
	glUniform1f(glGetUniformLocation(program, "shininess"), mat.shininess);

	//// Setting point light
	glUniform3f(glGetUniformLocation(program, "ambientColour"), pLight.ambientColour.x, pLight.ambientColour.y, pLight.ambientColour.z);
	glUniform1f(glGetUniformLocation(program, "ambientStrength"), pLight.ambientStrength);
	glUniform3f(glGetUniformLocation(program, "diffuseColour"), pLight.diffuseColour.x, pLight.diffuseColour.y, pLight.diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "diffuseStrength"), pLight.diffuseStrength);

	glUniform3f(glGetUniformLocation(program, "lightPosition"), pLight.position.x, pLight.position.y, pLight.position.z);
	glUniform1f(glGetUniformLocation(program, "falloffStart"), pLight.falloffStart);
	glUniform1f(glGetUniformLocation(program, "falloffEnd"), pLight.falloffEnd);

	//Setting up light inside the crystal
	glUniform3f(glGetUniformLocation(program, "ambientColour"), pCrystalLight.ambientColour.x, pCrystalLight.ambientColour.y, pCrystalLight.ambientColour.z);
	glUniform1f(glGetUniformLocation(program, "ambientStrength"), pCrystalLight.ambientStrength);
	glUniform3f(glGetUniformLocation(program, "diffuseColour"), pCrystalLight.diffuseColour.x, pCrystalLight.diffuseColour.y, pCrystalLight.diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "diffuseStrength"), pCrystalLight.diffuseStrength);

	glUniform3f(glGetUniformLocation(program, "lightPosition"), pCrystalLight.position.x, pCrystalLight.position.y, pCrystalLight.position.z);
	glUniform1f(glGetUniformLocation(program, "falloffStart"), pCrystalLight.falloffStart);
	glUniform1f(glGetUniformLocation(program, "falloffEnd"), pCrystalLight.falloffEnd);


	glUniform3f(glGetUniformLocation(program, "ambientColour"), pCrystalBackLight.ambientColour.x, pCrystalBackLight.ambientColour.y, pCrystalBackLight.ambientColour.z);
	glUniform1f(glGetUniformLocation(program, "ambientStrength"), pCrystalBackLight.ambientStrength);
	glUniform3f(glGetUniformLocation(program, "diffuseColour"), pCrystalBackLight.diffuseColour.x, pCrystalBackLight.diffuseColour.y, pCrystalBackLight.diffuseColour.z);
	glUniform1f(glGetUniformLocation(program, "diffuseStrength"), pCrystalBackLight.diffuseStrength);

	glUniform3f(glGetUniformLocation(program, "lightPosition"), pCrystalBackLight.position.x, pCrystalBackLight.position.y, pCrystalBackLight.position.z);
	glUniform1f(glGetUniformLocation(program, "falloffStart"), pCrystalBackLight.falloffStart);
	glUniform1f(glGetUniformLocation(program, "falloffEnd"), pCrystalBackLight.falloffEnd);

	projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	
	// Camera is set up in the display function
		
		GLint width, height;
		unsigned char* image = SOIL_load_image("Leather.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
		if (!image) { cout << "Image failed to load" << endl; }
		glActiveTexture(GL_TEXTURE0);
		bindTexture(leather_tex, 0, width, height, image);

		image = SOIL_load_image("Cone.png", &width, &height, 0, SOIL_LOAD_RGBA);
		if (!image) { cout << "Image failed to load" << endl; }
		glActiveTexture(GL_TEXTURE1);
		bindTexture(cone_tex, 1, width, height, image);

		image = SOIL_load_image("Cylinder.png", &width, &height, 0, SOIL_LOAD_RGBA);
		if (!image) { cout << "Image failed to load" << endl; }
		glActiveTexture(GL_TEXTURE2);
		bindTexture(cylinder_tex, 2, width, height, image);

		image = SOIL_load_image("Diamond.png", &width, &height, 0, SOIL_LOAD_RGBA);
		if (!image) { cout << "Image failed to load" << endl; }
		glActiveTexture(GL_TEXTURE3);
		bindTexture(diamond_tex, 3, width, height, image);

		image = SOIL_load_image("Disc.png", &width, &height, 0, SOIL_LOAD_RGBA);
		if (!image){ cout << "Image failed to load" << endl; }
		glActiveTexture(GL_TEXTURE4);
		bindTexture(disc_tex, 4, width, height, image);

		image = SOIL_load_image("Hexagon.png", &width, &height, 0, SOIL_LOAD_RGBA);
		if (!image) { cout << "Image failed to load" << endl; }
		glActiveTexture(GL_TEXTURE5);
		bindTexture(hexagon_tex, 5, width, height, image);

		image = SOIL_load_image("Plane.png", &width, &height, 0, SOIL_LOAD_RGBA);
		if (!image) { cout << "Image failed to load" << endl; }
		glActiveTexture(GL_TEXTURE6);
		bindTexture(plane_tex, 6, width, height, image);

		image = SOIL_load_image("Pyramid.png", &width, &height, 0, SOIL_LOAD_RGBA);
		if (!image) { cout << "Image failed to load" << endl; }
		glActiveTexture(GL_TEXTURE7);
		bindTexture(pyramid_tex, 7, width, height, image);


	g.GenrateShape(Cube, glm::vec3(0.5f, 0.5f, 0.0f));
	g.GenrateShape(Plane, glm::vec3(0.0f, 0.0f, 1.0f));
	g.GenrateShape(Wedge, glm::vec3(0.5f, 0.5f, 0.0f));
	g.GenrateShape(Pyrmaind, glm::vec3(0.5f, 0.5f, 0.0f));
	g.GenrateShape(Pentagon, glm::vec3(0.5f, 0.5f, 0.0f));
	g.GenrateShape(Hexagon, glm::vec3(0.5f, 0.5f, 0.5f));
	g.GenrateShape(Diamond, glm::vec3(0.5f, 0.5f, 0.0f));
	g.GenrateShape(Disc, glm::vec3(0.5f, 0.5f, 0.0f));
	g.GenrateShape(Cone, glm::vec3(0.5f, 0.5f, 0.0f));
	g.GenrateShape(Cylinder, glm::vec3(0.5f, 0.5f, 0.0f));

	// Enable depth test.
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	// Enable face culling.
	glEnable(GL_CULL_FACE);
	glFrontFace(GL_CW);
	glCullFace(GL_BACK);
}

//---------------------------------------------------------------------
//
// calculateView
//
void calculateView()
{
	frontVec.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	frontVec.y = sin(glm::radians(pitch));
	frontVec.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	frontVec = glm::normalize(frontVec);
	rightVec = glm::normalize(glm::cross(frontVec, worldUp));
	upVec = glm::normalize(glm::cross(rightVec, frontVec));

	// Update the eye (or camera) position and the direction
	view = glm::lookAt(position, position + frontVec, upVec);
	glUniform3f(glGetUniformLocation(program, "eyePosition"), position.x, position.y, position.z);
	glUniform3f(glGetUniformLocation(program, "eyeDirection"), frontVec.x, frontVec.y, frontVec.z);
	
	// Updates the light's position as well
	glUniform3f(glGetUniformLocation(program, "lightPosition"), pLight.position.x, pLight.position.y, pLight.position.z);

	//Also crystal lights wee
	glUniform3f(glGetUniformLocation(program, "lightPosition"), pCrystalLight.position.x, pCrystalLight.position.y, pCrystalLight.position.z);

	glUniform3f(glGetUniformLocation(program, "lightPosition"), pCrystalBackLight.position.x, pCrystalBackLight.position.y, pCrystalBackLight.position.z);


}


//---------------------------------------------------------------------
//
// transformModel
//

void transformObject(float scale, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 translation) {
	glm::mat4 Model;
	Model = glm::mat4(1.0f);
	Model = glm::translate(Model, translation);
	Model = glm::rotate(Model, glm::radians(rotationAngle), rotationAxis);
	Model = glm::scale(Model, glm::vec3(scale));
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(projID, 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
}

void transformObject(glm::vec3 scale, glm::vec3 rotationAxis, float rotationAngle, glm::vec3 translation) {
	glm::mat4 Model;
	Model = glm::mat4(1.0f);
	Model = glm::translate(Model, translation);
	Model = glm::rotate(Model, glm::radians(rotationAngle), rotationAxis);
	Model = glm::scale(Model, scale);
	glUniformMatrix4fv(modelID, 1, GL_FALSE, &Model[0][0]);
	glUniformMatrix4fv(projID, 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(viewID, 1, GL_FALSE, &view[0][0]);
}

//---------------------------------------------------------------------
//
// display
//

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	
	// Camera matrix
	calculateView();

	//-------------------------------------
	// Draw Castle
	//--------------------------------------
	
	//Draw Plane
	glBindVertexArray(g.GetShape(Plane).getVao());
	glBindTexture(GL_TEXTURE_2D, plane_tex);
	transformObject(5.5f, Y_AXIS, 0, glm::vec3(0.0f, 0.0f, 0.0f));
	glDrawElements(GL_TRIANGLES, g.GetShape(Plane).getIndices(), GL_UNSIGNED_SHORT, 0);

	//Draw Hexagon
	glBindVertexArray(g.GetShape(Hexagon).getVao());
	glBindTexture(GL_TEXTURE_2D, hexagon_tex);
	transformObject(glm::vec3(2.0f, 2.0f, 1.0f), X_AXIS, 90, glm::vec3(0.0f, 0.5f, 0.0f));
	glDrawElements(GL_TRIANGLES, g.GetShape(Hexagon).getIndices(), GL_UNSIGNED_SHORT, 0);

	//Draw Door
	glBindVertexArray(g.GetShape(Pentagon).getVao());
	glBindTexture(GL_TEXTURE_2D, leather_tex);
	transformObject(0.3f, X_AXIS, 0, glm::vec3(0.0f, 0.5f, 1.7f));
	glDrawElements(GL_TRIANGLES, g.GetShape(Pentagon).getIndices(), GL_UNSIGNED_SHORT, 0);

	//Draw Wedge
	glBindVertexArray(g.GetShape(Wedge).getVao());
	glBindTexture(GL_TEXTURE_2D, cone_tex);
	transformObject(glm::vec3(1.0f, 0.5f, 0.5f), Y_AXIS, 90, glm::vec3(0.0f, 0.25f, 2.0f));
	glDrawElements(GL_TRIANGLES, g.GetShape(Wedge).getIndices(), GL_UNSIGNED_SHORT, 0);

	//Draw Disc
	glBindVertexArray(g.GetShape(Disc).getVao());
	glBindTexture(GL_TEXTURE_2D, disc_tex);
	transformObject(1.0f, X_AXIS, 0, glm::vec3(0.0f, 1.5f, -0.8f));
	glDrawElements(GL_TRIANGLES, g.GetShape(Disc).getIndices(), GL_UNSIGNED_SHORT, 0);
	transformObject(1.0f, X_AXIS, 180, glm::vec3(0.0f, 1.5f, -0.8f));
	glDrawElements(GL_TRIANGLES, g.GetShape(Disc).getIndices(), GL_UNSIGNED_SHORT, 0);

	//Draw Crystal
	glBindVertexArray(g.GetShape(Diamond).getVao());
	glBindTexture(GL_TEXTURE_2D, diamond_tex);
	transformObject(glm::vec3(0.25f, 0.5f, 0.25f), Y_AXIS, rotAngle -= 5, glm::vec3(0.0f, 2.0f, -0.8f));
	glDrawElements(GL_TRIANGLES, g.GetShape(Diamond).getIndices(), GL_UNSIGNED_SHORT, 0);

	//Draw Cylinders
	glBindVertexArray(g.GetShape(Cylinder).getVao());
	glBindTexture(GL_TEXTURE_2D, cylinder_tex);
	{
		transformObject(0.4f, Y_AXIS, 0, glm::vec3(1.0f, 0.8f, 1.8f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cylinder).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.4f, Y_AXIS, 0, glm::vec3(-1.0f, 0.8f, 1.8f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cylinder).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.4f, Y_AXIS, 0, glm::vec3(-1.0f, 0.8f, -1.8f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cylinder).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.4f, Y_AXIS, 0, glm::vec3(1.0f, 0.8f, -1.8f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cylinder).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.4f, Y_AXIS, 0, glm::vec3(2.0f, 0.8f, 0.0f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cylinder).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.4f, Y_AXIS, 0, glm::vec3(-2.0f, 0.8f, 0.0f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cylinder).getIndices(), GL_UNSIGNED_SHORT, 0);
	}
	//Draw Cones
	glBindVertexArray(g.GetShape(Cone).getVao());
	glBindTexture(GL_TEXTURE_2D, cone_tex);
	{
		transformObject(0.4f, Y_AXIS, 0, glm::vec3(1.0f, 1.6f, 1.8f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cone).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.4f, Y_AXIS, 0, glm::vec3(-1.0f, 1.6f, 1.8f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cone).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.4f, Y_AXIS, 0, glm::vec3(-1.0f, 1.6f, -1.8f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cone).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.4f, Y_AXIS, 0, glm::vec3(1.0f, 1.6f, -1.8f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cone).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.4f, Y_AXIS, 0, glm::vec3(2.0f, 1.6f, 0.0f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cone).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.4f, Y_AXIS, 0, glm::vec3(-2.0f, 1.6f, 0.0f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cone).getIndices(), GL_UNSIGNED_SHORT, 0);
	}
	//Draw Pyramids
	glBindVertexArray(g.GetShape(Pyrmaind).getVao());
	glBindTexture(GL_TEXTURE_2D, pyramid_tex);
	{
		transformObject(0.2f, Y_AXIS, 0, glm::vec3(0.25f, 1.1f, 1.6f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Pyrmaind).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.2f, Y_AXIS, 0, glm::vec3(0.25f, 1.1f, -1.6f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Pyrmaind).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.2f, Y_AXIS, 0, glm::vec3(-0.25f, 1.1f, 1.6f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Pyrmaind).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.2f, Y_AXIS, 0, glm::vec3(-0.25f, 1.1f, -1.6f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Pyrmaind).getIndices(), GL_UNSIGNED_SHORT, 0);

		transformObject(0.2f, Y_AXIS, 120, glm::vec3(1.2f, 1.1f, 1.1f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Pyrmaind).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.2f, Y_AXIS, 120, glm::vec3(-1.2f, 1.1f, 1.1f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Pyrmaind).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.2f, Y_AXIS, 120, glm::vec3(1.5f, 1.1f, 0.6f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Pyrmaind).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.2f, Y_AXIS, 120, glm::vec3(-1.5f, 1.1f, 0.6f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Pyrmaind).getIndices(), GL_UNSIGNED_SHORT, 0);

		transformObject(0.2f, Y_AXIS, 120, glm::vec3(1.2f, 1.1f, -1.1f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Pyrmaind).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.2f, Y_AXIS, 120, glm::vec3(-1.2f, 1.1f, -1.1f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Pyrmaind).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.2f, Y_AXIS, 120, glm::vec3(1.5f, 1.1f, -0.6f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Pyrmaind).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.2f, Y_AXIS, 120, glm::vec3(-1.5f, 1.1f, -0.6f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Pyrmaind).getIndices(), GL_UNSIGNED_SHORT, 0);
	}
	//Draw Windows
	glBindVertexArray(g.GetShape(Cube).getVao());
	glBindTexture(GL_TEXTURE_2D, leather_tex);
	{
		transformObject(0.2f, Y_AXIS, 60, glm::vec3(1.3f, 0.5f, 1.1f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cube).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.2f, Y_AXIS, -60, glm::vec3(-1.3f, 0.5f, 1.1f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cube).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.2f, Y_AXIS, 60, glm::vec3(1.6f, 0.5f, 0.6f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cube).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.2f, Y_AXIS, -60, glm::vec3(-1.6f, 0.5f, 0.6f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cube).getIndices(), GL_UNSIGNED_SHORT, 0);

		transformObject(0.2f, Y_AXIS, 120, glm::vec3(1.3f, 0.5f, -1.1f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cube).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.2f, Y_AXIS, -120, glm::vec3(-1.3f, 0.5f, -1.1f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cube).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.2f, Y_AXIS, 120, glm::vec3(1.6f, 0.5f, -0.6f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cube).getIndices(), GL_UNSIGNED_SHORT, 0);
		transformObject(0.2f, Y_AXIS, -120, glm::vec3(-1.6f, 0.5f, -0.6f));
		glDrawElements(GL_TRIANGLES, g.GetShape(Cube).getIndices(), GL_UNSIGNED_SHORT, 0);
	}

	glBindVertexArray(0); // Can optionally unbind the vertex array to avoid modification.

	glutSwapBuffers(); // Now for a potentially smoother render.
}

void idle()
{
	// glutPostRedisplay();
}

void timer(int id)
{
	glutPostRedisplay();
	glutTimerFunc(30, timer, 0);
}

void keyDown(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 'w':
		position += frontVec * moveSpeed;
		break;
	case 's':
		position -= frontVec * moveSpeed;
		break;
	case 'a':
		position -= rightVec * moveSpeed;
		break;
	case 'd':
		position += rightVec * moveSpeed;
		break;
	case 'i':
		pLight.position.x += moveSpeed;
		break;
	case 'j':
		pLight.position.x -= moveSpeed;
		break;
	case 'k':
		pLight.position.y += moveSpeed;
		break;
	case 'l':
		pLight.position.y -= moveSpeed;
		break;
	case ' ':
		resetView();
		break;
	}
}

void mouseMove(int x, int y)
{
	//cout << "Mouse pos: " << x << "," << y << endl;
	if (mouseClicked)
	{
		pitch -= (GLfloat)((y - lastY) * 0.1);
		yaw += (GLfloat)((x - lastX) * 0.1);
		lastY = y;
		lastX = x;
	}
}

void mouseClick(int btn, int state, int x, int y)
{
	/*cout << "Clicked: " << (btn == 0 ? "left " : "right ") << (state == 0 ? "down " : "up ") <<
		"at " << x << "," << y << endl;*/
	if (state == 0)
	{
		lastX = x;
		lastY = y;
		mouseClicked = true;
		glutSetCursor(GLUT_CURSOR_NONE);
		cout << "Mouse clicked." << endl;
	}
	else
	{
		mouseClicked = false;
		glutSetCursor(GLUT_CURSOR_INHERIT);
		cout << "Mouse released." << endl;
	}
}

void clean()
{
	cout << "Cleaning up!" << endl;
}

//---------------------------------------------------------------------
//
// main
//

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowSize(1024, 1024);
	glutCreateWindow("Araujo Silva Flavio Jorge 101173562");

	glewInit();	//Initializes the glew and prepares the drawing pipeline.

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	init();

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutTimerFunc(30, timer, 0);
	glutKeyboardFunc(keyDown);
	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseMove);

	atexit(clean); // This GLUT function calls specified function before terminating program. Useful!
	glutMainLoop();
}
