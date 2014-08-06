#pragma once
#include <iostream>
#include <sstream>
#include <string> 
#include <vector>
#include "tinyxml.h" 
#include "basicDefs.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

using namespace std;
using boost::lexical_cast;
using boost::algorithm::to_lower;

/// <summary>
/// Training parameters. Ideally all the interesting variables should be collected in this struct
/// </summary>
struct EnvironmentParameters
{
    string databaseFolder_;              // location of annotated database in file system
    int train_numThreads_;              // num threads for training
    int test_numThreads_;               // num threads for testing
    string stringEnvironmentSettingsFileName_;

    /// <summary>
    /// Visualizating parameters for debug purposes
    /// </summary>
    void writeToConsole()
    {
        cout << "| \n";
        cout << "| ----------     Environment settings  ---------------------------- \n";
        cout << "|\n";
        cout << "|  Environment settings file:" << stringEnvironmentSettingsFileName_ << "\n";
        cout << "|\n";
        cout << "|   databaseFolder   " << databaseFolder_ << "\n";
        cout << "|\n";
        cout << "|   train_numThreads        " << train_numThreads_ << "\n";
        cout << "|   test_numThreads         " << test_numThreads_ << "\n";
        cout << "|\n";

    }
};


struct EnvironmentSettingsFileManager
{
	/// pxmlNode is base class ptr
	static bool readElementAndItsChildren(TiXmlNode* pxmlNode, EnvironmentParameters& structEnvironmentParams);

    /// <summary>
    /// Simple xml parser to read environment settings from file
    /// </summary>
    static bool readFromFile(const string& xmlSettingsFileName_IN, EnvironmentParameters& structEnvironmentParams);


};

