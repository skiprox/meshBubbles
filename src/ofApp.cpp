#include "ofApp.h"

#define LINE_SIZE 300
#define FRAMERATE 24

//--------------------------------------------------------------
void ofApp::setup(){
	ofBackground(0);
	ofSetFrameRate(FRAMERATE);
	gui.setup();
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
			} else {
				ekgLines.push_back(0.0);
				ekgLinesSaved.push_back(0.0);
			}
		}
	} else {
		for (int i = 0; i < LINE_SIZE; i++) {
			ekgLines.push_back(0.0);
			ekgLinesSaved.push_back(0.0);
		}
	}
	// Delete one row
	for (int i = 0; i < LINE_SIZE; i++) {
		ekgLines.erase(ekgLines.begin() + i);
		ekgLinesSaved.erase(ekgLinesSaved.begin() + i);
	}
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
