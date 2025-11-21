#include "CanvasPoint.h"

// Default Constructor in CPP
// Example Usage: CanvasPoint myPoint;
// It is called when you create a CanvasPoint object
CanvasPoint::CanvasPoint() :
	// It initializes the texturePoint member to (-1,-1) -> which means that it is indicating
	// that the point does not have a texture coordinate
		texturePoint(-1, -1) {}

// This is a constructor that creates a CanvasPoint object with a specific x and y position
// Example Usage: CanvasPoint start(10.0, ,20.0)
CanvasPoint::CanvasPoint(float xPos, float yPos) :
	// It uses a member initlizer list to set x => xPos, y=> yPos
	// Also sets default value for depth, brightness and texture point
		x(xPos),
		y(yPos),
		depth(0.0),
		brightness(1.0),
		texturePoint(-1, -1) {}

// This constructor is for creating a point in 3D space, as it allows us to specify a depth
// value on top of the x and y coordinates
// Example usage: Canvas pointIn3D(50.0, 50.0, 0.5)
CanvasPoint::CanvasPoint(float xPos, float yPos, float pointDepth) :
		x(xPos),
		y(yPos),
		depth(pointDepth),
		brightness(1.0),
		texturePoint(-1, -1) {}

CanvasPoint::CanvasPoint(float xPos, float yPos, float pointDepth, float pointBrightness) :
		x(xPos),
		y(yPos),
		depth(pointDepth),
		brightness(pointBrightness),
		texturePoint(-1, -1) {}

std::ostream &operator<<(std::ostream &os, const CanvasPoint &point) {
	os << "(" << point.x << ", " << point.y << ", " << point.depth << ") " << point.brightness;
	return os;
}
