#pragma once

#include "../yttrium/yttrium.h"

#include "../../libInfinite/tags/Tag.h"
#include "../../libInfinite/tags/handles/modeHandle.h"

#include <thread>
#include <functional>
#include <vector>
#include <mutex>

class IEViewer{
public:
	enum WindowTypes{
		IEV_GLFW
	};
	int createWindow(int type, int width, int height);
	int createGLFWBasedWindow(int width, int height);
	int startLoop();
	void stopLoop();
	void loadLODMesh(void* geo, void* lodData, Tag* tag);
	void addMesh(void* geo, void* meshData, Tag* tag);
	void addRenderGeo(render_geometryHandle* handle, uint32_t index, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale);
	void addRenderGeo(render_geometryHandle* handle, uint32_t index, glm::vec3 position, glm::mat4 rotation, glm::vec3 scale);

	void loadBuffers(Tag* tag);
private:
	int mainLoop();

	// this function will wait until the work queue is unlocked, so it may wait for a bit if there is some work that has to be completed first
	void emplaceInQueue(std::function<void(void)> const& func);

	bool initialized = false;
	bool shouldRun = false;
	ytr::RendererInterface* renderer;
	ytr::World* world;
	ytr::ResourceManager* ResMan;

	// very much not ideal, but whatever
	ytr::Camera* cam;

	std::thread mainRendererThread;

	std::vector<std::function<void(void)>> work_queue;
	std::mutex workQueueMutex;
	uint32_t shader;
};
