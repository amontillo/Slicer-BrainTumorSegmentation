#pragma once

#include <vector>
#include <string>
#include "tinyxml.h"  // AAM 4-26-2011, moved line earlier to avoid compiler errors
#include "DecisionTree.h"
#include "ForestParameters.h"
#include "EnvironmentSettingsFileManager.h"
#include "FilenameManager.h"
#include "basicDefs.h"
#include "Dataset.h"
//#include "perf_calc.h"
#include "FeatureVolumeType.h"
#include "itksys/Base64.h"
#include "DefaultForest.h"
#include <numeric>      // std::accumulate

// ITK
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkResampleImageFilter.h"
#include "itkRecursiveGaussianImageFilter.h"
#include "itkIdentityTransform.h"
#include "itkIntensityWindowingImageFilter.h"
#include "itkGDCMImageIO.h"
#include "itkGDCMSeriesFileNames.h"
#include "itkImageSeriesReader.h"
//#include "itkOrientImageFilter.h"
#include "itkSpatialOrientationAdapter.h"
#include "itkOrientImageFilter.h"


using boost::shared_ptr;


#ifdef TIXML_USE_STL
	#include <iostream>
	#include <sstream>
	using namespace std;
#else
	#include <stdio.h>
#endif

#if defined( WIN32 ) && defined( TUNE )
	#include <crtdbg.h>
	_CrtMemState startMemState;
	_CrtMemState endMemState;
#endif


extern char cFolderSeparator;




struct Controller
{



	// Fields
    bool bDebug_;

	vector<ImageUShortVolumeType::Pointer> ITKFeatureVolume_;

	// Trees 
    vector<ptrsmartDecisionTree> decisionForest_;             // random decision forest
    vector<ptrsmartDecisionTree> auxDecisionForest_;    // Auxilliary forest, eg for autocontext
    bool bAuxDecisionForestLoaded_;

	// Results 
    vector<byte> autoClassMAPLabels_;                    // output of forest: automatically computed class labels (MAP)
    ushort* autoClassPosterior3D_;             // output of forest: automatically computed class posteriors for a given class (3D)

    // Posterior autoClassPosterior4D_;            // output of forest: automatically computed class posteriors (4D)
    bool COMPUTED_POSTERIOR4D_;
    int selectedClassIndex_;                // default is 1 (next class after background)

	// Parameters
    string strForestSettingsFilename;           // xml file for all forest settings
    string parametersFolder_;
    bool FOREST_PARAMETERS_LOADED_;                         // flag
    ForestParameters forestParams_;     // forest parameters
    EnvironmentParameters structEnvironmentParameters_;

	// Datasets
	vector<ptrsmartDataset> trainingDatasets_;                 // All training datasets data (pixels, g.t. annotations etc.)
    bool TRAINING_DATASETS_LOADED_;

    vector<ptrsmartDataset> testingDatasets_;                  // All testing datasets data (pixels, g.t. annotations etc.)
    bool TESTING_DATASETS_LOADED_;

	// Environment
	bool bEnvironmentAndForestSettingsAreInSameFolder_;  // default is false (EnvironmentSettings.xml is found one directory above where ForestSettings.xml is, Set to true for environments where they are in same folder 

	// method declarations
    int LoadForest_DeriveChannelsFromITKTest_ThenTestAndWriteOutput(string& strT1VolumeFilename, string& strTissueSegmentationVolumeFilename,
		ImageUShortVolumeType::DirectionType& outputDirection, ImageUShortVolumeType::PointType& outputOrigin);
	int loadITKVolume(string& strT1VolumeFilename, FEATURE_VOLUME_TYPE iFeatureVolumeType);
	int saveByteVolumeToITK(ptrsmartDataset& psDataset, byte* labelVolume, 
										string& strTissueSegmentationVolumeFilename,ImageUShortVolumeType::DirectionType& outputDirection, ImageUShortVolumeType::PointType& outputOrigin);
	void LoadSettings();
	void selectSettingsFileAndLoad();


   // method definitions 
	// Ctor
	Controller()
	{
		bDebug_ = true;
		bAuxDecisionForestLoaded_ = false;

		// Results 
		autoClassPosterior3D_ = nullptr;             
		COMPUTED_POSTERIOR4D_ = false;
		selectedClassIndex_ = 1;                // default is 1 (next class after background)

		// Parameters
		bool FOREST_PARAMETERS_LOADED_ = false;                         // flag

		// Datasets
		TRAINING_DATASETS_LOADED_ = false;
		TESTING_DATASETS_LOADED_ = false;

		ITKFeatureVolume_.resize(FEATURE_VOLUME_TYPE_count);
		for (int i=0; i<ITKFeatureVolume_.size(); i++) ITKFeatureVolume_[i]=nullptr;

	}

	~Controller()
	{
		if (autoClassPosterior3D_!=nullptr)  { delete [] autoClassPosterior3D_; autoClassPosterior3D_=nullptr; }
	}



    bool forestInMemory()
    {
        if (!decisionForest_.empty())
        {   if (decisionForest_[0])   // if we have at least one trained tree (non null) 
			{
				if (decisionForest_[0]->nodes_!=nullptr)  // did we initialize the first tree already
				     if (decisionForest_[0]->nodes_[0]!=nullptr) return true;  // and if its root node of first tree is nonnull (we probably dont need this additional level of checking)
			}
        }
        return false;
    }

    /// <summary>
    ///   Display statemachine status 
    /// </summary>
    void displayStatemachineStatus()
    {
        if (bDebug_)
        {
            cout << "----------------------- Statemachine status -----------------------\n";
            cout << "parameters loaded: " << (FOREST_PARAMETERS_LOADED_ ? "Y" : "N") << "\n";
            cout << "train data loaded: " << (TRAINING_DATASETS_LOADED_ ? "Y" : "N") << "\n";
            cout << "test data loaded: " << (TESTING_DATASETS_LOADED_ ? "Y" : "N") << "\n";

            bool bForestInMemory = forestInMemory();
            cout << "forest trained or loaded: " << (bForestInMemory ? "Y" : "N") << "\n";
            cout << "Auxilliary forest trained or loaded: " << (bAuxDecisionForestLoaded_ ? "Y" : "N") << "\n";
            cout << "posterior computed: " << (COMPUTED_POSTERIOR4D_ ? "Y" : "N") << "\n";
            // cout << "View mode: " << viewMode_.ToString() << "\n";
            cout << "class labels computed: " << ((!autoClassMAPLabels_.empty() ? "Y" : "N")) << "\n";
            // cout << "test or train mode: " << (TEST_TRAIN ? "TEST" : "TRAIN") << "\n";
            cout << "===========================================\n";
        }

    }

    void loadAllTrainingDatasets()
    {
		trainingDatasets_.clear();
		trainingDatasets_.reserve( forestParams_.train_DatasetIndeces_.size());
        for (unsigned int i = 0; i < forestParams_.train_DatasetIndeces_.size(); i++)
        {
            string datasetFileName = structEnvironmentParameters_.databaseFolder_ +  cFolderSeparator + "Dataset" + lexical_cast<string>((unsigned int)forestParams_.train_DatasetIndeces_[i]) + cFolderSeparator + "dataset.xvd";
            string gtStructuresFileName = structEnvironmentParameters_.databaseFolder_ + cFolderSeparator + "Dataset" + lexical_cast<string>((unsigned int)forestParams_.train_DatasetIndeces_[i]) + cFolderSeparator + "gtStructures.bvd";

            // AAM: to conserve mem, load specific downsampled version from disk. If not there it's created
			if (forestParams_.modalIndeces_.empty()){
				ptrsmartDataset psDataset(new Dataset(datasetFileName, gtStructuresFileName, 
					forestParams_.volumeDownSampleFactor_, forestParams_.arrKnownGTLabelToPosteriorBinLUT_));
				trainingDatasets_.push_back(psDataset);
			}
			else{
				if (forestParams_.intHistind_.empty()){  // No histogram channels
					ptrsmartDataset psDataset(new Dataset(datasetFileName, gtStructuresFileName, 
						forestParams_.volumeDownSampleFactor_, forestParams_.arrKnownGTLabelToPosteriorBinLUT_,forestParams_.modalIndeces_,forestParams_.bAbsSym_));
					trainingDatasets_.push_back(psDataset);}
				else{
					ptrsmartDataset psDataset(new Dataset(datasetFileName, gtStructuresFileName, 
						forestParams_.volumeDownSampleFactor_, forestParams_.arrKnownGTLabelToPosteriorBinLUT_,forestParams_.modalIndeces_,forestParams_.intHistind_,forestParams_.Histlvls_,forestParams_.bAbsSym_));
					trainingDatasets_.push_back(psDataset);}
				}
			
            // trainingDatasets_[i].downSample(forestParams_.volumeDownSampleFactor_); // downsampling volume

			// for debugging
			// cout << "Dataset volume dimensions: width " << psDataset->dims_.Width << ", height " << psDataset->dims_.Height << ", Depth " << psDataset->dims_.Depth << "\n";

        }
        TRAINING_DATASETS_LOADED_ = true;
        displayStatemachineStatus();

    }

	
    void loadAllTestingDatasets()
    {
		testingDatasets_.clear();
        testingDatasets_.reserve( forestParams_.test_DatasetIndeces_.size());
        for (unsigned int i = 0; i < forestParams_.test_DatasetIndeces_.size(); i++)
        {
            string datasetFileName = structEnvironmentParameters_.databaseFolder_ + cFolderSeparator + "Dataset" + lexical_cast<string>((unsigned int)forestParams_.test_DatasetIndeces_[i]) + cFolderSeparator + "dataset.xvd";
            string gtStructuresFileName = structEnvironmentParameters_.databaseFolder_ + cFolderSeparator + "Dataset" + lexical_cast<string>((unsigned int)forestParams_.test_DatasetIndeces_[i]) + cFolderSeparator + "gtStructures.bvd";

			float fDownSampleFactor;
			// fDownSampleFactor=1.0f;  // Uncomment to force no downsampling for testing :
			fDownSampleFactor=forestParams_.volumeDownSampleFactor_; // otherwise use downsampling specified in parameter file

            // AAM: to conserve mem, load specific downsampled version from disk. If not there it's created
			if (forestParams_.modalIndeces_.empty()){
				ptrsmartDataset psDataset(new Dataset(datasetFileName, gtStructuresFileName, 
					fDownSampleFactor, forestParams_.arrKnownGTLabelToPosteriorBinLUT_));
				testingDatasets_.push_back(psDataset);
			}
			else{
				if (forestParams_.intHistind_.empty()){
					ptrsmartDataset psDataset(new Dataset(datasetFileName, gtStructuresFileName, 
						forestParams_.volumeDownSampleFactor_, forestParams_.arrKnownGTLabelToPosteriorBinLUT_,forestParams_.modalIndeces_,forestParams_.bAbsSym_));
					testingDatasets_.push_back(psDataset);}
				else{
					ptrsmartDataset psDataset(new Dataset(datasetFileName, gtStructuresFileName, 
						forestParams_.volumeDownSampleFactor_, forestParams_.arrKnownGTLabelToPosteriorBinLUT_,forestParams_.modalIndeces_,forestParams_.intHistind_,forestParams_.Histlvls_,forestParams_.bAbsSym_));
					testingDatasets_.push_back(psDataset);}
				}

            /*ptrsmartDataset psDataset(new Dataset(datasetFileName, gtStructuresFileName,
                    fDownSampleFactor, forestParams_.arrKnownGTLabelToPosteriorBinLUT_));

			testingDatasets_.push_back(psDataset); */
        }
        TESTING_DATASETS_LOADED_ = true;
        displayStatemachineStatus();
    }

    void loadOneCommandLineSpecifiedDataset()
    {
		testingDatasets_.clear();
        testingDatasets_.reserve( 1 );
        
        
        string gtStructuresFileName = "dummy"; 

		ptrsmartDataset psDataset(new Dataset(ITKFeatureVolume_, gtStructuresFileName, 
			forestParams_.volumeDownSampleFactor_, forestParams_.arrKnownGTLabelToPosteriorBinLUT_,
			forestParams_.modalIndeces_,forestParams_.bAbsSym_));
		testingDatasets_.push_back(psDataset);
			

        TESTING_DATASETS_LOADED_ = true;
        displayStatemachineStatus();
    }
     


	
bool LoadForest()
{
    bool bReturn = false;
    try
    {
       cout << "\n>> Loading decision forest (for testing) ...   \n";
	   decisionForest_.clear();
	   decisionForest_.reserve(forestParams_.test_numTreesInForest_);
        for (int treeIndex = 0; treeIndex < forestParams_.test_numTreesInForest_; treeIndex++)
        {
			ptrsmartDecisionTree psDecisionTree(new DecisionTree());

			if (!strForestSettingsFilename.empty()) { // Load tree from file in directory that contains the forest parameters XML file
				string strTreeFilepath=parametersFolder_ + "DecisionTree_" + lexical_cast<string>(treeIndex) + ".dtf";
				bReturn=psDecisionTree->readFromBinaryDTFFile(strTreeFilepath);
			} else {  // No forest parameters files so load tree from file default internal string  
				// Concatenate fractionated Base64 literal into one long Base64 string
				// Reason: max length of string literal is very limited so fractionated to get it to compile
				std::stringstream ss;
				int numLines;
				switch (treeIndex) {
					case 0: numLines=sizeof_default_DecisionTree_0/sizeof(default_DecisionTree_0[0]);
						    for (int ii=0; ii<numLines; ii++) ss << default_DecisionTree_0[ii];
							break;
					case 1: numLines=sizeof_default_DecisionTree_1/sizeof(default_DecisionTree_1[0]);
						    for (int ii=0; ii<numLines; ii++) ss << default_DecisionTree_1[ii];
							break;
					case 2: numLines=sizeof_default_DecisionTree_2/sizeof(default_DecisionTree_2[0]);
						    for (int ii=0; ii<numLines; ii++) ss << default_DecisionTree_2[ii];
							break;
					case 3: numLines=sizeof_default_DecisionTree_3/sizeof(default_DecisionTree_3[0]);
						    for (int ii=0; ii<numLines; ii++) ss << default_DecisionTree_3[ii];
							break;
					case 4: numLines=sizeof_default_DecisionTree_4/sizeof(default_DecisionTree_4[0]);
						    for (int ii=0; ii<numLines; ii++) ss << default_DecisionTree_4[ii];
							break;
					default: 
                            cout << "| Error default forest has only 5 trees, and attempting to use more. Possible mismatch between default ForestParameters.xml and default forest files. \n";
				}; // end switch

				std::string singleString = ss.str();
				unsigned char *decodedBuffer= new unsigned char[singleString.size()](); // pre-allocate and use () to value initialize all elements to zero

				// convert value from Base64
				unsigned int decodedLengthActual = static_cast<unsigned int>(
					itksysBase64_Decode(
						(const unsigned char *) singleString.c_str(),
						static_cast<unsigned long>( 0 ),
						(unsigned char *) decodedBuffer,
						static_cast<unsigned long>( singleString.size())
						));

				// string debugInspect=(const char*)decodedBuffer;
				
				bReturn=psDecisionTree->readFromBinaryStream(decodedBuffer, decodedLengthActual);  // populates the DecisionTree with values in the stream
				delete []decodedBuffer;
			}

			if (!bReturn) 
			{
					break;
			}

			decisionForest_.push_back(psDecisionTree);
        }

		if (bReturn) cout << "| Loading decision forest (for testing) done.\n";
		else cout << "| Error loading the forest.\n";
    }
    catch (...)
    {
       cout << "| ERROR: Loading decision forest.\n";
	   decisionForest_.clear();
    }

    if (bReturn) displayStatemachineStatus();
    return bReturn;
}


};

