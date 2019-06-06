#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "ofxXmlSettings.h"

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void easeAllValues();
		void updateZValue();
		void updateColors();

		float easeInOutQuad(float t);

		// mesh stuff
		ofEasyCam cam;
		ofMesh mesh;
		ofxPanel gui;
		ofParameter<bool> drawWires;
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
		
};
