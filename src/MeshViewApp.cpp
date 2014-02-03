#include "cinder/app/AppNative.h"
#include "cinder/params/Params.h"
#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Light.h"
#include "cinder/gl/Texture.h"
#include "Debug.h"
#include "FileMonitor.h"
#include "Config.h"
#include "AssimpLoader.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace mndl;
using namespace mndl::assimp;

#define DBG_INFO "Info"
#define DBG_ERROR "Error"

class MeshViewApp : public AppNative
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
	void fileDrop(FileDropEvent event);

private:
	void loadConfig(const std::string& fileName, bool isReload = false);
	void setupCamera(bool inTheMiddleOfY = false);
	void loadShader(const std::string& fileName);
	bool isInitialized() const
	{
		return (m_shader && m_assimpLoader.getNumMeshes() > 0);
	}

	CameraPersp m_camera;
	MayaCamUI m_mayaCamera;
	Matrix44f m_matrix;
	gl::Light* m_light1;
	gl::Light* m_light2;
	gl::GlslProgRef m_shader;
	gl::TextureRef m_texDiffuse;
	gl::TextureRef m_texNormal;
	gl::TextureRef m_texSpecular;
	gl::TextureRef m_texAO;
	gl::TextureRef m_texEmissive;
	Vec3f m_matAmbient;
	Vec3f m_matDiffuse;
	Vec3f m_matSpecular;
	float m_matShininess;
	FileMonitorRef m_fileMonitorVert;
	FileMonitorRef m_fileMonitorFrag;
	FileMonitorRef m_fileMonitorConfig;
	params::InterfaceGlRef m_params;
	bool m_diffuseEnabled;
	bool m_aoEnabled;
	bool m_emissiveEnabled;
	bool m_normalEnabled;
	bool m_specularEnabled;
	float m_texDiffusePower;
	float m_texNormalPower;
	float m_texSpecularPower;
	float m_texAOPower;
	float m_texEmissivePower;
	float m_gamma;
	bool m_rotateMesh;
	float m_time;
	AssimpLoader m_assimpLoader;
	std::string m_configFileName;
	std::string m_shaderFileName;
};

void MeshViewApp::prepareSettings(Settings* settings)
{
	settings->setWindowSize(1024, 768);
	settings->setTitle("Mesh view");

	m_light1 = NULL;
	m_light2 = NULL;
}

void MeshViewApp::setup()
{
	loadConfig("configs/gaztank.ini");

	setupCamera();

	// Create lights
	m_light1 = new gl::Light(gl::Light::DIRECTIONAL, 0);
	m_light1->setDirection(Vec3f(0, 0, 1).normalized());
	m_light1->setAmbient(Color(0.0f, 0.0f, 0.1f));
	m_light1->setDiffuse(Color(0.9f, 0.6f, 0.3f));
	m_light1->setSpecular(Color(0.9f, 0.6f, 0.3f));

	m_light2 = new gl::Light(gl::Light::DIRECTIONAL, 1);
	m_light2->setDirection(Vec3f(0, 0, -1).normalized());
	m_light2->setAmbient(Color(0.0f, 0.0f, 0.0f));
	m_light2->setDiffuse(Color(0.2f, 0.6f, 1.0f));
	m_light2->setSpecular(Color(0.2f, 0.2f, 0.2f));

	// Setup matrix
	m_matrix.setToIdentity();
	m_matrix.translate(Vec3f::zero());
	m_matrix.rotate(Vec3f::zero());
	m_matrix.scale(Vec3f::one());

	m_rotateMesh = false;

	// Create a parameter window
	m_params = params::InterfaceGl::create(getWindow(), "Properties", Vec2i(180, 240));
	m_params->addText("LMB + drag - rotate");
	m_params->addText("RMB + drag - zoom");
	m_params->addSeparator();
	m_params->addButton("Full screen", [&] { setFullScreen(!isFullScreen()); });
	m_params->addParam("Auto rotate", &m_rotateMesh);
	m_params->addSeparator();
	m_params->addParam("Diffuse", &m_diffuseEnabled);
	m_params->addParam("Normal", &m_normalEnabled);
	m_params->addParam("Specular", &m_specularEnabled);
	m_params->addParam("AO", &m_aoEnabled);
	m_params->addParam("Emissive", &m_emissiveEnabled);
	m_params->addSeparator();
	m_params->addParam("Gamma", &m_gamma, "min=0.0 max=10.0 step=0.1");

	m_time = (float)getElapsedSeconds();
}

void MeshViewApp::shutdown()
{
	// Safely delete lights
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

void MeshViewApp::loadConfig(const std::string& fileName, bool isReload)
{
	try
	{
		if (fs::exists(fileName))
			m_configFileName = fileName; 
		else
			m_configFileName = getAssetPath(fileName).string();

		m_fileMonitorConfig = FileMonitor::create(m_configFileName);

		Config cfg(m_configFileName);
		m_shaderFileName = cfg.getString("Shader", "FileName");
		loadShader(m_shaderFileName);

		if (!isReload)
		{
			m_assimpLoader = AssimpLoader(getAssetPath(cfg.getString("Model", "FileName")), false);
			m_assimpLoader.setAnimation(0);
			m_assimpLoader.enableTextures(false);
			m_assimpLoader.enableSkinning(false);
			m_assimpLoader.enableAnimation(false);
			m_assimpLoader.enableMaterials(false);
		}

		cfg.setSection("Textures");

		if(m_texDiffuse)
			m_texDiffuse = NULL;
		if (m_texNormal)
			m_texNormal = NULL;
		if (m_texSpecular)
			m_texSpecular = NULL;
		if(m_texAO)
			m_texAO = NULL;
		if(m_texEmissive)
			m_texEmissive = NULL;

		m_texDiffusePower = 1.0f;
		m_texNormalPower = 1.0f;
		m_texSpecularPower = 1.0f;
		m_texAOPower = 1.0f;
		m_texEmissivePower = 1.0f;

		m_diffuseEnabled = false;
		m_normalEnabled = false;
		m_specularEnabled = false;
		m_aoEnabled = false;
		m_emissiveEnabled = false;

		std::string diffuseFileName = cfg.getString("Diffuse");
		if(diffuseFileName != std::string())
		{
			m_texDiffuse = gl::Texture::create(loadImage(loadAsset(diffuseFileName)));
			m_texDiffusePower = cfg.getFloat("DiffusePower");
			m_diffuseEnabled = true;
		}

		std::string normalFileName = cfg.getString("Normal");
		if (normalFileName != std::string())
		{
			m_texNormal = gl::Texture::create(loadImage(loadAsset(normalFileName)));
			m_texNormalPower = cfg.getFloat("NormalPower");
			m_normalEnabled = true;
		}

		std::string specularFileName = cfg.getString("Specular");
		if (specularFileName != std::string())
		{
			m_texSpecular = gl::Texture::create(loadImage(loadAsset(specularFileName)));
			m_texSpecularPower = cfg.getFloat("SpecularPower");
			m_specularEnabled = true;
		}

		std::string aoFileName = cfg.getString("AO");
		if(aoFileName != std::string())
		{
			m_texAO = gl::Texture::create(loadImage(loadAsset(aoFileName)));
			m_texAOPower = cfg.getFloat("AOPower");
			m_aoEnabled = true;
		}

		std::string emissiveFileName = cfg.getString("Emissive");
		if(emissiveFileName != std::string())
		{
			m_texEmissive = gl::Texture::create(loadImage(loadAsset(emissiveFileName)));
			m_texEmissivePower = cfg.getFloat("EmissivePower");
			m_emissiveEnabled = true;
		}

		cfg.setSection("Material");
		m_matAmbient = cfg.getVec3f("Ambient");
		m_matDiffuse = cfg.getVec3f("Diffuse");
		m_matSpecular = cfg.getVec3f("Specular");
		m_matShininess = cfg.getFloat("Shininess");
		m_gamma = cfg.getFloat("Gamma");
	}
	catch(const std::exception& e)
	{
		console() << "Failed to load assets:" << std::endl;
		console() << e.what();
	}
}

void MeshViewApp::loadShader(const std::string& fileName)
{
	DBG_REMOVE(DBG_INFO);
	DBG_REMOVE(DBG_ERROR);

	try
	{
		std::string vertexFile = fileName + ".vert";
		std::string fragmentFile = fileName + ".frag";

		m_fileMonitorVert = FileMonitor::create(getAssetPath(vertexFile));
		m_fileMonitorFrag = FileMonitor::create(getAssetPath(fragmentFile));
		m_shader = gl::GlslProg::create(loadAsset(vertexFile), loadAsset(fragmentFile));
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

void MeshViewApp::update()
{
	// Track the time
	float elapsed = (float) getElapsedSeconds() - m_time;
	m_time += elapsed;

	if (m_fileMonitorConfig->hasChanged())
	{
		loadConfig(m_configFileName, true);
	}

	if(m_fileMonitorVert->hasChanged() || m_fileMonitorFrag->hasChanged())
	{
		loadShader(m_shaderFileName);
	}

	if(m_rotateMesh)
	{
		float rotateAngle = elapsed * 0.2f;
		m_matrix.rotate(Vec3f::yAxis(), rotateAngle);
	}

	if(isInitialized())
	{
		m_assimpLoader.setTime(elapsed);
		m_assimpLoader.update();
	}
}

void MeshViewApp::draw()
{
	// Clear the window
	gl::clear();
	gl::color(Color::white());

	if(isInitialized())
	{
		// Get ready to draw in 3D
		gl::pushMatrices();
		gl::setMatrices(m_camera);

		gl::enableDepthRead();
		gl::enableDepthWrite();

		// Bind textures
		if(m_texDiffuse)
			m_texDiffuse->enableAndBind();

		if (m_texNormal)
			m_texNormal->bind(1);

		if (m_texSpecular)
			m_texSpecular->bind(2);

		if(m_texAO)
			m_texAO->bind(3);

		if(m_texEmissive)
			m_texEmissive->bind(4);

		// Bind shader
		m_shader->bind();
		m_shader->uniform("texDiffuse", 0);
		m_shader->uniform("texNormal", 1);
		m_shader->uniform("texSpecular", 2);
		m_shader->uniform("texAO", 3);
		m_shader->uniform("texEmissive", 4);
		m_shader->uniform("texDiffusePower", m_texDiffusePower);
		m_shader->uniform("texNormalPower", m_texNormalPower);
		m_shader->uniform("texSpecularPower", m_texSpecularPower);
		m_shader->uniform("texAOPower", m_texAOPower);
		m_shader->uniform("texEmissivePower", m_texEmissivePower);
		m_shader->uniform("diffuseEnabled", m_diffuseEnabled);
		m_shader->uniform("normalEnabled", m_normalEnabled);
		m_shader->uniform("specularEnabled", m_specularEnabled);
		m_shader->uniform("aoEnabled", m_aoEnabled);
		m_shader->uniform("emissiveEnabled", m_emissiveEnabled);

		m_shader->uniform("material.Ka", m_matAmbient);
		m_shader->uniform("material.Kd", m_matDiffuse);
		m_shader->uniform("material.Ks", m_matSpecular);
		m_shader->uniform("material.Shininess", m_matShininess);
		m_shader->uniform("gamma", m_gamma);

		// Enable lights
		m_light1->enable();
		m_light2->enable();

		// Render model
		gl::pushModelView();
		gl::multModelView(m_matrix);
		m_assimpLoader.draw();
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

		// Render parameter window
		if(m_params)
			m_params->draw();
	}

	// Render debug information
	Debug::get().draw(ColorAf::white());
}

void MeshViewApp::resize()
{
	m_camera.setAspectRatio(getWindowAspectRatio());
	m_mayaCamera.setCurrentCam(m_camera);
}

void MeshViewApp::mouseDown(MouseEvent event)
{
	m_mayaCamera.setCurrentCam(m_camera);
	m_mayaCamera.mouseDown(event.getPos());
}

void MeshViewApp::mouseDrag(MouseEvent event)
{
	m_mayaCamera.mouseDrag(event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown());
	m_camera = m_mayaCamera.getCamera();
}

void MeshViewApp::keyDown(KeyEvent event)
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
		case KeyEvent::KEY_SPACE:
		{
			m_rotateMesh = !m_rotateMesh;
			break;
		}
		case KeyEvent::KEY_c:
		{
			setupCamera(true);
			break;
		}
	}
}

void MeshViewApp::fileDrop(FileDropEvent event)
{
	loadConfig(event.getFile(0).string());
	setupCamera();
}

void MeshViewApp::setupCamera(bool inTheMiddleOfY)
{
	m_camera.setNearClip(0.1f);
	m_camera.setFarClip(10000.0f);
	AxisAlignedBox3f bbox = m_assimpLoader.getBoundingBox();
	Vec3f size = bbox.getSize();
	float max = size.x;
	max = max < size.y ? size.y : max;
	max = max < size.z ? size.z : max;

	if(inTheMiddleOfY)
	m_camera.setEyePoint(Vec3f(0.0f, size.y / 2, max * 2.0f));
	else
	m_camera.setEyePoint(Vec3f(0.0f, max, max * 2.0f));

	m_camera.setCenterOfInterestPoint(bbox.getCenter());
}

CINDER_APP_NATIVE(MeshViewApp, RendererGl)
