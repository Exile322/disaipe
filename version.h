#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "18";
	static const char MONTH[] = "05";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.05";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 0;
	static const long MINOR = 16;
	static const long BUILD = 1617;
	static const long REVISION = 9005;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 2962;
	#define RC_FILEVERSION 0,16,1617,9005
	#define RC_FILEVERSION_STRING "0, 16, 1617, 9005\0"
	static const char FULLVERSION_STRING[] = "0.16.1617.9005";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 17;
	

}
#endif //VERSION_H
