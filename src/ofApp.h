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
    	ofParameter<ofColor> colorNear;
    	ofParameter<ofColor> colorFar;
		vector<float> ekgLines;
		vector<float> ekgLinesSaved;
		// How often we have a line that has random values
		int lineActionFrequency = 30;
		// Distance between random values in a given line
		int actionPointsDistance = 30;
		
};
