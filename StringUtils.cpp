#include "StringUtils.h"
#include <sstream>

std::string getFilename(std::string path){
	// separate the file name from the rest of the path
	std::stringstream stream(path);
	std::string part;
	while(std::getline(stream,part,'/')){
	}
	// remove the file extension (just after the first dot)
	std::stringstream nameStream(part);
	std::getline(nameStream, part,'.');
	return part;
}
