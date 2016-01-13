#include "testApp.h"

#include <iostream>   // std::cout
#include <string>     // std::string, std::stof

//--------------------------------------------------------------
void testApp::setup(){
	ofSetVerticalSync(true);
	
	ofBackground(150);
	ofSetLogLevel(OF_LOG_VERBOSE);
    ofSetLineWidth(3);
	
	font.load("DIN.otf", 16);
	
	serial.listDevices();
	vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
	
	// this should be set to whatever com port your serial device is connected to.
	// (ie, COM4 on a pc, /dev/tty.... on linux, /dev/tty... on a mac)
	// arduino users check in arduino app....
	int baud = 115200;
	serial.setup(1, baud); //open the first device
	//serial.setup("COM4", baud); // windows example
	//serial.setup("/dev/tty.usbserial-A4001JEC", baud); // mac osx example
	//serial.setup("/dev/ttyUSB0", baud); //linux example
	histPosition = 0;
}

uint8_t testApp::findSerial(){
    while (!serial.available()) {
    }
    return serial.readByte();
}

float testApp::nextFloat() {
    char floatValue[10];
    uint8_t lastByte = findSerial();
    int j = 0;
    floatValue[j] = 0x00;
    while (lastByte != ',') { 
        memcpy (floatValue + j, (char *)&lastByte, 1);
        lastByte = findSerial();
        j++;
    }
    lastByte = findSerial();
    floatValue[j] = 0x00;
    return std::strtof (floatValue, NULL);
}

void testApp::findDelimiter(const char* name) {
    uint8_t lastByte = 0x00;
    while (lastByte != '\n') {
        lastByte = findSerial();
    }
    char delimiter[100];
    int i = 0;
    lastByte = findSerial();
    while (lastByte != ':') {
        delimiter [i] = lastByte;
        i++;
        lastByte = findSerial();
    }
    delimiter[i] = 0x00;
    if (strcmp(name, delimiter) != 0) {
        findDelimiter(name);
    }
}

bool testApp::readNextFloat (float* data, int index ) {
    uint8_t lastByte = 0x00;
    char floatValue[10];
    float tempFloat;
    lastByte = findSerial();
    while (lastByte != ',' && lastByte != '|') {
        int j = 0;
        floatValue[j] = 0x00;
        while (lastByte != ',' && lastByte != '|') {
            memcpy (floatValue + j, (char *)&lastByte, 1);
            lastByte = findSerial();
            j++;
        }
        floatValue[j] = 0x00;
        tempFloat = std::strtof (floatValue, NULL);
        data[index] = -tempFloat;
    }
    return lastByte != '|';
}
//--------------------------------------------------------------
void testApp::update(){
	
    findDelimiter("RUN");
    readNextFloat(&saThreshold, 0);
    readNextFloat(&saAverage, 0);
    readNextFloat(&saGain, 0);
    readNextFloat(&saRunning, 0);
    readNextFloat(&power, 0);
    histPosition++;
    saThresholdHist[histPosition] = saThreshold;
    saAverageHist[histPosition] = saAverage;
    saGainHist[histPosition] = saGain;
    saRunningHist[histPosition] = saRunning;
    smoothPower = smoothPower + (power - smoothPower) / 10.0;
    saPowerHist[histPosition] = smoothPower;
    
    // Collect audio input data
    findDelimiter("INP");
    int i = 0;
    while (readNextFloat(tower.audioSampleInput, i * 2) && i < FFT_LEN / 2) {
        i++;
    }
    
    // Collect frequency magnitude data
    findDelimiter("MAG");
    i = 0;
    while (readNextFloat(tower.audioMagnitudeOutput, i) && i < FFT_LEN / 2) {
        i++;
    }
    readNextFloat(&timeSpent, 0);
//    cout << timeSpent << endl;
}

//--------------------------------------------------------------
void testApp::draw(){
	
	ofSetColor(220);
    int rowStart = 50, rowSpacing = 40;

	string msg;
	msg += "TimeSpent: ";
    msg += ofToString(-timeSpent);
    msg += "ms";
	font.drawString(msg, 50, rowStart + rowSpacing * 0);
    msg.clear();
	msg += "Reference: ";
    msg += ofToString(-reference);
	font.drawString(msg, 50, rowStart + rowSpacing * 1);
    msg.clear();
	msg += "Gain: ";
    msg += ofToString(-gain);
	font.drawString(msg, 50, rowStart + rowSpacing * 2);
    ofSetHexColor(0x00FF00);
    //msg.clear();
	//msg += "Average: ";
    //msg += ofToString(gain);
	//font.drawString(msg, 50, 250);
    
    // Display input data
    ofSetHexColor(0x00FF00);
    for (int i = 0; i < fftLen / 2; i++) {
        ofDrawLine(i * 6 + 10, tower.audioSampleInput[i * 2] * 10 + 256, (i + 1) * 6 + 10, tower.audioSampleInput[i * 2 + 2] * 10 + 256);
    }
    
    // Compute frequency buckets
    tower.updateFrequencyBuckets();
    tower.updateSpectrumLevels();
    tower.displayData(true, true, true);
    
    // Display running audio magnitude controls
    int height = 850;
    int width = 49 * 16 - 10;
    for (int i = 0; i < histPosition - 1; i++) {
        // draw the line:
        ofSetLineWidth(1);
        ofSetColor(255, 0, 0);
        ofDrawLine(i + 10, height - -saThresholdHist[i], i + 11, height - -saThresholdHist[i + 1]);
        ofSetColor(255, 255, 0);
        ofDrawLine(i + 10, height - -saAverageHist[i], i + 11, height - -saAverageHist[i + 1]);
        ofSetColor(0, 255, 255);
        ofDrawLine(i + 10, height - -saGainHist[i] * 10, i + 11, height - -saGainHist[i + 1] * 10);
        ofSetColor(0, 255, 0);
        //ofDrawLine(i + 10, height - -saRunningHist[i], i + 11, height - -saRunningHist[i + 1]);
        ofSetColor(255, 0, 255);
        ofDrawLine(i + 10, height - -saPowerHist[i] * 0.1 + 1, i + 11, height - -saPowerHist[i + 1] * 0.1);
        ofSetLineWidth(3);
    }
    msg.clear();
    msg += "threshold: ";
    msg += ofToString(-saThresholdHist[histPosition]);
    ofSetColor(255, 0, 0);
	font.drawString(msg, 50, rowStart + rowSpacing * 3);
    msg.clear();
    msg += "movingAverage: ";
    msg += ofToString(-saAverageHist[histPosition]);
    ofSetColor(255, 255, 0);
	font.drawString(msg, 50, rowStart + rowSpacing * 4);
    msg.clear();
	msg += "gain: ";
    msg += ofToString(-saGainHist[histPosition]);
    ofSetColor(0, 255, 255);
	font.drawString(msg, 50, rowStart + rowSpacing * 5);
    msg.clear();
	msg += "runningAverage: ";
    msg += ofToString(-saRunningHist[histPosition]);
    ofSetColor(0, 255, 0);
	font.drawString(msg, 50, rowStart + rowSpacing * 6);
    msg.clear();
	msg += "power: ";
    msg += ofToString(-saPowerHist[histPosition]);
    ofSetColor(255, 0, 255);
	font.drawString(msg, 50, rowStart + rowSpacing * 7);
    // at the edge of the screen, go back to the beginning:
    if (histPosition >= 49 * 16) {
        histPosition = 0;
    }
    
    // Display LED commanded colors
    int i;
    int boxSize = 13;
    for (i = 0; i < bucketCount - 2; i++) {
        ofSetColor(tower.nonEmbedRed[i], tower.nonEmbedGreen[i], tower.nonEmbedBlue[i]);
        ofDrawRectangle(i * 16 + 10, 512 - tower.runningMagnitude[i] / 2, 10, tower.runningMagnitude[i] / 2);
        ofDrawRectangle(80 * 10 + 20, (LED_COUNT - i) * boxSize, boxSize, boxSize);
        if ((i + 1) % 10 == 0) {
            msg.clear();
            msg += ofToString(i + 1);
            font.drawString(msg, i * 16 + 10, 532);
        }
    }
    
    // Display frequency magnitudes
    ofSetHexColor(0x0000FF);
    for (int i = 0; i < fftLen / 2 - 1; i++) {
        ofDrawLine(i * 6 + 10, tower.audioMagnitudeOutput[i] + 700, i * 6 + 6 + 10, tower.audioMagnitudeOutput[i + 1] + 700);
        if ((i + 1) % 20 == 0) {
            msg.clear();
            msg += ofToString(i + 1);
            font.drawString(msg, i * 6 + 10, 720);
        }
    }
}

//--------------------------------------------------------------
void testApp::keyPressed  (int key){ 
	
    switch (key) {
        case 357:
            // Key up
//            gain -= 2.0;
            break;
        case 359:
            // Key down
//            gain += 2.0;
            break;
        case 356:
            // Key right
            reference -= 2.0;
            break;
        case 358:
            // Key down
            reference += 2.0;
            break;
        default:
            break;
    }
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
	
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y){
	
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
	
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
	
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
	
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 
	
}

