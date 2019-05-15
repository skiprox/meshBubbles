#include "ofApp.h"

#define LINE_SIZE 300
#define FRAMERATE 60

//--------------------------------------------------------------
void ofApp::setup(){
	ofBackground(0);
	ofSetFrameRate(FRAMERATE);
	gui.setup();
	gui.add(noiseScale.set("Noise Scale", 0.01, 0.0, 0.05));
	gui.add(noiseMultiplier.set("Noise Multiplier", 5.0, 0.0, 10.0));
	gui.add(noiseFreq.set("Noise Frequency", 0.5, 0.0, 1.0));
	gui.add(colorNear.set("Color Near", ofColor(101, 114, 235), ofColor(0,0,0), ofColor(255,255,255)));
	gui.add(colorFar.set("Color Far", ofColor(203, 255, 181), ofColor(0,0,0), ofColor(255,255,255)));
	float width = ofGetWidth();
	float height = ofGetHeight();
	int rowsColsVal = LINE_SIZE;
	// Create the mesh
	for (int c = 0; c<rowsColsVal; c++){
	    for (int r = 0; r<rowsColsVal; r++){
	        glm::vec3 pos;      // grid centered at 0,0,0
	        float halfWidth     = width * 0.5;
	        float halfHeight    = height * 0.5;
	        pos.x = ofMap(r, 0, rowsColsVal-1, -halfWidth, halfWidth);
	        pos.y = ofMap(c, 0, rowsColsVal-1, halfHeight, -halfHeight);    // Y+ is up in 3D!
	        pos.z = 0;    // add depth later
	        // add the point to the mesh
	        mesh.addVertex(pos);
	        // add a color for the point
	        mesh.addColor(ofColor());
	        if (r > 0 && c > 0) {
	            int index = r * rowsColsVal + c;
	            // triangle 1
	            mesh.addIndex(index);               // A    - this pt
	            mesh.addIndex(index - 1);           // B    - + col
	            mesh.addIndex(index - rowsColsVal);        // C    - + row
	            // triangle 2
	            mesh.addIndex(index - 1);           // B
	            mesh.addIndex(index - 1 - rowsColsVal);    // D
	            mesh.addIndex(index - rowsColsVal);        // C
	        }
	    }
	}
	// Create the ekgLines vectors
	for (int i = 0; i < mesh.getVertices().size(); i++) {
		ekgLines.push_back(0.0);
		ekgLinesSaved.push_back(0.0);
		ekgLinesStable.push_back(0.0);
	}
}

//--------------------------------------------------------------
void ofApp::update(){
	// Create one new row,
	// but check if it's an action row or not
	if (ofGetFrameNum() % lineActionFrequency == 0) {
		for (int i = 0; i < LINE_SIZE; i++) {
			if (i % actionPointsDistance == 0) {
				float ran = ofRandom(-100.0, 100.0);
				ekgLines.push_back(ran);
				ekgLinesSaved.push_back(ran);
				ekgLinesStable.push_back(ran);
			} else {
				ekgLines.push_back(0.0);
				ekgLinesSaved.push_back(0.0);
				ekgLinesStable.push_back(0.0);
			}
		}
	} else {
		for (int i = 0; i < LINE_SIZE; i++) {
			ekgLines.push_back(0.0);
			ekgLinesSaved.push_back(0.0);
			ekgLinesStable.push_back(0.0);
		}
	}
	// Delete one row
	for (int i = 0; i < LINE_SIZE; i++) {
		ekgLines.erase(ekgLines.begin() + i);
		ekgLinesSaved.erase(ekgLinesSaved.begin() + i);
		ekgLinesStable.erase(ekgLinesStable.begin() + i);
	}
	// update all action values with noise
	for (int i = 0; i < ekgLines.size(); i++) {
		if (ekgLinesSaved[i] != 0.0) {
			float noise = ofSignedNoise ( mesh.getVertices()[i].x * noiseScale,    // x pos
	                        mesh.getVertices()[i].y * noiseScale,    // y pos
	                        ofGetElapsedTimef() * noiseFreq    // time (z) to animate
	                       ) * noiseMultiplier;
			//ekgLines[i] += noise;
			ekgLinesSaved[i] += noise;
			ekgLinesStable[i] += noise;
		}
	}
	// Animate the random high/low points to swing up towards the center,
	// and back down towards the end
	for (int i = 0; i < ekgLines.size(); i++) {
		if (ekgLinesSaved[i] != 0.0) {
			int row = (int)i/LINE_SIZE;
			float lerpValue = ofMap(abs((float)row - (float)LINE_SIZE/2.0), 0.0, (float)LINE_SIZE/2.0, ekgLinesStable[i], 0.0);
			ekgLinesSaved[i] = lerpValue;
		}
	}
	// Ease all the values
	easeAllValues();
	// Update the Z values and the colors
	updateZValue();
	updateColors();
}

//--------------------------------------------------------------
void ofApp::draw(){
	cam.begin();
    ofEnableDepthTest();
    mesh.draw();
    ofDisableDepthTest();
    cam.end();
    gui.draw();
}

//--------------------------------------------------------------
void ofApp::easeAllValues(){
	//int lineActionIncrementer = ofGetFrameNum() % lineActionFrequency;
	//cout << lineActionIncrementer << endl;
	int rowsColsVal = LINE_SIZE;
	for (int i = 0; i < rowsColsVal; i++) {
		// If we can find a non-zero value in the row,
		// then we know we have a row with action points
		if (ekgLinesSaved[i * rowsColsVal] != 0.0) {
			// Loop through once and ease all values in the row
			for (int j = 0; j < rowsColsVal; j++) {
				// This will be an action point
				if (j % actionPointsDistance == 0) {
					for (int x = 0; x < actionPointsDistance; x++) {
						int pointToEase = (i * rowsColsVal + j) + x;
						if (pointToEase >= 0 && pointToEase <= ekgLines.size()) {
							float easedValue = easeInOutQuad((x % (actionPointsDistance))/(float)(actionPointsDistance));
							float mappedValue = ofMap(easedValue, 0.0, 1.0, ekgLinesSaved[i * rowsColsVal + j], ekgLinesSaved[i * rowsColsVal + j + actionPointsDistance]);
							ekgLines[pointToEase] = mappedValue;
						}
					}
				}
			}
			// loop through again, easing all values in other rows
			for (int j = 0; j < rowsColsVal; j++) {
				// For every point in this row,
				// we want to ease the values around it in other rows
				for (int x = 0; x < lineActionFrequency; x++) {
					// pointToEase is the ekgLines index to ease
					int pointToEase = (i * rowsColsVal + j) + (x * rowsColsVal);
					// If the pointToEase is within our ekgLines vector
					if (pointToEase >= 0 && pointToEase <= ekgLines.size()) {
						float easedValue = easeInOutQuad((x % (lineActionFrequency))/(float)(lineActionFrequency));
						float mappedValue = ofMap(easedValue, 0.0, 1.0, ekgLines[i * rowsColsVal + j], ekgLines[i * rowsColsVal + j + (lineActionFrequency * rowsColsVal)]);
						ekgLines[pointToEase] = mappedValue;
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------
void ofApp::updateZValue(){
	for (int i = 0; i < mesh.getVertices().size(); i++) {
		glm::vec3& vertex = mesh.getVertices()[i];
		vertex.z = ekgLines[i];
	}
}

// --------------------------------
void ofApp::updateColors(){

    // map colors based on vertex z / depth

    for (int i = 0; i < mesh.getVertices().size(); i++){
        // 1 color per vertex
        glm::vec3& vertex = mesh.getVertices()[i];
        // get depth as percent of noise range
        float depthPercent = ofMap(vertex.z, -100.0, 100.0, 0, 1, true);    // map 0-1
        // lerp color
        ofColor color = colorFar.get().getLerped(colorNear.get(), depthPercent);
        mesh.setColor(i, color);        // set mesh color
    }

}

// --------------------------------
// t = value between [0, 1] to add ease to
// https://github.com/jesusgollonet/ofpennereasing/blob/master/PennerEasing/Quad.cpp
float ofApp::easeInOutQuad(float t) {
	if (t < 0.5) {
		return (2.0 * t * t);
	} else {
		return (-1.0 + (4.0 - 2.0 * t) * t);
	}

}
