#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "14";
	static const char MONTH[] = "05";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.05";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 0;
	static const long MINOR = 13;
	static const long BUILD = 1352;
	static const long REVISION = 7520;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 2451;
	#define RC_FILEVERSION 0,13,1352,7520
	#define RC_FILEVERSION_STRING "0, 13, 1352, 7520\0"
	static const char FULLVERSION_STRING[] = "0.13.1352.7520";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 52;
	

}
#endif //VERSION_H
