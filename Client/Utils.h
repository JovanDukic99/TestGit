#pragma once
#include <string>
#include <vector>
#include "Square.h"
#include "AStarAlgorithm.h"
class Utils
{
public:
	static void loadMap(std::string filePath, std::vector<Square>& blocks, float unitWidth, float unitHeight);
	static void loadMap(std::string filePath, std::vector<Square>& blocks, AStarAlgorithm& algorithm, float unitWidth, float unitHeight);

	template <typename T>
	static bool contains(std::vector<T>& nodes, T node);
};

template<typename T>
inline bool Utils::contains(std::vector<T>& nodes, T node) {
	for (int i = 0; i < nodes.size(); i++) {
		if (nodes[i] == node) {
			return true;
		}
	}
	return false;
}
