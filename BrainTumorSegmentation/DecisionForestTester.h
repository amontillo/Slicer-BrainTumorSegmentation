#pragma once
#include "basicDefs.h"
#include "Posterior.h"
#include "DecisionTree.h"
#include "EnvironmentSettingsFileManager.h"
#include "Dataset.h"
#include "ForestParameters.h"
#include "FeatureFarm.h"
#include "TimeSpanCastManager.h"
#include <boost/timer.hpp>
#include <boost/shared_ptr.hpp>
#include <blitz/array.h>

using namespace std;
using boost::lexical_cast;
using TimeSpanCastManager::timespan_cast;


/// <summary>
/// Parallel testing of all decision trees in forest on a single input dataset
/// </summary>
struct DecisionForestTester
{
	// references
    vector<ptrsmartDecisionTree>& decisionForest_;          // The already trained decision forest
    vector<ptrsmartDecisionTree>& auxDecisionForest_;    // auxilliary forest for other functionality, e.g. autocontext
	vector<byte>&  classIndeces_;                           // All the classes we have trained for

    int numTestTrees_;
    int maxTestTreeDepth_;
    byte* outClassLabels_;                         // output of forest: byte class labels (MAP)
	
    Posterior* posterior_;                           // this is a 4D float array

    
    ptrsmartDataset psDataset_;  // The single input dataset to be tested



    VolumeDimensions volDims_;    // Dataset dimensions 

	// Borrowed pointers (Like FeatureComputer, this DecisionForestTester doesnt own this memory, kept as native rather than smartptr so no chance of performance penalty)
    ushort* intensVolume_;                                 // intensity volume for input dataset
    ushort** AdditionalIntensVolumes_;             // intensity volume for input dataset
    double* dintensVolume_;                                 // double version of intensity data
    double** dAdditionalIntensVolumes_;            // additional channels beyond intensVolume
	blitz::Array<ushort,4>* pAvolume;
	blitz::Array<long long,4>* pAIvolume;

	vector<double> AxisSym;
	vector<double> scales_;

    int numTestingPoints_;                          // number of testing points = volume of input dataset

    int numTestThreads_;                                             // AAM: max number of parallel threads for testing (applying the forest to the test data); read in from EnvironmentSettings.xml
    vector<vector<int> > testIndeces_;                         // tree index lists in jagged array. These contain the lists of trees which will be trained by each of the four threads 

    const float quantizPrecision;  

	ForestParameters& forestParams_;  // for computing filename paths 
        
    // limitation is that now we can have at most 16trees max, or we will have overflow in the ushort array in posterior_
    // Change quantizPrecision to 2000 will allow at most 32 trees before overflow. For details, see my notes in TestFile1.txt 

	~DecisionForestTester()
	{
		if (outClassLabels_!=nullptr) {delete [] outClassLabels_;   outClassLabels_=nullptr; };
		if (posterior_!=nullptr) { delete posterior_; posterior_=nullptr; };
	};

	DecisionForestTester(vector<ptrsmartDecisionTree>& decisionForest, ForestParameters& forestParameters, EnvironmentParameters& structEnvironmentParameters, 
	                                      vector<ptrsmartDecisionTree>& auxDecisionForest);
	void parallelTest(ptrsmartDataset psDataset, string outputFolder);


	byte* outClassLabels() { return outClassLabels_; } ;   // MAP class label volume
    Posterior* outClassPosterior() { return posterior_; } ;   // class posterior 4D volume

    void testNode_and_SendDataToChildren_NodeList(ptrsmartDecisionTree psDT, int nodeIndex, ptrsmartIndexVolume psIndexVolume, ptrsmartFeatureComputer psFeatComp, int nHeightWidth, int nWidth);
 

	void accumulatePosterior(ptrsmartDecisionTree psDT, ptrsmartIndexVolume psIndexVolume, Posterior* posterior);
	void computeNodePosterior(TreeNode& node, float* nodePosteriorNormalized, int numClasses);
	void computeMeanPosteriorOverForest(Posterior* posterior, int numTrees);
	byte* mapAssignments(Posterior* posterior);

	void threadCore(int objThreadID);

};


typedef boost::shared_ptr<DecisionForestTester> ptrsmartDecisionForestTester;



