#pragma once


#include <string>
#include "basicDefs.h"
#include "FilenameManager.h"
#include "xvdTags.h"
#include "BvdFileManager.h"
#include "tinyxml.h" 
#include <blitz/array.h>



struct XvdFileManager
{
	private:
		/// pxmlNode is base class ptr
		static bool readElementAndItsChildren(TiXmlNode* pxmlNode, xvdTags_ver1& xvdtags)
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
				// TiXmlAttribute* pAttrib;

				int xmlNodeType = pxmlNode->Type();

				switch(xmlNodeType)  // BREAK #1 for debugging
				{
					case TiXmlNode::TINYXML_DOCUMENT:
								if (bDebug) cout << "Document\n";
								break;

					case TiXmlNode::TINYXML_ELEMENT:
								strElementName=pxmlNode->Value();
								if (bDebug) cout << "Element [" << strElementName << "]\n";
								break;

					case TiXmlNode::TINYXML_TEXT:
								pText = pxmlNode->ToText();   // BREAK #2 for debugging
								strVal=pText->Value();  
								strLowerVal=pText->Value(); 
								to_lower(strLowerVal); 
								if (bDebug) cout <<  "Text: [" << strVal << "]\n"; 


                        if (strElementName == "versionNumber")
                        {
                            if ((lexical_cast<double>(strVal) != 1.0) && (lexical_cast<double>(strVal) != 1.1))
                            {
                                cout << "Error: expecting version 1 or 1.1 xvd file\n";
                                xvdtags.formatOK = false;
								bReturn=false;
                                return bReturn;
                            }
                            else xvdtags.formatOK = true;
                        }
                        else if (strElementName == "binaryFileName") xvdtags.binaryFileName = strVal;
                        else if (strElementName == "patientName") xvdtags.patientName = strVal;
                        else if (strElementName == "patientSex") xvdtags.patientSex = strVal;
                        else if (strElementName == "modality") xvdtags.modality = strVal;
                        else if (strElementName == "contrastAgent") xvdtags.contrastAgent = strVal;
                        else if (strElementName == "volumeWidth") xvdtags.volumeWidth = lexical_cast<int>(strVal);
                        else if (strElementName == "volumeHeight") xvdtags.volumeHeight = lexical_cast<int>(strVal);
                        else if (strElementName == "volumeDepth") xvdtags.volumeDepth = lexical_cast<int>(strVal);
                        else if (strElementName == "pixScaleX") xvdtags.pixScaleX = lexical_cast<double>(strVal);
                        else if (strElementName == "pixScaleY") xvdtags.pixScaleY = lexical_cast<double>(strVal);
                        else if (strElementName == "pixScaleZ") xvdtags.pixScaleZ = lexical_cast<double>(strVal);
                        else if (strElementName == "rescaleSlope") xvdtags.rescaleSlope = lexical_cast<double>(strVal);
                        else if (strElementName == "rescaleIntercept") xvdtags.rescaleIntercept = lexical_cast<double>(strVal);

                        else if (strElementName == "strChannelInterpretation") xvdtags.strChannelInterpretation = strVal;
                        else if (strElementName == "channelDisplayWidth") xvdtags.channelDisplayWidth = lexical_cast<int>(strVal);
                        else if (strElementName == "channelDisplayHeight") xvdtags.channelDisplayHeight = lexical_cast<int>(strVal);
                        else if (strElementName == "channelDisplayDepth") xvdtags.channelDisplayDepth = lexical_cast<int>(strVal);
						break;
					default:
						break;
				}

				// Depth first recursion on children of current node.
				for ( pxmlChildNode = pxmlNode->FirstChild(); pxmlChildNode != 0; pxmlChildNode = pxmlChildNode->NextSibling()) 
				{
					readElementAndItsChildren( pxmlChildNode, xvdtags );
			
				}
				return bReturn;
		}

	public:

		// Simply read in the xvd header file and return its info in xvdtags struct
		static bool readHeaderFile(xvdTags_ver1& xvdtags, string& xmlFileName)
		{
			// Read in the xvd tags ---
			TiXmlDocument xmldoc("xvdTags");
			bool bReturn=xmldoc.LoadFile(xmlFileName);
			if (bReturn) bReturn=readElementAndItsChildren(&xmldoc, xvdtags);

			return bReturn;
		}
        /// <summary>
        /// Loads both the xvd tags from the xvd file and the volume data from the bvd file
		/// Loads the volume for each modality channel (stored in Avolume) 
		/// Computes each volume modality channel's integral image (stored in AIvolume) 
        /// </summary>
		static bool readDataFromFile_ver1(vector<string>& lstModalBVDFilename, xvdTags_ver1& xvdtags, string& xmlFileName, string& xmlFilenameWithoutModal, vector<string>& modalNames, blitz::Array<ushort,4>& Avolume, blitz::Array<long long,4>& AIvolume )
        {
			bool bReturn=true;

			// Read in the xvd tags ---
			TiXmlDocument xmldoc("xvdTags");
			bReturn=xmldoc.LoadFile(xmlFileName);
			if (bReturn) bReturn=readElementAndItsChildren(&xmldoc, xvdtags);


			// switch xvdtags.strChannelInterpretation
			to_lower(xvdtags.strChannelInterpretation);
            
			//Allocate the size of the multimodal volumes
			Avolume.resize(xvdtags.volumeWidth,xvdtags.volumeHeight,xvdtags.volumeDepth,modalNames.size()); //store volume 
			AIvolume.resize(xvdtags.volumeWidth+1,xvdtags.volumeHeight+1,xvdtags.volumeDepth+1,modalNames.size());//store integral image
			
            // load the predictor data from bvd file:
			for (int i = 0; i < modalNames.size(); i++){
				//string bvdFileName = FilenameManager::ExtractDirectory(xmlFileName) + xvdtags.binaryFileName;
				lstModalBVDFilename[i] = FilenameManager::ChangeExtension(xmlFilenameWithoutModal, modalNames[i] + ".bvd");
				BvdFileHeader header;
				bReturn=BvdFileManager::loadVolume_originalCT(lstModalBVDFilename[i], xvdtags, header, Avolume, AIvolume, i);
				if (!bReturn) return bReturn;
			}
            
            return bReturn;
        }

		


		static bool saveDataToFile_ver1(xvdTags_ver1& xvdtags, string& xmlFileName, string& strTitle)
		{
			bool bReturn=true;
			double dVersion =1.1;   // The version below correspondes to xvd format version 1.1 

			try
			{
				ofstream outFile(xmlFileName.c_str());
    
				outFile << "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n";
				outFile << "<volumetricScan>\n";
				outFile << "  <title>" << strTitle << "</title>\n";
				outFile << "  <versionNumber>" << dVersion << "</versionNumber>\n";
				outFile << "  <binaryFileName>" << xvdtags.binaryFileName<< "</binaryFileName>\n";
				outFile << "  <patientName>" << xvdtags.patientName << "</patientName>\n";
				outFile << "  <patientSex>" << xvdtags.patientSex << "</patientSex>\n";
				outFile << "  <modality>" << xvdtags.modality << "</modality>\n";
				outFile << "  <contrastAgent>" << xvdtags.contrastAgent<< "</contrastAgent>\n";
				outFile << "  <volumeWidth>" << xvdtags.volumeWidth << "</volumeWidth>\n";
				outFile << "  <volumeHeight>" << xvdtags.volumeHeight<< "</volumeHeight>\n";
				outFile << "  <volumeDepth>" << xvdtags.volumeDepth << "</volumeDepth>\n";
				outFile << "  <strChannelInterpretation>" << xvdtags.strChannelInterpretation<< "</strChannelInterpretation>\n";
				outFile << "  <channelDisplayWidth>" << xvdtags.channelDisplayWidth << "</channelDisplayWidth>\n";
				outFile << "  <channelDisplayHeight>" << xvdtags.channelDisplayHeight << "</channelDisplayHeight>\n";
				outFile << "  <channelDisplayDepth>" << xvdtags.channelDisplayDepth << "</channelDisplayDepth>\n";
				outFile << "  <pixScaleX>" << xvdtags.pixScaleX << "</pixScaleX>\n";
				outFile << "  <pixScaleY>" << xvdtags.pixScaleY << "</pixScaleY>\n";
				outFile << "  <pixScaleZ>" << xvdtags.pixScaleZ<< "</pixScaleZ>\n";
				outFile << "  <rescaleSlope>" << xvdtags.rescaleSlope<< "</rescaleSlope>\n";
				outFile << "  <rescaleIntercept>" << xvdtags.rescaleIntercept << "</rescaleIntercept>\n";
				outFile << "</volumetricScan>\n";
				outFile.close();
			}
            catch (...)
			{   cout << "Error: could not write xvd data header file: " << xmlFileName << "\n";
				bReturn=false;  
			}; 

			return bReturn;

		}
};


