#include "Renderer.h"
#include "GLSL_Line.h"
#include "GLSL_Point.h"
#include "GLSL_Square.h"
#include "GLSL_Circle.h"
#include "Light.h"
#include "EngineConfig.h"
#include <TTF/SDL_ttf.h>
#include <GL/glew.h>
#include <iostream>

Renderer::Renderer() : vertexArrays(), vertexBuffers(), offset(0), textureOffset(0), lightOffset(0), visionRadius(0.0f){

}

Renderer::Renderer(Camera2D& camera) : vertexArrays(), vertexBuffers(), offset(0), textureOffset(0), lightOffset(0){
	init();
}

void Renderer::init() {
	initVertexArray();
}

void Renderer::init(Camera2D& camera) {
	if (check()) {
		initVertexArray();
		initShaderProgram(camera);
	}
}

void Renderer::initVertexArray() {
	glGenVertexArrays(3, &vertexArrays[0]);
	glGenBuffers(3, &vertexBuffers[0]);

	// bind shaderProgram buffer
	glBindVertexArray(vertexArrays[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[0]);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));

	glBindVertexArray(0);

	// bind textureProgram buffer
	glBindVertexArray(vertexArrays[1]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[1]);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	glBindVertexArray(0);

	// bind lightProgram buffer
	glBindVertexArray(vertexArrays[2]);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[2]);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(Vertex), (void*)offsetof(Vertex, color));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

	glBindVertexArray(0);
}

void Renderer::initShaderProgram(Camera2D& camera) {
	// the order of attributes must be the same as order used in shaders
	shaderProgram.initShaders(camera, VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);
	shaderProgram.addAttribute("vertexPosition");
	shaderProgram.addAttribute("vertexColor");
	shaderProgram.linkShaders();

	visionProgram.initShaders(camera, VISION_VERTEX_PATH, VISION_FRAGMENT_PATH);
	visionProgram.addAttribute("vertexPosition");
	visionProgram.addAttribute("vertexColor");
	visionProgram.linkShaders();

	textureProgram.initShaders(camera, TEXTURE_VERTEX_PATH, TEXTURE_FRAGMENT_PATH);
	textureProgram.addAttribute("vertexPosition");
	textureProgram.addAttribute("vertexColor");
	textureProgram.addAttribute("vertexUV");
	textureProgram.linkShaders();

	lightProgram.initShaders(camera, LIGHT_VERTEX_PATH, LIGHT_FRAGMENT_PATH);
	lightProgram.addAttribute("vertexPosition");
	lightProgram.addAttribute("vertexColor");
	lightProgram.addAttribute("vertexUV");
	lightProgram.linkShaders();
}

void Renderer::begin() {
	reset();
}

void Renderer::end() {
	uploadVertexData();
	draw();
}

void Renderer::uploadVertexData() {
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[1]);
	glBufferData(GL_ARRAY_BUFFER, textureVetrices.size() * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, textureVetrices.size() * sizeof(Vertex), textureVetrices.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffers[2]);
	glBufferData(GL_ARRAY_BUFFER, lightVertices.size() * sizeof(Vertex), nullptr, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, lightVertices.size() * sizeof(Vertex), lightVertices.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::draw() {
	bindVertexArray(vertexArrays[0]);
	if (visionRadius <= 0.0f) {
		// draw geometry
		shaderProgram.use();
		drawGeometry();
		shaderProgram.unuse();
	}
	else {
		visionProgram.use();

		GLint location = visionProgram.getUniformValueLocation("visionRadius");
		glUniform1f(location, visionRadius);

		location = visionProgram.getUniformValueLocation("visionCenter");
		glUniform2f(location, visionCenter.getX(), visionCenter.getY());

		drawGeometry();

		visionProgram.unuse();
	}

	bindVertexArray(vertexArrays[1]);
	// draw texture
	textureProgram.use();
	uploadTextureUnit();
	drawTexture();
	textureProgram.unuse();

	bindVertexArray(vertexArrays[2]);
	// draw light
	lightProgram.use();

	// additive blending
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);

	drawLight();
	lightProgram.unuse();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	unbindVertexArray();
}

void Renderer::drawGeometry() {
	for (size_t i = 0; i < geometryObjects.size(); i++) {
		GLSL_Object object = geometryObjects[i];
		glDrawArrays(object.getMode(), object.getOffset(), object.getVertexNumber());
	}
}

void Renderer::drawTexture() {
	for (size_t i = 0; i < textureObjects.size(); i++) {
		GLSL_Texture texture = textureObjects[i];
		glBindTexture(GL_TEXTURE_2D, texture.getTextureID());
		glDrawArrays(texture.getMode(), texture.getOffset(), texture.getVertexNumber());
	}
}

void Renderer::drawLight() {
	for (size_t i = 0; i < lightObject.size(); i++) {
		GLSL_Light light = lightObject[i];
		glDrawArrays(light.getMode(), light.getOffset(), light.getVertexNumber());
	}
}

void Renderer::bindVertexArray(GLuint vertexArrayID) {
	glBindVertexArray(vertexArrayID);
}

void Renderer::unbindVertexArray() {
	glBindVertexArray(0);
}

void Renderer::uploadTextureUnit() {
	glActiveTexture(GL_TEXTURE0);
	GLuint textureLocation = textureProgram.getUniformValueLocation("asset");
	glUniform1i(textureLocation, 0);
}

// GLSL drawing functions
void Renderer::drawSquare(float x, float y, float width, float height, Color color) {
	geometryObjects.emplace_back(GLSL_Square(x, y, width, height, color, offset, vertices));
}

void Renderer::drawCircle(float x, float y, float radius, int segments, Color color) {
	geometryObjects.emplace_back(GLSL_Circle(x, y, radius, segments, color, offset, vertices));
}

void Renderer::drawLine(float x, float y, float x1, float y1, Color color) {
	geometryObjects.emplace_back(GLSL_Line(x, y, x1, y1, color, offset, vertices));
}

void Renderer::drawPoint(float x, float y, Color color) {
	geometryObjects.emplace_back(GLSL_Point(x, y, color, offset, vertices));
}

// geometry drawing functions with color
void Renderer::drawSquare(Square square, Color color) {
	drawSquare(square.getX(), square.getY(), square.getWidth(), square.getHeight(), color);
}

void Renderer::drawCircle(Circle circle, Color color) {
	drawCircle(circle.getX(), circle.getY(), circle.getRadius(), circle.getSegments(), color);
}

void Renderer::drawLine(Line line, Color color) {
	drawLine(line.getPoints()[0], line.getPoints()[1], line.getPoints()[2], line.getPoints()[3], color);
}

void Renderer::drawPoint(Point point, Color color) {
	drawPoint(point.getX(), point.getY(), color);
}

// geometry drawing functions
void Renderer::drawSquare(Square square) {
	geometryObjects.emplace_back(GLSL_Square(square, offset, vertices));
}

void Renderer::drawCircle(Circle circle) {
	geometryObjects.emplace_back(GLSL_Circle(circle, offset, vertices));
}

void Renderer::drawLine(Line line) {
	geometryObjects.emplace_back(GLSL_Line(line, offset, vertices));
}

void Renderer::drawPoint(Point point) {
	geometryObjects.emplace_back(GLSL_Point(point, offset, vertices));
}

// draw texture
void Renderer::drawTexture(float x, float y, float width, float height, GLTexture texture) {
	textureObjects.emplace_back(x, y, width, height, texture, textureOffset, textureVetrices);
}

void Renderer::drawTexture(float x, float y, float width, float height, TextureAtlas textureAtlas, int textureIndex) {
	textureObjects.emplace_back(x, y, width, height, textureAtlas.getTexture(), textureAtlas.getUV(textureIndex), textureOffset, textureVetrices);
}

void Renderer::drawTexture(Square square, GLTexture texture) {
	textureObjects.emplace_back(square, texture, textureOffset, textureVetrices);
}

void Renderer::drawTexture(Square square, TextureAtlas textureAtlas, int textureIndex) {
	textureObjects.emplace_back(square, textureAtlas.getTexture(), textureAtlas.getUV(textureIndex), textureOffset, textureVetrices);
}

// draw light
void Renderer::drawLight(float x, float y, float width, float height, Color color) {
	lightObject.emplace_back(x, y, width, height, color, lightOffset, lightVertices);
}

void Renderer::drawLight(Light light) {
	lightObject.emplace_back(light, lightOffset, lightVertices);
}

void Renderer::reset() {
	vertices.clear();
	textureVetrices.clear();
	lightVertices.clear();

	geometryObjects.clear();
	textureObjects.clear();
	lightObject.clear();

	offset = 0;
	textureOffset = 0;
	lightOffset = 0;
}

bool Renderer::check() {
	return vertexArrays[0] == 0;
}

// setters
void Renderer::setVision(Point visionCenter, float visionRadius) {
	setVisionCenter(visionCenter);
	setVisionRadius(visionRadius);
}

void Renderer::setVisionCenter(Point visionCenter) {
	this->visionCenter = visionCenter;
}

void Renderer::setVisionRadius(float visionRadius) {
	this->visionRadius = visionRadius;
}

