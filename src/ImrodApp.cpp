#include <vector>
#include "cinder/app/AppNative.h"
#include "cinder/Camera.h"
#include "cinder/MayaCamUI.h"
#include "cinder/ObjLoader.h"
#include "cinder/ImageIo.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Light.h"
#include "cinder/gl/Texture.h"
#include "cinder/gl/GlslProg.h"
#include "cinder/gl/Vbo.h"
#include "Debug.h"
#include "FileMonitor.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class ImrodApp : public AppNative
{
	public:
		void prepareSettings(Settings* settings);
		void setup();
		void update();
		void draw();
		void resize();
		void mouseDown(MouseEvent event);
		void mouseDrag(MouseEvent event);
		void keyDown(KeyEvent event);

	private:
		CameraPersp m_camera;
		MayaCamUI m_mayaCamera;
		TriMesh m_triMesh;
		std::vector<gl::Light> m_lights;
		gl::GlslProgRef m_shader;
		Matrix44f m_matrix;
		gl::VboMeshRef m_mesh;
		gl::TextureRef m_texDiffuse;
		gl::TextureRef m_texAO;
		gl::TextureRef m_texIllumination;
		gl::TextureRef m_texNormal;
		gl::TextureRef m_texSpecular;
		bool m_shaderHasErrors;
		FileMonitorRef m_fileMonitorVert;
		FileMonitorRef m_fileMonitorFrag;
		GLuint bindas1;
		int uniformas1;
		GLuint bindas2;
		int uniformas2;
};

void ImrodApp::prepareSettings(Settings* settings)
{
	settings->setWindowSize(1280, 720);
	settings->setResizable(true);
	settings->setTitle("Imrod");
}

void ImrodApp::setup()
{
	bindas1 = 0;
	uniformas1 = 0;
	bindas2 = 0;
	uniformas2 = 0;

	// Load mesh
	ObjLoader loader(loadAsset("imrod.obj"));
	loader.load(&m_triMesh);

	// Set VBO mesh
	gl::VboMesh::Layout layout;
	layout.setStaticPositions();
	layout.setStaticNormals();
	layout.setStaticTexCoords2d();
	layout.setStaticIndices();
	m_mesh = gl::VboMesh::create(m_triMesh, layout);

	// Load textures
	try
	{
		m_texDiffuse = gl::Texture::create(loadImage(loadAsset("imrod_Diffuse.png")));
		m_texAO = gl::Texture::create(loadImage(loadAsset("imrod_ao.png")));
		m_texIllumination = gl::Texture::create(loadImage(loadAsset("imrod_Illumination.png")));
		m_texNormal = gl::Texture::create(loadImage(loadAsset("imrod_norm.png")));
		m_texSpecular = gl::Texture::create(loadImage(loadAsset("imrod_spec.png")));
	}
	catch(const std::exception& exc)
	{
		ci::app::console() << "Failed to load image:" << std::endl;
		ci::app::console() << exc.what();
	}

	// Set up the camera
	m_camera.setEyePoint(Vec3f(0.0f, 0.0f, 100.0f));
	AxisAlignedBox3f bbox = m_triMesh.calcBoundingBox();
	m_camera.setEyePoint(Vec3f(0.0f, bbox.getMax().y, bbox.getMax().y * 1.7f));
	m_camera.setCenterOfInterestPoint(Vec3f(0.0f, bbox.getMax().y / 2, 0.0f));

	// Create lights
	/*gl::Light light1(gl::Light::DIRECTIONAL, 0);
	light1.setDirection(Vec3f(0.0f, 0.0f, 1.0f).normalized());
	light1.setAmbient(Color(0.5f, 0.5f, 0.5f));
	light1.setDiffuse(Color(1.0f, 1.0f, 1.0f));
	light1.setSpecular(Color(1.0f, 1.0f, 1.0f));
	light1.enable();
	m_lights.push_back(light1);*/

	/*gl::Light light2(gl::Light::POINT, 1);
	light2.setPosition(Vec3f(100, 100, 100));
	light2.setAmbient(Color(1.0f, 1.0f, 1.0f));
	light2.setDiffuse(Color(1.0f, 1.0f, 1.0f));
	light2.setSpecular(Color(1.0f, 1.0f, 1.0f));
	light2.enable();
	m_lights.push_back(light2);

	gl::Light light3(gl::Light::POINT, 2);
	light3.setPosition(Vec3f(-100, 100, 100));
	light3.setAmbient(Color(1.0f, 1.0f, 1.0f));
	light3.setDiffuse(Color(1.0f, 1.0f, 1.0f));
	light3.setSpecular(Color(1.0f, 1.0f, 1.0f));
	light3.enable();
	m_lights.push_back(light3);*/

	// Load shader
	try
	{
		m_fileMonitorVert = FileMonitor::create(getAssetPath("shader.vert"));
		m_fileMonitorFrag = FileMonitor::create(getAssetPath("shader.frag"));
		m_shader = gl::GlslProg::create(loadAsset("shader.vert"), loadAsset("shader.frag"));
		m_shaderHasErrors = false;
	}
	catch(gl::GlslProgCompileExc& exc)
	{
		m_shaderHasErrors = true;
		app::console() << "Shader load or compile error:" << std::endl;
		app::console() << exc.what() << std::endl;
		DBG("GlslProgCompileExc", std::string(exc.what()));
	}

	// Setup matrix
	m_matrix.setToIdentity();
	m_matrix.translate(Vec3f::zero());
	m_matrix.rotate(Vec3f::zero());
	m_matrix.scale(Vec3f::one());
}

void ImrodApp::update()
{
	if(m_fileMonitorVert->hasChanged() || m_fileMonitorFrag->hasChanged())
	{
		try
		{
			m_shader = gl::GlslProg::create(loadAsset("shader.vert"), loadAsset("shader.frag"));
			m_shaderHasErrors = false;
		}
		catch(gl::GlslProgCompileExc& exc)
		{
			m_shaderHasErrors = true;
			app::console() << "Shader load or compile error:" << std::endl;
			app::console() << exc.what() << std::endl;
			DBG("GlslProgCompileExc", std::string(exc.what()));
		}
	}

	DBG("bindas1", (int)bindas1);
	DBG("uniformas1", uniformas1);
	DBG("bindas2", (int)bindas2);
	DBG("uniformas2", uniformas2);
}

void ImrodApp::draw()
{
	// Clear the window
	gl::clear(ColorAf::gray(0.6f));
	gl::color(Color::white());
	gl::enable(GL_TEXTURE_2D);
	//gl::enable(GL_NORMALIZE);
	//gl::enable(GL_LIGHTING);
	// Setup camera
	gl::pushMatrices();
	gl::setMatrices(m_mayaCamera.getCamera());

	//m_texDiffuse->bind(0);
	//gl::drawCube(Vec3f::zero(), Vec3f(10.0f, 10.0f, 10.0f));
	//return;

	/*for(auto light : m_lights)
	{
		light.update(m_camera);
	}*/

	// Enable 3D rendering
	gl::enableDepthRead();
	gl::enableDepthWrite();

	// Draw ----------------------------------------------------------------------
	if(!m_shaderHasErrors)
	{
		m_texDiffuse->bind(bindas1);
		m_texNormal->bind(bindas2);
		//m_texAO->bind(2);

		// Bind shader
		m_shader->bind();
		m_shader->uniform("projection", m_mayaCamera.getCamera().getProjectionMatrix());
		m_shader->uniform("view", m_mayaCamera.getCamera().getModelViewMatrix());
		m_shader->uniform("model", m_matrix);

		m_shader->uniform("texDiffuse", uniformas1);
		m_shader->uniform("texNormal", uniformas2);
		//m_shader->uniform("texAO", 2);

		GLint texDiffuseLocation = m_shader->getUniformLocation("texDiffuse");
		//GLint texNormalLocation = m_shader->getUniformLocation("texNormal");
		//GLint texAOLocation = m_shader->getUniformLocation("texAO");

		DBG("uniform location", texDiffuseLocation);

		//GLint index = 0;
		//glUniform1i(texDiffuseLocation, index);
		//glActiveTexture(GL_TEXTURE0 + index);
		//glBindTexture(GL_TEXTURE_2D, m_texDiffuse->getId());
		////glBindSampler(index, texDiffuseLocation);

		//index = 1;
		//glUniform1i(texNormalLocation, index);
		//glActiveTexture(GL_TEXTURE0 + index);
		//glBindTexture(GL_TEXTURE_2D, m_texNormal->getId());
		////glBindSampler(index, texDiffuseLocation);

		//index = 2;
		//glUniform1i(texAOLocation, index);
		//glActiveTexture(GL_TEXTURE0 + index);
		//glBindTexture(GL_TEXTURE_2D, m_texAO->getId());
		////glBindSampler(index, texDiffuseLocation);

		gl::pushModelView();
		gl::multModelView(m_matrix);
		gl::draw(m_mesh);
		gl::popModelView();

		// Unbind shader
		m_shader->unbind();
	}
	// End draw ----------------------------------------------------------------------

	//gl::disable(GL_LIGHTING);

	// Disable 3D rendering
	gl::disableDepthWrite();
	gl::disableDepthRead();



	// Restore matrices
	gl::popMatrices();

	// Enable 2D rendering
	gl::setMatricesWindow(getWindowSize());
	gl::setViewport(getWindowBounds());

	//if(m_shaderHasErrors)
	{
		Debug::get().draw(ColorAf::white());
	}
}

void ImrodApp::resize()
{
	m_camera.setAspectRatio(getWindowAspectRatio());
	m_mayaCamera.setCurrentCam(m_camera);
}

void ImrodApp::mouseDown(MouseEvent event)
{
	m_mayaCamera.mouseDown(event.getPos());
}

void ImrodApp::mouseDrag(MouseEvent event)
{
	m_mayaCamera.mouseDrag(event.getPos(), event.isLeftDown(), event.isMiddleDown(), event.isRightDown());
}

void ImrodApp::keyDown(KeyEvent event)
{
	switch(event.getCode())
	{
// 		case KeyEvent::KEY_f:
// 		{
// 			setFullScreen(!isFullScreen());
// 			break;
// 		}
		case KeyEvent::KEY_ESCAPE:
		{
			quit();
			break;
		}
		case KeyEvent::KEY_F5:
		{
			try
			{
				m_shader = gl::GlslProg::create(loadAsset("shader.vert"), loadAsset("shader.frag"));
				m_shaderHasErrors = false;
			}
			catch(gl::GlslProgCompileExc& exc)
			{
				m_shaderHasErrors = true;
				app::console() << "Shader load or compile error:" << std::endl;
				app::console() << exc.what() << std::endl;
				DBG("GlslProgCompileExc", std::string(exc.what()));
			}

			break;
		}
		case KeyEvent::KEY_q:
			{
				bindas1++;
				break;
			}
		case KeyEvent::KEY_a:
			{
				bindas1--;
				break;
			}
		case KeyEvent::KEY_w:
			{
				uniformas1++;
				break;
			}
		case KeyEvent::KEY_s:
			{
				uniformas1--;
				break;
			}
		case KeyEvent::KEY_e:
			{
				bindas2++;
				break;
			}
		case KeyEvent::KEY_d:
			{
				bindas2--;
				break;
			}
		case KeyEvent::KEY_r:
			{
				uniformas2++;
				break;
			}
		case KeyEvent::KEY_f:
			{
				uniformas2--;
				break;
			}
	}
}


CINDER_APP_NATIVE(ImrodApp, RendererGl)
