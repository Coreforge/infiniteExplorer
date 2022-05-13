
class FileEntry{

public:
	// onClick functions. If both aren't nullptr, onClickID has higher priority. Of none are set, nothing happens
	// onClick with an ID passed to it to identify the associated file
	void (*onClickID)(int) = nullptr;

	// onClick that takes a path
	void (*onClickPath)(char*) = nullptr;

	int ID;
	char* path;
	char* name;

	// internal for FileList
	bool selected;
};
