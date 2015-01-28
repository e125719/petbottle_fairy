#include <iostream>
#include "objDetect.h"

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
    objDetect detector;
    
    detector.getBackImg();
    
    cout << "getBackImg completed" << endl;
    cout << "please press key" << endl;
    
    getchar();
    
    detector.detectObject();

    detector.makeTexture();

    detector.getTexture();
    
    detector.getPosition();
    
}