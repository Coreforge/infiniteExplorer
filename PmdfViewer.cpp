#include "PmdfViewer.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

#include "MainWindow.h"
extern MainWindow* globalWindowPointer;

PmdfViewer::PmdfViewer(){
	builder = Gtk::Builder::create_from_file("res/bsp.glade");
	Gtk::Grid* root;
	builder->get_widget("root", root);
	add(*root);
	root->show();

	Gtk::Button* loadBSP;
	builder->get_widget("loadMap", loadBSP);
	loadBSP->signal_clicked().connect([this]{
		if(tag != nullptr){
			/*int c = tag->getGeoInstanceCount();
			for(int i = 0; i < c; i++){
				auto inst = tag->getGeoInstanceInfo(i);
				if(inst.geo == nullptr){
					// later, maybe add an error here (maybe like source?), but for now, just skip it
					continue;
				}
				glm::mat3 rot_mat(inst.forward, inst.left, inst.up);
				//glm::mat3 rot_mat(glm::vec3(inst.up.x,inst.forward.x,inst.left.x), glm::vec3(inst.up.y,inst.forward.y,inst.left.y), glm::vec3(inst.up.z,inst.forward.z,inst.left.z));
				//glm::mat3 rot_mat(inst.up, inst.forward, inst.left);
				glm::mat4 bigrotmat(rot_mat);
				//bigrotmat = glm::transpose(bigrotmat);
				glm::vec3 rotation = glm::eulerAngles(glm::quat_cast(rot_mat));
				rotation = glm::degrees(rotation);
				glm::vec4 rotatedPos = glm::rotate(glm::radians(-90.0f), glm::vec3(1.0,0.0,0.0)) * glm::vec4(inst.position,1.0f);
				globalWindowPointer->viewer3D.addRenderGeo(&inst.geo->geoHandle, inst.meshIndex, inst.position, bigrotmat, -inst.scale); // glm::vec3(rotatedPos.x,rotatedPos.y,rotatedPos.z)
			}*/
			globalWindowPointer->viewer3D.addRenderGeo(&(dynamic_cast<pmdfHandle*>(tag))->geoHandle, 0, glm::vec3(0.0,0.0,0.0), glm::vec3(0.0,0.0,0.0), glm::vec3(1.0,1.0,1.0));
		}
	});


	tag = nullptr;
	item = nullptr;
}

PmdfViewer::~PmdfViewer(){

}

void PmdfViewer::setItem(Item* item){
	this->item = item;

	if(item->tagManager != nullptr){
		Tag* tmptag = item->tagManager->getTag(item->moduleItem->assetID);
		tag = dynamic_cast<pmdfHandle*>(tmptag);
		if(tag == nullptr){
			return;
		}
	}
}
