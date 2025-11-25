#include <TextureMap.h>
#include <CanvasTriangle.h>
#include <CanvasPoint.h>
#include <Colour.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <vector>
#include <glm/glm.hpp>

#define WIDTH 1920
#define HEIGHT 1080

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
	else{
		float percentage = (pairedValue - from.x) / xRange;
		return from.y + percentage * yRange;
	}
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

void drawFlatTopTriangle(DrawingWindow &window, CanvasPoint bottomPoint, CanvasPoint v0, CanvasPoint v1, Colour colour){
	// Set Up Colour
	uint32_t packedColour = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
	
	// Figure out which is left or right side
	// If vertex_0 is lesser than vertex_1, left vertex is vertex_0, else vextex_1
	CanvasPoint left = v0.x < v1.x ? v0 : v1;
	CanvasPoint right = v0.x < v1.x ? v1 : v0;
	
	int yStart = v0.y;
	int yEnd = bottomPoint.y;

	for (int i = yStart; i <= yEnd; i++){
		int xLeft = interpolateOnEdge(left,bottomPoint,i,0);
		int xRight = interpolateOnEdge(right,bottomPoint,i,0);
		for (int j = xLeft; j <= xRight; j++){
			window.setPixelColour(j,i, packedColour);
		}
	}
} 

void drawFlatBottomTriangle(DrawingWindow &window, CanvasPoint TopPoint, CanvasPoint v0, CanvasPoint v1, Colour colour){
	//Extra note: TopPoint would mean visual top point which in out case is y value that is the lowest
	
	// Set Up Colour
	uint32_t packedColour = (255 << 24) + (colour.red << 16) + (colour.green << 8) + colour.blue;
	
	// Figure out which is left or right side
	// If vertex_0 is lesser than vertex_1, left vertex is vertex_0, else vextex_1
	CanvasPoint left = v0.x < v1.x ? v0 : v1;
	CanvasPoint right = v0.x < v1.x ? v1 : v0;
	
	int yStart = TopPoint.y;
	int yEnd = v0.y;

	for (int i = yStart; i <= yEnd; i++){
		int xLeft = interpolateOnEdge(left,TopPoint,i,0);
		int xRight = interpolateOnEdge(right,TopPoint,i,0);
		for (int j = xLeft; j <= xRight; j++){
			window.setPixelColour(j,i, packedColour);
		}
	}
} 

// <Drawing Filled Triangle> Week 3 Task 4
// Draws 3 whiter edge and fills it up based on the colours we chose
void drawFilledTriangle(DrawingWindow &window, CanvasTriangle triangle, Colour colour){
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
		drawFlatBottomTriangle(window, a,b,c, colour);
	} 
	else if (a.y == b.y){
		drawFlatBottomTriangle(window, a,b,c,colour);
	}
	else{
		// Split triangle
			float newXCoodinate = interpolateOnEdge(a, c, b.y, 0);
			CanvasPoint d(newXCoodinate,b.y);
			drawFlatTopTriangle(window,a,b,d,colour);
			drawFlatBottomTriangle(window,c,b,d,colour);
	}
	drawUnfilledTriangle(window, triangle, Colour(255,255,255));
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
			
            drawFilledTriangle(window, tri, col);
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
	CanvasPoint point1(160,10);
	point1.texturePoint = TexturePoint(195,5);
	CanvasPoint point2(300,230);
	point2.texturePoint = TexturePoint(395,380);
	CanvasPoint point3(10,150);
	point3.texturePoint = TexturePoint(65,330);
	CanvasTriangle sampleTri(point1,point2,point3);
	mapTexture(window,sampleTri,texture);
	// Week 2 Task 4
	//drawRainbowBaycentricTriangle(window, glm::vec2(120,600), glm::vec2(1000,1000), glm::vec2(300,100));
	// TASK TESTS END ----------------------------------------------------------------------------------------------------------------->

	SDL_Event event;
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		window.renderFrame();
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
	}
}

