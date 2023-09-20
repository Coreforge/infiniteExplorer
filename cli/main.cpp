#include <argparse/argparse.hpp>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <vector>

#include "main.h"

#include <libInfinite/logger/ConsoleLogger.h>
#include <libInfinite/tags/TagManager.h>


#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

#include <3D/AssImpExporter.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../stb_image_write.h"


#ifdef _WIN64
#include <windows.h>
#include <fileapi.h>
//#include <direct.h>

//#warning WIN64 detected
#endif

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char* argv[]){
	argparse::ArgumentParser parser("infinteExplorerCLI");

	parser.add_argument("-d","--deploy").help("Path which is searched for module files")
			.default_value<std::vector<std::string>>({}).append();
	int verbosity = 0;
	parser.add_argument("-V", "--verbose").help("increase logging verbosity").action([&](const auto &) {++verbosity;}).
			append().default_value(false).implicit_value(true).nargs(0);

	parser.add_argument("-i", "--globalid").help("global ID of the main tag to load. This is an unsigned 32bit number").scan<'i',uint32_t>();

	parser.add_argument("-e", "--export").help("Export data from the tag. Currently only SBSP").default_value(false).implicit_value(true);

	parser.add_argument("-l", "--hash-lut").help("LUT for string IDs").default_value("");

	parser.add_argument("-o", "--output").help("Output path").default_value("--");

	parser.add_argument("-t", "--texture-out").help("Output for textures. The full path needs to already exist").default_value("");

	parser.add_argument("-m", "--mipmap").help("mipmap level to export textures at").default_value(0).scan<'i',int>();

	try{
		parser.parse_args(argc, argv);
	} catch(const std::runtime_error& err){
		std::cerr << err.what() << std::endl;
		std::cerr << parser;
		return -1;
	}

	FILE* loggerfile;

	if(parser.get<std::string>("-o") == "--"){
		loggerfile = stderr;
	} else {
		loggerfile = stdout;
	}

	int generalLogLevel = std::max(LOG_LEVEL_ERROR - verbosity,0);
	ConsoleLogger generalLogger(generalLogLevel, "", loggerfile);

	StringIDLUT stringLUT;
	stringLUT.setLogger(&generalLogger);
	if(parser.get<std::string>("-l") != ""){
		std::ifstream hashLUT(parser.get<std::string>("-l"));
		if(hashLUT.is_open()){
			std::string lutData;
			std::stringstream stream;
			stream << hashLUT.rdbuf();
			//hashLUT >> lutData;
			stringLUT.loadMap(stream.str());
		} else {
			generalLogger.log(LOG_LEVEL_ERROR, "Could not open stringID LUT at %s\n", parser.get<std::string>("-l"));
		}
	} else {
		generalLogger.log(LOG_LEVEL_WARNING, "No stringID LUT provided. stringIDs will just be converted to hex numbers!\n");
	}

	int libInfLogLevel = std::max(LOG_LEVEL_ERROR - verbosity,0);
	ConsoleLogger libInfLogger(libInfLogLevel,"[libInfinite] ", loggerfile);
	ModuleManager modman(&libInfLogger);
	TagManager tagman(&modman,&libInfLogger);

	for(auto path : parser.get<std::vector<std::string>>("-d")){
		loadPathRecursive(path, &libInfLogger, modman);
	}

	if(modman.modules.size() == 0){
		generalLogger.log(LOG_LEVEL_CRITICAL, "No modules loaded! Aborting\n");
		return -1;
	}
	generalLogger.log(LOG_LEVEL_INFO,"Loaded %d modules\n",modman.modules.size());
	modman.buildNodeTree();

	uint32_t globalId = 0;
	bool globalIdSet = false;
	if(auto id = parser.present<uint32_t>("--globalid")){
		globalId = *id;
		globalIdSet = true;
	}

	// check if this global ID exists, and get the associated item
	if(modman.assetIdItems.contains(globalId) && globalIdSet){
		if(parser.get<bool>("-e")){
			Tag* tag = tagman.getTag(globalId);
			if(modman.assetIdItems[globalId]->tagType == 'sbsp'){
				if(exportBSP(dynamic_cast<sbspHandle*>(tag), parser.get<std::string>("-o"), &generalLogger, stringLUT, &modman, parser.get<std::string>("-t"), parser.get<int>("-m"))){
					return -1;
				}
			}
		}
	} else if(globalIdSet) {
		generalLogger.log(LOG_LEVEL_CRITICAL, "That global ID doesn't exist!\n");
		return -1;
	}

}

int exportBSP(sbspHandle* handle, std::string out, Logger* logger, StringIDLUT& lut, ModuleManager* modman, std::string texout, int mipmap){
	if(handle == nullptr){
		logger->log(LOG_LEVEL_CRITICAL, "No BSP tag handle to export!\n");
		return -1;
	}
	AssImpExporter exporter;
	exporter.setLogger(logger);
	exporter.newScene();
	exporter.setStringIDLUT(lut);

	int c = handle->getGeoInstanceCount();
	int f = 0;
	int i;
	for(i = 0; i < c; i++){
		auto inst = handle->getGeoInstanceInfo(i);
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
		//globalWindowPointer->viewer3D.addRenderGeo(&inst.geo->geoHandle, inst.meshIndex, inst.position, bigrotmat, -inst.scale); // glm::vec3(rotatedPos.x,rotatedPos.y,rotatedPos.z)
		exporter.addRenderGeo(&inst.geo->geoHandle, inst.meshIndex, inst.position, rot_mat, inst.scale, inst.geo->item->moduleItem->path, inst.materials);
	}
	logger->log(LOG_LEVEL_INFO, "%d Instances in total, %d included in export (shadowcasters are excluded)\n", i, f);
	exporter.exportScene(out);
	if(texout != ""){
		// export textures here
		exporter.exportBitmaps(texout, modman, mipmap);
	}
	return 0;
}
// duplicate code, since this is currently in ModuleDisplayManager, which is only used in the GUI

void loadPathRecursive(std::string path, Logger* logger, ModuleManager& modMan){

	// this has to be done differently on POSIX-compliant systems and /windows/ ...sigh

#ifdef _WIN64
	// windows version
	WIN32_FIND_DATA find;
	HANDLE dirHandle;

	std::string searchPath = path + "/*";

	dirHandle = FindFirstFile(searchPath.c_str(), &find);

	if (dirHandle == INVALID_HANDLE_VALUE) {
		logger->log(LOG_LEVEL_WARNING,"Could not open %s\n", path.c_str());
		return;
	}

	do {
		std::string cPath = path + "\\" + find.cFileName;
		//printf("Found %s\n",cPath.c_str());
		if (!(strncmp(find.cFileName, ".", 2) && strncmp(find.cFileName, "..", 3))) {
			// . or .., skip those
			continue;
		}

		if (find.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			// is a directory
			//printf("This is a directory\n");
			loadPathRecursive(cPath, logger, modMan);

			// don't do the other stuff, which is only for files
			continue;
		}

		// this is only for files

		std::string name(find.cFileName);

		// all of this can just be copied from the posix version
		if (name.size() < 7) {
			// name is too short
			continue;
		}
		if (name.substr(name.size() - 7, 7) == ".module") {
			// extension should match
			if (modMan.addModule(cPath.c_str())) {
				// the module couldn't be loaded
				logger->log(LOG_LEVEL_ERROR,"Failed to load module %s, skipping!\n", cPath.c_str());
			}
		}


	} while (FindNextFile(dirHandle,&find));
#else
	// the posix version
	DIR* dir;
	dir = opendir(path.c_str());	// open the current directory
	if(!dir){
		// something went wrong opening the directory
		logger->log(LOG_LEVEL_WARNING,"Failed to open %s\n",path.c_str());
		return;
	}
	struct dirent* ent;
	ent = readdir(dir);
	struct stat buf;
	while(ent){
		// for every entry
		std::string cPath = path + "/" + ent->d_name;
		if(stat(cPath.c_str(),&buf)){
			// something went wrong with stat
			// report the error and just skip this entry
			logger->log(LOG_LEVEL_WARNING,"Failed to stat %s!\n",cPath.c_str());	// warning because it might not be an actual issue
			goto next;	// skip to the next entry
			continue;
		}
		if(S_ISDIR(buf.st_mode)){
			// another directory, check that one too
			if(strncmp(ent->d_name,".",2) && strncmp(ent->d_name,"..",3)){
				//printf("Looking in %s\n",cPath.c_str());
				loadPathRecursive(cPath, logger, modMan);
			}

		}
		if(S_ISREG(buf.st_mode)){
			// regular file, but that doesn't mean it's a module
			// first check the file extension (.module, the last 7 characters of the name)
			std::string name(ent->d_name);
			if(name.size() < 7){
				// name is too short
				goto next;
			}
			if(name.substr(name.size() - 7, 7) == ".module"){
				// extension should match
				if(modMan.addModule(cPath.c_str())){
					// the module couldn't be loaded
					logger->log(LOG_LEVEL_ERROR,"Failed to load module %s, skipping!\n",cPath.c_str());
				}
			}
		}
		next:
		ent = readdir(dir);	// next entry
	}
#endif
}
