#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/ObjLoader.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Light.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/Vbo.h"
#include "Debug.h"
#include "FileMonitor.h"

using namespace ci;
using namespace ci::app;
using namespace std;

#define DBG_INFO "Info"
#define DBG_ERROR "Error"

class ImrodApp : public AppNative
{
public:
	void prepareSettings(Settings* settings);
	void setup();
	void shutdown();
	void update();
	void draw();
	void resize();
	void mouseDown(MouseEvent event);
	void mouseDrag(MouseEvent event);
	void keyDown(KeyEvent event);

private:
	void loadShader();
	bool isInitialized() const { return (m_mesh && m_shader && m_light1 && m_light2 && m_texDiffuse && 
		m_texIllumination && m_texNormal && m_texSpecular && m_texAO ); }
private:
	CameraPersp m_camera;
	MayaCamUI m_mayaCamera;
	TriMesh m_triMesh;
	gl::VboMeshRef m_mesh;
	Matrix44f m_matrix;
	gl::Light* m_light1;
	gl::Light* m_light2;
	gl::GlslProgRef m_shader;
	gl::TextureRef m_texDiffuse;
	gl::TextureRef m_texAO;
	gl::TextureRef m_texIllumination;
	gl::TextureRef m_texNormal;
	gl::TextureRef m_texSpecular;
	FileMonitorRef m_fileMonitorVert;
	FileMonitorRef m_fileMonitorFrag;
};

void ImrodApp::prepareSettings(Settings* settings)
{
	settings->setWindowSize(1024, 768);
	settings->setTitle("Imrod");

	m_light1 = NULL;
	m_light2 = NULL;
}

void ImrodApp::setup()
{
	// Load assets
	try
	{
		ObjLoader loader(loadAsset("imrod.obj"));
		loader.load(&m_triMesh);

		gl::VboMesh::Layout layout;
		layout.setStaticPositions();
		layout.setStaticNormals();
		layout.setStaticTexCoords2d();
		layout.setStaticIndices();
		m_mesh = gl::VboMesh::create(m_triMesh, layout);

		m_texDiffuse = gl::Texture::create(loadImage(loadAsset("imrod_Diffuse.png")));
		m_texAO = gl::Texture::create(loadImage(loadAsset("imrod_ao.png")));
		m_texIllumination = gl::Texture::create(loadImage(loadAsset("imrod_Illumination.png")));
		m_texNormal = gl::Texture::create(loadImage(loadAsset("imrod_norm.png")));
		m_texSpecular = gl::Texture::create(loadImage(loadAsset("imrod_spec.png")));
	}
	catch(const std::exception& e)
	{
		console() << "Failed to load assets:" << std::endl;
		console() << e.what();
		quit();
	}

	// Set up the camera
	m_camera.setEyePoint(Vec3f(0.0f, 0.0f, 100.0f));
	AxisAlignedBox3f bbox = m_triMesh.calcBoundingBox();
	m_camera.setEyePoint(Vec3f(0.0f, bbox.getMax().y, bbox.getMax().y * 1.7f));
	m_camera.setCenterOfInterestPoint(Vec3f(0.0f, bbox.getMax().y / 2, 0.0f));

	// Create lights
	m_light1 = new gl::Light(gl::Light::DIRECTIONAL, 0);
	m_light1->setAmbient( Color(0.0f, 0.0f, 0.1f) );
	m_light1->setDiffuse( Color(0.9f, 0.6f, 0.3f) );
	m_light1->setSpecular( Color(0.9f, 0.6f, 0.3f) );

	m_light2 = new gl::Light(gl::Light::DIRECTIONAL, 1);
	m_light2->setAmbient( Color(0.0f, 0.0f, 0.0f) );
	m_light2->setDiffuse( Color(0.2f, 0.6f, 1.0f) );
	m_light2->setSpecular( Color(0.2f, 0.2f, 0.2f) );

	// Load shader
	loadShader();

	// Setup matrix
	m_matrix.setToIdentity();
	m_matrix.translate(Vec3f::zero());
	m_matrix.rotate(Vec3f::zero());
	m_matrix.scale(Vec3f::one());
}

void ImrodApp::shutdown()
{
	// Safely delete our lights
	if(m_light1)
	{
		delete m_light1;
		m_light1 = NULL;
	}

	if(m_light2)
	{
		delete m_light2;
		m_light2 = NULL;
	}
}

void ImrodApp::loadShader()
{
	DBG_REMOVE(DBG_INFO);
	DBG_REMOVE(DBG_ERROR);

	try
	{
		m_fileMonitorVert = FileMonitor::create(getAssetPath("shader.vert"));
		m_fileMonitorFrag = FileMonitor::create(getAssetPath("shader.frag"));
		m_shader = gl::GlslProg::create(loadAsset("shader.vert"), loadAsset("shader.frag"));
		const std::string log = m_shader->getShaderLog(m_shader->getHandle());

		if(log != std::string())
			DBG(DBG_INFO, m_shader->getShaderLog(m_shader->getHandle()));
	}
	catch(gl::GlslProgCompileExc& e)
	{
		console() << "Shader load or compile error:" << std::endl;
		console() << e.what() << std::endl;
		DBG(DBG_ERROR, std::string(e.what()));
	}
}

void ImrodApp::update()
{
	if(m_fileMonitorVert->hasChanged() || m_fileMonitorFrag->hasChanged())
	{
		loadShader();
	}
}

void ImrodApp::draw()
{
	// Clear the window
	gl::clear(ColorAf::gray(0.6f));
	gl::color(Color::white());

	if(isInitialized())
	{
		// Get ready to draw in 3D
		gl::pushMatrices();
		gl::setMatrices(m_camera);

		gl::enableDepthRead();
		gl::enableDepthWrite();

		// Bind textures
		m_texDiffuse->enableAndBind();
		m_texNormal->bind(1);
		m_texAO->bind(2);

		// Bind shader
		m_shader->bind();
		m_shader->uniform("texDiffuse", 0);
		m_shader->uniform("texNormal", 1);
		m_shader->uniform("texAO", 2);

		// Enable lights
		m_light1->enable();
		m_light2->enable();

		// Render model
		gl::pushModelView();
		gl::multModelView(m_matrix);
		gl::draw(m_mesh);
		gl::popModelView();

		// Disable lights
		m_light1->disable();
		m_light2->disable();

		// Unbind shader
		m_shader->unbind();

		// Unbind textures
		gl::disable(m_texDiffuse->getTarget());

		// Disable 3D rendering
		gl::disableDepthWrite();
		gl::disableDepthRead();

		// Restore matrices
		gl::popMatrices();

		// Enable 2D rendering
		gl::setMatricesWindow(getWindowSize());
		gl::setViewport(getWindowBounds());
	}

	// Render debug information
	Debug::get().draw(ColorAf::white());
}

void ImrodApp::resize()
{
	m_camera.setAspectRatio(getWindowAspectRatio());
	m_mayaCamera.setCurrentCam(m_camera);
}

void ImrodApp::mouseDown(MouseEvent event)
{
	m_mayaCamera.setCurrentCam( m_camera );
	m_mayaCamera.mouseDown(event.getPos());
}

void ImrodApp::mouseDrag(MouseEvent event)
{
	m_mayaCamera.mouseDrag(event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown());
	m_camera = m_mayaCamera.getCamera();
}

void ImrodApp::keyDown(KeyEvent event)
{
	switch(event.getCode())
	{
	case KeyEvent::KEY_f:
		{
			setFullScreen(!isFullScreen());
			break;
		}
	case KeyEvent::KEY_ESCAPE:
		{
			quit();
			break;
		}
	case KeyEvent::KEY_F5:
		{
			loadShader();
			break;
		}
	}
}

CINDER_APP_NATIVE(ImrodApp, RendererGl)
