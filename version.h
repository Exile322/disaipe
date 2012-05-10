#ifndef VERSION_H
#define VERSION_H

namespace AutoVersion{
	
	//Date Version Types
	static const char DATE[] = "10";
	static const char MONTH[] = "05";
	static const char YEAR[] = "2012";
	static const char UBUNTU_VERSION_STYLE[] = "12.05";
	
	//Software Status
	static const char STATUS[] = "Alpha";
	static const char STATUS_SHORT[] = "a";
	
	//Standard Version Type
	static const long MAJOR = 0;
	static const long MINOR = 10;
	static const long BUILD = 1047;
	static const long REVISION = 5810;
	
	//Miscellaneous Version Types
	static const long BUILDS_COUNT = 1926;
	#define RC_FILEVERSION 0,10,1047,5810
	#define RC_FILEVERSION_STRING "0, 10, 1047, 5810\0"
	static const char FULLVERSION_STRING[] = "0.10.1047.5810";
	
	//These values are to keep track of your versioning state, don't modify them.
	static const long BUILD_HISTORY = 47;
	

}
#endif //VERSION_H
