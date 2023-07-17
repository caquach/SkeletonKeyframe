
//
//  Starter file for Project 3 - Skeleton Builder
//
//  This file includes functionality that supports selection and translate/rotation
//  of scene objects using the mouse.
//
//  Modifer keys for rotatation are x, y and z keys (for each axis of rotation)
//
//  (c) Kevin M. Smith  - 24 September 2018
// 
//  Calvin Quach - 7 December 2022
//  - implemented additional methods and parameters as referenced in ofApp.h
//

#include "ofApp.h"

/**
* Method that sets the scene along with the cameras and lights.
* Framerate is set to 60.
*/
void ofApp::setup() {
	ofSetFrameRate(60);
	ofSetBackgroundColor(ofColor::black);
	ofEnableDepthTest();
	mainCam.setDistance(15);
	mainCam.setNearClip(.1);
	
	sideCam.setPosition(40, 0, 0);
	sideCam.lookAt(glm::vec3(0, 0, 0));
	topCam.setNearClip(.1);
	topCam.setPosition(0, 16, 0);
	topCam.lookAt(glm::vec3(0, 0, 0));
	ofSetSmoothLighting(true);


	// setup one point light
	//
	light1.enable();
	light1.setPosition(5, 5, 0);
	light1.setDiffuseColor(ofColor(255.f, 255.f, 255.f));
	light1.setSpecularColor(ofColor(255.f, 255.f, 255.f));

	theCam = &mainCam;
	
	mainCam.disableMouseInput();

	//  create a scene consisting of a ground plane with 2x2 blocks
	//  arranged in semi-random positions, scales and rotations
	//
	// ground plane
	//
	scene.push_back(new Plane(glm::vec3(0, -2, 0), glm::vec3(0, 1, 0)));

	gui.setup();
	gui.add(dur.setup("Animation Duration", 1, 0.5, 3.0));
}

 
/**
* Method to update the positions and rotations of animations and models if applicable.
* This update is called by every frame (60 frames per second)
*/
void ofApp::update(){
	if (playing)
	{
		playing = animation.playback();
	}

	for (int i = 0; i < mods.size(); i++)
	{
		if (models[i].name.compare("engineerfriend.obj") == 0)
		{
			models[i].mesh.setPosition(mods[i]->getPosition().x - 0.1, mods[i]->getPosition().y - 0.25, mods[i]->getPosition().z + 0.3);
			models[i].mesh.setRotation(0, mods[i]->rotation.x - 90, 1, 0, 0);
		}
		else
		{
			models[i].mesh.setPosition(mods[i]->getPosition().x, mods[i]->getPosition().y - 0.25, mods[i]->getPosition().z);
			models[i].mesh.setRotation(0, mods[i]->rotation.x, 1, 0, 0);
		}
		models[i].mesh.setRotation(1, mods[i]->rotation.z, 0, -1, 0);
		models[i].mesh.setRotation(2, mods[i]->rotation.y, 0, 0, 1);
	}
}

//--------------------------------------------------------------
void ofApp::draw(){

	// draw gui
	glDepthMask(false);
	if (!bHide) gui.draw();
	glDepthMask(true);

	theCam->begin();
	ofNoFill();
	drawAxis();
	ofEnableLighting();

	//  draw the objects in scene
	//
	material.begin();
	ofFill();
	for (int i = 0; i < scene.size(); i++) {
		if (objSelected() && scene[i] == selected[0])
			ofSetColor(ofColor::white);
		else ofSetColor(scene[i]->diffuseColor);
		scene[i]->draw();
	}

	for (int i = 0; i < models.size(); i++)
	{
		models[i].draw();
	}

	material.end();
	ofDisableLighting();
	theCam->end();
}

// 
// Draw an XYZ axis in RGB at transform
//
void ofApp::drawAxis(glm::mat4 m, float len) {

	ofSetLineWidth(1.0);

	// X Axis
	ofSetColor(ofColor(255, 0, 0));
	ofDrawLine(glm::vec3(m*glm::vec4(0, 0, 0, 1)), glm::vec3(m*glm::vec4(len, 0, 0, 1)));


	// Y Axis
	ofSetColor(ofColor(0, 255, 0));
	ofDrawLine(glm::vec3(m*glm::vec4(0, 0, 0, 1)), glm::vec3(m*glm::vec4(0, len, 0, 1)));

	// Z Axis
	ofSetColor(ofColor(0, 0, 255));
	ofDrawLine(glm::vec3(m*glm::vec4(0, 0, 0, 1)), glm::vec3(m*glm::vec4(0, 0, len, 1)));
}

// print C++ code for obj tranformation channels. (for debugging);
//
void ofApp::printChannels(SceneObject *obj) {
	cout << "position = glm::vec3(" << obj->position.x << "," << obj->position.y << "," << obj->position.z << ");" << endl;
	cout << "rotation = glm::vec3(" << obj->rotation.x << "," << obj->rotation.y << "," << obj->rotation.z << ");" << endl;
	cout << "scale = glm::vec3(" << obj->scale.x << "," << obj->scale.y << "," << obj->scale.z << ");" << endl;
}

/**
* Helper Method to print the family tree of the of the selected node.
* The parent and children of node will be printed if applicable.
*/
void ofApp::printFamily(SceneObject* obj)
{
	cout << obj->name << " family:" << endl;
	if (obj->parent != NULL)
	{
		cout << "Parent: " << obj->parent->name << endl;
	}

	if (obj->childList.size() > 0)
	{
		cout << "Children: ";
	}

	for (int i = 0; i < obj->childList.size(); i++)
	{
		cout << obj->childList[i]->name << ", ";
	}
	cout << endl << endl;
}

/**
* Method to save the current configuration of the joints to a file.
* The file created/saved is called model.txt
* Each joint is saved in the format:
* create -joint joint1 -rotate <0, 0, 0> -translate <0.04, -1.01, 0> -parent joint0;
*/
void ofApp::saveToFile()
{
	// check if root exists
	bool bRootExists = false;
	for (int i = 1; i < scene.size(); i++)
	{
		if (scene[i]->parent == NULL)
		{
			bRootExists = true;
			break;
		}
	}
	if (!bRootExists)
	{
		cout << "Root does not exist, save failed" << endl;
		return;
	}

	// creates and build fresh file
	skeleton.open("model.txt", ofFile::WriteOnly);
	skeleton.create(); 
	
	string parentName;
	for (int i = 1; i < scene.size(); i++)
	{
		parentName = "";
		if (scene[i]->parent != NULL)
		{
			parentName = scene[i]->parent->name;
		}

		// format each number to two decimal places
		glm::vec3 formattedRot = scene[i]->rotation;
		formattedRot.x = ((int)(formattedRot.x * 100 + .5)) / 100.0f;
		formattedRot.y = ((int)(formattedRot.y * 100 + .5)) / 100.0f;
		formattedRot.z = ((int)(formattedRot.z * 100 + .5)) / 100.0f;

		glm::vec3 formattedPos = scene[i]->position;
		formattedPos.x = ((int)(formattedPos.x * 100 + .5)) / 100.0f;
		formattedPos.y = ((int)(formattedPos.y * 100 + .5)) / 100.0f;
		formattedPos.z = ((int)(formattedPos.z * 100 + .5)) / 100.0f;

		skeleton << "create -joint " << scene[i]->name <<
			" -rotate <" << formattedRot <<
			"> -translate <" << formattedPos <<
			"> -parent " << parentName << ";";

		if (i != scene.size() - 1)
		{
			skeleton << endl;
		}
	}
	skeleton.close();
	cout << "Sucessfully saved joints!" << endl;
}

/**
* Method to load a saved joint configuration file overwriting the any current joints present.
* This method will parse each line from the input file and create a joint system.
* All Keyframes and Models are deleted upon loading.
*/
void ofApp::loadFromFile()
{
	if (!skeleton.doesFileExist("model.txt"))
	{
		cout << "The file doesn't exist, no model to load!";
		return;
	}

	skeleton.open("model.txt", ofFile::Append, false);

	// clear any objects on screen and reset keyframes
	//
	scene.clear();
	scene.push_back(new Plane(glm::vec3(0, -2, 0), glm::vec3(0, 1, 0)));
	animation.addedNodes.clear();
	animation.nStartPos.clear();
	animation.nEndPos.clear();
	animation.nStartRot.clear();
	animation.nEndRot.clear();
	models.clear();
	mods.clear();

	// read from file into buffer
	ofBuffer buffer = ofBufferFromFile(skeleton);
	string line = buffer.getNextLine();
	vector<string> splitted;

	while (!line.empty())
	{
		// split line by spaces
		stringstream ss(line);
		string word;
		while (ss >> word)
		{
			splitted.push_back(word);
		}

		// joint creation
		glm::vec3 lPos = glm::vec3(
			stof(splitted[8].substr(1, splitted[8].size() - 1)),
			stof(splitted[9].substr(0, splitted[9].size() - 1)),
			stof(splitted[10].substr(0, splitted[10].size() - 1)));
		
		Joint* loaded = new Joint(lPos, radius);
		loaded->name = splitted[2];
		loaded->rotation = glm::vec3(
			stof(splitted[4].substr(1, splitted[4].size() - 1)), 
			stof(splitted[5].substr(0, splitted[5].size() - 1)),
			stof(splitted[6].substr(0, splitted[6].size() - 1)));

		// parent child links
		string pName = splitted[12].substr(0, splitted[12].size() - 1);
		if (!pName.empty())
		{
			for (int j = 1; j < scene.size(); j++)
			{
				if (scene[j]->name.compare(pName) == 0)
				{
					scene[j]->addChild(loaded);
				}
			}
		}

		// push object onto scene
		scene.push_back(loaded);

		// sync jointNumber count
		jointNumber = max(jointNumber, stoi(splitted[2].substr(splitted[2].size() - 1, splitted[2].size())));
	
		// next line
		splitted.clear();
		line = buffer.getNextLine();
	}

	jointNumber++;
	skeleton.close();
	cout << "Sucessfully loaded joints!" << endl;
}

/**
* Method to create a joint at the mouse point.
* If a joint is selected, then that joint is the parent of the created node.
*/
void ofApp::createJoint()
{
	glm::vec3 point;
	mouseToDragPlane(mouseX, mouseY, point);
	Joint* created = new Joint(glm::vec3(0, 0, 0), radius, ofColor::blue);
	created->name = created->name + std::to_string(jointNumber);
	
	if (objSelected()) // create parent child relation between nodes
	{
		// created point is set at mouse point regardless of level of tree
		created->setPosition(point - selected[0]->getPosition());
		selected[0]->addChild(created);
	}
	else
	{
		created->setPosition(point);
	}
	scene.push_back(created);
	jointNumber++;
}

/**
* Method to delete a selected joint.
* Corresponding parent-child relationships will be removed
* Orphaned children become children of the parent of the deleted joint if applicable.
* All Keyframes and obj models are deleted.
*/
void ofApp::removeJoint()
{
	// if nothing selected, exit function
	if (!objSelected())
	{
		return;
	}

	int eraseIndex = -1;
	int re = -1;
	// remove corresponding links of selected node
	for (int i = 1; i < scene.size(); i++)
	{
		if (selected[0] == scene[i])
		{
			eraseIndex = i;
			if (scene[i]->parent != NULL) // is not root node
			{
				// parent of selected will have children of selected as their children
				for (int j = 0; j < scene[i]->childList.size(); j++)
				{
					scene[i]->parent->addChild(scene[i]->childList[j]);
					re = scene[i]->childList.size();
				}

				// delete links between deleted and parent
				if (scene[i]->childList.size() == 0) // if selected has no child
				{
					scene[i]->parent->childList.erase(scene[i]->parent->childList.begin() + (scene[i]->parent->childList.size() - re - 2));
				}
				else
				{
					scene[i]->parent->childList.erase(scene[i]->parent->childList.begin() + (scene[i]->parent->childList.size() - re - 1));
				}
			}
			else // is root node
			{
				for (int j = 0; j < scene[i]->childList.size(); j++)
				{
					scene[i]->childList[j]->parent = NULL;
				}
			}
		}
	}

	// erasw selected node
	scene.erase(scene.begin() + eraseIndex);

	// remove selection and keyframes upon delete
	selected.clear(); 
	animation.addedNodes.clear();
	animation.nStartPos.clear();
	animation.nEndPos.clear();
	animation.nStartRot.clear();
	animation.nEndRot.clear();
	models.clear();
	mods.clear();
}
//--------------------------------------------------------------
void ofApp::keyReleased(int key){

	switch (key) {
	case OF_KEY_ALT:
		bAltKeyDown = false;
		mainCam.disableMouseInput();
		break;
	case 'X':
	case 'x':
		bRotateX = false;
		break;
	case 'Y':
	case 'y':
		bRotateY = false;
		break;
	case 'Z':
	case 'z':
		bRotateZ = false;
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key) {
	switch (key) {
	case '1':
		if (objSelected()) animation.setStartValues(selected[0]);
		break;
	case '2':
		if (objSelected()) animation.setEndValues(selected[0]);
		break;
	case 'C':
	case 'c':
		if (mainCam.getMouseInputEnabled()) mainCam.disableMouseInput();
		else mainCam.enableMouseInput();
		break;
	case 'F':
	case 'f':
		ofToggleFullscreen();
		break;
	case 'h':
		bHide = !bHide;
		break;
	case 'i':
		if (objSelected()) printFamily(selected[0]);
		break;
	case 'J':
	case 'j':
		createJoint();
		break;
	case 'L':
	case 'l':
		loadFromFile();
	case 'n':
		break;
	case 'p':
		if (!playing)
		{
			playing = true;
			animation.setTheStage(false, dur / 2.0);
		}
		break;
	case 'r':
		if (!playing)
		{
			playing = true;
			animation.setTheStage(true, dur / 2.0);
		}
		break;
	case 'S':
	case 's':
		saveToFile();
		break;
	case 'X':
	case 'x':
		bRotateX = true;
		break;
	case 'Y':
	case 'y':
		bRotateY = true;
		break;
	case 'Z':
	case 'z':
		bRotateZ = true;
		break;
	case OF_KEY_F1: 
		theCam = &mainCam;
		break;
	case OF_KEY_F2:
		//theCam = &sideCam;
		break;
	case OF_KEY_F3:
		//theCam = &topCam;
		break;
	case OF_KEY_ALT:
		bAltKeyDown = true;
		if (!mainCam.getMouseInputEnabled()) mainCam.enableMouseInput();
		break;
	case OF_KEY_BACKSPACE:
		removeJoint();
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button) {

	if (objSelected() && bDrag) {
		glm::vec3 point; 
		mouseToDragPlane(x, y, point);
		if (bRotateX) {
			selected[0]->rotation += glm::vec3((point.x - lastPoint.x) * 20.0, 0, 0);
		}
		else if (bRotateY) {
			selected[0]->rotation += glm::vec3(0, (point.x - lastPoint.x) * 20.0, 0);
		}
		else if (bRotateZ) {
			selected[0]->rotation += glm::vec3(0, 0, (point.x - lastPoint.x) * 20.0);
		}
		else {
			selected[0]->position += (point - lastPoint);
		}
		lastPoint = point;
	}

}

//  This projects the mouse point in screen space (x, y) to a 3D point on a plane
//  normal to the view axis of the camera passing through the point of the selected object.
//  If no object selected, the plane passing through the world origin is used.
//
bool ofApp::mouseToDragPlane(int x, int y, glm::vec3 &point) {
	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	float dist;
	glm::vec3 pos;
	if (objSelected()) {
		pos = selected[0]->position;
	}
	else pos = glm::vec3(0, 0, 0);
	if (glm::intersectRayPlane(p, dn, pos, glm::normalize(theCam->getZAxis()), dist)) {
		point = p + dn * dist;
		return true;
	}
	return false;
}

//--------------------------------------------------------------
//
// Provides functionality of single selection and if something is already selected,
// sets up state for translation/rotation of object using mouse.
//
void ofApp::mousePressed(int x, int y, int button){

	// if we are moving the camera around, don't allow selection
	//
	if (mainCam.getMouseInputEnabled()) return;

	// clear selection list
	//
	selected.clear();

	//
	// test if something selected
	//
	vector<SceneObject *> hits;

	glm::vec3 p = theCam->screenToWorld(glm::vec3(x, y, 0));
	glm::vec3 d = p - theCam->getPosition();
	glm::vec3 dn = glm::normalize(d);

	// check for selection of scene objects
	//
	for (int i = 0; i < scene.size(); i++) {
		
		glm::vec3 point, norm;
		
		//  We hit an object
		//
		if (scene[i]->isSelectable && scene[i]->intersect(Ray(p, dn), point, norm)) {
			hits.push_back(scene[i]);
		}
	}


	// if we selected more than one, pick nearest
	//
	SceneObject *selectedObj = NULL;
	if (hits.size() > 0) {
		selectedObj = hits[0];
		float nearestDist = std::numeric_limits<float>::infinity();
		for (int n = 0; n < hits.size(); n++) {
			float dist = glm::length(hits[n]->position - theCam->getPosition());
			if (dist < nearestDist) {
				nearestDist = dist;
				selectedObj = hits[n];
			}	
		}
	}
	if (selectedObj) {
		selected.push_back(selectedObj);
		bDrag = true;
		mouseToDragPlane(x, y, lastPoint);
	}
	else {
		selected.clear();
	}
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
	bDrag = false;

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

/**
* Method to drag an obj file into the scene at the mouse point.
* The model is bound to a selected joint.
* Joints may only have one object bound to them.
*/
void ofApp::dragEvent(ofDragInfo dragInfo){
	if (!objSelected())
	{
		return;
	}

	for (int i = 0; i < mods.size(); i++)
	{
		if (selected[0] == mods[i])
		{
			return;
		}
	}

	ofxAssimpModelLoader model;
	if (model.loadModel(dragInfo.files[0])) {
		model.setScaleNormalization(false);
		model.setScale(0.2, 0.2, 0.2);
		model.setPosition(0, 0, 0);

		int slash = 0;
		for (int i = dragInfo.files[0].length() - 1; i >= 0; i--)
		{
			// find the first backslash from the end
			if (dragInfo.files[0].at(i) == '\\')
			{
				break;
			}
			slash++;
		}

		string temp = dragInfo.files[0].substr(dragInfo.files[0].length() - slash, dragInfo.files[0].length());
		if (temp.compare("engineerfriend.obj") == 0)
		{
			model.setScale(0.01, 0.01, 0.01);
		}
		models.push_back(Mesh(model, temp));
		mods.push_back(selected[0]);
	}
}