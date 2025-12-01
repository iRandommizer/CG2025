#include <ModelTriangle.h>
#include <TextureMap.h>
#include <CanvasTriangle.h>
#include <CanvasPoint.h>
#include <Colour.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>

#define WIDTH 640
#define HEIGHT 640

TextureMap texture("../../../03 Triangles and Textures/texture.ppm");

void draw(DrawingWindow &window) { // I'm assuming these are runnign frame by frame
	window.clearPixels(); //Is this just for cleanliness? - this is used for showing motion, like in animation, you don't let the previous frame persist
	for (size_t y = 0; y < window.height; y++) { // go through every pixel in the y axis
		for (size_t x = 0; x < window.width; x++) { // go through every pixel in the x axis 
			float red = rand() % 256; //?? What does the rand() function do? What does the % do? -> This gives us a random value which is confined within the range of 0 and 256
			float green = static_cast<int>(random() - red) % 256;
			float blue = 0.0;
			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue); 
			//   255 << 24        -> places the alpha channel at the highest 8 bits (fully opaque?)
			//   int(red) << 16   -> places the red value in the next 8 bits 
			//   int(green) << 16 -> places the green value in the next 8 bits
			//   int(blue) << 16  -> places the blue in the last 8 bits
			window.setPixelColour(x, y, colour);
		}
	}
}
/// SUMMARY
/// Where is the size_t coming from? It was never established
/// What is unit32_t? why is this t notation with some of these variables? Is it correct to call them variables?

// Single Element Numerical Interpolation
std::vector<float>interpolateSingleFloats(float from,float to,float numberOfValues){
    //Find the difference between the end and the start
    //Number of values is inclusive of the start and the beginning
    //start = x, end = x + (n-1)u
    // Exampl start is 10, end is 20, number is 6
    // 10, 12, 14, 16, 18, 20, x == 10, y = 20, n = 6, u = 2
    std::vector<float> list;
    float difference = to - from;
    float intervalValue = difference/(numberOfValues - 1);
    for(int i = 0; i < numberOfValues; ++i){
        float curVal = from + i * intervalValue;
        list.push_back(curVal);
    }
    return list;
}


// Single Dimension Greyscale Interpolation
void drawGreyScaleInterpolation(DrawingWindow &window) { 
	window.clearPixels(); //Is this just for cleanliness? - this is used for showing motion, like in animation, you don't let the previous frame persist
	for (size_t y = 0; y < window.height; y++) { // go through every pixel in the y axis
		for (size_t x = 0; x < window.width; x++) { // go through every pixel in the x axis 
			float red = 256.0f - x * (255/(window.width - 1.0f));
			
			float green = red;
			float blue = red;
			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue); 
			//   255 << 24        -> places the alpha channel at the highest 8 bits (fully opaque?)
			//   int(red) << 16   -> places the red value in the next 8 bits 
			//   int(green) << 16 -> places the green value in the next 8 bits
			//   int(blue) << 16  -> places the blue in the last 8 bits
			window.setPixelColour(x, y, colour);
        }
    }
}

float interpolateOnEdge(CanvasPoint from, CanvasPoint to, float pairedValue ,int chosenDimension){
	float xRange = to.x - from.x;
	float yRange = to.y - from.y;
	// If we are looking for x value
	if (chosenDimension == 0){
		float percentage = (pairedValue - from.y) / yRange;
		return from.x + percentage * xRange;
	}
	// If we are looking for y value
	else if (chosenDimension == 1){
		float percentage = (pairedValue - from.x) / xRange;
		return from.y + percentage * yRange;
	}
	else if (chosenDimension == 2){
		if (yRange == 0) return from.depth; // just in case
		float percentage = (pairedValue - from.y) / yRange;
		return from.depth + percentage * (to.depth - from.depth);
	}
}

CanvasPoint interpolateOnEdge(CanvasPoint from, CanvasPoint to, float y){
	float yRange = to.y - from.y;
	if (yRange ==0) return from; // Just incase we divide by 0

	float percentage = (y - from.y ) / yRange;

	float x = from.x + (to.x - from.x) * percentage;
	// Interpolate Texture Coordinates
    float texX = from.texturePoint.x + (to.texturePoint.x - from.texturePoint.x) * percentage;
    float texY = from.texturePoint.y + (to.texturePoint.y - from.texturePoint.y) * percentage;

    CanvasPoint result(x, y);
    result.texturePoint = TexturePoint(texX, texY);
    return result;
}

// Three Element Numerical Interpolation
std::vector<glm::vec3> interpolateThreeElementValues(glm::vec3 from, glm::vec3 to, int numberOfValues){
	std::vector<glm::vec3> list;
	float xDiff = to.x - from.x;
	float yDiff = to.y - from.y;
	float zDiff = to.z - from.z;
	float xUnit = xDiff/(numberOfValues-1);
	float yUnit = yDiff/(numberOfValues-1);
	float zUnit = zDiff/(numberOfValues-1);
	for(int i = 0; i < numberOfValues; ++ i){
		float curXVal = from.x + i * xUnit;
		float curYVal = from.y + i * yUnit;
		float curZVal = from.z + i * zUnit;
		list.push_back(glm::vec3(curXVal, curYVal, curZVal));
	}
	return list;
}

// Help Function for Pixel to GLM Colour
glm::vec3 pixelToVec3(uint32_t pixel) {
    // Unpack RGB (ignore alpha: (pixel >> 24) & 0xFF)
    float red = static_cast<float>((pixel >> 16) & 0xFF);   // 0-255
    float green = static_cast<float>((pixel >> 8) & 0xFF);
    float blue = static_cast<float>(pixel & 0xFF);
    return glm::vec3(red, green, blue);  // Matches your format: e.g., glm::vec3(255, 0, 0)
}

// Two Dimension Colour Interpolation
void drawSquareRainbow(DrawingWindow &window, int delta) { 
	//Assign each corner their respective values, 
	window.clearPixels(); 
	glm::vec3 topLeft(255, 0, 0);        // red 
	glm::vec3 topRight(0, 0, 255);       // blue 
	glm::vec3 bottomRight(0, 255, 0);    // green 
	glm::vec3 bottomLeft(255, 255, 0);   // Yellow
	//Interpolate from left to right
	std::vector<glm::vec3> redBlueList = interpolateThreeElementValues(topLeft, topRight, window.width);
	for(int x = 0; x < window.width; x++){
		float red = redBlueList[x].x ;
		float green = redBlueList[x].y;
		float blue = redBlueList[x].z;
		uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue); 
		window.setPixelColour(x, 0, colour);
	}
	std::vector<glm::vec3> yellowGreenList = interpolateThreeElementValues(bottomLeft, bottomRight, window.width);
	for(int x = 0; x < window.width; x++){
		float red = yellowGreenList[x].x ;
		float green = yellowGreenList[x].y;
		float blue = yellowGreenList[x].z;
		uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue); 
		window.setPixelColour(x, window.height-1, colour);
	}
	for(int x = 0; x < delta; x++){
		std::vector<glm::vec3> interpolatedColourList = interpolateThreeElementValues(pixelToVec3(window.getPixelColour(x, 0)), pixelToVec3(window.getPixelColour(x, window.height-1)), window.height);
		for(int y = 1; y < window.height-1; y++){
			float red = interpolatedColourList[y].x ;
			float green = interpolatedColourList[y].y;
			float blue = interpolatedColourList[y].z;
			uint32_t colour = (255 << 24) + (int(red) << 16) + (int(green) << 8) + int(blue); 
			window.setPixelColour(x, y, colour);
		}
	}
}
// Barycentric Triangular Interpolation
// create coordinate points for A,B,C
// Assign color values to the respective vertices
// Do your regular for loop for every pixel per row, per col
// Do an if statement if u, v & w is [0,1] & u+v+w is = 1
void drawRainbowBaycentricTriangle(DrawingWindow &window, glm::vec2 v0, glm::vec2 v1, glm::vec2 v2){
	glm::vec3 v0Color(255,0,0);
	glm::vec3 v1Color(0,255,0);
	glm::vec3 v2Color(0,0,255);
	window.clearPixels();
	for (int y = 0; y < window.height; y++){
		for (int x = 0; x < window.width; x++){
			glm::vec2 curCoor(x,y);
			glm::vec3 baycentricCoor = convertToBarycentricCoordinates(v0, v1, v2, curCoor);
			float u = baycentricCoor.x;
			float v = baycentricCoor.y;
			float w = baycentricCoor.z;
			float tolerance = 1e-5;
			if(u >= 0 && v >= 0 && w >= 0 && u <= 1 && v <= 1 && w <= 1 && std::abs(u + v + w - 1) < tolerance){ // I needed to include "std::abs"" to use the absolute function
				glm::vec3 pixelColour = w*v0Color + v*v1Color + u*v2Color; // it's this order becasue its blue, green, red rmember
				uint32_t colour = (255 << 24) + (int(pixelColour.x) << 16) + (int(pixelColour.y) << 8) + int(pixelColour.z); 
				window.setPixelColour(x, y, colour);
			}
		}
	}
}

// -------------------------------------
// < Line Drawing >
// WHats the function type? 
void drawLine(DrawingWindow &window, CanvasPoint from,CanvasPoint to,Colour colour){
	// Sample Values of "from", "to", & "colour"
	// from: CanvasPoint topLeft(0, 0)
	// to: CanvasPoint center(500, 500)
	// colour: Colour col(255,255,255)
	//Psudocode
	// Get difference in x-axis
	float xDiff = to.x - from.x;
	// Get difference in y-axis
	float yDiff = to.y - from.y;
	float maxDiff = std::max(abs(xDiff), abs(yDiff)); // This is used to figure out how much is each unit for both x and y steps
	float xUnit = xDiff/maxDiff;
	float yUnit = yDiff/maxDiff;
	for(int i = 0; i < maxDiff; i++){
		float x = from.x + i*xUnit;
		float y = from.y + i*yUnit;
		uint32_t packedColour = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
		window.setPixelColour(round(x), round(y), packedColour);
	}
	// Interpolate both x and y values from to to from
	// Add pixel values within each step
}

// <Drawing Unfilled Triangle> Week 3 Task 3
// Draws three outer edge lines of a triangle that is passed as paramters
void drawUnfilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour){
	//Input
	// - CanvasTriangle Object which takes in 3 canvas points
	// - Colour Object to dictate the colour of our triangle
	
	// The CanvasTriangle Object allows us to save 3 CanvasPoints object which for these points, we can create lines from and make triangles
	drawLine(window, triangle.v0(), triangle.v1(), colour);
	drawLine(window, triangle.v1(), triangle.v2(), colour);
	drawLine(window, triangle.v2(), triangle.v0(), colour);
}

void drawFlatTopTriangle(DrawingWindow &window, CanvasPoint bottomPoint, CanvasPoint v0, CanvasPoint v1, Colour colour, std::vector<std::vector<float>> &depthBuffer){
	// Set Up Colour
	uint32_t packedColour = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
	
	// Figure out which is left or right side
	// If vertex_0 is lesser than vertex_1, left vertex is vertex_0, else vextex_1
	CanvasPoint left = v0.x < v1.x ? v0 : v1;
	CanvasPoint right = v0.x < v1.x ? v1 : v0;
	
	int yStart = v0.y;
	int yEnd = bottomPoint.y;

	for (int y = yStart; y <= yEnd; y++){
		float xLeft = interpolateOnEdge(left,bottomPoint,y,0);
		float xRight = interpolateOnEdge(right,bottomPoint,y,0);

		// Interpolate depth at the start and end of the row
		float depthLeft = interpolateOnEdge(left, bottomPoint, y, 2);
		float depthRigth = interpolateOnEdge(right, bottomPoint, y, 2);

		int intXLeft = std::round(xLeft);
		int intXRight = std::round(xRight);

		if (intXLeft > intXRight) {
			std::swap(intXLeft, intXRight);
			std::swap(depthLeft, depthRigth); // Important for correct depth interpolation!
		}

		for (int x = intXLeft; x <= intXRight; x++){
			float xRange = xRight - xLeft;
			float depthRange = depthRigth - depthLeft;

			float ratio = (xRange != 0) ? (float)(x - xLeft) / xRange : 0.0f;
			float pixleDepth = depthLeft + (ratio * depthRange);

			// Okay now we need to check the z-buffer
			if (x >= 0 && x < window.width && y >= 0 && y < window.height){
				if (pixleDepth > depthBuffer[x][y]){
					depthBuffer[x][y] = pixleDepth; //to update the depthbuffer per pixel
					window.setPixelColour(x,y, packedColour);
				}
			}
		}
	}
} 

void drawFlatTopTriangle(DrawingWindow &window, CanvasPoint bottomPoint, CanvasPoint v0, CanvasPoint v1, TextureMap texture){
	// Figure out which is left or right side
	// If vertex_0 is lesser than vertex_1, left vertex is vertex_0, else vextex_1
	CanvasPoint left = v0.x < v1.x ? v0 : v1;
	CanvasPoint right = v0.x < v1.x ? v1 : v0;
	
	// Define start and ending y points to intepolate agaisnt
	int yStart = v0.y;
	int yEnd = bottomPoint.y;

	// For every y, find the starting and ending x values
	for (int y = yStart; y <= yEnd; y++){
		CanvasPoint curLeft = interpolateOnEdge(left,bottomPoint,y);
		CanvasPoint curRight = interpolateOnEdge(right,bottomPoint,y);
		float xDiff = curRight.x - curLeft.x;
		// For every x and y values, let that pixel be assigned colour		
		for (int x = curLeft.x; x <= curRight.x; x++){
			float horizontalProgress = (x-curLeft.x)/xDiff;
			// Formula: Origin + (xdiff or ydiff) * % = current x or y
			float texX = curLeft.texturePoint.x + (curRight.texturePoint.x - curLeft.texturePoint.x)*horizontalProgress;
			float texY = curLeft.texturePoint.y + (curRight.texturePoint.y - curLeft.texturePoint.y)*horizontalProgress;
			// Capping the values of texX or texY, especially when rounding values to an int goes out of bound
			int intX = std::max(0, std::min((int)texX, (int)texture.width - 1));
			int intY = std::max(0, std::min((int)texY, (int)texture.height - 1));
			// This is to get the valid index of the colour we are looking for from the texturemap class
			int idx = (intY * texture.width) + intX;
			window.setPixelColour(x,y, texture.pixels[idx]);
		}
	}
} 

void drawFlatBottomTriangle(DrawingWindow &window, CanvasPoint topPoint, CanvasPoint v0, CanvasPoint v1, Colour colour, std::vector<std::vector<float>> &depthBuffer){
    uint32_t packedColour = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
    
    CanvasPoint left = v0.x < v1.x ? v0 : v1;
    CanvasPoint right = v0.x < v1.x ? v1 : v0;
    
    int yStart = topPoint.y;
    int yEnd = v0.y;

	// At every row
    for (int y = yStart; y <= yEnd; y++){
        // Get the left and right edges
        float xLeft = interpolateOnEdge(left, topPoint, y, 0);
        float xRight = interpolateOnEdge(right, topPoint, y, 0);

        // get the depth by interpolaiting according to y value
        float depthLeft = interpolateOnEdge(left, topPoint, y, 2);
        float depthRight = interpolateOnEdge(right, topPoint, y, 2);

        int intXLeft = std::round(xLeft);
        int intXRight = std::round(xRight);

		if (intXLeft > intXRight) {
			std::swap(intXLeft, intXRight);
			std::swap(depthLeft, depthRight); // Important for correct depth interpolation!
		}

        for (int x = intXLeft; x <= intXRight; x++){
            // now within the same row, we need to interpolate the depth but accoridng to x
            float xRange = xRight - xLeft;
            float depthRange = depthRight - depthLeft;
            float ratio = (xRange != 0) ? (float)(x - xLeft) / xRange : 0.0f;
            float pixelDepth = depthLeft + (ratio * depthRange);

            //  Z-Buffer Check
            if (x >= 0 && x < window.width && y >= 0 && y < window.height){
                if (pixelDepth > depthBuffer[x][y]) {
                    depthBuffer[x][y] = pixelDepth;
                    window.setPixelColour(x, y, packedColour);
                }
            }
        }
    }
}


void drawFlatBottomTriangle(DrawingWindow &window, CanvasPoint topPoint, CanvasPoint v0, CanvasPoint v1, TextureMap texture){
	//Extra note: TopPoint would mean visual top point which in out case is y value that is the lowest
	
	// Figure out which is left or right side
	// If vertex_0 is lesser than vertex_1, left vertex is vertex_0, else vextex_1
	CanvasPoint left = v0.x < v1.x ? v0 : v1;
	CanvasPoint right = v0.x < v1.x ? v1 : v0;
	
	// Define start and ending y points to intepolate agaisnt
	int yStart = topPoint.y;
	int yEnd = v0.y;

	// For every y, find the starting and ending x values
	for (int y = yStart; y <= yEnd; y++){
		CanvasPoint curLeft = interpolateOnEdge(left,topPoint,y);
		CanvasPoint curRight = interpolateOnEdge(right,topPoint,y);
		float xDiff = curRight.x - curLeft.x;
		// For every x and y values, let that pixel be assigned colour		
		for (int x = curLeft.x; x <= curRight.x; x++){
			float horizontalProgress = (x-curLeft.x)/xDiff;
			// Formula: Origin + (xdiff or ydiff) * % = current x or y
			float texX = curLeft.texturePoint.x + (curRight.texturePoint.x - curLeft.texturePoint.x)*horizontalProgress;
			float texY = curLeft.texturePoint.y + (curRight.texturePoint.y - curLeft.texturePoint.y)*horizontalProgress;
			// Capping the values of texX or texY, especially when rounding values to an int goes out of bound
			int intX = std::max(0, std::min((int)texX, (int)texture.width - 1));
			int intY = std::max(0, std::min((int)texY, (int)texture.height - 1));
			// This is to get the valid index of the colour we are looking for from the texturemap class
			int idx = (intY * texture.width) + intX;
			window.setPixelColour(x,y, texture.pixels[idx]);
		}
	}
} 

// <Drawing Filled Triangle> Week 3 Task 4
// Draws 3 whiter edge and fills it up based on the colours we chose
void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour, std::vector<std::vector<float>> &depthBuffer){
	//3 Core Algorithms
	//	1) Check if the triangle has a completely horizontal line, if not, split triangle
	//	2) Rasterise Triangle/Triangles
	//  3) Draw Outline for the coordinates to check our rasterising process

	CanvasPoint a = triangle.v0();
	CanvasPoint b = triangle.v1();
	CanvasPoint c = triangle.v2();
	// Sort vertices by decreasing y value, higher y value means, lower visually (went higher visually)
	if (a.y < b.y) std::swap(a,b);
	if (a.y < c.y) std::swap(a,c);
	if (b.y < c.y) std::swap(b,c);

	if (b.y == c.y){
		drawFlatBottomTriangle(window, a,b,c, colour,depthBuffer);
	} 
	else if (a.y == b.y){
		drawFlatTopTriangle(window, c,b,a,colour,depthBuffer);
	}
	else{
		// Split triangle
			float newXCoodinate = interpolateOnEdge(a, c, b.y, 0);
			// Interpolate Depth
			float newDepth = interpolateOnEdge(a,c,b.y,2);
			CanvasPoint d(newXCoodinate,b.y, newDepth);
			drawFlatBottomTriangle(window,c,b,d,colour, depthBuffer);
			drawFlatTopTriangle(window,a,b,d,colour,depthBuffer);
	}
	// Not Needed afterwards
	//drawUnfilledTriangle(window, triangle, Colour(255,255,255));
}

// <Mapping Textures> Week 3 Task 5
// With reference to the PPM file, we create a triangle and properly map PPM data to the output data
// Reminder: "&" helps us just reference the object itself instead of creature a "copy" of it
void mapTexture(DrawingWindow &window, CanvasTriangle triangle, TextureMap &texture){
	// Get the vertices
	CanvasPoint v0 = triangle.v0();
	CanvasPoint v1 = triangle.v1();
	CanvasPoint v2 = triangle.v2();

	// Convert to vec2 so we can use the "convertToBarycentricCoordinates"
	glm::vec2 vector0(v0.x,v0.y);
	glm::vec2 vector1(v1.x,v1.y);
	glm::vec2 vector2(v2.x,v2.y);

	for (int y = 0; y <= window.height; y++){
		for (int x = 0; x <= window.width; x++){
			// Get the Barycentric Weights
			glm::vec2 curCoor(x,y);
			glm::vec3 weights = convertToBarycentricCoordinates(vector0,vector1,vector2,curCoor);
			float u = weights.x;
			float v = weights.y;
			float w = weights.z;
			// Since we are working with floats, the numbers migth not be exact and be out of our range and hence needs tolerance
			float tolerance = 1e-5;
			// If it's within our triangle:
			if(u >= 0 && v >= 0 && w >= 0 && u <= 1 && v <= 1 && w <= 1 && std::abs(u+v+w -1) < tolerance){
				// Formula is like: Proportion * Total Size
				float texX = w * v0.texturePoint.x + u * v1.texturePoint.x + v * v2.texturePoint.x;
				float texY = w * v0.texturePoint.y + u * v1.texturePoint.y + v * v2.texturePoint.y;
				// Since we are dealing with int, we must convert our float values to int
				int intX = static_cast<int>(texX);
				int intY = static_cast<int>(texY);
				// Round down edge values
				intX = std::max(0,std::min(intX, (int)texture.width - 1));
				intY = std::max(0,std::min(intY, (int)texture.height - 1));

				// We need to convert our 2D Dimension Variables into 1D Dimension Index of the texture class
				int idx = (intY * texture.width) + intX;
				uint32_t colour = texture.pixels[idx];

				window.setPixelColour(x,y,colour);
			}
		}
	}
}

// <Mapping Textures> Week 3 Task 5
// With reference to the PPM file, we create a triangle and properly map PPM data to the output data
// It is very similar to our drawfilled triangle function, just that we are using a texture map
void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, TextureMap texture){
	//3 Core Algorithms
	//	1) Check if the triangle has a completely horizontal line, if not, split triangle
	//	2) Rasterise Triangle/Triangles
	//  3) Draw Outline for the coordinates to check our rasterising process
	CanvasPoint a = triangle.v0();
	CanvasPoint b = triangle.v1();
	CanvasPoint c = triangle.v2();
	// Sort vertices by decreasing y value, higher y value means, lower visually (went higher visually)
	if (a.y < b.y) std::swap(a,b);
	if (a.y < c.y) std::swap(a,c);
	if (b.y < c.y) std::swap(b,c);

	if (b.y == c.y){
		drawFlatBottomTriangle(window, a,b,c, texture);
	} 
	else if (a.y == b.y){
		drawFlatTopTriangle(window,c,b,a,texture);
	}
	else{
		// Split triangle
			CanvasPoint d = interpolateOnEdge(a,c,b.y);
			d.x = round(d.x);
			drawFlatBottomTriangle(window,c,b,d,texture);
			drawFlatTopTriangle(window,a,b,d,texture);		
	}
	drawUnfilledTriangle(window, triangle, Colour(255,255,255));
}

// Week 4 Task 2 
// How to read and process .obj objects, specifically to turn it into a dynamic array of model triangles
std::vector<ModelTriangle> loadObjModel(std::string filename, float scalingFactor, std::vector<Colour> palette){
	// Setup Contrainers 
	std::vector<ModelTriangle> triangles;
	std::vector<glm::vec3> tempVertices;

	// Open the file
	std::ifstream file(filename); // ifstream stands for -> input file stream
	std::string line; // Temp variable for reading cur line

	// Added on from Wk 4 Task 3
	// Lets give a defualt colour, just in case, no colour was given
	Colour currentColour = Colour(255,255,255);

	// Read the file line by line
	while (std::getline(file, line)){ // try to grab the next line from "file", if you manage to , run the loop
		std::vector<std::string> tokens = split(line, ' '); // it is basically creating individual values from a line everytime there is a ' ' between characters
		
		// if there is a line that is empty, we skip it
		if (tokens.empty()) continue;

		// Added on from Wk 4 Task 3
		if (tokens[0] == "usemtl"){
			std::string neededColour = tokens[1];

			// Lets look through our palette for the colour
			bool found = false;
			for (size_t i = 0; i < palette.size(); i++){
				if(palette[i].name == neededColour){
					currentColour = palette[i];
					found = true;
					break;
				} 
			}
		}
		// We have the vertex processing
		else if (tokens[0] == "v"){
			// We are to immediately scale the positions (x,y,z) when they are read in
			// std::stof means: string to float
			float x = std::stof(tokens[1]) * scalingFactor;
			float y = std::stof(tokens[2]) * scalingFactor;
			float z = std::stof(tokens[3]) * scalingFactor;

			tempVertices.push_back(glm::vec3(x,y,z));
		}

		// Then the Face processing
		else if (tokens[0] == "f") {
			// Since obj uses 1 for it's first idx, we need to minus it by 1
			// std::stoi means: string to int
			int v1Idx = std::stoi(tokens[1]) - 1;
			int v2Idx = std::stoi(tokens[2]) - 1;
			int v3Idx = std::stoi(tokens[3]) - 1;
			// We then retrieve the actual points using the indices
			glm::vec3 pt1 = tempVertices[v1Idx];
			glm::vec3 pt2 = tempVertices[v2Idx];
			glm::vec3 pt3 = tempVertices[v3Idx];
			
			// Now having both, we cna create the triangle with the colour inputed
			ModelTriangle triangle(pt1, pt2, pt3, currentColour);
			triangles.push_back(triangle);
		}
	}

	return triangles;
}

// Week 4 Task 3
// Function to read .mtl files and extract "colour" data from it to make a palette
std::vector<Colour> loadMaterials(std::string filename) {
	// Used to store our colours
	std::vector<Colour> palette;

	std::ifstream file(filename);
	std::string line; // store the line of information
	std::string currentName;

	// format of .mtl:
	// "newmtl White 
	//	Kd 1.000000 1.000000 1.000000"
	// We have to tokenize as well
	while(std::getline(file,line)){
		std::vector<std::string> tokens = split(line, ' ');
		if (tokens.empty()) continue;

		// Extract the names
		if (tokens[0] == "newmtl"){
			currentName = tokens[1];
		}

		// extract the rgb values
		else if (tokens[0] == "Kd"){
			int r = (int)(std::stof(tokens[1]) * 255);
			int g = (int)(std::stof(tokens[2]) * 255);
			int b = (int)(std::stof(tokens[3]) * 255);

			Colour c(currentName, r,g,b);
			palette.push_back(c);
		}
	}
	return palette;
}

// Week 4 Task 4 & 5 & eventually 6
// Projection function where from the points in 3D space, we calculate where it should land in a 2D screenQ
// Input:
// - vertex positions
// - focal length
// Output:
// - calculated 2D pixel coordinates
CanvasPoint projectVertOnCanvasPoint(glm::vec3 cameraPos, glm::vec3 vertPos, float focalLength, float imageScale, DrawingWindow &window){
	// Calculate the relative position
	float x = vertPos.x - cameraPos.x;
	float y = vertPos.y - cameraPos.y;
	float z = vertPos.z - cameraPos.z; // Positive z is going towards you, negative z is away from you
	// apply the perspective projection
	// formula: u = -f * (x/z) + (W/2)
	// formula: v = f * (y/z) + (H/2)
	float u = -(focalLength * (x/z)) * imageScale + (window.width/2.0f);
	float v = (focalLength * (y/z)) * imageScale + (window.height/2.0f);
	
	float distance = -z;
	float inverseDepth = (distance > 0) ? (1.0f/distance) : 0.0f;
	return CanvasPoint(u,v, inverseDepth);
}

// Week 4 Task 6
void drawPointCloud(std::vector<ModelTriangle> model, glm::vec3 cameraPos, float focalLength, float imageScale, DrawingWindow &window){
	// loop through all triangles
	for (size_t i = 0; i < model.size(); i++){
		ModelTriangle triangle = model[i];
		// for each triangles' vert
		for (size_t j = 0; j < 3; j++){
			glm::vec3 vertex = triangle.vertices[j];
			// we project the 3d vert to 2d screen
			CanvasPoint screenPos = projectVertOnCanvasPoint(cameraPos, vertex, focalLength, imageScale, window);

			// draw the white pixel at right location
			uint32_t white = (255 << 24) + (255 << 16) + (255 << 8) + 255;
			window.setPixelColour(screenPos.x, screenPos.y, white);
		}
	}
}

// Lets make a helper function for task 7 and 8 since this is a common step for the 2
CanvasTriangle project3DTriangle(ModelTriangle currentTri, glm::vec3 cameraPos, float focalLength, float imageScale, DrawingWindow &window){
	// Convert from 3d vert to 2d points for each point of the triangle
	CanvasPoint pt0 = projectVertOnCanvasPoint(cameraPos, currentTri.vertices[0], focalLength, imageScale, window);
	CanvasPoint pt1 = projectVertOnCanvasPoint(cameraPos, currentTri.vertices[1], focalLength, imageScale, window);
	CanvasPoint pt2 = projectVertOnCanvasPoint(cameraPos, currentTri.vertices[2], focalLength, imageScale, window);
	return CanvasTriangle(pt0,pt1,pt2);
}

// Week 4 Task 7
// We are now going to try to connect the dots by using the triangles data and out past drawUnfilledTriangle function
void drawWireframe(std::vector<ModelTriangle> model, glm::vec3 cameraPos, float focalLength, float imageScale, DrawingWindow &window){
	for (size_t i = 0; i < model.size(); i++){
		ModelTriangle curTri = model[i];

		// Create the 2D triangle
		// Apparenlty we cannot have numbers as the start of the variable name
		CanvasTriangle tri2D = project3DTriangle(curTri, cameraPos, focalLength, imageScale, window);
		drawUnfilledTriangle(window, tri2D, Colour(255,255,255));
	}
}

// Week 4 Task 8
// very simialr to task 6 and 8 but now we will just add the colour from the pallette and also use our drawFilledTriangle Function
void drawRasterisedView(std::vector<ModelTriangle> model, glm::vec3 cameraPos, float focalLength, float imageScale, DrawingWindow &window){
	std::vector<std::vector<float>> depthBuffer(window.width, std::vector<float>(window.height, 0.0f));
	
	for (size_t i = 0; i < model.size(); i++){
		ModelTriangle curTri = model[i];

		CanvasTriangle tri2D = project3DTriangle(curTri, cameraPos, focalLength, imageScale, window);
		Colour triColour = curTri.colour;
		drawFilledTriangle(window, tri2D, triColour, depthBuffer);
		if (curTri.colour.name == "Green") { // Or check RGB values
			std::cout << "Green Triangle Z values: " 
					<< curTri.vertices[0].z << ", "
					<< curTri.vertices[1].z << ", "
					<< curTri.vertices[2].z << std::endl;
		}
		if (curTri.colour.name == "Green") {
			std::cout << "Green 2D Points:" << std::endl;
			std::cout << "  P0: (" << tri2D.v0().x << ", " << tri2D.v0().y << ")" << std::endl;
			std::cout << "  P1: (" << tri2D.v1().x << ", " << tri2D.v1().y << ")" << std::endl;
			std::cout << "  P2: (" << tri2D.v2().x << ", " << tri2D.v2().y << ")" << std::endl;
		}
	}
}

// Week 4 Task 9
// Let's fix the render based on taking the z-depth in mind
// We need to update some of our older functions to fully integarte this idea


void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
		// Create random unfilled triangles
		else if (event.key.keysym.sym == SDLK_u) {
			// Create random points
            CanvasPoint a(rand() % window.width,  rand() % window.height);
            CanvasPoint b(rand() % window.width,  rand() % window.height);
            CanvasPoint c(rand() % window.width,  rand() % window.height);

            CanvasTriangle tri(a, b, c);

            // Create random colors
            Colour col(rand() % 256, rand() % 256, rand() % 256);
			
            drawUnfilledTriangle(window, tri, col);
        }
		// Create random triangles
		else if (event.key.keysym.sym == SDLK_f) {
			// Create random points
            CanvasPoint a(rand() % window.width,  rand() % window.height);
            CanvasPoint b(rand() % window.width,  rand() % window.height);
            CanvasPoint c(rand() % window.width,  rand() % window.height);

            CanvasTriangle tri(a, b, c);

            // Create random colors
            Colour col(rand() % 256, rand() % 256, rand() % 256);
			
            //drawFilledTriangle(window, tri, col,);
        }
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}

int main(int argc, char *argv[]) {
	std::vector<glm::vec3> result;
    result = interpolateThreeElementValues(glm::vec3(1.0f, 4.0f, 9.2f), glm::vec3(4.0f, 1.0f, 9.8f), 4);

    // Debug: Confirm size
    std::cout << "Result size: " << result.size() << std::endl;  // Should print "4"

    // Fixed print: Manually output each vec3's components
    for (size_t i = 0; i < result.size(); ++i) {
        std::cout << "(" << result[i].x << ", " << result[i].y << ", " << result[i].z << ") ";
    }
    std::cout << std::endl;
	
	DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

	// TASK TESTS START ----------------------------------------------------------------------------------------------------------------->
	// Week 3 Task 4
	/*drawFilledTriangle(window, CanvasTriangle(CanvasPoint(200,200), CanvasPoint(700,700), CanvasPoint(1400,600)), Colour(235,50,123));
	drawFlatBottomTriangle(window, CanvasPoint(500,10), CanvasPoint(50,400), CanvasPoint(1000, 400), Colour(255,255,255));
	// Week 3 Task 2
	drawLine(window, CanvasPoint(0,0), CanvasPoint(window.width/2, window.height/2), Colour(255,255,255));
	drawLine(window, CanvasPoint(window.width/2,0), CanvasPoint(window.width/2, window.height), Colour(255,255,255));
	drawLine(window, CanvasPoint(window.width-1,0), CanvasPoint(window.width/2, window.height/2), Colour(255,255,255));
	drawLine(window, CanvasPoint(window.width/3,window.height/2), CanvasPoint(window.width/3*2, window.height/2), Colour(255,255,255));
	// Weel 3 Task 3
	drawUnfilledTriangle(window, CanvasTriangle(CanvasPoint(20,500), CanvasPoint(400,100), CanvasPoint(window.width/1.5f, window.height/1.5f)),Colour(155,100,20));	
	*/
	// Week 3 Task 5
	/*
	CanvasPoint point1(160,10);
	point1.texturePoint = TexturePoint(195,5);
	CanvasPoint point2(300,230);
	point2.texturePoint = TexturePoint(395,380);
	CanvasPoint point3(10,150);
	point3.texturePoint = TexturePoint(65,330);
	CanvasTriangle sampleTri(point1,point2,point3);
	drawFilledTriangle(window,sampleTri,texture);
	*/
	// Week 2 Task 4
	//drawRainbowBaycentricTriangle(window, glm::vec2(120,600), glm::vec2(1000,1000), glm::vec2(300,100));
	// Week 4 Task 2
	/*
	std::vector<ModelTriangle> model = loadObjModel("../../../04 Wireframes and Rasterising/models/cornell-box.obj",0.35);
	for (int i = 0; i < model.size(); i++){
		std::cout << "Triangle " << i << ": " << model[i] << std::endl;
	*/
	// Week 4 Task 3
	/*std::vector<Colour> palette = loadMaterials("../../../04 Wireframes and Rasterising/models/cornell-box.mtl");
	for (size_t i = 0; i < palette.size(); i++){
		std::cout << "Color " << i << ": " << palette[i].name << " (" << palette[i].red << ", " << palette[i].green << ", " << palette[i].blue << ")" << std::endl;
	}
	*/
	// Week 4 Task 4 & 5 Test (obselete, function was updated)
	/*
	glm::vec3 testVert(1.0,1.0,0.0);
	float focalLength = 2.0;

	CanvasPoint projectedPoints = projectVertOnCanvasPoint(testVert, focalLength, window);
	std::cout << "U : " << projectedPoints.x << std::endl;
	std::cout << "V : " << projectedPoints.y << std::endl;
	*/
	// Week 4 Task 6 & 7 & 8 Test
	std::vector<Colour> palette = loadMaterials("../../../04 Wireframes and Rasterising/models/cornell-box.mtl");
	std::vector<ModelTriangle> model = loadObjModel("../../../04 Wireframes and Rasterising/models/cornell-box.obj",0.35, palette);
	glm::vec3 cameraPos(0.0,0.5,4.0);
	float focalLength = 2.0f;
	float imageScale = 150;
	//drawPointCloud(model, cameraPos, focalLength, imageScale, window);
	//drawWireframe(model, cameraPos, focalLength, imageScale, window);
	drawRasterisedView(model, cameraPos, focalLength,imageScale, window);
	// TASK TESTS END ----------------------------------------------------------------------------------------------------------------->

	SDL_Event event;
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		window.renderFrame();
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
	}
}

