#pragma once

#include "ofMain.h"
#include "rtl-sdr.h"
#include "ofxGui.h"
#include "ofxXmlSettings.h"

#define BYTES_TO_READ (0xFFFF + 1)
#define PHASE_SIZE 100

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void easeAllValues();
		void updateZValue();
		void updateColors();

		float easeInOutQuad(float t);

		// called in setup
		unsigned char buffer[BYTES_TO_READ];
		int bytesRead;
		rtlsdr_dev_t *device;
		double phaseData[PHASE_SIZE + 2];
		double phaseDataDifference[PHASE_SIZE];

		// mesh stuff
		ofEasyCam cam;
		ofMesh mesh;
		ofxPanel gui;
    	ofParameter<ofColor> colorNear;
    	ofParameter<ofColor> colorFar;
    	ofParameter<float> noiseFreq;
    	ofParameter<float> noiseMultiplier;
    	ofParameter<float> noiseScale;
		vector<float> ekgLines;
		vector<float> ekgLinesSaved;
		vector<float> ekgLinesStable;
		// How often we have a line that has random values
		int lineActionFrequency = 30;
		// Distance between random values in a given line
		int actionPointsDistance = 30;
		// Increamenter in the update function
		int updateIncrementer = 0;

		// not called until update
		int iIdx = 0;
		unsigned char iData[BYTES_TO_READ / 2];
		int qIdx = 0;
		unsigned char qData[BYTES_TO_READ / 2];
		
};
