#include <iostream>
#include <vector>
#include <string>

// Fn 1. Print Hello World
void printHelloWorld(){
    std::cout << "Hello, World" << std::endl;
    // from the stnadard library, get the function cout? and stream the value hello world followed by , from the standard library, moving the cursor to the next line
}

// Fn 2. Adding integers
int addInts(int a, int b){
    return a + b;
}

// Fn 3. Get max of 2 floats
float maxfloat(float a, float b){
    return (a > b) ? a : b;
    // if a is greater than b, return a, else return b
    // -- interstingly, you don't need an if inn the beginning
}

// Fn 4. Calculate length of string
int calcLengthOfStr(const std::string& word){
    return word.size();
}

// Fn 5. Fill Vector with 
std::vector<int> firstNNumbers(int N){
    std::vector<int> list;
    for(int i=1; i <= N; ++i){
        list.push_back(i);
    }
    return list;
}

int main(){
    printHelloWorld();
    std::cout << addInts(3,10) << std::endl;
    int nameLength = calcLengthOfStr("Mishra, Eminent One");
    std::cout << "Length of Name: Mishra, Eminent One -> " << std::to_string(nameLength)  << std::endl; // instead of using "+", we use "<<" for putting strings tgt
    std::vector<int> vectorNum = firstNNumbers(20);
    std::cout << vectorNum.back() << std::endl;
    vectorNum.pop_back();
    std::cout << vectorNum.back() << std::endl;
    vectorNum.pop_back();
    std::cout << vectorNum.back() << std::endl;
    vectorNum.pop_back();
    }

// We have to recompile after we make changes