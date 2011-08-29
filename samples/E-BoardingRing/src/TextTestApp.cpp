#include "cinder/app/AppNative.h"
#include "cinder/app/LocationEvent.h"
#include "cinder/app/HeadingEvent.h"
#include "cinder/app/DeviceOrientationEvent.h"
#include "cinder/app/TextField.h"

#include "cinder/gl/gl.h"
#include "cinder/gl/Fbo.h"
#include "cinder/gl/Texture.h"
#include "cinder/ImageIo.h"
#include "cinder/Text.h"
#include "cinder/Utilities.h"
#include "cinder/Font.h"
#include "cinder/app/AppCocoaTouch.h"
#include "cinder/Camera.h"

#include "cinder/CinderMath.h"
#include "cinder/System.h"
#include "cinder/Rand.h"
#include "cinder/TriMesh.h"


#include "cinder/Xml.h"
#include "cinder/Url.h"

#include "SimpleGUI.h"
using namespace mowa::sgui;

#include "Choreograph.h"
using namespace ci::tween;

#include "Resources.h"

using namespace ci;
using namespace ci::app;

#include <vector>
#include <map>
using namespace std;

#include <list>
using std::list;

static const bool PREMULT = false;

#pragma mark -
#pragma mark - Declaration -
#pragma mark -
class TextTestApp : public AppCocoaTouch {
    
private:
	SimpleGUI*		gui;
    Sequence        mSequence;
public:
	virtual void	setup();
    virtual void    update();
	virtual void	draw();
    
	virtual void	accelerated( AccelEvent event );
	virtual void	prepareSettings( Settings *settings );
	virtual void	touchesBegan( TouchEvent event );
	virtual void	touchesMoved( TouchEvent event );
	virtual void	touchesEnded( TouchEvent event );
	virtual void    compassUpdated(HeadingEvent heading);
    virtual void    didUpdateToLocation(LocationEvent oldLocation, LocationEvent newLocation);
    virtual void    didChangeDeviceOrientation(DeviceOrientationEvent newOrientation);
    
	void            textMove(float value);
    void			renderGridToFbo();
    void            renderColorToFbo();
    void            renderTextPortrait(std::string text);
    void            renderTextLandscape(std::string text);
    
    void            parseRSS(std::string url);
    
	Matrix44f		mModelView;
	CameraPersp		mCam;
    gl::Fbo			mFbo;
    gl::Fbo         mFboBis;
	gl::Texture	    mText;
	gl::Texture		myImage;
    gl::Texture     mEditImage;
    gl::Texture     fboTexture;
    
	float			mPortraitPosX;
	float			mPortraitPosY;
    float			mLandscapeLeftPosX;
	float			mLandscapeLeftPosY;
    float			mLandscapeRightPosX;
	float			mLandscapeRightPosY;
    float			mPortraitUpsideDownPosX;
	float			mPortraitUpsideDownPosY;
    
    bool            perpendiculiaire;
    bool            incliner;
    int             showParameter;
    bool            isRetina;
    int             coefRetina;
    
    int             mSizeSquare;
    float           mSizePoint;
	float			mAlphaPoint;
    float           mAlphaLine;
    
    float           compassDegree;
    
    Orientation     mOrientation;
    string          textLong;
    string          textRSS;
    TextField       mTextField;
    bool            showTextField;
    bool onTextDone( TextField *textField );
};

#pragma mark -
#pragma mark - Implementation 
#pragma mark - 

#pragma mark view cycle -



void TextTestApp::setup()
{
    showTextField=false;
    mTextField.setup();
    mTextField.registerTextDone( this, &TextTestApp::onTextDone );
    mTextField.setPosition(0, 0, 300, 50);
    //mTextField.hideTextField(true);
    console()<<textRSS.max_size()<<endl;
    textRSS="";
    //parseRSS("http://rss.lemonde.fr/c/205/f/3050/index.rss");
    //parseRSS("http://rss.macgeneration.com");
    //parseRSS("http://rss.lefigaro.fr/lefigaro/laune");
    //parseRSS("http://rss.macbidouille.com/macbidouille.rss");
    //parseRSS("http://www.metrofrance.com/rss.xml?=section-news");
    //console()<<textRSS.size()<<endl;
    hideStatusBar(true);
    //detection of the retina display
    if (getWindowSize().x==640) {
        isRetina=true;
        coefRetina=1;
    }
    else{
        isRetina=false;
        coefRetina=2;
    }
    
    mPortraitPosX = 0/coefRetina;
    mPortraitPosY = 0/coefRetina;
    mLandscapeLeftPosX=0/coefRetina;
    mLandscapeLeftPosY=-640/coefRetina;
    mLandscapeRightPosX=-960/coefRetina;
    mLandscapeRightPosY=0/coefRetina;
    mPortraitUpsideDownPosX=-640/coefRetina;
    mPortraitUpsideDownPosY=-960/coefRetina;
    
    //initialisations
    gl::Fbo::Format format;
    format.setSamples( 8 ); // 8x antialiasing
	mFbo = gl::Fbo( 100, 100, format );
    mFboBis = gl::Fbo (500, 500, format );
	showParameter =1;
    
    //SimpleGUI
    gui = new SimpleGUI(this);
    gui->setGuiValue(400,50,isRetina);
	gui->lightColor = ColorA(1, 1, 0, 1);	//couleur de la barre
	gui->addParam("Opacité Point", &mAlphaPoint, 0, 1.0, 0.);
    gui->addParam("Taille Point", &mSizePoint, 4.5, 6.5, 5.); 	
    gui->addParam("Opacité Ligne", &mAlphaLine, 0., 1., 1.);
    gui->addParam("Taille carroyage", &mSizeSquare, 30,10,10);//4 and 12
    gui->addParam("90°", &perpendiculiaire, true, 1, false); 
	gui->addParam("45°", &incliner, false, 1, true);
    
    /*create the texte*/
	string text[]={"Bonjour Édouard, où êtes vous ? ",
        "Je suis actuellement en déplacement",
        "Je reviens à Paris le 18 janvier.",
        "Lorem Ipsum is simply dummy text of the printing",
        "and typesetting industry.Lorem Ipsum has been ",
        "the industry's standard dummy text ever since ",
        "the 1200s, when an unknown printer took a ",
        "galley of type and scrambled it to make a type ",
        "specimen book. It has survived not only five , ",
        "centuries but also the leap into electronic ",
        "typesset. ",
        " ",	
        "Dynamically incubate bleeding-edge metrics ",
        "without global innovation. Appropriately ",
        "incentivize flexible materials through",
        "functionalized synergy. Phosfluorescently maintain",
        "leading-edge applications via 2.0 web services.",
        "	",
        "Monotonectally brand an expanded array of ",
        "functionalities before leveraged solutions. ",
        "Phosfluorescently synergize frictionless ",
        "convergence without end-to-end bandwidth. ",
        "Phosfluorescently mesh  customized resources ",
        "through client-focused value.",
        "	",
        "Assertively predominate ubiquitous technologies",
        "before impactful e-services. Distinctively ",
        "embrace stand-alone e-business with effective ",
        "materials. Phosfluorescently evolve diverse ",
        "benefits whereas leading-edge benefits. ",
        "	",
        "Continually integrate leading-edge growth",
        "strategies for visionary e-business. ",
        "Energistically e-enable wireless models with ",
        "viral paradigms. Interactively target just in ",
        "time services without backend technologies.",
        "                                                                                                                                                        "
    };
    
    textLong="Bonjour Édouard\n, où êtes vous ? Je reviens à Paris le 18 janvier. Monotonectally brand an expanded array of Phosfluorescently mesh  customized resources, Bonjour Édouard, où êtes vous ? Je reviens à Paris le 18 janvier. Monotonectally brand an expanded array of Phosfluorescently mesh  customized resources où êtes vous ? Je reviens à Paris le 18 janvier. Monotonectally brand an expanded array of Phosfluorescently mesh  customized resources, Bonjour Édouard, où êtes vous ? Je reviens à Paris le 18 janvier. Monotonectally brand an expanded array of Phosfluorescently mesh  customized resources où êtes vous ? Je reviens à Paris le 18 janvier. Monotonectally brand an expanded array of Phosfluorescently mesh  customized resources, Bonjour Édouard, où êtes vous ? Je reviens à Paris le 18 janvier. Monotonectally brand an expanded array of Phosfluorescently mesh  customized resources où êtes vous ? Je reviens à Paris le 18 janvier. Monotonectally brand an expanded array of Phosfluorescently mesh  customized resources, Bonjour Édouard, où êtes vous ? Je reviens à Paris le 18 janvier. Monotonectally brand an expanded array of Phosfluorescently mesh  customized resources où êtes vous ? Je reviens à Paris le 18 janvier. Monotonectally brand an expanded array of Phosfluorescently mesh  customized resources, Bonjour Édouard, où êtes vous ? Je reviens à Paris le 18 janvier. Monotonectally brand an expanded array of Phosfluorescently mesh  customized resources où êtes vous ? Je reviens à Paris le 18 janvier. Monotonectally brand an expanded array of Phosfluorescently mesh  customized resources, Bonjour Édouard, où êtes vous ? Je reviens à Paris le 18 janvier. Monotonectally brand an expanded array of Phosfluorescently mesh  customized resources où êtes vous ? Je reviens à Paris le 18 janvier. Monotonectally brand an expanded array of Phosfluorescently mesh  customized resources, Bonjour Édouard, où êtes vous ? Je reviens à Paris le 18 janvier. Monotonectally brand an expanded array of Phosfluorescently mesh  customized resources où êtes vous ? Je reviens à Paris le 18 janvier. Monotonectally brand an expanded array of Phosfluorescently mesh  customized resources, Bonjour Édouard, où êtes vous ? Je reviens à Paris le 18 janvier. Monotonectally brand an expanded array of Phosfluorescently mesh  customized resources où êtes vous ? Je reviens à Paris le 18 janvier. Monotonectally brand an expanded array of Phosfluorescently mesh  customized resources, Bonjour Édouard, où êtes vous ? Je reviens à Paris le 18 janvier. Monotonectally brand an expanded array of Phosfluorescently mesh  customized resources";
    

    //set text layout
    //	TextLayout Tlayout;
    //	Tlayout.setBorder(10,10);
    //	Tlayout.clear( ColorA( 0.1f, 0.1f, 0.1f, 0.3f ) );
    //    Tlayout.setFont( Font( normalFont, 24/coefRetina) );
    //	Tlayout.setColor( Color( 1, 1, 1 ) );    
    //	for (int i=0; i<37; i++) {
    //		Tlayout.addLine( text[i] );
    //	}
    //    Surface8u rendered = Tlayout.render( true, PREMULT );
    //console()<<textRSS<<endl;
    
    //textLong=textRSS;
    renderTextPortrait(textLong);
    myImage  = gl::Texture( loadImage( loadResource( "image77.png" ) ) );
	mEditImage  = gl::Texture( loadImage( loadResource( "edit.jpg" ) ) );
    
    enableDeviceOrientationSupport();
    mOrientation=getCurrentDeviceOrientation().getDeviceOrientation();
	/*enable accelerometer infos*/
	enableAccelerometer();
    enableLocationSevices(); 
    startUpdatingHeading();
    shouldDisplayHeadingCalibration(true);
	/*setup camera view port*/
	mCam.setPerspective( 60, getWindowAspectRatio(), 1, 1000 );
	mCam.lookAt( Vec3f( 0, 0, 3 ), Vec3f::zero() );	
}

bool TextTestApp::onTextDone( TextField *textField )
{
    mOrientation=getCurrentDeviceOrientation().getDeviceOrientation();
    string text = textField->getText();
    parseRSS(text);
    if (textRSS!=""){
    textLong=textRSS;
    }
    switch (mOrientation) {
        case DeviceOrientationPortrait:
            renderTextPortrait(textLong);
            break;
        case DeviceOrientationLandscapeLeft:
            renderTextLandscape(textLong);
            break;
        case DeviceOrientationLandscapeRight:
            renderTextLandscape(textLong);
            break;
        case DeviceOrientationPortraitUpsideDown:
            renderTextPortrait(textLong);
            break;
        default:
            break;
    }
    return true;
}

void TextTestApp::update()
{
    // step our animation forward
	mSequence.step(1.0/60.0);
    
    //refresh the grid
    renderGridToFbo();  
    
    //console()<<getFpsSampleInterval()<<endl;
}

void TextTestApp::draw()
{
    // Clear the openGL Render view
	gl::clear( Color::black() );
    glClear( GL_COLOR_BUFFER_BIT );
    
    gl::pushMatrices();
    mCam.setPerspective( 60, getWindowAspectRatio(), 1, 1000 );
    if (perpendiculiaire) {
        mCam.lookAt( Vec3f( 0, 0, 3 ), Vec3f::zero() );
    }
    if (incliner) {
        double l=sqrt(2)*1.5;
        mCam.lookAt( Vec3f( 0, -l, l ), Vec3f::zero() );
    }
    
	//Some setup of the window
	//gl::setMatricesWindow( getWindowSize() );
	gl::enableDepthRead();	 
	gl::enableDepthWrite();
	//Setup the view and save the precedent modelview parameters for cube drawing
    gl::setMatrices( mCam );
	gl::multModelView( mModelView);
    // set the viewport to match our window
	gl::setViewport( getWindowBounds());
    
    //draw cube
    glEnable( GL_TEXTURE_2D );
    mFboBis.bindTexture();
    gl::color(Color::white());    
    gl::drawCube( Vec3f::zero(), Vec3f( 2.2001f, 2.2001f, 2.2001f ));
    mFboBis.unbindTexture();
    gl::disableDepthRead();	 
	gl::disableDepthWrite();
    
    //draw text
    gl::setMatricesWindow(getWindowSize());
	gl::disableDepthRead();	
	gl::disableDepthWrite();		
	gl::enableAlphaBlending();
    gl::color( Color::white() );
    
    switch (mOrientation) {
        case DeviceOrientationPortrait:
            //renderTextPortrait(textLong);
            gl::rotate(0);
            gl::draw( mText, Vec2f( mPortraitPosX, mPortraitPosY ));
            (isRetina)? gl::draw( myImage, Vec2f(600,890.)) : gl::draw( myImage, Vec2f(290,430.));
            (isRetina)? gl::draw( mEditImage, Vec2f(600,20.)) : gl::draw( mEditImage, Vec2f(290,10.));
            //console()<<"portrait"<<endl;
            break;
        case DeviceOrientationLandscapeLeft:
            //renderTextLandscape(textLong);
            gl::rotate(90);
            gl::draw( mText, Vec2f( mLandscapeLeftPosX, mLandscapeLeftPosY ));
            (isRetina)? gl::draw( myImage, Vec2f(900,-60)) : gl::draw( myImage, Vec2f(440,-50));
            (isRetina)? gl::draw( mEditImage, Vec2f(900,-620)) : gl::draw( mEditImage, Vec2f(440,-310));
            //console()<<"left"<<endl;
            break;
        case DeviceOrientationLandscapeRight:
            //renderTextLandscape(textLong);
            gl::rotate(-90);
            gl::draw( mText, Vec2f( mLandscapeRightPosX, mLandscapeRightPosY ));
            (isRetina)? gl::draw( myImage, Vec2f(-60,580)) : gl::draw( myImage, Vec2f(-40,270));
            (isRetina)? gl::draw( mEditImage, Vec2f(-60,20)) : gl::draw( mEditImage, Vec2f(-40,10.));
            //console()<<"right"<<endl;
            break;
        case DeviceOrientationPortraitUpsideDown:
            //renderTextPortrait(textLong);
            gl::rotate(180);
            gl::draw( mText, Vec2f( mPortraitUpsideDownPosX, mPortraitUpsideDownPosY ));
            (isRetina)? gl::draw( myImage, Vec2f(-40,-70.)) : gl::draw( myImage, Vec2f(-30,-45));
            (isRetina)? gl::draw( mEditImage, Vec2f(-40,-940)) : gl::draw( mEditImage, Vec2f(-30,-470));
            //console()<<"portrait down"<<endl;
            break;
            
        default:
            break;
    }
    
    //(isRetina)? gl::draw( myImage, Vec2f(580.,890.)) : gl::draw( myImage, Vec2f(270.,430.));
    gl::disableAlphaBlending();
	gl::enableDepthRead();
	gl::enableDepthWrite();
	gl::color(ColorA(1,1,1,1));
	gl::popMatrices();
    
    //draw GUI
    if (showParameter%2==0){ 
        gui->setEnabled(true);         
    }
    else{
        gui->setEnabled(false);
    }
    //    console()<<"method draw"<<endl;
    gui->draw();
}

#pragma mark - Events -
void TextTestApp::prepareSettings( Settings *settings )
{
	settings->enableMultiTouch();
}

#pragma mark Touch delegate methods 
void TextTestApp::touchesBegan( TouchEvent event )
{
    for( vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt ){
       // console()<<touchIt->getPrevX()<<", "<<touchIt->getPrevY()<<endl;
        switch (mOrientation) {
            case DeviceOrientationPortrait:
                if(touchIt->getPrevY()>800/coefRetina && touchIt->getPrevX()>550/coefRetina){
                    showParameter++;
                }
                if(touchIt->getPrevY()<60/coefRetina && touchIt->getPrevX()>550/coefRetina){
                    mTextField.hideTextField(!showTextField);
                    showTextField=!showTextField;
                }
                break;
            case DeviceOrientationLandscapeLeft:
                if(touchIt->getPrevY()>850/coefRetina && touchIt->getPrevX()<90/coefRetina){
                    showParameter++;
                }
                if(touchIt->getPrevY()>850/coefRetina && touchIt->getPrevX()>580/coefRetina){
                    mTextField.hideTextField(!showTextField);
                    showTextField=!showTextField;
                }
                break;
            case DeviceOrientationLandscapeRight:
                if(touchIt->getPrevY()<100/coefRetina && touchIt->getPrevX()>550/coefRetina){
                    showParameter++;
                }
                if(touchIt->getPrevY()<60/coefRetina && touchIt->getPrevX()<60/coefRetina){
                    mTextField.hideTextField(!showTextField);
                    showTextField=!showTextField;
                }
                break;
            case DeviceOrientationPortraitUpsideDown:
                if(touchIt->getPrevY()<100/coefRetina && touchIt->getPrevX()<90/coefRetina){
                    showParameter++;
                }
                if(touchIt->getPrevY()>890/coefRetina && touchIt->getPrevX()<60/coefRetina){
                    mTextField.hideTextField(!showTextField);
                    showTextField=!showTextField;
                }
                break;
            default:
                break;
        }
    }
}

void TextTestApp::touchesMoved( TouchEvent event )
{
	for( vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt ){
        float distance;
        switch (mOrientation) {
            case DeviceOrientationPortrait:
                distance=touchIt->getY() - touchIt->getPrevY();
                break;
            case DeviceOrientationLandscapeLeft:
                distance=touchIt->getX() - touchIt->getPrevX();
                break;
            case DeviceOrientationLandscapeRight:
                distance=touchIt->getX() - touchIt->getPrevX();
                break;
            case DeviceOrientationPortraitUpsideDown:
                distance=touchIt->getY() - touchIt->getPrevY();
                break;
                
            default:
                break;
        }
		if(distance > 10){
			textMove(distance);
		}
		else if (distance < -10){
			textMove(distance);
		}
	}
}

void TextTestApp::touchesEnded( TouchEvent event )
{
    //    for( vector<TouchEvent::Touch>::const_iterator touchIt = event.getTouches().begin(); touchIt != event.getTouches().end(); ++touchIt ){
    //        console()<<"("<<touchIt->getPrevX()<<", "<<touchIt->getPrevY()<<")"<<endl;
    //    }
}

#pragma mark Accelerometer delegate methods
void TextTestApp::accelerated( AccelEvent event )
{
	mModelView = event.getMatrixFromVectorOrientation(Vec3f(0,0,-1));  //get a matrix 4 by 4
	if( event.isShake() )
		console() << "Shake!" << std::endl;
}

#pragma mark Location delegate methods
void TextTestApp::compassUpdated(HeadingEvent heading)
{
    compassDegree=heading.getMagneticHeading();
}

void TextTestApp::didUpdateToLocation(LocationEvent oldLocation, LocationEvent newLocation)
{
    // console()<<"from ("<<oldLocation.getLatitude()<<", "<<oldLocation.getLongitude()<< ") to ("<<newLocation.getLatitude()<<", "<<newLocation.getLongitude()<< ")"<<endl;
}

#pragma mark Device Orientation method
void TextTestApp::didChangeDeviceOrientation(DeviceOrientationEvent newOrientation)
{
    mOrientation=newOrientation.getDeviceOrientation();
    switch (mOrientation) {
        case DeviceOrientationPortrait:
            mTextField.setPosition(10, 10, 280, 30);
            renderTextPortrait(textLong);
            break;
        case DeviceOrientationLandscapeLeft:
            mTextField.setPosition(160, 140, 280, 30);//(280, 10, 30, 280);
            renderTextLandscape(textLong);
            break;
        case DeviceOrientationLandscapeRight:
            mTextField.setPosition(-120, 320, 280, 30);//(10, 280, 30, 280);
            renderTextLandscape(textLong);
            break;
        case DeviceOrientationPortraitUpsideDown:
            mTextField.setPosition(30, 440, 280, 30);
            renderTextPortrait(textLong);
            break;
            
        default:
            break;
    }
    mTextField.hideTextField(showTextField);
}

#pragma mark - methods -

void TextTestApp::textMove(float value){
    mSequence.reset();
    switch (mOrientation) {
        case DeviceOrientationPortrait:
            mSequence.replace( &mPortraitPosY, mPortraitPosY+value/coefRetina*5, .5, Expo::easeOut);
            mSequence.replace( &mLandscapeLeftPosY, mLandscapeLeftPosY+value/coefRetina*5, .5, Expo::easeOut);
            mSequence.replace( &mLandscapeRightPosY, mLandscapeRightPosY+value/coefRetina*5, .5, Expo::easeOut);
            mSequence.replace( &mPortraitUpsideDownPosY, mPortraitUpsideDownPosY+value/coefRetina*5, .5, Expo::easeOut);
            break;
        case DeviceOrientationLandscapeLeft:
            mSequence.replace( &mPortraitPosY, mPortraitPosY-value/coefRetina*5, .5, Expo::easeOut);
            mSequence.replace( &mLandscapeLeftPosY, mLandscapeLeftPosY-value/coefRetina*5, .5, Expo::easeOut);
            mSequence.replace( &mLandscapeRightPosY, mLandscapeRightPosY-value/coefRetina*5, .5, Expo::easeOut);
            mSequence.replace( &mPortraitUpsideDownPosY, mPortraitUpsideDownPosY-value/coefRetina*5, .5, Expo::easeOut);
            break;
        case DeviceOrientationLandscapeRight:
            mSequence.replace( &mPortraitPosY, mPortraitPosY+value/coefRetina*5, .5, Expo::easeOut);
            mSequence.replace( &mLandscapeLeftPosY, mLandscapeLeftPosY+value/coefRetina*5, .5, Expo::easeOut);
            mSequence.replace( &mLandscapeRightPosY, mLandscapeRightPosY+value/coefRetina*5, .5, Expo::easeOut);
            mSequence.replace( &mPortraitUpsideDownPosY, mPortraitUpsideDownPosY+value/coefRetina*5, .5, Expo::easeOut);
            break;
        case DeviceOrientationPortraitUpsideDown:
            mSequence.replace( &mPortraitPosY, mPortraitPosY-value/coefRetina*5, .5, Expo::easeOut);
            mSequence.replace( &mLandscapeLeftPosY, mLandscapeLeftPosY-value/coefRetina*5, .5, Expo::easeOut);
            mSequence.replace( &mLandscapeRightPosY, mLandscapeRightPosY-value/coefRetina*5, .5, Expo::easeOut);
            mSequence.replace( &mPortraitUpsideDownPosY, mPortraitUpsideDownPosY-value/coefRetina*5, .5, Expo::easeOut);
            break;
            
        default:
            break;
    }
}

void TextTestApp::renderColorToFbo()
{
    gl::SaveFramebufferBinding bindingSaver;
    mFbo.bindFramebuffer();
	// setup the viewport to match the dimensions of the FBO
	gl::setViewport( mFbo.getBounds() );
	// setup our camera to render the torus scene
	CameraPersp cam( mFbo.getWidth(), mFbo.getHeight(), 60.0f );
	cam.setPerspective( 90, mFbo.getAspectRatio(), 1, 1000);
    
    //change the size with the degree for filling the grid after
    int degree=compassDegree;
    degree=degree%90;
    degree=45-degree;
    float rate=1-abs(degree/45.f);
    float distanceCam;
    distanceCam=80-30*rate;
	cam.lookAt( Vec3f( 0, 0,  distanceCam),Vec3f( 0, 0, 0.f ));//mSizeSquare  65
	gl::setMatrices( cam );
    gl::rotate(-compassDegree);
    
    //Draw texture FBO
    gl::clear( Color::black() ); 
    glEnable(GL_BLEND);
    GLfloat tmp[]={-50,50, 50,50, 50,-50, 50,-50, -50,-50, -50,50};
    GLfloat tmpColor[]={1,0,0,mAlphaLine, 0,1,0,mAlphaLine, 0,0,1,mAlphaLine, 0,0,1,mAlphaLine, 1,1,0,mAlphaLine, 1,0,0,mAlphaLine};
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glLineWidth(2);
    glColorPointer(4, GL_FLOAT, 0,tmpColor);
    glVertexPointer(2, GL_FLOAT, 0, tmp);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisable(GL_BLEND);
    mFbo.unbindFramebuffer();
    fboTexture=mFbo.getTexture();
}


// Render the Grid into the FBO
void TextTestApp::renderGridToFbo()
{
    //refresh color before drawing the grid
    renderColorToFbo();
    
	// this will restore the old framebuffer binding when we leave this function
	// on non-OpenGL ES platforms, you can just call mFbo.unbindFramebuffer() at the end of the function
	// but this will restore the "screen" FBO on OpenGL ES, and does the right thing on both platforms
    gl::SaveFramebufferBinding bindingSaver;
	// bind the framebuffer - now everything we draw will go there
	mFboBis.bindFramebuffer();
	// setup the viewport to match the dimensions of the FBO
	gl::setViewport( mFboBis.getBounds() );
	// setup our camera to render the torus scene
	CameraPersp cam( mFboBis.getWidth(), mFboBis.getHeight(), 60.0f );
	cam.setPerspective( 90, mFboBis.getAspectRatio(), 1, 1000 );
	cam.lookAt( Vec3f( 49.5f, 49.5f,  mSizeSquare),Vec3f( 49.5f, 49.5f, 0.f ));//mSizeSquare 30
    gl::setMatrices( cam );
    
    //Texture FBO 
    int tailleCube=101;
    int nbPoints=101*101;
    GLfloat cubeVertices[tailleCube*2*4+1];
    GLfloat cubeVerticesTexture[tailleCube*2*4+1];
    GLfloat colorLines[tailleCube*2*4*2+1];//tailleCube*2=nombre de point(x,y), *4 pour RGBA, *2 pour une ligne
    GLfloat points[nbPoints*2+1];//[(x,y),...]
    GLfloat color[nbPoints*4+1];//[(R,G,B,A),...]
    
    int cmp=0;
    int cmpBis=0;
    int cmpBBis=0;
    int cmpColorLines=0;
    
    for (int j=0; j<=99;j++){
        cubeVertices[cmp]=0;
        cubeVertices[cmp+1]=j;
        cubeVertices[cmp+2]=99;
        cubeVertices[cmp+3]=j;
        
        cubeVerticesTexture[cmp]=0;
        cubeVerticesTexture[cmp+1]=j/100.f;
        cubeVerticesTexture[cmp+2]=99/100.f;
        cubeVerticesTexture[cmp+3]=j/100.f;
        
        //red pour (cmp,cmp+1)
        colorLines[cmpColorLines]=1.f;
        colorLines[cmpColorLines+1]=0.f;
        colorLines[cmpColorLines+2]=0.f;
        colorLines[cmpColorLines+3]=1;//mAlphaLine;
        
        //green pour (cmp+2,cmp+3)
        colorLines[cmpColorLines+4]=0.f;
        colorLines[cmpColorLines+5]=1.f;
        colorLines[cmpColorLines+6]=0.f;
        colorLines[cmpColorLines+7]=1;//mAlphaLine;
        
        cubeVertices[cmp+4]=j;
        cubeVertices[cmp+5]=0;
        cubeVertices[cmp+6]=j;
        cubeVertices[cmp+7]=99;
        
        cubeVerticesTexture[cmp+4]=j/100.f;
        cubeVerticesTexture[cmp+5]=0/100.f;
        cubeVerticesTexture[cmp+6]=j/100.f;
        cubeVerticesTexture[cmp+7]=99/100.f;
        
        //blue pour (cmp+4, cmp+5)
        colorLines[cmpColorLines+8]=0.f;
        colorLines[cmpColorLines+9]=0.f;
        colorLines[cmpColorLines+10]=1.f;
        colorLines[cmpColorLines+11]=0.5;//mAlphaLine;
        
        //yellow pour (cmp+6, cmp+7)
        colorLines[cmpColorLines+12]=1.f;
        colorLines[cmpColorLines+13]=1.f;
        colorLines[cmpColorLines+14]=0.f;
        colorLines[cmpColorLines+15]=0.5;//mAlphaLine;
        
        cmp+=8;
        cmpColorLines+=16;
        
        for (int i=0;i<=99;i++){
            points[cmpBis]=j;
            points[cmpBis+1]=i;
            
            color[cmpBBis]=1.f;//0+random()%10/10.f;//1.f;
            color[cmpBBis+1]=1.f;//0+random()%10/10.f;//1.f;
            color[cmpBBis+2]=1.f;//0+random()%10/10.f;//1.f;
            color[cmpBBis+3]=mAlphaPoint;
            
            cmpBBis=cmpBBis+4;
            cmpBis=cmpBis+2;
        }
    }
    
    //Draw texture FBO
    gl::disableDepthRead();	 
	gl::disableDepthWrite();
    glEnable( GL_TEXTURE_2D );
    glEnable( GL_LINE_SMOOTH );
    glEnable( GL_BLEND );
    // Specifies pixel arithmetic
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //glShadeModel(GL_FLAT); 
    
    // Clear the openGL Render view
	gl::clear( Color::black() );
	glClear( GL_COLOR_BUFFER_BIT );
    gl::color( Color::white());
    fboTexture.bind();
    glEnableClientState(GL_VERTEX_ARRAY);
    //glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    //glColorPointer(4, GL_FLOAT, 0,colorLines);
    glVertexPointer(2, GL_FLOAT, 0, cubeVertices);
    glTexCoordPointer(2,GL_FLOAT,0, cubeVerticesTexture);
    glLineWidth(2.5);
    glDrawArrays(GL_LINES, 0, (cmp)/2);
    fboTexture.unbind();
    //glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LINE_SMOOTH);
    
    //Draw dots 
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glPointSize(mSizePoint);
    glEnable(GL_POINT_SMOOTH);
    glColorPointer(4, GL_FLOAT, 0,color);
    glVertexPointer(2, GL_FLOAT, 0, points);
    glDrawArrays(GL_POINTS , 1.0, cmpBis/2);
    glDisable(GL_POINT_SMOOTH);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    
    glDisable(GL_BLEND);
    mFboBis.unbindFramebuffer();
}

void TextTestApp::renderTextPortrait(std::string text)
{
    console()<<"render text portrait"<<endl;
    std::string normalFont( "Arial" );
	std::string boldFont( "Arial-BoldMT" );
	std::string differentFont( "AmericanTypewriter" );
    TextBox mTextBox=TextBox();
    //set width
    mTextBox.setSize(Vec2i(580/coefRetina,2000));
    mTextBox.setFont(Font( normalFont, 24/coefRetina));
    mTextBox.setText(text);
    //Vec2f v=mTextBox.measure();
    //console()<<"taille reelle est "<<v.y<<endl;
    //Vec2i vBis=Vec2i(ceil(v.x),ceil(v.y));
    //reset the size with the length of the text
    //mTextBox.setSize(vBis);
    Surface8u rendered =mTextBox.render();
	mText = gl::Texture( rendered );
}

void TextTestApp::renderTextLandscape(std::string text)
{
    console()<<"render text landscape"<<endl;
    std::string normalFont( "Arial" );
	std::string boldFont( "Arial-BoldMT" );
	std::string differentFont( "AmericanTypewriter" );
    TextBox mTextBox=TextBox();
    //set width
    mTextBox.setSize(Vec2i(880/coefRetina,2000));
    mTextBox.setFont(Font( normalFont, 24/coefRetina));
    mTextBox.setText(text);
    //Vec2f v=mTextBox.measure();
    //Vec2i vBis=Vec2i(ceil(v.x),ceil(v.y));
    //reset the size with the length of the text
    //mTextBox.setSize(vBis);
    Surface8u rendered =mTextBox.render();
	mText = gl::Texture( rendered );
}

void TextTestApp::parseRSS(std::string url)
{
    try {
        const XmlTree xml( loadUrl( Url( url ) ) );
        int cmp=0;
        for( XmlTree::ConstIter itemIter = xml.begin( "RSS/channel/item" ); itemIter != xml.end(); ++itemIter ) {
            if (cmp<10){
                string titleLine( itemIter->getChild( "title" ).getValue() );
                textRSS.append(titleLine);
                textRSS.append("\n");
                
                
                string descriptionLine( itemIter->getChild( "description" ).getValue() );
                size_t open,close;
                string tmp;
                //console()<<descriptionLine<<endl; 
                //int hello=int(descriptionLine.find( "jflsjfmsjfldsjfl" ));
                //console()<<hello<<endl;
                while (int(descriptionLine.find( '<' ))!=-1) {
                    //console()<<"description: "<<descriptionLine<<endl;
                    open = descriptionLine.find( '<' );
                    close = descriptionLine.find( '>' );
                    tmp = descriptionLine.substr(0, open);
                    //console()<<"text append: "<<tmp<<endl<<endl;
                    textRSS.append(tmp);
                    descriptionLine=descriptionLine.substr(close+1,descriptionLine.size());
                }
                textRSS.append("\n");
                string pubDateLine( itemIter->getChild( "pubDate" ).getValue() );
                textRSS.append(pubDateLine);
                textRSS.append("\n\n");
            }
            else{
                console()<<"finit"<<endl;
                break;
            }
            cmp++;
        }
        showTextField=false;
    }
    catch (...) {
        console()<<"error"<<endl;    
    }
}

#pragma mark -

// This line tells Cinder to actually create the application
CINDER_APP_NATIVE( TextTestApp, RendererGl )