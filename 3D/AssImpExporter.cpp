#include "AssImpExporter.h"
#include "AssimpHandle.h"

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>

#include <thread>
#include "../stb_image_write.h"
#include "libInfinite/BitmapBatchExtractor.h"
#include "libInfinite/MaterialJsonExporter.h"

//#include <png++/png.hpp>

#define INDEX_BUFFER_TYPE_TRIANGLE_LIST 3
#define INDEX_BUFFER_TYPE_TRIANGLE_STRIP 5

int getIndex(int stride, int index, void* data){
	if(stride == 2){
		return ((uint16_t*)data)[index];
	}
	if(stride == 4){
		return ((uint32_t*)data)[index];
	}
	return 0;
}

int AssImpExporter::indexReducerMap::addIndex(int index){
	if(indexMap.contains(index)){
		// has already been used
		return indexMap[index];
	}
	usedIndicies.emplace_back(index);
	indexMap[index] = indexMap.size();	// incrementing index
	return indexMap[index];
}

int AssImpExporter::indexReducerMap::getSize(){
	return indexMap.size();
}

int AssImpExporter::indexReducerMap::getIndex(int index){
	if(indexMap.contains(index)){
		// has already been used
		return indexMap[index];
	}
	return 0;	// ideally this should never be used, but eh
}

void AssImpExporter::addRenderGeo(render_geometryHandle* handle, uint32_t index, glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, std::string name){
	addRenderGeo(handle, index, position, glm::mat3(1.0), scale, name);
}

void AssImpExporter::addUV(render_geometryHandle* handle, bufferInfo& inf, aiMesh* aimesh, indexReducerMap& reducermap, int uvChannel){
	aimesh->mNumUVComponents[uvChannel] = 2;
	aimesh->mTextureCoords[uvChannel] = new aiVector3D[reducermap.getSize()];//(aiVector3D*)malloc(sizeof(aiVector3D) * reducermap.getSize());
	float uvScale[2], uvOffset[2];
	handle->UVCompressionOffset(uvOffset, uvChannel);
	handle->UVCompressionScale(uvScale, uvChannel);

	for(int mappedV = 0; mappedV < reducermap.usedIndicies.size(); mappedV++){
		int v = reducermap.usedIndicies[mappedV];
		aimesh->mTextureCoords[uvChannel][mappedV].Set(((((uint16_t*)inf.data)[v*2] /65536.0f) * uvScale[0]) + uvOffset[0],
				((((uint16_t*)inf.data)[(v*2) + 1] /65536.0f) * uvScale[1]) + uvOffset[1], 0);
	}
}

int AssImpExporter::addGeoPart(render_geometryHandle* handle, uint32_t index, uint32_t lod, uint32_t start, uint32_t count, unsigned int material){
	// create a mesh
	scene->mMeshes = (aiMesh**)realloc(scene->mMeshes, (scene->mNumMeshes+1) * sizeof(aiMesh*));
	aiMesh* aimesh = new aiMesh();//(aiMesh*)malloc(sizeof(aiMesh));
	//memset(aimesh,0,sizeof(aiMesh));
	scene->mMeshes[scene->mNumMeshes] = aimesh;
	scene->mNumMeshes++;
	int thisMesh = scene->mNumMeshes - 1;

	float posOffset[3];
	float posScale[3];
	handle->positionCompressionOffset(posOffset);
	handle->positionCompressionScale(posScale);

	aimesh->mAABB.mMin.Set(0+posOffset[0], 0+posOffset[1], 0+posOffset[2]);
	aimesh->mAABB.mMax.Set(0+posOffset[0]+posScale[0], 0+posOffset[1]+posScale[1], 0+posOffset[2]+posScale[2]);
	aimesh->mMaterialIndex = material;


	/*
	 * With each Assimp mesh only using one material (containing one part), copying the entire vertex buffer is rather useless.
	 * Only the indices that are actually used by this part get added to the map.
	 *
	 * To get the original index from the reduced index, use the usedIndicies vector
	 * To get the reduced index from the original index, use getIndex (or the unordered_map)
	 */
	indexReducerMap reducermap;
	if(handle->getMeshFlags(index) & 1<<4){
		// no index buffer, everything is used. Just create a map that's 1:1, makes the rest of the code easier
		for(int i = 0; i < count; i++){
			reducermap.addIndex(i);
		}
	} else {
		// there is an index buffer
		uint16_t idx_idx = handle->getIndexBufferIndex(index, lod);
		bufferInfo inf = handle->getIndexBuffer(idx_idx);
		for(int i = start; i < start + count; i++){
			reducermap.addIndex(getIndex(inf.stride, i, inf.data));
		}
	}

	int vertCount = 0;
	for(int i = 0; i < 19; i++){
		uint16_t vtxidx = handle->getVertexBufferIndex(index, lod, i);
		if(vtxidx == 0xffff){
			continue;
		}

		bufferInfo inf = handle->getVertexBuffer(vtxidx);

		/*
		 * This isn't all that memory efficient, as all vertices will be added for each part, while a lot are probably not used
		 * I could filter them out, but I don't think it's that big of a deal (just some more memory usage, and maybe the file is a bit bigger)
		 */
		switch(i){
		case 0:
			// position

			aimesh->mNumVertices = reducermap.getSize();
			vertCount = reducermap.getSize();
			aimesh->mVertices = new aiVector3D[reducermap.getSize()];//(aiVector3D*)malloc(sizeof(aiVector3D) * reducermap.getSize());	// may be a memory leak, idc
			for(int mappedV = 0; mappedV < reducermap.usedIndicies.size(); mappedV++){
				// kinda unreadable, but it just normalises the uint and applies the offset/scale
				int v = reducermap.usedIndicies[mappedV];
				aimesh->mVertices[mappedV].Set((((uint16_t*)inf.data)[v*4]/65535.0f)*posScale[0]+posOffset[0],
						(((uint16_t*)inf.data)[(v*4)+1]/65535.0f)*posScale[1]+posOffset[1],
						(((uint16_t*)inf.data)[(v*4)+2]/65535.0f)*posScale[2]+posOffset[2]);
			}
			break;

		case 1:
			// UV0

			addUV(handle, inf, aimesh, reducermap, 0);
			break;
		case 2:
			addUV(handle, inf, aimesh, reducermap, 1);
			break;

		case 3:
			addUV(handle, inf, aimesh, reducermap, 2);
			break;

		case 5:
			// normals

			aimesh->mNumVertices = reducermap.getSize();
			vertCount = reducermap.getSize();
			aimesh->mNormals = new aiVector3D[reducermap.getSize()];//(aiVector3D*)malloc(sizeof(aiVector3D) * reducermap.getSize());	// may be a memory leak, idc
			for(int mappedV = 0; mappedV < reducermap.usedIndicies.size(); mappedV++){
				// kinda unreadable, but it just normalises the uint and applies the offset/scale
				int v = reducermap.usedIndicies[mappedV];
				double xn = ((uint32_t*)inf.data)[v] & 0x3ff;
				double yn = (((uint32_t*)inf.data)[v] >> 10) & 0x3ff;
				double zn = (((uint32_t*)inf.data)[v] >> 20) & 0x3ff;
				double wn = (((uint32_t*)inf.data)[v] >> 30) & 0x3;
				xn = (xn / 1023.0f) * 2 - 1;
				yn = (yn / 1023.0f) * 2 - 1;
				zn = (zn / 1023.0f) * 2 - 1;
				wn = wn - 2;
				double wclampedthingy = (int)(wn > 0) - (int)(wn < 0);
				aimesh->mNormals[mappedV].Set(xn,
						yn,
						zn);
			}
			break;
		}
	}

	uint8_t idxBufferType = handle->getIndexBufferType(index);
	int numFaces = 0;
	if(handle->getMeshFlags(index) & 1<<4){
		// no index buffer
		// PMDFs don't have parts from what I've seen, so I don't care about that here for now
		if(idxBufferType == INDEX_BUFFER_TYPE_TRIANGLE_LIST){
			numFaces = vertCount / 3;
			aimesh->mFaces = new aiFace[numFaces];//(aiFace*)malloc(sizeof(aiFace) * numFaces);
			int cnt = 0;
			for(int i = 0; i < numFaces; i++){
				aimesh->mFaces[i].mNumIndices = 3;
				aimesh->mFaces[i].mIndices = (unsigned int*)malloc(sizeof(unsigned int) * 3);
				aimesh->mFaces[i].mIndices[0] = reducermap.getIndex(cnt++);
				aimesh->mFaces[i].mIndices[1] = reducermap.getIndex(cnt++);
				aimesh->mFaces[i].mIndices[2] = reducermap.getIndex(cnt++);
			}
		}
		if(idxBufferType == INDEX_BUFFER_TYPE_TRIANGLE_STRIP){
			numFaces = vertCount - 2;
			aimesh->mFaces = new aiFace[numFaces];//(aiFace*)malloc(sizeof(aiFace) * numFaces);
			for(int i = 0; i < numFaces; i++){
				aimesh->mFaces[i].mNumIndices = 3;
				aimesh->mFaces[i].mIndices = (unsigned int*)malloc(sizeof(unsigned int) * 3);
				aimesh->mFaces[i].mIndices[0] = reducermap.getIndex(i);
				aimesh->mFaces[i].mIndices[1] = reducermap.getIndex(i + 1);
				aimesh->mFaces[i].mIndices[2] = reducermap.getIndex(i + 2);
			}
		}

	} else {
		// index buffer is present
		uint16_t idx_idx = handle->getIndexBufferIndex(index, lod);
		bufferInfo inf = handle->getIndexBuffer(idx_idx);

		if(idxBufferType == INDEX_BUFFER_TYPE_TRIANGLE_LIST){
			numFaces = count / 3;
			aimesh->mFaces = new aiFace[numFaces];//(aiFace*)malloc(sizeof(aiFace) * numFaces);
			int cnt = start;
			for(int i = 0; i < numFaces && cnt < start + count; i++){
				aimesh->mFaces[i].mNumIndices = 3;
				aimesh->mFaces[i].mIndices = new unsigned int[3];//(unsigned int*)malloc(sizeof(unsigned int) * 3);
				// there's a chance for a segfault here if count is not divisible by 3 (though that would mean bad part info)
				aimesh->mFaces[i].mIndices[0] = reducermap.getIndex(getIndex(inf.stride, cnt++, inf.data));
				aimesh->mFaces[i].mIndices[1] = reducermap.getIndex(getIndex(inf.stride, cnt++, inf.data));
				aimesh->mFaces[i].mIndices[2] = reducermap.getIndex(getIndex(inf.stride, cnt++, inf.data));
			}
		}

		if(idxBufferType == INDEX_BUFFER_TYPE_TRIANGLE_STRIP){
			// due to restarts, this could be a bit more complicated than I'm willing to deal with rn (count the restarts)
			//numFaces = inf.count / 3;
		}
	}
	aimesh->mNumFaces = numFaces;
	aimesh->mPrimitiveTypes |= aiPrimitiveType_TRIANGLE;
	return thisMesh;
}

void AssImpExporter::addRenderGeo(render_geometryHandle* handle, uint32_t index, glm::vec3 position, glm::mat3 rotation, glm::vec3 scale, std::string name, std::vector<mat_Handle*> materials, glm::mat4 meshbasetransform){
	if(scene == nullptr){
		logger->log(LOG_LEVEL_INFO, "No exporter scene, autocreating one\n");
		newScene();
	}
	// create the node for this instance
	aiNode* node = new aiNode();//(aiNode*)malloc(sizeof(aiNode));
	//memset(node,0,sizeof(aiNode));
	scene->mRootNode->addChildren(1, &node);
	node->mName = aiString(name);
	//node->mName = name;


	float posOffset[3];
	float posScale[3];
	handle->positionCompressionOffset(posOffset);
	handle->positionCompressionScale(posScale);




	aiMatrix4x4 transmat;//(aiVector3D(scale.x,scale.y,scale.z), aiQuaternion(), aiVector3D(0,0,0));
	aiMatrix4x4 scalemat;
	scalemat.Scaling(aiVector3D(scale.x,scale.y,scale.z), scalemat);
	aiMatrix4x4 rotmat(aiMatrix3x3(rotation[0][0],rotation[1][0],rotation[2][0],
			rotation[0][1], rotation[1][1], rotation[2][1],
			rotation[0][2], rotation[1][2], rotation[2][2]));
	transmat.Translation(aiVector3D(position.x,position.y,position.z), transmat);

	// also the base transform (stored in the rtgo)
	aiMatrix4x4 baseTransform(meshbasetransform[0][0], meshbasetransform[1][0], meshbasetransform[2][0], meshbasetransform[3][0],
			meshbasetransform[0][1], meshbasetransform[1][1], meshbasetransform[2][1], meshbasetransform[3][1],
			meshbasetransform[0][2], meshbasetransform[1][2], meshbasetransform[2][2], meshbasetransform[3][2],
			meshbasetransform[0][3], meshbasetransform[1][3], meshbasetransform[2][3], meshbasetransform[3][3]);

	aiMatrix4x4 outmat = transmat * rotmat * scalemat * baseTransform;


	node->mTransformation = outmat;//aiMatrix4x4(aiVector3D(scale.x,scale.y,scale.z), aiQuaternion(), aiVector3D(position.x,position.y,position.z));

	auto meshinfo = handle->getMeshInfo<AssimpHandle>(index, 0, 1);

	uint32_t partCount = handle->getPartCount(index, 0);
	bool impliedPart = false;

	// PMDF uses an implied part
	if(partCount == 0){
		impliedPart = true;
		partCount = 1;
	}
	node->mNumMeshes = partCount;
	node->mMeshes = new unsigned int[partCount];//(unsigned int*)malloc(sizeof(int) * partCount);

	// currently I don't have a way to clear one channel, so this could mess things up
	if(meshinfo->initialized && meshinfo->indicies.size() == partCount){
		// all parts have already been added
		// all parts should ba added at once, or things get messed up a bit
		for(int i = 0; i < meshinfo->indicies.size(); i++){
			node->mMeshes[i] = meshinfo->indicies[i];
		}
		// nothing more to do
		return;
	}
	meshinfoHandles.emplace_back(meshinfo);		// keep a pointer around until the scene gets reset

	// load all the parts as individual meshes, as assimp only allows one material per mesh

	if(impliedPart){
		uint16_t vtxidx = handle->getVertexBufferIndex(index, 0, 0);	// pos buffer
		if(vtxidx != 65535){
			bufferInfo inf = handle->getVertexBuffer(vtxidx);
			int meshidx = addGeoPart(handle, index, 0, 0, inf.count);
			meshinfo->indicies.emplace_back(meshidx);
			node->mMeshes[0] = meshidx;
		}
	} else {
		for(int part = 0; part < partCount; part++){
			auto pInfo = handle->getPartInfo(index, 0, part);
			int matIndex = 0;
			if(pInfo.materialIndex < materials.size()){
				// correct material is in the vector
				matIndex = useMaterial(materials[pInfo.materialIndex]);
			}
			int meshidx = addGeoPart(handle, index, 0, pInfo.indexStart, pInfo.indexCount, matIndex);
			meshinfo->indicies.emplace_back(meshidx);
			node->mMeshes[part] = meshidx;
		}
	}

	meshinfo->initialized = true;

}

unsigned int AssImpExporter::useMaterial(mat_Handle* matHandle){
	uint32_t matGlobalId = matHandle->item->moduleItem->assetID;
	if(materialMap.contains(matHandle->item->moduleItem->assetID)){
		// already added
		return materialMap[matHandle->item->moduleItem->assetID];
	}
	usedMaterials.emplace(matHandle);

	// material needs to be added
	int matIndex = scene->mNumMaterials;
	scene->mMaterials = (aiMaterial**)realloc(scene->mMaterials, (++scene->mNumMaterials) * sizeof(aiMaterial*));
	aiMaterial* aimat = new aiMaterial();
	scene->mMaterials[matIndex] = aimat;
	materialMap[matGlobalId] = matIndex;
	auto params = matHandle->getParameters();
	std::string materialName = matHandle->item->moduleItem->path;
	aiString matName(materialName);
	aimat->AddProperty(&matName, AI_MATKEY_NAME);


	for(auto& [nameId, parameter] : params){
		std::string paramName = idLUT.lookupID(nameId);

		paramName = "$raw." + paramName;
		std::string stringRepr = parameter->toString();
		aiString aistr(stringRepr);


		/*
		 * Bitmap, Real, Bool, Int, and Color need special treatment, String and Preset are fine with the default toString
		 */
		switch(parameter->typeInt){
		case materialParameterBase::TYPE_BITMAP:
			usedBitmaps.emplace_back(std::dynamic_pointer_cast<bitmapParameter>(parameter)->globalId);
			break;
		default:
			break;
		}
		aimat->AddProperty(&aistr, paramName.c_str());
	}

	return matIndex;
}


void AssImpExporter::exportBitmaps(std::string path, ModuleManager* modman, int mipmap){
	std::unordered_map<uint32_t,int> exported;	// just used to quickly check if a bitmap has been exported

	std::vector<uint32_t> uniqueBitmaps;

	for(auto globalId : usedBitmaps){
		if(exported.contains(globalId)){
			continue;
		}
		exported[globalId] = 0;	// the value doesn't matter, I'm only using the map as an easy way to check for duplicates
		if(!modman->assetIdItems.contains(globalId)){
			logger->log(LOG_LEVEL_ERROR, "Bitmap 0x%08x cannot be found in the loaded modules!\n", globalId);
			continue;
		}
		uniqueBitmaps.emplace_back(globalId);
	}
	BitmapBatchExtractor batcher(modman, logger);
	batcher.extract(path, uniqueBitmaps, mipmap);

}

void AssImpExporter::exportScene(std::string path){
	if(scene == nullptr){
		logger->log(LOG_LEVEL_ERROR, "No Scene to export!\n");
		return;
	}
	//std::cout << jsonMaterials;
	auto ret = assExporter.Export(scene, "collada", path, aiProcess_ConvertToLeftHanded);
	std::ofstream jsonOutput(path + ".json");
	if(jsonOutput.is_open()){
		MaterialJsonExtractor jsonEx(idLUT);
		jsonOutput << jsonEx.toJson(std::vector<mat_Handle*>(usedMaterials.begin(), usedMaterials.end()));
		jsonOutput.flush();
		jsonOutput.close();
	}

}

void AssImpExporter::setLogger(Logger* logger){
	this->logger = logger;
}

void AssImpExporter::setStringIDLUT(StringIDLUT lut){
	this->idLUT = lut;
}

void AssImpExporter::newScene(){
	const aiScene* cscene;
	cscene = importer.ReadFileFromMemory("# Blender v2.93.3 OBJ File: ''\n# www.blender.org\nmtllib untitled.mtl",69,0,"obj");	// this is kinda dumb, but the best way I found
	aiCopyScene(cscene, &scene);
	//scene = (aiScene*)importer.GetOrphanedScene();
	meshinfoHandles.clear();
	materialMap.clear();
	usedMaterials.clear();
	usedBitmaps.clear();
	// the obj exporter needs at least one material
	scene->mMaterials = (aiMaterial**)realloc(scene->mMaterials, (++scene->mNumMaterials) * sizeof(aiMaterial*));
	aiMaterial* aimat = new aiMaterial();
	scene->mMaterials[0] = aimat;

	// rotate everything to be correct (I think) in openGL coordinates
	scene->mRootNode->mTransformation.b2 = 0;
	scene->mRootNode->mTransformation.b3 = 1;
	scene->mRootNode->mTransformation.c2 = -1;
	scene->mRootNode->mTransformation.c3 = 0;

	logger->log(LOG_LEVEL_INFO, "Created new Scene\n");
}
