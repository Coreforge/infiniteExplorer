#include "BspViewer.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

#include <algorithm>

#include "MainWindow.h"
extern MainWindow* globalWindowPointer;

BspViewer::BspViewer(){
	builder = Gtk::Builder::create_from_file("res/bsp.glade");
	Gtk::Grid* root;
	builder->get_widget("root", root);
	add(*root);
	root->show();

	Gtk::Button* loadBSP;
	builder->get_widget("loadMap", loadBSP);
	loadBSP->signal_clicked().connect([this]{
		if(tag != nullptr){
			int c = tag->getGeoInstanceCount();
			int f = 0;
			for(int i = 0; i < c; i++){
				auto inst = tag->getGeoInstanceInfo(i);
				if(inst.geo == nullptr){
					// later, maybe add an error here (maybe like source?), but for now, just skip it
					continue;
				}
				if(inst.mesh_flags_override & MESH_FLAGS_OVERRIDE_MESH_IS_CUSTOM_SHADOW_CASTER){
					// just don't to anything with shadowcasters for now. Other wizards are allowed though.
					continue;
				}
				f++;

				auto meshdata = inst.geo->getMeshData(inst.meshIndex);

				glm::mat3 meshrot_mat(meshdata.forward, meshdata.left, meshdata.up);
				glm::mat4 meshposmat = glm::translate(meshdata.position);
				glm::mat4 meshscalemat = glm::scale(meshdata.scale);
				glm::mat4 meshtransform = meshposmat * glm::mat4(meshrot_mat) * meshscalemat;


				glm::mat3 rot_mat(inst.forward, inst.left, inst.up);
				//glm::mat3 rot_mat(glm::vec3(inst.up.x,inst.forward.x,inst.left.x), glm::vec3(inst.up.y,inst.forward.y,inst.left.y), glm::vec3(inst.up.z,inst.forward.z,inst.left.z));
				//glm::mat3 rot_mat(inst.up, inst.forward, inst.left);
				glm::mat4 bigrotmat(rot_mat);
				//bigrotmat = glm::transpose(bigrotmat);
				glm::vec3 rotation = glm::eulerAngles(glm::quat_cast(rot_mat));
				rotation = glm::degrees(rotation);
				glm::vec4 rotatedPos = glm::rotate(glm::radians(-90.0f), glm::vec3(1.0,0.0,0.0)) * glm::vec4(inst.position,1.0f);
				globalWindowPointer->viewer3D.addRenderGeo(&inst.geo->geoHandle, inst.meshIndex, inst.position, bigrotmat, -inst.scale); // glm::vec3(rotatedPos.x,rotatedPos.y,rotatedPos.z)
				//globalWindowPointer->currentExporter->addRenderGeo(&inst.geo->geoHandle, inst.meshIndex, inst.position, rot_mat, inst.scale,"owo");
			}
		}
	});

	Gtk::Button* exportBSP;
	builder->get_widget("addExportScene", exportBSP);
	exportBSP->signal_clicked().connect([this]{
		if(tag != nullptr){
			int c = tag->getGeoInstanceCount();
			int f = 0;
			std::vector<uint32_t> mats;
			for(int i = 0; i < c; i++){
				auto inst = tag->getGeoInstanceInfo(i);
				if(inst.geo == nullptr){
					// later, maybe add an error here (maybe like source?), but for now, just skip it
					continue;
				}
				if(inst.mesh_flags_override & MESH_FLAGS_OVERRIDE_MESH_IS_CUSTOM_SHADOW_CASTER){
					// just don't to anything with shadowcasters for now. Other wizards are allowed though.
					continue;
				}
				f++;

				auto meshdata = inst.geo->getMeshData(inst.meshIndex);
				for(int m = 0; m < inst.materials.size(); m++){
					if(inst.materials[m] == nullptr){
						continue;
					}
					int paramCount = inst.materials[m]->getParameterCount();
					for(int p = 0; p < paramCount; p++){
						auto param = inst.materials[m]->getParameter(p);
					}
					auto shader = inst.materials[m]->getShader();
					paramCount = shader->getParameterCount();
					if(std::find(mats.begin(), mats.end(), shader->item->moduleItem->assetID) == mats.end()){
						mats.emplace_back(shader->item->moduleItem->assetID);
					}
					for(int p = 0; p < paramCount; p++){
						auto param = shader->getParameter(p);
						std::string paramName = globalWindowPointer->lut.lookupID(param->nameId);
						if(param->typeInt == 0){
							// bitmap only for now
							auto bitmapparam = dynamic_cast<bitmapParameter*>(param.get());

						}
					}
				}

				glm::mat3 meshrot_mat(meshdata.forward, meshdata.left, meshdata.up);
				glm::mat4 meshposmat = glm::translate(meshdata.position);
				glm::mat4 meshscalemat = glm::scale(meshdata.scale);
				glm::mat4 meshtransform = meshposmat * glm::mat4(meshrot_mat) * meshscalemat;


				glm::mat3 rot_mat(inst.forward, inst.left, inst.up);
				//glm::mat3 rot_mat(glm::vec3(inst.up.x,inst.forward.x,inst.left.x), glm::vec3(inst.up.y,inst.forward.y,inst.left.y), glm::vec3(inst.up.z,inst.forward.z,inst.left.z));
				//glm::mat3 rot_mat(inst.up, inst.forward, inst.left);
				glm::mat4 bigrotmat(rot_mat);
				//bigrotmat = glm::transpose(bigrotmat);
				glm::vec3 rotation = glm::eulerAngles(glm::quat_cast(rot_mat));
				rotation = glm::degrees(rotation);
				glm::vec4 rotatedPos = glm::rotate(glm::radians(-90.0f), glm::vec3(1.0,0.0,0.0)) * glm::vec4(inst.position,1.0f);
				//globalWindowPointer->viewer3D.addRenderGeo(&inst.geo->geoHandle, inst.meshIndex, inst.position, bigrotmat, -inst.scale); // glm::vec3(rotatedPos.x,rotatedPos.y,rotatedPos.z)
				globalWindowPointer->currentExporter->addRenderGeo(&inst.geo->geoHandle, inst.meshIndex, inst.position, rot_mat, inst.scale,"owo");
			}
		}
	});

	tag = nullptr;
	item = nullptr;
}

BspViewer::~BspViewer(){

}

void BspViewer::setItem(Item* item){
	this->item = item;

	if(item->tagManager != nullptr){
		Tag* tmptag = item->tagManager->getTag(item->moduleItem->assetID);
		tag = dynamic_cast<sbspHandle*>(tmptag);
		if(tag == nullptr){
			return;
		}
	}
}
