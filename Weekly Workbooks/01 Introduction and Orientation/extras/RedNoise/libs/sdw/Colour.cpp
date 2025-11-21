#include "Colour.h"
#include <utility>

// Default Constructor
Colour::Colour() = default;

// Constructor with RGB
Colour::Colour(int r, int g, int b) : 
	red(r),
	green(g),
	blue(b) {}

// Constructor with RGB + Name
// Name could be used for debugging
Colour::Colour(std::string n, int r, int g, int b) :
		name(std::move(n)),
		red(r), green(g), blue(b) {}

// Set of instructions for what to do when you try to send a colour object through the output
// stream 	
std::ostream &operator<<(std::ostream &os, const Colour &colour) {
	os << colour.name << " ["
	   << colour.red << ", "
	   << colour.green << ", "
	   << colour.blue << "]";
	return os;
}
