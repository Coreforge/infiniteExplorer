#include "AssImpExporter.h"
#include "AssimpHandle.h"

#include <stdlib.h>


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
	aimesh->mTextureCoords[uvChannel] = (aiVector3D*)malloc(sizeof(aiVector3D) * reducermap.getSize());
	float uvScale[2], uvOffset[2];
	handle->UVCompressionOffset(uvOffset, 0);
	handle->UVCompressionScale(uvScale, 0);

	for(int mappedV = 0; mappedV < reducermap.usedIndicies.size(); mappedV++){
		int v = reducermap.usedIndicies[mappedV];
		aimesh->mTextureCoords[uvChannel][mappedV].Set( ((((uint16_t*)inf.data)[v*2] /65535.0f) * uvScale[0]) + uvOffset[0],
				((((uint16_t*)inf.data)[(v*2) + 1] /65535.0f) * uvScale[1]) + uvOffset[1], 0);
	}
}

int AssImpExporter::addGeoPart(render_geometryHandle* handle, uint32_t index, uint32_t lod, uint32_t start, uint32_t count){
	// create a mesh
	scene->mMeshes = (aiMesh**)realloc(scene->mMeshes, (scene->mNumMeshes+1) * sizeof(aiMesh*));
	aiMesh* aimesh = (aiMesh*)malloc(sizeof(aiMesh));
	memset(aimesh,0,sizeof(aiMesh));
	scene->mMeshes[scene->mNumMeshes] = aimesh;
	scene->mNumMeshes++;
	int thisMesh = scene->mNumMeshes - 1;

	float posOffset[3];
	float posScale[3];
	handle->positionCompressionOffset(posOffset);
	handle->positionCompressionScale(posScale);

	aimesh->mAABB.mMin.Set(0+posOffset[0], 0+posOffset[1], 0+posOffset[2]);
	aimesh->mAABB.mMax.Set(0+posOffset[0]+posScale[0], 0+posOffset[1]+posScale[1], 0+posOffset[2]+posScale[2]);


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
			aimesh->mVertices = (aiVector3D*)malloc(sizeof(aiVector3D) * reducermap.getSize());	// may be a memory leak, idc
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
		}
	}

	uint8_t idxBufferType = handle->getIndexBufferType(index);
	int numFaces = 0;
	if(handle->getMeshFlags(index) & 1<<4){
		// no index buffer
		// PMDFs don't have parts from what I've seen, so I don't care about that here for now
		if(idxBufferType == INDEX_BUFFER_TYPE_TRIANGLE_LIST){
			numFaces = vertCount / 3;
			aimesh->mFaces = (aiFace*)malloc(sizeof(aiFace) * numFaces);
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
			aimesh->mFaces = (aiFace*)malloc(sizeof(aiFace) * numFaces);
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
			aimesh->mFaces = (aiFace*)malloc(sizeof(aiFace) * numFaces);
			int cnt = start;
			for(int i = 0; i < numFaces && cnt < start + count; i++){
				aimesh->mFaces[i].mNumIndices = 3;
				aimesh->mFaces[i].mIndices = (unsigned int*)malloc(sizeof(unsigned int) * 3);
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

void AssImpExporter::addRenderGeo(render_geometryHandle* handle, uint32_t index, glm::vec3 position, glm::mat3 rotation, glm::vec3 scale, std::string name, glm::mat4 meshbasetransform){
	if(scene == nullptr){
		logger->log(LOG_LEVEL_INFO, "No exporter scene, autocreating one\n");
		newScene();
	}
	// create the node for this instance
	aiNode* node = (aiNode*)malloc(sizeof(aiNode));
	memset(node,0,sizeof(aiNode));
	scene->mRootNode->addChildren(1, &node);
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
	node->mMeshes = (unsigned int*)malloc(sizeof(int) * partCount);

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
			int meshidx = addGeoPart(handle, index, 0, pInfo.indexStart, pInfo.indexCount);
			meshinfo->indicies.emplace_back(meshidx);
			node->mMeshes[part] = meshidx;
		}
	}

	meshinfo->initialized = true;

/*	// create a mesh
	scene->mMeshes = (aiMesh**)realloc(scene->mMeshes, (scene->mNumMeshes+1) * sizeof(aiMesh*));
	aiMesh* aimesh = (aiMesh*)malloc(sizeof(aiMesh));
	memset(aimesh,0,sizeof(aiMesh));
	scene->mMeshes[scene->mNumMeshes] = aimesh;
	scene->mNumMeshes++;
	meshinfo->index = scene->mNumMeshes - 1;
	meshinfo->initialized = true;
	node->mMeshes[0] = meshinfo->index;	// use the last added mesh

	aimesh->mAABB.mMin.Set(0+posOffset[0], 0+posOffset[1], 0+posOffset[2]);
	aimesh->mAABB.mMax.Set(0+posOffset[0]+posScale[0], 0+posOffset[1]+posScale[1], 0+posOffset[2]+posScale[2]);

	int vertCount = 0;
	for(int i = 0; i < 19; i++){
		uint16_t vtxidx = handle->getVertexBufferIndex(index, 0, i);
		if(vtxidx == 0xffff){
			continue;
		}

		bufferInfo inf = handle->getVertexBuffer(vtxidx);
		switch(i){
		case 0:
			// vertex

			aimesh->mNumVertices = inf.count;
			vertCount = inf.count;
			aimesh->mVertices = (aiVector3D*)malloc(sizeof(aiVector3D) * inf.count);	// may be a memory leak, idc
			for(int v = 0; v < inf.count; v++){
				// kinda unreadable, but it just normalises the uint and applies the offset/scale
				aimesh->mVertices[v].Set((((uint16_t*)inf.data)[v*4]/65535.0f)*posScale[0]+posOffset[0],
						(((uint16_t*)inf.data)[(v*4)+1]/65535.0f)*posScale[1]+posOffset[1],
						(((uint16_t*)inf.data)[(v*4)+2]/65535.0f)*posScale[2]+posOffset[2]);
			}
			break;
		}
	}

	uint8_t idxBufferType = handle->getIndexBufferType(index);
	int numFaces = 0;
	if(handle->getMeshFlags(index) & 1<<4){
		// no index buffer
		if(idxBufferType == INDEX_BUFFER_TYPE_TRIANGLE_LIST){
			numFaces = vertCount / 3;
			aimesh->mFaces = (aiFace*)malloc(sizeof(aiFace) * numFaces);
			int cnt = 0;
			for(int i = 0; i < numFaces; i++){
				aimesh->mFaces[i].mNumIndices = 3;
				aimesh->mFaces[i].mIndices = (unsigned int*)malloc(sizeof(unsigned int) * 3);
				aimesh->mFaces[i].mIndices[0] = cnt++;
				aimesh->mFaces[i].mIndices[1] = cnt++;
				aimesh->mFaces[i].mIndices[2] = cnt++;
			}
		}
		if(idxBufferType == INDEX_BUFFER_TYPE_TRIANGLE_STRIP){
			numFaces = vertCount - 2;
			aimesh->mFaces = (aiFace*)malloc(sizeof(aiFace) * numFaces);
			for(int i = 0; i < numFaces; i++){
				aimesh->mFaces[i].mNumIndices = 3;
				aimesh->mFaces[i].mIndices = (unsigned int*)malloc(sizeof(unsigned int) * 3);
				aimesh->mFaces[i].mIndices[0] = i;
				aimesh->mFaces[i].mIndices[1] = i + 1;
				aimesh->mFaces[i].mIndices[2] = i + 2;
			}
		}

	} else {
		// index buffer is present
		uint16_t idx_idx = handle->getIndexBufferIndex(index, 0);
		bufferInfo inf = handle->getIndexBuffer(idx_idx);

		if(idxBufferType == INDEX_BUFFER_TYPE_TRIANGLE_LIST){
			numFaces = inf.count / 3;
			aimesh->mFaces = (aiFace*)malloc(sizeof(aiFace) * numFaces);
			int cnt = 0;
			for(int i = 0; i < numFaces; i++){
				aimesh->mFaces[i].mNumIndices = 3;
				aimesh->mFaces[i].mIndices = (unsigned int*)malloc(sizeof(unsigned int) * 3);
				aimesh->mFaces[i].mIndices[0] = getIndex(inf.stride, cnt++, inf.data);
				aimesh->mFaces[i].mIndices[1] = getIndex(inf.stride, cnt++, inf.data);
				aimesh->mFaces[i].mIndices[2] = getIndex(inf.stride, cnt++, inf.data);
			}
		}

		if(idxBufferType == INDEX_BUFFER_TYPE_TRIANGLE_STRIP){
			// due to restarts, this could be a bit more complicated than I'm willing to deal with rn (count the restarts)
			//numFaces = inf.count / 3;
		}
	}
	aimesh->mNumFaces = numFaces;
	aimesh->mPrimitiveTypes |= aiPrimitiveType_TRIANGLE;*/


}

void AssImpExporter::exportScene(std::string path){
	if(scene == nullptr){
		logger->log(LOG_LEVEL_ERROR, "No Scene to export!\n");
		return;
	}
	auto ret = assExporter.Export(scene, "collada", path, aiProcess_ConvertToLeftHanded);
}

void AssImpExporter::setLogger(Logger* logger){
	this->logger = logger;
}

void AssImpExporter::newScene(){
	const aiScene* cscene;
	cscene = importer.ReadFileFromMemory("# Blender v2.93.3 OBJ File: ''\n# www.blender.org\nmtllib untitled.mtl",69,0,"obj");	// this is kinda dumb, but the best way I found
	aiCopyScene(cscene, &scene);
	//scene = (aiScene*)importer.GetOrphanedScene();
	meshinfoHandles.clear();
	// the obj exporter needs at least one material
	scene->mMaterials = (aiMaterial**)realloc(scene->mMaterials, (++scene->mNumMaterials) * sizeof(aiMaterial*));
	aiMaterial* aimat = (aiMaterial*)malloc(sizeof(aiMaterial));
	memset(aimat,0,sizeof(aiMaterial));
	scene->mMaterials[0] = aimat;

	// rotate everything to be correct (I think) in openGL coordinates
	scene->mRootNode->mTransformation.b2 = 0;
	scene->mRootNode->mTransformation.b3 = 1;
	scene->mRootNode->mTransformation.c2 = 1;
	scene->mRootNode->mTransformation.c3 = 0;

	logger->log(LOG_LEVEL_INFO, "Created new Scene\n");
}
