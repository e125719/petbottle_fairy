#include <iostream>
#include <objDetect.h>

using namespace std;
using namespace cv;

int main(int argc, char *argv[])
{
    objDetect detector;
    int t=0;
    
    detector.getBackImg();
    
    cout << "getBackImg completed" << endl;
    cout << "please press key" << endl;
    
    getchar();

    t = detector.detectObject();
    
    if((t=0)){
        detector.detectObject();
    }
    else{
        detector.makeTexture();
    }
    
    
    detector.getTexture();
    
    detector.getPosition();
    
}