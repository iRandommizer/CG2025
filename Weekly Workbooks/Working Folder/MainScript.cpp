#include <CanvasTriangle.h>
#include <DrawingWindow.h>
#include <Utils.h>
#include <fstream>
#include <iostream>
#include <vector>

#define WIDTH 320
#define HEIGHT 320 

// --------------------------------------------
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

void handleEvent(SDL_Event event, DrawingWindow &window) {
	if (event.type == SDL_KEYDOWN) {
		if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
		else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
		else if (event.key.keysym.sym == SDLK_UP) std::cout << "UP" << std::endl;
		else if (event.key.keysym.sym == SDLK_DOWN) std::cout << "DOWN" << std::endl;
	} else if (event.type == SDL_MOUSEBUTTONDOWN) {
		window.savePPM("output.ppm");
		window.saveBMP("output.bmp");
	}
}
// --------------------------------------------

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

int main(){
    std::vector<float> result;
    result = interpolateSingleFloats(10, 30, 6);
    for(size_t i=0; i<result.size(); i++) std::cout << result[i] << " ";
    std::cout << std::endl;

    DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);
	SDL_Event event;
	while (true) {
		// We MUST poll for events - otherwise the window will freeze !
		if (window.pollForInputEvents(event)) handleEvent(event, window);
		draw(window);
		// Need to render the frame at the end, or nothing actually gets shown on the screen !
		window.renderFrame();
	}
}

// Single Dimension Greyscale Interpolation