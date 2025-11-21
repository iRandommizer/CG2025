#pragma once

#include "CanvasPoint.h"
#include <iostream>
#include <array>

struct CanvasTriangle {
	std::array<CanvasPoint, 3> vertices{}; // Says that vetices is an array of 3 CanvasPoints, the "{}" means default-initialize

	CanvasTriangle();
	CanvasTriangle(const CanvasPoint &v0, const CanvasPoint &v1, const CanvasPoint &v2);
	//----
	//Helper functions to access each vertex by name rather than by idx
	// the "&" means that the function returns a reference. Hence we can both read the point and modify it
	CanvasPoint &v0();
	CanvasPoint &v1();
	CanvasPoint &v2();
	//----
	CanvasPoint operator[](size_t i) const;
	CanvasPoint &operator[](size_t i);
	friend std::ostream &operator<<(std::ostream &os, const CanvasTriangle &triangle);
};

std::ostream &operator<<(std::ostream &os, const CanvasTriangle &triangle);
