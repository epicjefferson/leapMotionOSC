#include "ofApp.h"


//--------------------------------------------------------------
void ofApp::setup(){
    
    ofSetFrameRate(60);
    ofSetVerticalSync(true);
    ofSetLogLevel(OF_LOG_VERBOSE);
    
    leap.open();
    
    cam.setOrientation(ofPoint(0, 0, 0));
    
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    
    //setup OSC
    oscSender.setup(HOST, PORT);
    
    //smooth lines
    ofEnableSmoothing();
}

//--------------------------------------------------------------
void ofApp::update(){
    fingersFound.clear();
    
    //here is a simple example of getting the hands and drawing each finger and joint
    //the leap data is delivered in a threaded callback - so it can be easier to work with this copied hand data
    
    //if instead you want to get the data as it comes in then you can inherit ofxLeapMotion and implement the onFrame method.
    //there you can work with the frame data directly.
    
    
    
    //Option 1: Use the simple ofxLeapMotionSimpleHand - this gives you quick access to fingers and palms.
    
    
    simpleHands = leap.getSimpleHands();
    //
    if( leap.isFrameNew() && simpleHands.size() ){
        
        leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2);
        leap.setMappingY(90, 490, -ofGetHeight()/2, ofGetHeight()/2);
        leap.setMappingZ(-150, 150, -200, 200);
        
        fingerType fingerTypes[] = {THUMB, INDEX, MIDDLE, RING, PINKY};
        
        for(int i = 0; i < simpleHands.size(); i++){
            for (int f=0; f<5; f++) {
                int id = simpleHands[i].fingers[ fingerTypes[f] ].id;
                ofPoint mcp = simpleHands[i].fingers[ fingerTypes[f] ].mcp; // metacarpal
                ofPoint pip = simpleHands[i].fingers[ fingerTypes[f] ].pip; // proximal
                ofPoint dip = simpleHands[i].fingers[ fingerTypes[f] ].dip; // distal
                ofPoint tip = simpleHands[i].fingers[ fingerTypes[f] ].tip; // fingertip
                fingersFound.push_back(id);
            }
        }
    }
    
    
    
    //Option 2: Work with the leap data / sdk directly - gives you access to more properties than the simple approach
    //uncomment code below and comment the code above to use this approach. You can also inhereit ofxLeapMotion and get the data directly via the onFrame callback.
    
    //     vector <Hand> hands = leap.getLeapHands();
    //     if( leap.isFrameNew() && hands.size() ){
    //
    //         //leap returns data in mm - lets set a mapping to our world space.
    //         //you can get back a mapped point by using ofxLeapMotion::getMappedofPoint with the Leap::Vector that tipPosition returns
    //         leap.setMappingX(-230, 230, -ofGetWidth()/2, ofGetWidth()/2);
    //         leap.setMappingY(90, 490, -ofGetHeight()/2, ofGetHeight()/2);
    //         leap.setMappingZ(-150, 150, -200, 200);
    //
    //         fingerType fingerTypes[] = {THUMB, INDEX, MIDDLE, RING, PINKY};
    //
    //         for(int i = 0; i < hands.size(); i++){
    //             for(int j = 0; j < 5; j++){
    //                 ofPoint pt;
    //
    //                 const Finger & finger = hands[i].fingers()[ fingerTypes[j] ];
    //
    //                 //here we convert the Leap point to an ofPoint - with mapping of coordinates
    //                 //if you just want the raw point - use ofxLeapMotion::getofPoint
    //                 pt = leap.getMappedofPoint( finger.tipPosition() );
    ////                 pt = leap.getMappedofPoint( finger.jointPosition(finger.JOINT_DIP) );
    //
    //                 fingersFound.push_back(finger.id());
    //             }
    //         }
    //     }
    
    if(senderSwitch){
        //Pressing spacebar activates send all leap data over OSC
        hands = leap.getLeapHands();
        
        if (hands.size() > 0) {
            for (int h = 0; h < hands.size(); h++){
                
                ofxOscMessage m;
                
                // Get the current hand
                Hand & hand = hands[h];
                
                string handType = hand.isLeft() ? "Left" : "Right";     //pretty sweet way of knowing which hand
                
                //send handType over OSC
                m.setAddress("/handType");
                m.addStringArg(handType);
                oscSender.sendMessage(m);
                m.clear();
                
                //get palm position
                //cout << ", palm" << hand.palmPosition() << endl;
                
                m.setAddress("/" + handType);
                m.addStringArg("/palm");
                m.addFloatArg(hand.palmPosition()[0]);      // X
                m.addFloatArg(hand.palmPosition()[1]);      // Y
                m.addFloatArg(hand.palmPosition()[2]);      // Z
                oscSender.sendMessage(m);
                m.clear();
                
                
                //get grab Strength
                m.setAddress("/" + handType);
                m.addStringArg("/grabStrength");
                m.addFloatArg(hand.grabStrength());
                oscSender.sendMessage(m);
                m.clear();
                
                
                // Calculate the hand's pitch, roll, and yaw angles
                const Vector normal = hand.palmNormal();
                const Vector direction = hand.direction();
                
                
                m.setAddress("/" + handType);
                m.addStringArg("/pitch");
                m.addFloatArg(direction.pitch() * RAD_TO_DEG);
                oscSender.sendMessage(m);
                m.clear();
                
                m.setAddress("/" + handType);
                m.addStringArg("/roll");
                m.addFloatArg(normal.roll() * RAD_TO_DEG);
                oscSender.sendMessage(m);
                m.clear();
                
                m.setAddress("/" + handType);
                m.addStringArg("/yaw");
                m.addFloatArg(direction.yaw() * RAD_TO_DEG);
                oscSender.sendMessage(m);
                m.clear();
                
                
                // Get the Arm bone
                Arm arm = hand.arm();
                
                
                m.setAddress("/" + handType);
                m.addStringArg("/armDirection");
                m.addFloatArg(arm.direction()[0]);       // x
                m.addFloatArg(arm.direction()[1]);       // y
                m.addFloatArg(arm.direction()[2]);       // z
                oscSender.sendMessage(m);
                m.clear();
                
                m.setAddress("/" + handType);
                m.addStringArg("/wristPosition");
                m.addFloatArg(arm.wristPosition()[0]);       // x
                m.addFloatArg(arm.wristPosition()[1]);       // y
                m.addFloatArg(arm.wristPosition()[2]);       // z
                oscSender.sendMessage(m);
                m.clear();
                
                m.setAddress("/" + handType);
                m.addStringArg("/elbowPosition");
                m.addFloatArg(arm.elbowPosition()[0]);       // x
                m.addFloatArg(arm.elbowPosition()[1]);       // y
                m.addFloatArg(arm.elbowPosition()[2]);       // z
                oscSender.sendMessage(m);
                m.clear();
                
                
                // Get fingers
                const FingerList fingers = hand.fingers();
                for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
                    const Finger finger = *fl;
                    
                    m.setAddress("/" + handType);
                    m.addStringArg("/fingers");
                    m.addStringArg("/" + fingerNames[finger.type()]);
                    m.addStringArg("/tip");
                    m.addFloatArg(finger.tipPosition()[0]) ;       // x
                    m.addFloatArg(finger.tipPosition()[1]) ;       // Y
                    m.addFloatArg(finger.tipPosition()[2]) ;       // Z
                    oscSender.sendMessage(m);
                    m.clear();
                    
                    // Get finger bones
                    for (int b = 0; b < 4; ++b) {
                        Bone::Type boneType = static_cast<Bone::Type>(b);
                        Bone bone = finger.bone(boneType);
                        
                        
                        
                        //sending current bones' previous joint
                        m.setAddress("/" + handType);
                        m.addStringArg("/fingers");
                        m.addStringArg("/" + fingerNames[finger.type()]);
                        m.addStringArg("/" + boneNames[boneType]);
                        m.addStringArg("/prevJoint");
                        m.addFloatArg(bone.prevJoint()[0]);       // x
                        m.addFloatArg(bone.prevJoint()[1]);       // y
                        m.addFloatArg(bone.prevJoint()[2]);       // z
                        oscSender.sendMessage(m);
                        m.clear();
                        
                        //sending current bones' next joint
                        m.setAddress("/" + handType);
                        m.addStringArg("/fingers");
                        m.addStringArg("/" + fingerNames[finger.type()]);
                        m.addStringArg("/" + boneNames[boneType]);
                        m.addStringArg("/nextJoint");
                        m.addFloatArg(bone.nextJoint()[0]);       // x
                        m.addFloatArg(bone.nextJoint()[1]);       // y
                        m.addFloatArg(bone.nextJoint()[2]);       // z
                        oscSender.sendMessage(m);
                        m.clear();
                        
                    }
                    
                }
                
            }
        }
    }
    
    //IMPORTANT! - tell ofxLeapMotion that the frame is no longer new.
    leap.markFrameAsOld();
    
}

//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
void ofApp::draw(){
    
    ofBackground(166,170,176);
    ofSetColor(50);
    ofDrawBitmapString("Leap Connected?: " + ofToString(leap.isConnected()), 20, 20);
    ofDrawBitmapString("Hands found: " + ofToString(simpleHands.size()), 20, 40);
    ofDrawBitmapString("Press spacebar to send OSC, NO=0, YES=1: " + ofToString(senderSwitch), 20, 60);
    
    
    cam.begin();
    cam.setOrientation(ofPoint(-20,0,0));
    
    //draw leap grid
    ofPushMatrix();
    ofRotate(90, 0, 0, 1);
    ofSetColor(237, 227, 238);
    ofDrawGridPlane(300, 5, false);
    ofPopMatrix();
    
    fingerType fingerTypes[] = {THUMB, INDEX, MIDDLE, RING, PINKY};
    
    
    
    //draw hand
    for(int i = 0; i < simpleHands.size(); i++){
        
        bool isLeft        = simpleHands[i].isLeft;
        ofPoint handPos    = simpleHands[i].handPos;
        ofPoint handNormal = simpleHands[i].handNormal;
        
        //draw sphere in center of palm and line for palm direction
        ofSetColor(0,0,230);
        ofDrawSphere(handPos.x, handPos.y, handPos.z, 10);
        ofSetColor(180,236,40);
        ofDrawArrow(handPos, handPos + 50*handNormal);
        
        for (int f=0; f < 5; f++) {
            ofPoint mcp = simpleHands[i].fingers[ fingerTypes[f] ].mcp;  // metacarpal
            ofPoint pip = simpleHands[i].fingers[ fingerTypes[f] ].pip;  // proximal
            ofPoint dip = simpleHands[i].fingers[ fingerTypes[f] ].dip;  // distal
            ofPoint tip = simpleHands[i].fingers[ fingerTypes[f] ].tip;  // fingertip
            
            //draw joints
            ofSetColor(0);
            ofDrawSphere(mcp.x, mcp.y, mcp.z, 7);
            ofDrawSphere(pip.x, pip.y, pip.z, 7);
            ofDrawSphere(dip.x, dip.y, dip.z, 7);
            ofDrawSphere(tip.x, tip.y, tip.z, 7);
            
            //drawbones
            ofSetColor(0);
            ofSetLineWidth(5);
            ofLine(mcp.x, mcp.y, mcp.z, pip.x, pip.y, pip.z);
            ofLine(pip.x, pip.y, pip.z, dip.x, dip.y, dip.z);
            ofLine(dip.x, dip.y, dip.z, tip.x, tip.y, tip.z);
            
            
            
            ofPopMatrix();
        }
        
    }
    
    cam.end();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key == ' '){
        senderSwitch = !senderSwitch;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){
    
}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){
    ofxOscMessage m;
    m.setAddress( "/test" );
    m.addStringArg( "testing OSC" );
    oscSender.sendMessage( m );
    m.clear();
    
}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo) {
    
}

//--------------------------------------------------------------
void ofApp::exit(){
    // let's close down Leap and kill the controller
    leap.close();
}
