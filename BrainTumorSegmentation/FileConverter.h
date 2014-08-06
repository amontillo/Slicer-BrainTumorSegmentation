#include "basicDefs.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#ifdef WIN32
   #include <direct.h>   // for mkdir
#else
   #include <sys/stat.h> // for mkdir
#endif

using namespace std;
using boost::lexical_cast;

extern char cFolderSeparator;

struct FileConverter
{

static void createFilepaths(string& strInputDataFilepath, string& strInputDirectoryToCreate, string& strInputSubDirectoryToCreate, string& strBVDDataFilepath, string& strBVFGroundTruthFilepath, 
	string& strXVDFilepath, string& strDatabaseFolder);

// Convert a  single CSV file into two BVD files and an XVD file 
static bool convertCSVToBVD(string& strInputDataFilepath, int nColumnContainingClassificationTarget);


};
