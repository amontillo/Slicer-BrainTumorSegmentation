#pragma once

#include <iostream>
#include <sstream>
#include <string> 
#include <vector>
#include "tinyxml.h" 
#include "basicDefs.h"
#include "EntropyImplementationType.h"



using namespace std;


/// <summary>
/// Parameters governing forest construction and application (training and testing)
/// </summary>
struct ForestParameters
    {
         int train_numTreesInForest_;         // number of trees in forest (for training)
         int train_treeDepth_;                // max depth of trees (for training)
         int train_numFeaturesPerNode_;       // number of randomly generated features to be tried when training each node
         int train_numThresholdsPerFeature_;  // number of thresholds to be tried when optimizing each feature in each node
		 
		 // Feature types
         bool bChannelIntensityRef_; // true to use this feature , false otherwise
         bool bAbsoluteIntensity_;  // true to use this feature , false otherwise
         bool bRelativeIntensity_;  //true to use this feature , false otherwise
         bool bRelativePosterior_;  //true to use this feature , false otherwise
         bool bRatioIntensity_;  // true to use this feature , false otherwise
         bool bDiffOf2Probes_;  // true to use this feature , false otherwise
         bool bAbsoluteSpatialAppearanceEntanglement1a_;  // true to use this feature , false otherwise
         bool bSimilarityOfAppearancesEntanglement1b_;  // true to use this feature , false otherwise
         bool bMAPLabelEntanglement2a_;  // true to use this feature , false otherwise
         bool bPosteriorBinEntanglement2b_;  // true to use this feature , false otherwise
         bool bTop2Classes_;  // true to use this feature , false otherwise
         bool bTop3Classes_;  // true to use this feature , false otherwise
         bool bTop4Classes_;  // true to use this feature , false otherwise
         bool bAutocontextMAPLabel_; // true to use this feature , false otherwise
		 bool bHistoDiff_; // true to use this feature , false otherwise
		 bool bHistoMean_; // true to use this feature , false otherwise
		 bool bHistoVar_; // true to use this feature , false otherwise
		 bool bHistoSkew_; // true to use this feature , false otherwise
		 bool bHistoKurt_; // true to use this feature , false otherwise
		 bool bHistoEnt_; // true to use this feature , false otherwise
		 bool bAbsSym_;// true to use this feature , false otherwise

         //
         float train_maxFeatureRadius_;       // def. 100 (in mm)
         double train_stopCrit_minInfoGain_;  // stopping criterion 1. Min info gain
         double train_stopCrit_minNumPoints_; // stopping criterion 2. Min num points in node
         int test_numTreesInForest_;          // number of trees in forest (for testing)
         int test_treeDepth_;                 // max depth of trees (for testing)
         float volumeDownSampleFactor_;       // 2 or 4 are the only valid values for now

		 vector<byte>  classIndeces_;                // indeces representing the organ classes (defined in AnatomicalStructureManager)
         vector<string> arrstrAuxTreePathnames_; // pathnames of trees of an auxilliary forest
         vector<byte> train_DatasetIndeces_;        // indeces of datasets used for training
         vector<byte>  test_DatasetIndeces_;         // indeces of datasets used for testing
		 vector<int> arrKnownGTLabelToPosteriorBinLUT_; // AAM:  posterior class distn bin of each known label
		 vector<string> modalIndeces_; //extension names for the multiple modalities Anthony
		 vector<string> intHistind_;//index of integral histogram images

		 unsigned int Histlvls_;//max number of levels in the histogram the values can be from 0 to (Histlvls_-1)
		 
		 unsigned int minBox [3]; //minimum averaging box size
		 unsigned int maxBox [3]; //maximum averaging box size


         bool bPerformMerging_;  // true to perform merging, false otherwise
         string strMergingType_; // {WithinTree*, AcrossTrees}   * indicates default
         string strDistanceMeasureType_; // {Euclidean*, Dirichlet}   * indicates default
         double dMaxMergeDistance_; // default 100
         string strMergeLocationType_;  // {Level7,Level10*, AllLevels }  * indicates default
         string strCandidateMergeNodeType_; // { ToBeSplitAndLeaves*, AllPairs, ToBeSplit, Leaves } * indicates default
         string strForestParametersFilename_;
        // AAM: Proposal distribution parameters
         bool bUseAcceptedFeatureParameterDensities_; // 
         double dFractionDensityPedestal_; // Lift for the zeros in the densities so that there is some non-zero prob of all possible parameters
         bool bSpikeInProposalDistn_;  // true to preferentially offer one threshold often for various feature types... (such as zero (water) for absolute intensity)

        // AAM: Entropy fix
        enumComputeEntropyImplementationType nEntropyImplementationType_; // OriginalImplementation = 0, 

         bool bDeterministic_; // Whether or not to use a fixed, deterministic seed for all random number generation , rather than random time of execution
         int nSeed_; // Fixed seed to use when bDeterministic is true

        // FeatTypePropDistn
         bool bUseAcceptedFeatureTypeDensities_; // {true, false*}  true to use FeatTypePropDistn, false otherwise
        
        // Autocontext
         bool bUseAutocontext_; // {true, false*}  true to use autocontext, false otherwise
         string strAutocontextForestName_; // short name for forest, used as suffix for MAPLabelImages

         string strTrainingAlgorithm_;

		 int nNumCrossValidationFolds_;
		 bool bOutputIndividualFoldPerformance_;  // set to true to output the performance metrics for each fold (in addition to overall performance across all folds)

        ///// <summary>
        ///// Visualizating parameters for debug purposes
        ///// </summary>
         void writeToConsole();
    };

struct ForestSettingsFileManager
{
	   static bool readElementAndItsChildren(TiXmlNode* pxmlNode, ForestParameters& forestParams);

        /// <summary>
        /// Simple xml parser to read forest settings from file
        /// </summary>
         static bool readFromFile(const string& xmlSettingsFileName_IN, ForestParameters& forestParams);
};
