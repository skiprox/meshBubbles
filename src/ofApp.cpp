#include "ofApp.h"

#define LINE_SIZE 300
#define FRAMERATE 60

//--------------------------------------------------------------
void ofApp::setup(){
	ofBackground(0);
	ofSetFrameRate(FRAMERATE);
	ofEnableAlphaBlending();
	gui.setup();
	gui.add(drawWires.set("Draw Wireframes", true, true, false));
	gui.add(noiseScale.set("Noise Scale", 0.00175, 0.0, 0.05));
	gui.add(noiseMultiplier.set("Noise Multiplier", 4.38711, 0.0, 10.0));
	gui.add(noiseFreq.set("Noise Frequency", 0.588711, 0.0, 1.0));
	gui.add(camPositionX.set("Cam Position X", -2.84632, -ofGetWidth(), ofGetWidth()));
	gui.add(camPositionY.set("Cam Position Y", 171.168, -ofGetHeight(), ofGetHeight()));
	gui.add(camPositionZ.set("Cam Position Z", -271.795, -2400.0, 1200.0));
	gui.add(camTargetX.set("Cam Target X", 0.0, -ofGetWidth(), ofGetWidth()));
	gui.add(camTargetY.set("Cam Target Y", 0.0, -ofGetHeight(), ofGetHeight()));
	gui.add(camTargetZ.set("Cam Target Z", 0.0, -16000.0, 24000.0));
	gui.add(camDistance.set("Cam Distance", 404.913, 0.0, 1200.0));
	gui.add(colorNear.set("Color Near", ofColor(0, 0, 0, 255), ofColor(0, 0, 0, 255), ofColor(255, 255, 255, 255)));
	gui.add(colorFar.set("Color Far", ofColor(17, 255, 241, 255), ofColor(0, 0, 0, 255), ofColor(255, 255, 255, 255)));
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
	// camera overrides
	cam.disableMouseInput();
	// mouse overrides
	ofHideCursor();
}

//--------------------------------------------------------------
void ofApp::update(){
	// Create one new row,
	// but check if it's an action row or not
	if (ofGetFrameNum() % lineActionFrequency == 0) {
		for (int i = 0; i < LINE_SIZE; i++) {
			if (i % actionPointsDistance == 0) {
				float ran = ofRandom(-100.0, 100.0);
				// We push 0.0 to the back of ekgLines,
				// bc we want to generate that based on the saved and stable
				// values in the ekglines vectors
				ekgLines.push_back(0.0);
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
	// Set up where the cam is looking
	cam.setPosition(ofPoint(camPositionX, camPositionY, camPositionZ));
	cam.setTarget(glm::vec3(camTargetX, camTargetY, camTargetZ));
	cam.setDistance(camDistance);
	cam.begin();
    ofEnableDepthTest();
    if (drawWires) {
    	mesh.drawWireframe();
    } else {
    	mesh.draw();
    }
    ofDisableDepthTest();
    cam.end();
    // gui.draw();
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
						int firstLinePoint = i * rowsColsVal + j;
						int secondLinePoint = i * rowsColsVal + j + (lineActionFrequency * rowsColsVal);
						float firstLineValue = ekgLines[firstLinePoint];
						float secondLineValue;
						if (secondLinePoint >= 0 && secondLinePoint <= ekgLines.size()) {
							secondLineValue = ekgLines[secondLinePoint];
						} else {
							secondLineValue = 0.0;
						}
						float mappedValue = ofMap(easedValue, 0.0, 1.0, firstLineValue, secondLineValue);
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

// --------------------------------
void ofApp::keyReleased(int key){
	ofNode target = cam.getTarget();
	cout << "THE CAM POSITION " << cam.getPosition() << endl;
	cout << "THE CAM TARGET " << target.getPosition() << endl;
	cout << "THE CAM DISTANCE " << cam.getDistance() << endl;
	cout << "THE CAM UP AXIS " << cam.getUpAxis() << endl;
	cout << "THE CAM RELATIVE Y AXIS " << cam.getRelativeYAxis() << endl;
	cout << "THE CAM TRANSLATION KEY " << cam.getTranslationKey() << endl;
}
