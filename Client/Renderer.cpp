#include "Renderer.h"
#include "GLSL_Line.h"
#include "GLSL_Point.h"
#include "GLSL_Square.h"
#include "GLSL_Circle.h"
#include "EngineConfig.h"
#include <TTF/SDL_ttf.h>
#include <GL/glew.h>
#include <iostream>

Renderer::Renderer() : vertexArrays(), vertexBuffers(), offset(0), textureOffset(0), shaderProgram(), textureProgram() {

}

Renderer::Renderer(Camera2D& camera) : vertexArrays(), vertexBuffers(), offset(0), shaderProgram(camera, VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH) {
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
	glGenVertexArrays(2, &vertexArrays[0]);
	glGenBuffers(2, &vertexBuffers[0]);

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
}

void Renderer::initShaderProgram(Camera2D& camera) {
	// the order of attributes must be the same as order used in shaders
	shaderProgram.initShaders(camera, VERTEX_SHADER_PATH, FRAGMENT_SHADER_PATH);
	shaderProgram.addAttribute("vertexPosition");
	shaderProgram.addAttribute("vertexColor");
	shaderProgram.linkShaders();

	textureProgram.initShaders(camera, TEXTURE_VERTEX_PATH, TEXTURE_FRAGMENT_PATH);
	textureProgram.addAttribute("vertexPosition");
	textureProgram.addAttribute("vertexColor");
	textureProgram.addAttribute("vertexUV");
	textureProgram.linkShaders();
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
}

void Renderer::draw() {
	bindVertexArray(vertexArrays[0]);
	// draw geometry
	shaderProgram.use();
	drawGeometry();
	shaderProgram.unuse();

	bindVertexArray(vertexArrays[1]);
	// draw texture
	textureProgram.use();
	uploadTextureUnit();
	drawTexture();
	textureProgram.unuse();

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

void Renderer::drawTexture(Square square, GLTexture texture) {
	textureObjects.emplace_back(square, texture, textureOffset, textureVetrices);
}

void Renderer::reset() {
	vertices.clear();
	textureVetrices.clear();
	geometryObjects.clear();
	textureObjects.clear();
	offset = 0;
	textureOffset = 0;
}

bool Renderer::check() {
	return vertexArrays[0] == 0;
}

