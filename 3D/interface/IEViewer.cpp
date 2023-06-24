#include <yttriumGL.h>
#include <yttrium.h>
#include "IEViewer.h"

#include <Window/WindowInput.h>

#include <thread>
#include <chrono>

#include <fstream>
#include <iostream>
#include <string>

#include "InfGLModelRenderer.h"
#include "InfMesh.h"

float speed = 6.0f;
float rotationspeed = 0.05f;

const char vtxSrc[] = "#version 330 core\n"
                      "layout (location=0) in vec3 pos;\n"
		"layout (location=5) in vec4 normal;\n"
                      "uniform mat4 transmat;\n"
                      "uniform vec3 col;\n"
                      "out vec3 unicolor;\n"
                      "out vec2 texcoords;\n"
		"out vec3 norm;\n"
                      "layout (location=1) in vec2 albcoords;\n"
                      "void main(){\n"
					//"pos = pos * 1000;\n"
                      "gl_Position = transmat * vec4(pos,1.0);\n"
					//"gl_Position = vec4(pos.xy*1000,0.5,gl_Position.w);\n"
		//"gl_Position.w *= -1.0;\n"
		//"gl_Position.z *= -1.0;\n"
                      //"unicolor = vec3((transmat * vec4(0.0,0.0,0.0,1.0)).xyz);\n"
                      "unicolor = vec3(0.9,0.9,0.9);//col;\n"
					"gl_PointSize = 10;\n"
		"norm = normalize(normal.xyz);\n"
                      //"texcoords = albcoords;\n"
                      "}\n";

const char frgSrc[] = "#version 330 core\n"
                      "out vec4 color;\n"
                      "in vec3 unicolor;\n"
                      "in vec2 texcoords;\n"
		"in vec3 norm;\n"
                      //"uniform sampler2D albedoSampler;\n"
                      "void main(){\n"
		"vec3 light_dir = normalize(vec3(0.5,0.5,0.5));\n"
                      //"color=vec4(texcoords.rg,0.5,1.0);\n"
                      "color = vec4(1.0,1.0,1.0,1.0);"
		"color.rgb *= max(dot(light_dir, norm),0.15);\n"
                      //"color = texture(albedoSampler,texcoords);\n"
                      //"color = vec4(color.rg,gl_FragCoord.z,color.a);\n"
                      "}\n";


struct windowUserDataStr{
	ytr::Camera* cam;
	ytr::renderWindow* window;
	bool flyCam = true;
	float originDistance = 1.0f;
};

void moveCamera(ytr::Camera* camera,glm::vec4 offset){
    glm::mat4 mat(1.0f);
    // rotation matrix to convert the offset vector into world space (also defines on which axis the camera can be moved)
    mat = glm::rotate(mat,glm::radians(camera->transform.rotation.y),glm::vec3(0.0,1.0,0.0));
    // apply rotation matrix to get the offset for the camera
    offset = offset * mat;
    // add the offset to the current position
    camera->transform.position += glm::vec3(offset.x,offset.y,offset.z);
}

void arcballCameraUpdate(ytr::Camera* camera, float radius){
	/*glm::vec4 pos = glm::vec4(-1.0,-1.0,-1.0,1.0);
	glm::mat4 mov(1.0f);
	mov = glm::translate(mov, glm::vec3(-radius,0.0,0.0));
	glm::mat4 rot(1.0f);
	rot = glm::rotate(rot,glm::radians(camera->transform.rotation.x),glm::vec3(1.0,0.0,0.0));
	rot = glm::rotate(rot,glm::radians(camera->transform.rotation.y),glm::vec3(0.0,1.0,0.0));
	rot = glm::rotate(rot,glm::radians(camera->transform.rotation.z),glm::vec3(0.0,0.0,1.0));
	pos = pos * mov;
	mov = mov * rot;
	pos = pos * rot;
	pos = glm::normalize(pos) * radius;
	pos += glm::normalize(glm::vec4(-1.0,-1.0,-1.0,1.0)) * radius;
	glm::vec3 forward;
	forward.x = cos(glm::radians(camera->transform.rotation.y));
	forward.z = sin(glm::radians(camera->transform.rotation.y));
	forward.y = sin(glm::radians(camera->transform.rotation.x));
	//forward = glm::normalize(forward);
	forward *= radius;
	camera->transform.position = glm::vec3(pos.x,pos.y,pos.z);*/
	camera->offset = glm::vec3(-radius,-radius,-radius);
	printf("radius: %f\n",radius);
}

void wCallback(double delta,void* data){
    // how the camera is supposed to move relative to the camera
	windowUserDataStr* userdta = (windowUserDataStr*)data;
	ytr::Camera* cam = userdta->cam;
	if(userdta->flyCam){
		glm::vec4 mov(0.0,0.0,delta*speed,0.0);
    	moveCamera(cam,mov);
	} else {
		userdta->originDistance -= delta*speed;
		arcballCameraUpdate(cam, userdta->originDistance);
	}
    printf("position: {%f %f %f}\n",cam->transform.position.x,cam->transform.position.y,cam->transform.position.z);
}

void aCallback(double delta,void* data){
	windowUserDataStr* userdta = (windowUserDataStr*)data;
	ytr::Camera* cam = userdta->cam;
	if(userdta->flyCam){
		glm::vec4 mov(delta*speed,0.0,0.0,0.0);
    	moveCamera(cam,mov);
	}
    printf("position: {%f %f %f}\n",cam->transform.position.x,cam->transform.position.y,cam->transform.position.z);
}

void sCallback(double delta,void* data){
	windowUserDataStr* userdta = (windowUserDataStr*)data;
	ytr::Camera* cam = userdta->cam;
	if(userdta->flyCam){
		glm::vec4 mov(0.0,0.0,-delta*speed,0.0);
    	moveCamera(cam,mov);
	} else {
		userdta->originDistance += delta*speed;
		arcballCameraUpdate(cam, userdta->originDistance);
	}
    printf("position: {%f %f %f}\n",cam->transform.position.x,cam->transform.position.y,cam->transform.position.z);
}

void spaceCallback(double delta,void* data){
	windowUserDataStr* userdta = (windowUserDataStr*)data;
	ytr::Camera* cam = userdta->cam;
	if(userdta->flyCam){
    	cam->transform.position.y -= delta*speed;
	}
}

void shiftCallback(double delta,void* data){
	windowUserDataStr* userdta = (windowUserDataStr*)data;
	ytr::Camera* cam = userdta->cam;
	if(userdta->flyCam){
    	cam->transform.position.y += delta*speed;
	}
}

void dCallback(double delta,void* data){
	windowUserDataStr* userdta = (windowUserDataStr*)data;
	ytr::Camera* cam = userdta->cam;
	if(userdta->flyCam){
		glm::vec4 mov(-delta*speed,0.0,0.0,0.0);
    	moveCamera(cam,mov);
	}
    printf("position: {%f %f %f}\n",cam->transform.position.x,cam->transform.position.y,cam->transform.position.z);
}

void xCallback(double x, double delta, void* data){
    // Screen X (width) modifies world Y (side to side)
	windowUserDataStr* userdta = (windowUserDataStr*)data;
	ytr::Camera* cam = userdta->cam;

	if(!userdta->window->getKeyStatus(ytr::INPUT_KEY_MOUSE_LEFT)){
		return;
	}

    cam->transform.rotation.y = cam->transform.rotation.y + (-x * rotationspeed);

    if(!userdta->flyCam){
    	arcballCameraUpdate(cam, userdta->originDistance);
    }

    printf("camera rotation: {%f %f %f}\n",cam->transform.rotation.x,cam->transform.rotation.y,cam->transform.rotation.z);
}

void yCallback(double y, double delta, void* data){
    // Screen Y (height) modifies world X (up and down)
	windowUserDataStr* userdta = (windowUserDataStr*)data;
	ytr::Camera* cam = userdta->cam;
	if(!userdta->window->getKeyStatus(ytr::INPUT_KEY_MOUSE_LEFT)){
		return;
	}
    //glm::mat4 mat(1.0f);
    //mat = glm::rotate(mat,(float)glm::radians(-y * rotationspeed),glm::vec3(0.0,1.0,0.0));
    //world->camera.rotation = mat * world->camera.rotation;
    cam->transform.rotation.x = cam->transform.rotation.x + (-y * rotationspeed);
    if(cam->transform.rotation.x > 90.0){
        cam->transform.rotation.x = 90.0;
    }
    if(cam->transform.rotation.x < -90.0){
        cam->transform.rotation.x = -90.0;
    }

    if(!userdta->flyCam){
    	arcballCameraUpdate(cam, userdta->originDistance);
    }

    printf("camera rotation: {%f %f %f}\n",cam->transform.rotation.x,cam->transform.rotation.y,cam->transform.rotation.z);
}

int IEViewer::createWindow(int type, int width, int height){
	if(initialized){
		return -2;
	}
	int r;
	if(type == WindowTypes::IEV_GLFW){
		r = createGLFWBasedWindow(width, height);
		if(r!= 0){
			return r;
		}
		r = startLoop();
		if(r!= 0){
			return r;
		}
		return 0;

	}
	return -1;
}

std::string readFile(std::string path){
    std::ifstream vtxStream;
    vtxStream.open(path, std::ios::binary | std::ios::in | std::ios::ate);
    std::streampos size= vtxStream.tellg();
    char* vtxBlk = new char[size];
    vtxStream.seekg(0,std::ios::beg);
    vtxStream.read(vtxBlk, size);
    vtxStream.close();
    std::string text(vtxBlk,size);
    delete[] vtxBlk;
    return text;
}

int IEViewer::createGLFWBasedWindow(int width, int height){

	int r;

	renderer = (ytr::RendererInterface*)new ytr::GLRenderer();
	renderer->window = (ytr::renderWindow*)new ytr::GLWindow();
	((ytr::GLWindow*)renderer->window)->renderer = (ytr::GLRenderer*) renderer;
	ResMan = new ytr::SimpleResourceManager();

	ytr::windowSettings settings;
	settings.width = width;
	settings.height = height;
	settings.windowType = ytr::WINDOWED;
	settings.AALevel = 0;
	settings.major = 4;
	settings.minor = 5;

	r = renderer->window->createWindow(settings);
	if(r != 0){
		return r;
	}
	renderer->window->cursorNormal();
	renderer->window->setupInput();

	cam = new ytr::Camera;
	cam->nearClip = 0.001;
	cam->farClip = 10000;
	cam->fov = 50;
	cam->aspect = 16.0/9.0;
	cam->setupMatrices();
	cam->transform.position = glm::vec3(-1.0,0.0,0.0);
	cam->transform.rotation = glm::vec3(0.0,-90.0,0.0);
	((ytr::GLRenderer*)renderer)->cam = cam;

	world = new ytr::World();
	initialized = true;

	windowUserDataStr* userdta = new windowUserDataStr();
	userdta->cam = cam;
	userdta->window = renderer->window;


	// inputs
    renderer->window->registerInput(&wCallback,ytr::INPUT_KEY_W,ytr::INPUT_KEY_REPEAT,userdta);
    renderer->window->registerInput(&aCallback,ytr::INPUT_KEY_A,ytr::INPUT_KEY_REPEAT,userdta);
    renderer->window->registerInput(&sCallback,ytr::INPUT_KEY_S,ytr::INPUT_KEY_REPEAT,userdta);
    renderer->window->registerInput(&dCallback,ytr::INPUT_KEY_D,ytr::INPUT_KEY_REPEAT,userdta);
    renderer->window->registerInput(&spaceCallback,ytr::INPUT_KEY_SPACE,ytr::INPUT_KEY_REPEAT,userdta);
    renderer->window->registerInput(&shiftCallback,ytr::INPUT_KEY_LSHIFT,ytr::INPUT_KEY_REPEAT,userdta);
    renderer->window->registerAnalogInput(&xCallback,ytr::INPUT_MOUSE_X,userdta);
    renderer->window->registerAnalogInput(&yCallback,ytr::INPUT_MOUSE_Y,userdta);
    shader = 0;


    std::string vtxSource = readFile("res/vtx.glsl");
    std::string frgSource = readFile("res/frg.glsl");

    shader = ytr::GLShaderManager::createShaderProgram(vtxSource.c_str(), frgSource.c_str());
    printf("Shader GLName: %d\n",shader);

    glEnable(GL_CULL_FACE);

    ((ytr::GLWindow*)renderer->window)->detachContext();

	return 0;
}

int IEViewer::startLoop(){
	shouldRun = true;
	mainRendererThread = std::thread([this]{
		((ytr::GLWindow*)renderer->window)->useContext();
		mainLoop();
	});

	return 0;
}

void IEViewer::stopLoop(){
	shouldRun = false;
}

void IEViewer::loadLODMesh(void* geo, void* lodData, Tag* tag){
	emplaceInQueue([geo, lodData, tag, this] {
		InfGLModelRenderer::ensureBuffersForPart((render_geometry*)geo, (LOD_render_data*)lodData, tag,ResMan);
	});
}

void IEViewer::addMesh(void* geo, void* meshData, Tag* tag){
	InfMesh* m = new InfMesh((render_geometry*)geo, (meshes*)meshData, tag, ResMan);
	ytr::MeshWorldNode* mwn = new ytr::MeshWorldNode();
	mwn->mesh = m;
	//mwn->transform.scale = glm::vec3(100.0,100.0,100.0);
	emplaceInQueue([mwn, this,m] {
		m->shaderId = shader;
		world->rootNode->addChild(mwn);
	});
}

void IEViewer::addRenderGeo(render_geometryHandle* handle, uint32_t index, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale){
	emplaceInQueue([handle,index,position,rotation,scale, this]{
		InfMesh* m = new InfMesh(handle, index);

		ytr::MeshWorldNode* mwn = new ytr::MeshWorldNode();
		mwn->mesh = m;
		mwn->transform.position = position;
		mwn->transform.rotation = rotation;
		mwn->transform.scale = scale;
		m->shaderId = shader;
		world->rootNode->addChild(mwn);
	});
}

void IEViewer::addRenderGeo(render_geometryHandle* handle, uint32_t index, glm::vec3 position, glm::mat4 rotation, glm::vec3 scale){
	emplaceInQueue([handle,index,position,rotation,scale, this]{
		InfMesh* m = new InfMesh(handle, index);

		ytr::MeshWorldNode* mwn = new ytr::MeshWorldNode();
		mwn->mesh = m;
		mwn->transform.position = position;
		mwn->transform.rotationMat = rotation;
		mwn->transform.rotationType = ytr::Transform::ROTATION_MATRIX;
		mwn->transform.scale = scale;
		m->shaderId = shader;
		world->rootNode->addChild(mwn);
	});
}

int IEViewer::mainLoop(){
	std::chrono::high_resolution_clock::time_point timeStart,timeStop;
	double delta, deltaMicros;
	while(shouldRun){
		// timing
		timeStart = timeStop;
		timeStop = std::chrono::high_resolution_clock::now();

		// loop starts here

		// do stuff that's in the work queue
		{
			std::lock_guard<std::mutex> guars(workQueueMutex);
			for(auto it = work_queue.begin(); it != work_queue.end(); it++){
				(*it)();
			}
			work_queue.clear();
		}


		renderer->renderFrame(world);
		renderer->window->inputLoop(delta);

		// loop ends here
		// timing
		deltaMicros = std::chrono::duration_cast<std::chrono::microseconds>(timeStop-timeStart).count();

		delta = deltaMicros/1000000.0f;
	}
	return 0;
}

void IEViewer::emplaceInQueue(std::function<void(void)> const& func){
	std::lock_guard<std::mutex> guard(workQueueMutex);
	work_queue.emplace_back(func);
}

void IEViewer::loadBuffers(Tag* tag){
	if(ResMan->getResource(tag->item->moduleItem->assetID) == nullptr){
		/*tag->loadAllResources();
		ytr::GLBufferResource* bufres = new ytr::GLBufferResource();
		bufres->createObjects(tag->getResourceSize());
		bufres->setData(0, tag->getResourceSize(), tag->getResource(0, tag->getResourceSize()));
		bufferResMan->createResource(bufres);*/
	}
	// otherwise do nothing, it's already loaded
}
