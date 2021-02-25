#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <queue>
#include "PriorityQueue.h"
#include "SearchSpace.h"
#include "Square.h"
#include "Line.h"

class Utils
{
public:
	// static methods
	static void loadMap(std::string filePath, std::vector<Square>& blocks, float unitWidth, float unitHeight);
	static void loadMASP(std::string filePath, std::vector<Square>& blocks, SearchSpace& searchSpace, float unitWidth, float unitHeight);
	static void createEdges(SearchSpace& searchSpace, std::vector<Square>& blocks, std::vector<Edge*>& edges, float mapHeight, float unitWidth, float unitHeight);
	static void createEdgePoints(std::vector<Edge*>& edges, std::vector<glm::vec2>& edgePoints);
	static void rayTracing(std::vector<Edge*>& edges, std::vector<glm::vec2>& edgePoints, std::vector<glm::vec2>& lightPoints, glm::vec2 p);
	static std::vector<Point> convertToSquarePath(std::vector<Point> points, float mapHeight, float unitWidth, float unitHeight);
	static std::vector<Point>& convertToPlayerPath(std::vector<Point>& points, float endX, float endY);

	// calculations
	static glm::vec2 lineIntersection(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, bool* check);

	// templates
	template <typename T, typename F, typename G>
	static bool contains(std::priority_queue<T, F, G> queue, T node);
	template <typename T>
	static bool contains(std::vector<T>& nodes, T node);
	template <typename T>
	static std::vector<T> reverse(std::vector<T> vector);
};

template<typename T, typename F, typename G>
inline bool Utils::contains(std::priority_queue<T, F, G> queue, T node) {
	while (!queue.empty()) {
		T temp = queue.top();
		if (temp == node) {
			return true;
		}
		queue.pop();
	}
	return false;
}

template<typename T>
inline bool Utils::contains(std::vector<T>& nodes, T node) {
	for (size_t i = 0; i < nodes.size(); i++) {
		if (nodes[i] == node) {
			return true;
		}
	}
	return false;
}

template<typename T>
inline std::vector<T> Utils::reverse(std::vector<T> vector) {
	std::vector<T> result;
	result.resize(vector.size());
	for (size_t i = 0; i < vector.size(); i++) {
		result[vector.size() - 1 - i] = vector[i];
	}
	return result;
}
