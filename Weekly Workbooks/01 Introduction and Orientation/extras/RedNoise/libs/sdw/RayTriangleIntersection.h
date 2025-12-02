#pragma once

#include <glm/glm.hpp>
#include <iostream>
#include "ModelTriangle.h"

struct RayTriangleIntersection {
	glm::vec3 intersectionPoint; // hits are stored in 3D space (x,y,z)
	float distanceFromCamera; // How far the hit was 
	ModelTriangle intersectedTriangle; // which triangle was the hit
	size_t triangleIndex; // Index of the tirangle in the model array

	RayTriangleIntersection();
	RayTriangleIntersection(const glm::vec3 &point, float distance, const ModelTriangle &triangle, size_t index);
	friend std::ostream &operator<<(std::ostream &os, const RayTriangleIntersection &intersection);
};
