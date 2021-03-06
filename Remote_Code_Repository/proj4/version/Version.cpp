#include "stdafx.h"
#include "Version.h"

using namespace VersionSystem;
//initialize static member in source code
map<string, int> Version::versionStore;

void Version::add(const string& filename) {
	//if file name exists, update version
	if (versionStore.count(filename) == 1) {
		versionStore[filename]++;
	}
	//if not, simply create it
	else {
		versionStore[filename] = 1;
	}
}

//get the map
std::map<string, int> Version::getVersion() {
	return versionStore;
}

void Version::change(const string& filename, const int& version) {
	cout << endl;
	if (version >= 1) {
		//if filename exist,change the file version
		if (versionStore.count(filename) == 1) {
			cout << "  The newest version of " << filename << " is " << versionStore[filename] << endl;
			cout << "  Now change it to " << version << endl;
			versionStore[filename] = version;
		}
		else {
			Utilities::StringHelper::title(filename + " doesn't exist in version system, please enter filename without extension");
		}
	}
	else {
		Utilities::StringHelper::title("version number can't be less than 1");
	}
}


string Version::getFileName_Version(const string& fileName) {
	//bind file name and version
	if (versionStore[fileName] > 1) {
		return fileName + "." + to_string(versionStore[fileName]);
	}
	//if version is 1, ignore binding, example.h is version 1
	if (versionStore[fileName] == 1) {
		return fileName;
	}
	cout << "  " << fileName << " doesn't exist in version system" << endl;
	return "";
}


void Version::showVersion() {
	//show key and value in version system
	cout << endl;
	Utilities::StringHelper::title("show version system");
	cout << endl;
	for (auto item : versionStore)
	{
		cout << "  key: " << item.first << "  value: " << item.second << endl;
	}

}


//test stub
#ifdef TEST_VERSION
int main()
{
	//add "123" twice
	Version::add("123");
	Version::add("123");
	Version::add("456");
	//show version management 
	for (auto item : VersionSystem::Version::getVersion())
	{
		cout << "key: " << item.first << "  value: " << item.second << endl;
	}
	return 0;
}
#endif

