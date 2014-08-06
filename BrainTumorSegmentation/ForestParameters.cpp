#include "ForestParameters.h"
#include <iostream>
#include <sstream>
#include <string> 
#include "tinyxml.h" 
#include "tinyXMLDumpToStdOut.h"
#include "FeatureFarm.h"
#include "AnatomicalStructureManager.h"
#include "basicDefs.h"
#include "EntropyImplementationType.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "itksys/Base64.h"
#include "DefaultForest.h"

using namespace std;
using boost::lexical_cast;
using boost::algorithm::to_lower;


void ForestParameters::writeToConsole(void)
    {

        cout << "\n";
        cout << "|\n";
        cout << "| ------------------ Forest Parameter settings --- --- --- --- --- --- --- --- \n";
        cout << "|\n";
        cout << "|  Forest settings file:" << strForestParametersFilename_ << "\n";
        cout << "|\n";
        cout << "|   train_numTreesInForest         " << train_numTreesInForest_ << "\n";
        cout << "|   train_treeDepth                " << train_treeDepth_ << "\n";
        cout << "|   train_numFeaturesPerNode       " << train_numFeaturesPerNode_ << "\n";

        cout << "|   - - - - - - Feature space exploration: - - - - - - \n";
        cout << "|   train_numThresholdsPerFeature  " << train_numThresholdsPerFeature_ << "\n";
        cout << "|   train_maxFeatureRadius         " << train_maxFeatureRadius_ << "\n";
		cout << "|   maxBoxsize                     " << maxBox[0] << ", " << maxBox[1] << ", " << maxBox[2] << "\n";
		cout << "|   minBoxsize                     " << minBox[0] << ", " << minBox[1] << ", " << minBox[2] << "\n";
        cout << "|\n";
        cout << "|   = = = = = = Feature Parameter Proposal distribution: = = = = = = = = \n";
        cout << "|   bUseAcceptedFeatureParameterDensities  " << bUseAcceptedFeatureParameterDensities_ << "\n";
        cout << "|   dFractionDensityPedestal  " << dFractionDensityPedestal_ << "\n";
        cout << "|   bSpikeInProposalDistn     " << bSpikeInProposalDistn_ << "\n";
            
        cout << "|\n";
        cout << "|   AcceptedFeatureParameterDensities loaded for following features:\n";




        cout << "|\n";
        cout << "|   = = = = = = FeatureTypeProposal distribution: = = = = = = = = \n";
        cout << "|   bUseAcceptedFeatureTypeDensities  " << bUseAcceptedFeatureTypeDensities_ << "\n";
        cout << "|\n";
        cout << "|   - - - - - - Feature types: - - - - - - \n";
        cout << "|          bChannelIntensityRef " << bChannelIntensityRef_  << "\n";
        cout << "|          bAbsoluteIntensity " << bAbsoluteIntensity_ << "\n";
        cout << "|          bRelativeIntensity " << bRelativeIntensity_ << "\n";
        cout << "|          bRelativePosterior " << bRelativePosterior_<< "\n";
        cout << "|          bRatioIntensity " << bRatioIntensity_<< "\n";
        cout << "|          bDiffOf2Probes " << bDiffOf2Probes_<< "\n";
        cout << "|          bAbsoluteSpatialAppearanceEntanglement1a " << bAbsoluteSpatialAppearanceEntanglement1a_<< "\n";
        cout << "|          bSimilarityOfAppearancesEntanglement1b " << bSimilarityOfAppearancesEntanglement1b_<< "\n";
        cout << "|          bMAPLabelEntanglement2a " << bMAPLabelEntanglement2a_<< "\n";
        cout << "|          bPosteriorBinEntanglement2b " << bPosteriorBinEntanglement2b_<< "\n";
        cout << "|          bTop2Classes " << bTop2Classes_<< "\n";
        cout << "|          bTop3Classes " << bTop3Classes_<< "\n";
        cout << "|          bTop4Classes " << bTop4Classes_<< "\n";
        cout << "|          bAutocontextMAPLabel " << bAutocontextMAPLabel_<< "\n";
		cout << "|          bHistoDiff " << bHistoDiff_ << "\n";
		cout << "|          bSymDiff " << bAbsSym_ << "\n";
        cout << "|\n";
        cout << "|   train_stopCrit_minInfoGain     " << train_stopCrit_minInfoGain_ << "\n";
        cout << "|   train_stopCrit_minNumPoints    " << train_stopCrit_minNumPoints_ << "\n";;
        cout << "|\n";
        cout << "|   test_numTreesInForest          " << test_numTreesInForest_ << "\n";;
        cout << "|   test_treeDepth                 " << test_treeDepth_ << "\n";;
        cout << "|\n";
        cout << "|   volumeDownSampleFactor         " << volumeDownSampleFactor_ << "\n";
        cout << "|\n";
        cout << "|   n. training datasets           " << train_DatasetIndeces_.size() << "\n";
        cout << "|   n. testing datasets            " << test_DatasetIndeces_.size() << "\n";
        cout << "|\n";
        cout << "|\n";
        cout << "|   - - - - - -Merging parameters: - - - - - -\n";
        cout << "|          bPerformMerging " << bPerformMerging_  << "\n";
        cout << "|          strMergingType " << strMergingType_ << "\n";
        cout << "|          strDistanceMeasure " << strDistanceMeasureType_ << "\n";
        cout << "|          dMaxMergeDistance " << dMaxMergeDistance_ << "\n";
        cout << "|          strMergeLocationType " << strMergeLocationType_ << "\n";
        cout << "|          strCandidateMergeNodeType " << strCandidateMergeNodeType_  << "\n";
        cout << "|\n";
   	    cout << "|   Using " << classIndeces_.size() << " classIndeces:\n";


        AnatomicalStructureManager ASM;
		cout <<     "|    Class to learn       Index in list of all possible classes  ->   class name\n";
        for (unsigned int i = 0; i < classIndeces_.size(); i++)
            cout << "|          ["<< i << "]                             " << (unsigned int)classIndeces_[i] << "                     ->  " << ASM.anatStructIdentifier_To_StructName(classIndeces_[i]) <<"\n";
        cout << "|\n";
        cout << "|  nEntropyImplementationType " << arrstrEnumComputeEntropyImplementationType[(int)nEntropyImplementationType_] << "\n";
        cout << "|\n";


        cout << "|   = = = = = = Randomness: = = = = = = = = \n";
        cout << "|   bDeterministic  " << bDeterministic_<< "\n";
        cout << "|   nSeed  " << nSeed_<< "\n";
        cout << "|\n";
        cout << "|          TrainingAlgorithm " + strTrainingAlgorithm_ << "\n";

        cout << "|\n";
        cout << "|   = = = = = = Autocontext: = = = = = = = = \n";
        cout << "|   bUseAutocontext  " << bUseAutocontext_<< "\n";
        cout << "|   strAutocontextForestName " << strAutocontextForestName_<< "\n";

        if (arrstrAuxTreePathnames_.empty()) cout << "|    No aux trees\n";
        else for (unsigned int i = 0; i < arrstrAuxTreePathnames_.size(); i++)
                cout << "|                    aux tree(" << i << ")  " << arrstrAuxTreePathnames_[i] << "\n";

		cout << "\n";
		cout << "|   = = = = = = Modalities = = = = = = = =\n";
		if (modalIndeces_.empty()){
			cout << "|    There is only one Modality\n"; 
		}
		else{
			for (unsigned int i = 0; i < modalIndeces_.size(); i++)
				cout << "|    Modality:" << i << " is, " << modalIndeces_[i] << "\n";
		}
		cout << "|   = = = = = = Texture Channels = = = = = = = =\n";
		if (intHistind_.empty()) cout << "None..." << "\n";
		else for (unsigned int i = 0; i < intHistind_.size(); i++)
				cout << "|   Channel:" << i << " is, " << intHistind_[i] << "\n";

        cout << "|\n";
		cout << "|   = = = = = = Cross-validation: = = = = = = = = \n";
        cout << "|   nNumCrossValidationFolds  " << nNumCrossValidationFolds_<< "\n";        
		cout << "|   bOutputIndividualFoldPerformance  " << bOutputIndividualFoldPerformance_<< "\n";        
		

    }


///
/// pxmlNode is base class ptr
bool ForestSettingsFileManager::readElementAndItsChildren(TiXmlNode* pxmlNode, ForestParameters& forestParams)
{
	    bool bDebug=false; // set true to dump XML contents to cout

	    // static state variables =========================
	    static bool bReturn=true;
		static int modalPos = 0;
		static int chanPos = 0;
		static int maxBcount = 0;
		static int minBcount = 0;
	    static int classPos = 0;
		static int trainIndex = 0;
		static int testIndex = 0;
		static int auxTreeIndex = 0;
		static string strElementName="";
		
        AnatomicalStructureManager ASM;
        static bool TRAIN_TEST = true; // true for trainingparameters
        static bool TRAIN_TEST_DATASETS = true; // true for trainingparameters
		// end  static state variables =========================
		
		if ( !pxmlNode ) return bReturn;

		TiXmlNode* pxmlChildNode=nullptr;
		TiXmlText* pText;
		unsigned int indent=0;

		string strVal;
		string strLowerVal;

		int xmlNodeType = pxmlNode->Type();
		

		switch(xmlNodeType)  // BREAK #1 for debugging
		{
			case TiXmlNode::TINYXML_DOCUMENT:
						if (bDebug) cout << "Document\n";
						break;

			case TiXmlNode::TINYXML_ELEMENT:
						strElementName=pxmlNode->Value();
						if (bDebug) cout << "Element [" << strElementName << "]\n";


                        if (strElementName == "DecisionForestSettings")     // reading attributes for the current "decisionTree" element and class instantiations
                        {
							TiXmlAttribute* pAttrib=(pxmlNode->ToElement())->FirstAttribute();
							double ver=-1;
							if (pAttrib) pAttrib=pAttrib->Next();
							if (pAttrib) ver = lexical_cast<double>(pAttrib->Value());      // version of xml format
                            if (ver != 1) { cout << "Error: expecting version 1.0 xml file for parameters settings\n"; bReturn=false; return bReturn; }
                        }
                        if (strElementName == "trainingParameters")
                        {
                            TRAIN_TEST = true;
                        }
                        if (strElementName == "testingParameters")
                        {
                            TRAIN_TEST = false;
                        }
                        if (strElementName == "trainingDatasets")
                        {   TiXmlAttribute* pAttrib=(pxmlNode->ToElement())->FirstAttribute();
                            int numTrainingDatasets = lexical_cast<int>(pAttrib->Value());      
							forestParams.train_DatasetIndeces_.resize(numTrainingDatasets,-1);
                            trainIndex = 0;
                            TRAIN_TEST_DATASETS = true;
                        }
                        if (strElementName == "testingDatasets")
                        {   TiXmlAttribute* pAttrib=(pxmlNode->ToElement())->FirstAttribute();
                            int numTestingDatasets = lexical_cast<int>(pAttrib->Value());  
                            forestParams.test_DatasetIndeces_.resize(numTestingDatasets,-1);
                            testIndex = 0;
                            TRAIN_TEST_DATASETS = false;
                        }
                        if (strElementName == "classList")     
                        {   TiXmlAttribute* pAttrib=(pxmlNode->ToElement())->FirstAttribute();
                            int numClasses = lexical_cast<int>(pAttrib->Value());  
                            forestParams.classIndeces_.resize(numClasses,-1);
                            classPos = 0;
                        }
						if (strElementName == "modalities")
						{	TiXmlAttribute* pAttrib=(pxmlNode->ToElement())->FirstAttribute();
                            int numModalities = lexical_cast<int>(pAttrib->Value());
							forestParams.modalIndeces_.resize(numModalities,"");
                            modalPos = 0;
						}
						if (strElementName == "histochans")
						{	TiXmlAttribute* pAttrib=(pxmlNode->ToElement())->FirstAttribute();
                            int numHists = lexical_cast<int>(pAttrib->Value());
							forestParams.intHistind_.resize(numHists,"");
                            chanPos = 0;
						}
                        if (strElementName == "AuxForest")     
                        {   TiXmlAttribute* pAttrib=(pxmlNode->ToElement())->FirstAttribute();
                            int numAuxTrees = lexical_cast<int>(pAttrib->Value());  
                            forestParams.arrstrAuxTreePathnames_.resize(numAuxTrees,"");
                            auxTreeIndex = 0;
                        }


						break;

			case TiXmlNode::TINYXML_COMMENT:
						if (bDebug) cout << "Comment: [" << pxmlNode->Value() << "]\n"; 
						break;

			case TiXmlNode::TINYXML_UNKNOWN:
						if (bDebug) cout << "Unknown\n";
						break;

			case TiXmlNode::TINYXML_TEXT:
						pText = pxmlNode->ToText();   // BREAK #2 for debugging
						strVal=pText->Value();  
						strLowerVal=pText->Value(); 
						to_lower(strLowerVal); 
						if (bDebug) cout <<  "Text: [" << strVal << "]\n"; 

                        if (strElementName == "datasetIndex" && TRAIN_TEST_DATASETS) 
						  { 
							   forestParams.train_DatasetIndeces_[trainIndex] = atoi(strVal.c_str());trainIndex++; 
						  }
                        else if (strElementName == "datasetIndex" && !TRAIN_TEST_DATASETS) { forestParams.test_DatasetIndeces_[testIndex] = atoi(strVal.c_str()); testIndex++; }
                        else if (strElementName == "numTreesInForest")
                        {
                            int nNumTrees = lexical_cast<int>(strVal);
                            if (nNumTrees < 1)
                            { cout << "Error: number of trees must be positive"; bReturn=false; return bReturn; };
                            if (TRAIN_TEST) forestParams.train_numTreesInForest_ = nNumTrees;
                            else forestParams.test_numTreesInForest_ = nNumTrees;
                        }
                        else if (strElementName == "treeDepth")
                        {
                            int nTreeDepth =  lexical_cast<int>(strVal);
                            if (nTreeDepth < 1)
                            { cout << "Error: tree depth must be positive\n"; bReturn=false; return bReturn; };
                            if (TRAIN_TEST) forestParams.train_treeDepth_ = nTreeDepth;
                            else forestParams.test_treeDepth_ = nTreeDepth;
                        }
                        else if (strElementName == "numFeaturesPerNode") forestParams.train_numFeaturesPerNode_ =  lexical_cast<int>(strVal);
                        else if (strElementName == "numThresholdsPerFeature") forestParams.train_numThresholdsPerFeature_ =  lexical_cast<int>(strVal);
                        else if (strElementName == "stopCrit_minInfoGain") forestParams.train_stopCrit_minInfoGain_ =  lexical_cast<double>(strVal);
                        else if (strElementName == "stopCrit_minNumPoints") forestParams.train_stopCrit_minNumPoints_ =  lexical_cast<double>(strVal);
                        else if (strElementName == "maxFeatureRadius") forestParams.train_maxFeatureRadius_ = lexical_cast<float>(strVal);
						else if (strElementName == "maxBoxsize") {forestParams.maxBox[maxBcount] = lexical_cast<unsigned int>(strVal); maxBcount++;}
						else if (strElementName == "minBoxsize") {forestParams.minBox[minBcount] = lexical_cast<unsigned int>(strVal); minBcount++;}
                        else if (strElementName == "volumeDownSampleFactor") forestParams.volumeDownSampleFactor_ = lexical_cast<float>(strVal);
                        else if (strElementName == "className") 
						  { 
							   forestParams.classIndeces_[classPos] = (byte)ASM.anatStructName_To_StructIdentifier(strVal); classPos++; 
						  }
                        else if (strElementName == "treePathname") { forestParams.arrstrAuxTreePathnames_[auxTreeIndex] = strVal; auxTreeIndex++; }
						else if (strElementName == "modalityName") {	forestParams.modalIndeces_[modalPos] = strVal; modalPos++; }
						else if (strElementName == "chanName") {forestParams.intHistind_[chanPos] = strVal; chanPos++;}
						else if (strElementName == "MaxHistlvls") {forestParams.Histlvls_ = lexical_cast<unsigned int>(strVal);}
                        else if (strElementName == "volumeDownSampleFactor") forestParams.volumeDownSampleFactor_ = lexical_cast<float>(strVal);
                        // AAM: Merging parameters
                        else if (strElementName == "bPerformMerging") forestParams.bPerformMerging_ =  strLowerVal=="false"  ? false : true; 
                        else if (strElementName == "strMergingType") forestParams.strMergingType_ = strVal;
                        else if (strElementName == "strDistanceMeasureType") forestParams.strDistanceMeasureType_ = strVal;
                        else if (strElementName == "dMaxMergeDistance") forestParams.dMaxMergeDistance_ =  lexical_cast<double>(strVal);
                        else if (strElementName == "strMergeLocationType") forestParams.strMergeLocationType_ = strVal;
                        else if (strElementName == "strCandidateMergeNodeType") forestParams.strCandidateMergeNodeType_ = strVal;
                        // AAM: Feature types:
                        else if (strElementName == "bChannelIntensityRef") forestParams.bChannelIntensityRef_ =  strLowerVal=="false"  ? false : true; 
						else if (strElementName == "bAbsoluteIntensity") forestParams.bAbsoluteIntensity_ =  strLowerVal=="false"  ? false : true; 
                        else if (strElementName == "bRelativeIntensity") forestParams.bRelativeIntensity_ =  strLowerVal=="false"  ? false : true; 
                        else if (strElementName == "bRelativePosterior") forestParams.bRelativePosterior_ =  strLowerVal=="false"  ? false : true; 
                        else if (strElementName == "bRatioIntensity") forestParams.bRatioIntensity_ =  strLowerVal=="false"  ? false : true; 
                        else if (strElementName == "bDiffOf2Probes") forestParams.bDiffOf2Probes_ =  strLowerVal=="false"  ? false : true; 
                        else if (strElementName == "bAbsoluteSpatialAppearanceEntanglement1a") forestParams.bAbsoluteSpatialAppearanceEntanglement1a_ =  strLowerVal=="false"  ? false : true; 
                        else if (strElementName == "bSimilarityOfAppearancesEntanglement1b") forestParams.bSimilarityOfAppearancesEntanglement1b_ =  strLowerVal=="false"  ? false : true; 
                        else if (strElementName == "bMAPLabelEntanglement2a") forestParams.bMAPLabelEntanglement2a_ =  strLowerVal=="false"  ? false : true; 
                        else if (strElementName == "bPosteriorBinEntanglement2b") forestParams.bPosteriorBinEntanglement2b_ =  strLowerVal=="false"  ? false : true; 
                        else if (strElementName == "bTop2Classes") forestParams.bTop2Classes_ =  strLowerVal=="false"  ? false : true; 
                        else if (strElementName == "bTop3Classes") forestParams.bTop3Classes_ =  strLowerVal=="false"  ? false : true; 
                        else if (strElementName == "bTop4Classes") forestParams.bTop4Classes_ =  strLowerVal=="false"  ? false : true; 
                        else if (strElementName == "bAutocontextMAPLabel") forestParams.bAutocontextMAPLabel_ =  strLowerVal=="false"  ? false : true; 
						else if (strElementName == "bHistoDiff") forestParams.bHistoDiff_ =  strLowerVal=="false"  ? false : true;
						else if (strElementName == "bHistoMean") forestParams.bHistoMean_ =  strLowerVal=="false"  ? false : true; 
						else if (strElementName == "bHistoVar") forestParams.bHistoVar_ =  strLowerVal=="false"  ? false : true; 
						else if (strElementName == "bHistoSkew") forestParams.bHistoSkew_ =  strLowerVal=="false"  ? false : true; 
						else if (strElementName == "bHistoKurt") forestParams.bHistoKurt_ =  strLowerVal=="false"  ? false : true; 
						else if (strElementName == "bHistoEnt") forestParams.bHistoEnt_ =  strLowerVal=="false"  ? false : true; 
                        else if (strElementName == "bAbsSymDiff") forestParams.bAbsSym_ =  strLowerVal=="false"  ? false : true; 
                            
                        // AAM: Proposal distribution parameters: 
                        else if (strElementName == "bUseAcceptedFeatureParameterDensities") forestParams.bUseAcceptedFeatureParameterDensities_ =  strLowerVal=="false"  ? false : true; 
                        else if (strElementName == "dFractionDensityPedestal") forestParams.dFractionDensityPedestal_ =  lexical_cast<double>(strVal);
                        else if (strElementName == "bSpikeInProposalDistn") forestParams.bSpikeInProposalDistn_ =  strLowerVal=="false"  ? false : true; 

                        else if (strElementName == "nEntropyImplementationType")
                        {
                            int nEntropyImplementationType =  lexical_cast<int>(strVal);
                            if ((nEntropyImplementationType < 0) || (nEntropyImplementationType > 3))
                            { cout << "Error: nEntropyImplementationType must be btwn 0..3\n"; bReturn=false; return bReturn; };
                            forestParams.nEntropyImplementationType_ = (enumComputeEntropyImplementationType)nEntropyImplementationType;
                        }
						else if (strElementName == "bDeterministic") forestParams.bDeterministic_ =   strLowerVal=="false"  ? false : true; 
                        else if (strElementName == "nSeed") forestParams.nSeed_ =  lexical_cast<int>(strVal);
                        else if (strElementName == "TrainingAlgorithm") forestParams.strTrainingAlgorithm_ = strVal; 
                        // FeatTypePropDistn
                        else if (strElementName == "bUseAcceptedFeatureTypeDensities") forestParams.bUseAcceptedFeatureTypeDensities_ =  strLowerVal=="false"  ? false : true; 
                        else if (strElementName == "bUseAutocontext") forestParams.bUseAutocontext_ =  strLowerVal=="false"  ? false : true; 
                        else if (strElementName == "strAutocontextForestName") forestParams.strAutocontextForestName_= strVal;
						else if (strElementName == "nNumCrossValidationFolds") forestParams.nNumCrossValidationFolds_ =  lexical_cast<int>(strVal);
						else if (strElementName == "bOutputIndividualFoldPerformance") forestParams.bOutputIndividualFoldPerformance_ =   strLowerVal=="false"  ? false : true; 
						
						
				break;


			case TiXmlNode::TINYXML_DECLARATION:
						if (bDebug) cout <<  "Declaration\n";
						break;
			default:
						break;
		}

		

		// Depth first recursion on children of current node.
		for ( pxmlChildNode = pxmlNode->FirstChild(); pxmlChildNode != 0; pxmlChildNode = pxmlChildNode->NextSibling()) 
		{
			readElementAndItsChildren( pxmlChildNode, forestParams );
			
		}
		return bReturn;
}


bool ForestSettingsFileManager::readFromFile(const string& xmlSettingsFileName_IN, ForestParameters& forestParams)
{
	bool bReturn;
           
    forestParams.strForestParametersFilename_ = xmlSettingsFileName_IN;

    // // AAM: Feature types: ... set up defaults
    forestParams.bChannelIntensityRef_ = false;  // true to use this feature , false otherwise
    forestParams.bAbsoluteIntensity_=true;  // true to use this feature , false otherwise
    forestParams.bRelativeIntensity_=true;  //true to use this feature , false otherwise
    forestParams.bRelativePosterior_ = false; //true to use this feature , false otherwise
    forestParams.bRatioIntensity_=false;  // true to use this feature , false otherwise
    forestParams.bDiffOf2Probes_=true;  // true to use this feature , false otherwise
    forestParams.bAbsoluteSpatialAppearanceEntanglement1a_=false;  // true to use this feature , false otherwise
    forestParams.bSimilarityOfAppearancesEntanglement1b_ = false;  // true to use this feature , false otherwise
    forestParams.bMAPLabelEntanglement2a_ = false;  // true to use this feature , false otherwise
    forestParams.bPosteriorBinEntanglement2b_ = false;  // true to use this feature , false otherwise
    forestParams.bTop2Classes_ = false;  // true to use this feature , false otherwise
    forestParams.bTop3Classes_ = false;  // true to use this feature , false otherwise
    forestParams.bTop4Classes_ = false;  // true to use this feature , false otherwise
    forestParams.bAutocontextMAPLabel_ = false; // true to use this feature , false otherwise
	forestParams.bHistoDiff_ = false; // true to use this feature , false otherwise
	forestParams.bHistoMean_ = false; // true to use this feature , false otherwise
	forestParams.bHistoVar_ = false; // true to use this feature , false otherwise
	forestParams.bHistoSkew_ = false; // true to use this feature , false otherwise
	forestParams.bHistoKurt_ = false; // true to use this feature , false otherwise
	forestParams.bHistoEnt_ = false; // true to use this feature , false otherwise
	forestParams.bAbsSym_ = false; // true to use this feature , false otherwise

	forestParams.Histlvls_ = 0; //assume texture is not given
    // // AAM: Merging parameters: ... set up defaults
    forestParams.bPerformMerging_=true;  // true to perform merging, false otherwise
    forestParams.strMergingType_="WithinTree";
    forestParams.strDistanceMeasureType_="Euclidean";
    forestParams.dMaxMergeDistance_ = 100.0;
    forestParams.strMergeLocationType_="Level10";
    forestParams.strCandidateMergeNodeType_="ToBeSplitAndLeaves";
    // AAM: proposal distribution  ... set up defaults
    forestParams.bUseAcceptedFeatureParameterDensities_ = true;
    forestParams.dFractionDensityPedestal_ = 0.25;  //  by default proposal distn is 25% uniform distn, 75% from acceptance distn
    forestParams.bSpikeInProposalDistn_ = true; 
    // Entropy fix
	forestParams.nEntropyImplementationType_ = ENTROPY_TYPE_MultiplyClassPriorsAfterEntropyNoDirichlet; // default is Albert's fix 
    // Randomness
    forestParams.bDeterministic_ = false; // seed from non-determinsitic time by default
    forestParams.nSeed_ = 0;
    // FeatTypePropDistn
    forestParams.bUseAcceptedFeatureTypeDensities_ = false;
    // // Autocontext
    forestParams.bUseAutocontext_=false;
    forestParams.strAutocontextForestName_ = "";  // short name for forest, used as suffix for MAPLabelImages

    forestParams.strTrainingAlgorithm_ = "PixelScan";
	forestParams.nNumCrossValidationFolds_=10; 
	forestParams.bOutputIndividualFoldPerformance_=false;
	for (int i = 0; i<3; i++){
	forestParams.minBox [i] = 1;
	forestParams.maxBox [i] = 1;}//assume a point

	TiXmlDocument xmldocForestSettings("ForestSettings");
	

	if (!xmlSettingsFileName_IN.empty()) {
	   bReturn=xmldocForestSettings.LoadFile(xmlSettingsFileName_IN); 
	} else {
		unsigned char *decodedBuffer = new unsigned char[default_ForestSettings.size()](); // pre-allocate and use () to value initialize all elements to zero
        

		// convert value from Base64
		unsigned int decodedLengthActual = static_cast<unsigned int>(
			itksysBase64_Decode(
				(const unsigned char *) default_ForestSettings.c_str(),
				static_cast<unsigned long>( 0 ),
				(unsigned char *) decodedBuffer,
				static_cast<unsigned long>( default_ForestSettings.size())
				));
		// string debugInspect=(const char*)decodedBuffer;
		xmldocForestSettings.Parse((const char*)decodedBuffer, 0, TIXML_ENCODING_UTF8);
		bReturn=true;  // Parse() does not return success/failure.. so we will ge that from readElementAndItsChildren below
		delete []decodedBuffer;
	}
		
		
		

	if (bReturn) bReturn=readElementAndItsChildren(&xmldocForestSettings, forestParams);


	if (bReturn)
	{
			// AAM: for speed during training, we precompute direct mapping from structure index to posterior bin number that is constructed by this classifier
			AnatomicalStructureManager ASM;
			int nNumKnownLabels = ASM.numStructures_;
			forestParams.arrKnownGTLabelToPosteriorBinLUT_.resize(nNumKnownLabels, -1);
			for (unsigned int kk = 0; kk < forestParams.classIndeces_.size(); kk++)
			    forestParams.arrKnownGTLabelToPosteriorBinLUT_[forestParams.classIndeces_[kk]] = kk;
			
			// AAM: Sanity checks on random forest parameters
			if ((forestParams.train_numTreesInForest_>0) && (forestParams.test_numTreesInForest_ > forestParams.train_numTreesInForest_) )
			{ cout << "Error: Forest parameters cannot specify more test trees than it specifies for training.\n"; 
			    bReturn=false; return bReturn;  
			}
			if ((forestParams.train_treeDepth_>0) && (forestParams.test_treeDepth_ > forestParams.train_treeDepth_))
			{
			    cout << "Error: Forest parameters Settings cannot specify greater test tree depth than it specified for training.\n";
			    bReturn=false; return bReturn;  
			}

			if (forestParams.bUseAcceptedFeatureParameterDensities_)
			{
			}
	}
           
    return bReturn;
};

