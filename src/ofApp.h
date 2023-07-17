
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
//  - created additional methods and parameters for Joint Creation
//  - designed the Keyframe class
//  - implemented obj model rigging

#include "ofMain.h"
#include "box.h"
#include "Primitives.h"
#include "ofxGui.h"
#include "ofxAssimpModelLoader.h"

class Keyframe {
public:
	float frameRate = 60.0;
	float duration = 1.0;
	float frameNumber = 0.0;
	vector<SceneObject*> addedNodes;

	// Start and End Position Vectors
	vector<glm::vec3> nStartPos;
	vector<glm::vec3> nEndPos;

	// Start and End Rotation Vectors
	vector<glm::vec3> nStartRot;
	vector<glm::vec3> nEndRot;

	vector<glm::vec3> deltaPos;
	vector<glm::vec3> deltaRot;

	/**
	* Default Constructor
	*/
	Keyframe(){}

	/**
	* Helper method to return the index of the object in the SceneObject vector.
	*/
	int getIndex(SceneObject* obj)
	{
		int index = -1;
		for (int i = 0; i < addedNodes.size(); i++)
		{
			if (obj == addedNodes[i])
			{
				index = i;
			}
		}

		return index;
	}

	/**
	* Method to set the starting values of the inputted object for the keyframe.
	* 
	* If the object doesn't exist in the addedNodes vector, 
	* the object is pushed and the start and end values are the same.
	* 
	* If the object is found in the addedNodes vector, 
	* the position and rotation vectors are set to the position and rotation of the object
	*/
	void setStartValues(SceneObject* obj)
	{
		int i = getIndex(obj);

		if (i == -1)
		{
			addedNodes.push_back(obj);
			nStartPos.push_back(obj->position);
			nStartRot.push_back(obj->rotation);
			nEndPos.push_back(obj->position);
			nEndRot.push_back(obj->rotation);
		}
		else
		{
			nStartPos[i] = obj->position;
			nStartRot[i] = obj->rotation;
		}
		cout << obj->name << "'s starting valued saved" << endl;
	}

	/**
	* Method is similar to setStartValues but is for ending values.
	* The keyframes can be set in any order.
	*/
	void setEndValues(SceneObject* obj)
	{
		int i = getIndex(obj);

		if (i == -1)
		{
			addedNodes.push_back(obj);
			nStartPos.push_back(obj->position);
			nStartRot.push_back(obj->rotation);
			nEndPos.push_back(obj->position);
			nEndRot.push_back(obj->rotation);
		}
		else
		{
			nEndPos[i] = obj->position;
			nEndRot[i] = obj->rotation;
		}
		cout << obj->name << "'s ending valued saved" << endl;
	}

	/**
	* Method to reset the position and set the delta values.
	*/
	void setTheStage(bool rev, float second = 0.5)
	{
		duration = second;
		frameNumber = 0.0;
		deltaPos.clear();
		deltaRot.clear();

		// play foreward
		if (!rev)
		{
			for (int i = 0; i < addedNodes.size(); i++)
			{
				addedNodes[i]->position = nStartPos[i];
				addedNodes[i]->rotation = nStartRot[i];
				deltaPos.push_back((nEndPos[i] - nStartPos[i]) / (frameRate * duration));
				deltaRot.push_back((nEndRot[i] - nStartRot[i]) / (frameRate * duration));
			}
		}
		else // play in reverse
		{
			for (int i = 0; i < addedNodes.size(); i++)
			{
				addedNodes[i]->position = nEndPos[i];
				addedNodes[i]->rotation = nEndRot[i];
				deltaPos.push_back((nStartPos[i] - nEndPos[i]) / (frameRate * duration));
				deltaRot.push_back((nStartRot[i] - nEndRot[i]) / (frameRate * duration));
			}
		}
	}

	/**
	* Play the keyframe animation.
	* Return false on completion. (the animation is done and is not playing anymore)
	* Sinusoidal Interpolation function from http://gizma.com/easing/#sin3
	*/
	bool playback()
	{
		float cosFunc = (glm::cos(PI * frameNumber / (frameRate * duration)) - 1);
		for (int d = 0; d < addedNodes.size(); d++)
		{
			addedNodes[d]->position += -deltaPos[d] / 2.0 * cosFunc;
			addedNodes[d]->rotation += -deltaRot[d] / 2.0 * cosFunc;
		}
		frameNumber++;
		return frameNumber < (frameRate * duration * 2);
	}
};

class ofApp : public ofBaseApp{

	public:
		void setup();
		void update();
		void draw();

		void keyPressed(int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void mouseEntered(int x, int y);
		void mouseExited(int x, int y);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
		static void drawAxis(glm::mat4 transform = glm::mat4(1.0), float len = 1.0);
		bool mouseToDragPlane(int x, int y, glm::vec3 &point);
		void printChannels(SceneObject *);
		bool objSelected() { return (selected.size() ? true : false ); };
		
		// Additional Methods added by Calvin Quach
		void createJoint();
		void removeJoint();
		void printFamily(SceneObject *);
		void saveToFile();
		void loadFromFile();

		// Keyframe
		Keyframe animation;

		// models
		vector<Mesh> models;
		vector<SceneObject*> mods;
		bool bModelLoaded = false;

		// Gui
		ofxPanel gui;
		ofxFloatSlider dur;
		
		// File
		//
		ofFile skeleton;

		// Lights
		//
		ofLight light1;
	
		// Cameras
		//
		ofEasyCam  mainCam;
		ofCamera sideCam;
		ofCamera topCam;
		ofCamera  *theCam;    // set to current camera either mainCam or sideCam

		// Materials
		//
		ofMaterial material;


		// scene components
		//
		vector<SceneObject *> scene;
		vector<SceneObject *> selected;
		ofPlanePrimitive plane;

		// Addtional Parameters
		int jointNumber = 0;
		float radius = 0.2;

		// state
		bool bDrag = false;
		bool bHide = true;
		bool bAltKeyDown = false;
		bool bRotateX = false;
		bool bRotateY = false;
		bool bRotateZ = false;

		bool playing = false;
		bool animate = false;
		glm::vec3 lastPoint;
};