#include "Controller.h"
#include "DecisionForestTester.h"
#include "FilenameManager.h"
#include "BvdFileManager.h"
#include "FileConverter.h"
#include "basicDefs.h"
//#include "AccuracyEvaluator.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <iostream>               // for std::cout
// #include <boost/filesystem.hpp>   // Commented out to avoid building boost on every platform for now just to do a mkdir 
#ifdef WIN32
   #include <direct.h>   // for mkdir
#else
   #include <sys/stat.h> // for mkdir
#endif

using namespace std;
using boost::lexical_cast;


void Controller::selectSettingsFileAndLoad()
{
	parametersFolder_ = FilenameManager::ExtractDirectory(strForestSettingsFilename);

	string strEnvironmentFile_;

	if (!strForestSettingsFilename.empty())  // if forest is empty (unspecified), leave strEnvironmentFile_ empty as well
		if (bEnvironmentAndForestSettingsAreInSameFolder_) strEnvironmentFile_ = parametersFolder_ +  "EnvironmentSettings.xml";  
		else 	strEnvironmentFile_ = parametersFolder_ + ".." + cFolderSeparator +  "EnvironmentSettings.xml";  

	bool bEnvironmentParametersLoaded = EnvironmentSettingsFileManager::readFromFile(strEnvironmentFile_, structEnvironmentParameters_);
	if (!bEnvironmentParametersLoaded)  
	{ 
		 cout << "Error reading environment settings from " << strEnvironmentFile_ << "\n";
		 FOREST_PARAMETERS_LOADED_ = false;  // Indicate we are bailing
	}
	else 
	{ 
		structEnvironmentParameters_.writeToConsole();
    	bool bForestParametersLoaded = ForestSettingsFileManager::readFromFile(strForestSettingsFilename,forestParams_);

		if (!bForestParametersLoaded )
		{
			cout << "Error reading the parameter settings file " << strForestSettingsFilename << "\n";

			FOREST_PARAMETERS_LOADED_ = false;  // Indicate we are bailing
		}
		else
		{       FOREST_PARAMETERS_LOADED_=true;
				// decisionForest_ is currently a vector of smart pointers that are all null (zero) since they are untrained.
				decisionForest_.resize(forestParams_.train_numTreesInForest_);

			   forestParams_.writeToConsole();
		}
	}

}

void Controller::LoadSettings()
{
    selectSettingsFileAndLoad();  // if parametersFile_ is non-empty, then we dont prompt the user and simply load the settings XML file

	if (FOREST_PARAMETERS_LOADED_) displayStatemachineStatus();

}







