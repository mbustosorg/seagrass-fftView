#pragma once

#include "ofMain.h"
#include "towerPatterns.h"

#define fftLen 256
#define bucketCount 51

class testApp : public ofBaseApp {
	
	public:
		void setup();
		void update();
		void draw();
        void updateVolumeData();
        uint8_t findSerial();
        float nextFloat();
    
		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
        void findDelimiter(const char* name);
        bool readNextFloat (float* data, int index );
    
		ofTrueTypeFont		font;
    
        void eqColor(int index, float value);
		
		ofSerial	serial;
    
        towerPatterns tower;
        int histPosition;
        float saPowerHist[1024], saThresholdHist[1024], saAverageHist[1024], saGainHist[1024], saRunningHist[1024];
        float saThreshold, saAverage, saGain, saRunning;
        float timeSpent;
        float power = 0.0;
        float smoothPower = 0.0;
        float gain = -20.0;
        float reference = -10.0;
        float average = 0.0;

};

