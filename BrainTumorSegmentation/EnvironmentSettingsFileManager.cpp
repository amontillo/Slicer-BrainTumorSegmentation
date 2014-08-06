#include "EnvironmentSettingsFileManager.h"
#include "itksys/Base64.h"
#include "DefaultForest.h"

using namespace std;
using boost::lexical_cast;

bool EnvironmentSettingsFileManager::readElementAndItsChildren(TiXmlNode* pxmlNode, EnvironmentParameters& structEnvironmentParams)
{
		bool bDebug=false; // set true to dump XML contents to cout

		// static state variables =========================
		static bool bReturn=true;
		static string strElementName="";
		
		if ( !pxmlNode ) return bReturn;

		TiXmlNode* pxmlChildNode=nullptr;
		TiXmlText* pText;
		unsigned int indent=0;

		string strVal;
		string strLowerVal;
		double ver=-1;
		TiXmlAttribute* pAttrib;

		int xmlNodeType = pxmlNode->Type();

		switch(xmlNodeType)  // BREAK #1 for debugging
		{
			case TiXmlNode::TINYXML_DOCUMENT:
						if (bDebug) cout << "Document\n";
						break;

			case TiXmlNode::TINYXML_ELEMENT:
						strElementName=pxmlNode->Value();
						if (bDebug) cout << "Element [" << strElementName << "]\n";

						if (strElementName == "EnvironmentSettings")
						{
							pAttrib=(pxmlNode->ToElement())->FirstAttribute();
							if (pAttrib) pAttrib=pAttrib->Next();
							if (pAttrib) ver = lexical_cast<double>(pAttrib->Value());      // version of xml format
							if (ver != 1) { cout << "Error: expecting version 1.0 xml file for environment settings\n"; bReturn=false; return bReturn; }
						}
						break;

			case TiXmlNode::TINYXML_TEXT:
						pText = pxmlNode->ToText();   // BREAK #2 for debugging
						strVal=pText->Value();  
						strLowerVal=pText->Value(); 
						to_lower(strLowerVal); 
						if (bDebug) cout <<  "Text: [" << strVal << "]\n"; 

						if (strElementName == "databaseFolder") 
							{ 
								structEnvironmentParams.databaseFolder_ = strVal;
							}
						else if (strElementName == "numThreadsTraining")
						{
							int nNumThreads =  lexical_cast<int>(strVal);
							if (nNumThreads < 1)
							{ cout << "Error: number of training threads must be positive\n"; bReturn=false; return bReturn; };
							structEnvironmentParams.train_numThreads_ = nNumThreads;
						}
						else if (strElementName == "numThreadsTesting")
						{
							int nNumThreads =  lexical_cast<int>(strVal);
							if (nNumThreads < 1)
							{ cout << "Error: number of testing threads must be positive\n"; bReturn=false; return bReturn; };
							structEnvironmentParams.test_numThreads_ = nNumThreads;
						};
				break;
			default:
				break;
		}

		// Depth first recursion on children of current node.
		for ( pxmlChildNode = pxmlNode->FirstChild(); pxmlChildNode != 0; pxmlChildNode = pxmlChildNode->NextSibling()) 
		{
			readElementAndItsChildren( pxmlChildNode, structEnvironmentParams );
			
		}
		return bReturn;
}

/// <summary>
/// Simple xml parser to read environment settings from file
/// </summary>
bool EnvironmentSettingsFileManager::readFromFile(const string& xmlSettingsFileName_IN, EnvironmentParameters& structEnvironmentParams)
{
	bool bReturn=true;

//	if (foo=="Bar") bReturn=false;

    structEnvironmentParams.stringEnvironmentSettingsFileName_ = xmlSettingsFileName_IN;
        
    structEnvironmentParams.train_numThreads_ = 3; // assume quad core, w/1 core left free for other uses
    structEnvironmentParams.test_numThreads_ = 3;   

	TiXmlDocument xmldoc("EnvironmentSettings");

	if (!xmlSettingsFileName_IN.empty()) {
		bReturn=xmldoc.LoadFile(xmlSettingsFileName_IN);
	} else {
		unsigned char *decodedBuffer = new unsigned char[default_EnvironmentSettings.size()](); // pre-allocate and use () to value initialize all elements to zero
		// convert value from Base64
		unsigned int decodedLengthActual = static_cast<unsigned int>(
			itksysBase64_Decode(
				(const unsigned char *) default_EnvironmentSettings.c_str(),
				static_cast<unsigned long>( 0 ),
				(unsigned char *) decodedBuffer,
				static_cast<unsigned long>( default_EnvironmentSettings.size())
				));
		// string debugInspect=(const char*)decodedBuffer;
		xmldoc.Parse((const char*)decodedBuffer, 0, TIXML_ENCODING_UTF8);
		bReturn=true;  // Parse() does not return success/failure.. so we will ge that from readElementAndItsChildren below
		delete []decodedBuffer;
		
	}

	if (bReturn) bReturn=readElementAndItsChildren(&xmldoc, structEnvironmentParams);
        
    return bReturn;
}
