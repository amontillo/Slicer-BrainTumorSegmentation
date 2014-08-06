#include "DecisionForestTester.h"


using namespace std;
using boost::lexical_cast;

// ITK
#include "itkImage.h"
#include "itkCastImageFilter.h" 

// For progress meter 
// Using a global here so DecisionForestTester::threadcore() can report progress.
typedef itk::Image<unsigned short, 3> 	dummyInternalImageType;
typedef itk::Image<float, 3> 	dummyOutputImageType;
typedef itk::CastImageFilter<dummyInternalImageType, dummyOutputImageType> CastingFilterType;
extern CastingFilterType::Pointer caster;


/// <summary>
/// Constructor
/// </summary>
/// <param name="trainingDatasets">All training volumes, their gt labels and calib parameters</param>
DecisionForestTester::DecisionForestTester(vector<ptrsmartDecisionTree>& decisionForest, ForestParameters& forestParameters, EnvironmentParameters& structEnvironmentParameters, 
	                                    vector<ptrsmartDecisionTree>& auxDecisionForest)
		   :decisionForest_(decisionForest),           // input trained decision forest
			auxDecisionForest_(auxDecisionForest), // Auxilliary decision forest, e.g. for autocontext
			classIndeces_(forestParameters.classIndeces_), // organ class indeces
			forestParams_(forestParameters),
			quantizPrecision(32)  // If quantizPrecision=2000 = max #trees 32, 1000-->64,500-->128,250-->256,  125-->512, / 62-->1024 , 31 --> 2048 trees.   So at 5 we can have >9,000 trees
    {
		posterior_ = nullptr;
		outClassLabels_ = nullptr;

        numTestTrees_ = forestParameters.test_numTreesInForest_;
        maxTestTreeDepth_ = forestParameters.test_treeDepth_;
        if (maxTestTreeDepth_ > forestParameters.train_treeDepth_) maxTestTreeDepth_ = forestParameters.train_treeDepth_;

        numTestThreads_ = structEnvironmentParameters.test_numThreads_;
		// testIndeces_ = new int[numTestThreads_][];
		testIndeces_.resize(numTestThreads_);

    };

/// <summary>
/// integrates results across trees, from the lowest level 
/// Multithreaded testing of forest on single dataset
/// </summary>
void DecisionForestTester::parallelTest(ptrsmartDataset psDataset, string outputFolder)
{


	bool bReturn=true;  // [ ] Eventually, need to make parallelTest return success/failure

    cout << "\n\n| Forest testing started (multithreaded)...\n";
    cout << "| Testing " << numTestTrees_ << " trees, ";
    cout << " depth " << maxTestTreeDepth_ << "\n";

	boost::timer timerApplyForestToOneDataset;


	// *****************************************************
    // Initializations
	// *****************************************************
    // input dataset
    psDataset_ = psDataset;
    intensVolume_ = psDataset_->intensVolume_;
    AdditionalIntensVolumes_ = psDataset_->AdditionalIntensVolumes_;

    dintensVolume_ = psDataset_->dintensVolume_;   // double versions
    dAdditionalIntensVolumes_ = psDataset_->dAdditionalIntensVolumes_;

	pAvolume = &(psDataset->Avolume);//multimodal array
	pAIvolume = &(psDataset->AIvolume);//multimodal integral image

	AxisSym = psDataset->SymAxis_;
	scales_ = psDataset->scales_;

    volDims_ = psDataset_->dims_;
    numTestingPoints_ = psDataset_->volume();

	 
    // initializing posterior
	if (posterior_!=nullptr) { delete posterior_; posterior_=nullptr;};
    posterior_ = new Posterior(volDims_, (int)classIndeces_.size());    // initializing a 4D array which will accumulate the outputs of all tree posteriors

	// *****************************************************
	// Perform basic load balancing
	// *****************************************************
    if (numTestTrees_ < numTestThreads_)    // if less than trees in the forest
    {
		testIndeces_.resize(numTestTrees_);
        for (int i = 0; i < numTestTrees_; i++)
        {
            testIndeces_[i].resize(1); testIndeces_[i][0] = i;
        }
    }
    else                                // if >= numTestThreads , then some threads perform test multiple trees 
    {
        int div, rem;
		rem = numTestTrees_ % numTestThreads_;
		div = numTestTrees_ / numTestThreads_;

		testIndeces_.resize(numTestThreads_);
        for (int i = 0; i < rem; i++) testIndeces_[i].resize(div + 1);

        for (int i = rem; i < numTestThreads_; i++) testIndeces_[i].resize(div);

        for (int treeIndex = 0; treeIndex < numTestTrees_; treeIndex++)
        {
            int posInThread, threadIndex;
			posInThread = treeIndex / numTestThreads_;
			threadIndex = treeIndex % numTestThreads_;
            testIndeces_[threadIndex][posInThread] = treeIndex;
        }
    }


    for (int nThreadID = 0; nThreadID < numTestThreads_; nThreadID++)  
    {
		threadCore(nThreadID);
    }


	// *****************************************************
    // Conclusion
	// *****************************************************
            
    //// wait for all test threads to complete 
	
	bool bComputeLabels=true; 



	if (bComputeLabels)
	{   

		// Computing the mean posterior over all trees in forest
		computeMeanPosteriorOverForest(posterior_, (int)decisionForest_.size());   // (*posterior_).postArray_,600

		// Maximum a-posteriori voxel labels
		outClassLabels_ = mapAssignments(posterior_);

		double dElabsedTime=timerApplyForestToOneDataset.elapsed();
		cout << "| Forest testing completed in " << timespan_cast(dElabsedTime) << " seconds.\n";
	}

	// return bReturn;
};





/// <summary>
/// This method applies the stored test to each pixel and sends it to the respective child until it reaches a leaf node
/// </summary>
void DecisionForestTester::testNode_and_SendDataToChildren_NodeList(ptrsmartDecisionTree psDT, int nodeIndex, ptrsmartIndexVolume psIndexVolume, ptrsmartFeatureComputer psFeatComp, int nHeightWidth, int nWidth)
{
        // Dont process node that have been merged  (or invalidated)
        if (psDT->nodes_[nodeIndex]->nNodeID_ != nodeIndex) return; 

        #if _DEBUG
            int Debug_nNumPtsAdded = 0;
        #endif


		
		vector<int>* pListDatasetsWithNodeVoxelsLeftChild = nullptr; // currently we only test one dataset at a time, but future we might have more than 1 dataset in this list
		vector<vector<int>*>* pListOfListsDatasetVoxelOffsetsLeftChild = nullptr;
		vector<int>* pListDatasetsWithNodeVoxelsRightChild = nullptr;
		vector<vector<int>*>* pListOfListsDatasetVoxelOffsetsRightChild = nullptr;

		vector<int>* pListDatasetsWithNodeVoxels = psDT->nodes_[nodeIndex]->pListDatasetsWithNodeVoxels_; // the list of datasets that contain at least one voxel at the node
		vector<vector<int>*>* pListOfListsDatasetVoxelOffsets = psDT->nodes_[nodeIndex]->pListOfListsDatasetVoxelOffsets_;
		if ((pListOfListsDatasetVoxelOffsets == nullptr) || (pListDatasetsWithNodeVoxels==nullptr)) return; // no pixels at node
		if ((pListOfListsDatasetVoxelOffsets->empty()) || (pListDatasetsWithNodeVoxels->empty() )  ) return; // no pixels at node
		vector<int>* pListDatasetVoxelOffsets = (*pListOfListsDatasetVoxelOffsets)[0];  // we always take the voxels from the first dataset since currently we only test one dataset at a time

        // Children
        int nNumTestDatasetPointsInitialCapacityPerChild = (int)(0.25 * (*pListOfListsDatasetVoxelOffsets)[0]->size());  // 3.5 x slower (3.5 min) than LevAtATime, but still 10x faster than testNode_and_SendDataToChildren()
        // int nNumTestDatasetPointsInitialCapacityPerChild = (*pListOfListsDatasetVoxelOffsets)[0]->size();  // Also: 3.5 x slower (3.5 min) than LevAtATime
		vector<int>* pListDatasetVoxelOffsetsRightChild = new vector<int>;
		pListDatasetVoxelOffsetsRightChild->reserve(nNumTestDatasetPointsInitialCapacityPerChild);  // Dont known in advance what the capacity should be, MAYBE should try 1/4 size of node?
        // want to balance wasted space if too large vs wasted time reallocating the list ( array ) as we add points.
		vector<int>* pListDatasetVoxelOffsetsLeftChild = new vector<int>;
		pListDatasetVoxelOffsetsLeftChild->reserve(nNumTestDatasetPointsInitialCapacityPerChild); // Dont known in advance what the capacity should be
		// --
        int nLeftChildIndex = psDT->getLeftChildIndex(nodeIndex);    
        int nRightChildIndex = psDT->getRighChildIndex(nodeIndex);

        // Send voxels to their corresponding L/R child node
        TreeNode* pNode = psDT->nodes_[nodeIndex];
       
        int* PIndx = psIndexVolume->nodeIndexArray_;
        for (int iOfs = 0; iOfs < pListDatasetVoxelOffsets->size(); iOfs++) // TAG_Candidate_for_OpenMP_parallelization, but have to deal with merging pListDatasetVoxelOffsetsLeftChild from each task thread
        {
            int nOffset = (*pListDatasetVoxelOffsets)[iOfs];
            int* pTreeNodeIndexOfVoxel = PIndx + nOffset;

            // convert nOffset into x0,y0, z0
			int z0 = nOffset / nHeightWidth;
			int nRemainder = nOffset % nHeightWidth;
			int y0 = nRemainder / nWidth;
			int x0 = nRemainder % nWidth;
			blitz::Array<double,1> histProbe1(pNode->refHist.extent(0));
			double featureResponse;

			if ((*pAvolume).size() == 0) 
				featureResponse = psFeatComp->featureResponse(x0, y0, z0, pNode->dispFeature_, pNode->featureType_, pNode->uintFeatureParameter_);   // feature response
			else
				featureResponse = psFeatComp->featureResponse(x0, y0, z0, pNode->dispFeature_, pNode->featureType_, pNode->uintFeatureParameter_, pNode->Modality_, pNode->Boxsize_);   // feature response




            if (featureResponse > pNode->threshold_) 
            {   *pTreeNodeIndexOfVoxel = nRightChildIndex; // sending pixel to right child node
			    pListDatasetVoxelOffsetsRightChild->push_back(nOffset);
                #if _DEBUG
                Debug_nNumPtsAdded++;
                #endif
            } 
            else 
            {
                *pTreeNodeIndexOfVoxel = nLeftChildIndex;  // sending pixel to left child node
                pListDatasetVoxelOffsetsLeftChild->push_back(nOffset);
                #if _DEBUG
                Debug_nNumPtsAdded++;
                #endif
            }

        } // End sequential for  over voxel list
        

        if (!pListDatasetVoxelOffsetsRightChild->empty())
        {
            if (pListDatasetsWithNodeVoxelsRightChild == nullptr) pListDatasetsWithNodeVoxelsRightChild = new vector<int>;
            if (pListOfListsDatasetVoxelOffsetsRightChild == nullptr) pListOfListsDatasetVoxelOffsetsRightChild = new vector<vector<int>*>;
            pListDatasetsWithNodeVoxelsRightChild->push_back(0);  // currently we only test one dataset at a time
            pListOfListsDatasetVoxelOffsetsRightChild->push_back(pListDatasetVoxelOffsetsRightChild);
        }
		else
		{   // NOTE: Avoid a memory leak and explictly free when no voxels are stored in it
			if (pListDatasetVoxelOffsetsRightChild!=nullptr) {delete pListDatasetVoxelOffsetsRightChild; pListDatasetVoxelOffsetsRightChild=nullptr;} 
		}

        if (!pListDatasetVoxelOffsetsLeftChild->empty())
        {
            if (pListDatasetsWithNodeVoxelsLeftChild == nullptr) pListDatasetsWithNodeVoxelsLeftChild = new vector<int>;
            if (pListOfListsDatasetVoxelOffsetsLeftChild == nullptr) pListOfListsDatasetVoxelOffsetsLeftChild = new vector<vector<int>*>;
            pListDatasetsWithNodeVoxelsLeftChild->push_back(0); // currently we only test one dataset at a time
            pListOfListsDatasetVoxelOffsetsLeftChild->push_back(pListDatasetVoxelOffsetsLeftChild);
        }
		else
		{   // NOTE: Avoid a memory leak and explictly free when no voxels are stored in it
			if (pListDatasetVoxelOffsetsLeftChild!=nullptr) {delete pListDatasetVoxelOffsetsLeftChild; pListDatasetVoxelOffsetsLeftChild=nullptr;} 
		}

		// since we are done using these pointers, we null them out for safety; we are not trying to release memory in these next 2 lines
        pListDatasetVoxelOffsets = nullptr;
        pListOfListsDatasetVoxelOffsets = nullptr;

		// Now we release memory used by the parent node
        if (pNode->pListDatasetsWithNodeVoxels_ != nullptr)
		{
			delete pNode->pListDatasetsWithNodeVoxels_;
			pNode->pListDatasetsWithNodeVoxels_=nullptr;
		}
        if (pNode->pListOfListsDatasetVoxelOffsets_ != nullptr)
        {
            for (int i = 0; i < pNode->pListOfListsDatasetVoxelOffsets_->size(); i++)
            {
				vector<int>* localListOfDatasetVoxelOffests=(*(pNode->pListOfListsDatasetVoxelOffsets_))[i];
				if (localListOfDatasetVoxelOffests!=nullptr)
				{
					delete localListOfDatasetVoxelOffests;
					localListOfDatasetVoxelOffests=nullptr;
					(*(pNode->pListOfListsDatasetVoxelOffsets_))[i]=nullptr;
				}
            }

			delete pNode->pListOfListsDatasetVoxelOffsets_;
			pNode->pListOfListsDatasetVoxelOffsets_=nullptr;
        }
  
		// Count the number of voxels sent to left and right children
        int best_nPtsL_Scan2 = 0;
        if (pListOfListsDatasetVoxelOffsetsLeftChild != nullptr)
        {
            for (int i = 0; i < pListOfListsDatasetVoxelOffsetsLeftChild->size(); i++)
                if ((*pListOfListsDatasetVoxelOffsetsLeftChild)[i] != nullptr) best_nPtsL_Scan2 += (int) (*pListOfListsDatasetVoxelOffsetsLeftChild)[i]->size();
        }

        int best_nPtsR_Scan2 = 0;
        if (pListOfListsDatasetVoxelOffsetsRightChild != nullptr)
        {
            for (int i = 0; i < pListOfListsDatasetVoxelOffsetsRightChild->size(); i++)
                if ((*pListOfListsDatasetVoxelOffsetsRightChild)[i] != nullptr) best_nPtsR_Scan2 += (int)(*pListOfListsDatasetVoxelOffsetsRightChild)[i]->size();
        }


        // Set lists of nodes at children
        if (best_nPtsL_Scan2 > 0)
        {
            // When testing a merged tree we will need something like AddToPixelLists()
            // DT.nodes_[nLeftChildIndex].AddToPixelLists(ref listDatasetsWithNodeVoxelsLeftChild, ref listOfListsDatasetVoxelOffsetsLeftChild);
			psDT->nodes_[nLeftChildIndex]->pListDatasetsWithNodeVoxels_= pListDatasetsWithNodeVoxelsLeftChild;
			psDT->nodes_[nLeftChildIndex]->pListOfListsDatasetVoxelOffsets_= pListOfListsDatasetVoxelOffsetsLeftChild;
        }
        if (best_nPtsR_Scan2 > 0)
        {
            // DT.nodes_[nRightChildIndex].AddToPixelLists(ref  listDatasetsWithNodeVoxelsRightChild, ref  listOfListsDatasetVoxelOffsetsRightChild);
			psDT->nodes_[nRightChildIndex]->pListDatasetsWithNodeVoxels_= pListDatasetsWithNodeVoxelsRightChild;
            psDT->nodes_[nRightChildIndex]->pListOfListsDatasetVoxelOffsets_= pListOfListsDatasetVoxelOffsetsRightChild;
        }

}


/// <summary>
/// Adding posteriors from current tree to global cumulative posterior float array
/// </summary>
void DecisionForestTester::accumulatePosterior(ptrsmartDecisionTree psDT, ptrsmartIndexVolume psIndexVolume, Posterior* posterior)
{

	float* nodePosteriorNormalized = new float[psDT->numClasses_];

    int* PI = psIndexVolume->nodeIndexArray_;
        
    int* pI = PI;

    for (int z = 0; z < volDims_.Depth; z++)
    {
        for (int y = 0; y < volDims_.Height; y++)
        {
            for (int x = 0; x < volDims_.Width; x++)
            {
                int nodeIndex = *pI; // node index for current point. i.t. the node where the point terminates
						
                computeNodePosterior(*(psDT->getNode(nodeIndex)), nodePosteriorNormalized,  psDT->numClasses_);  // re-compute normalized posterior array, for every pixel mapped to this node

                for (int classIndex = 0; classIndex < psDT->numClasses_; classIndex++)
                {


					posterior->postArray_[((classIndex * volDims_.Depth + z) * volDims_.Height + y) * volDims_.Width + x] += (ushort)(quantizPrecision * nodePosteriorNormalized[classIndex]+ 0.5); // Add 0.5 and floor to simulate round() function   a la http://faculty.salisbury.edu/~dxdefino/RoundOff.htm 


                }


                pI++;
            }
        }
    }
        
	delete [] nodePosteriorNormalized;
}



/// <summary>
/// Normalizes the histogram stored at each tree node and returns it out
///  THIS ONLY NORMALIZES, NO CLASS WEIGHTING
/// Assumes nodePosteriorNormalized is an array with numClasses elements
/// </summary>
void DecisionForestTester::computeNodePosterior(TreeNode& node, float* nodePosteriorNormalized, int numClasses)     
{

    float sum = 0;
    for (int c = 0; c < numClasses; c++)
    {
        float val = (float)node.histogram_[c];
        nodePosteriorNormalized[c] = val;
        sum += val;
    }

    float invSum = 1.0f / sum;
    for (int c = 0; c < numClasses; c++)
        nodePosteriorNormalized[c] *= invSum;

}


/// <summary>
/// Computing the mean of posteriors by dividing cumulative array by number of trees in forest
/// </summary>
void DecisionForestTester::computeMeanPosteriorOverForest(Posterior* posterior, int numTrees)
{
    cout << "| Computing mean posterior.\n";
    float invNumTrees = 1.0f / numTrees;
    float invQuantizprecis = 1.0f / quantizPrecision;
    ushort* PP = posterior->postArray_;
        
    ushort* pP = PP;
		
	for (int i = 0; i < posterior->nSizePostArray_; i++)
    {
        *pP = (ushort)(numeric_limits<ushort>::max() * invNumTrees * invQuantizprecis * (*pP)); // posterior value \in [0, ushort.MaxValue]
        pP++;
    }
        
}

/// <summary>
/// Maximum Aposteriori assignment
/// Caller owns the returned array
/// </summary>
byte* DecisionForestTester::mapAssignments(Posterior* posterior)
{
    cout << "| Computing MAP labels...";
    byte* outClassLabels = new byte[numTestingPoints_];

    
        
    byte* p = outClassLabels;
    for (int z = 0; z < volDims_.Depth; z++)
    {
        for (int y = 0; y < volDims_.Height; y++)
        {
            for (int x = 0; x < volDims_.Width; x++)
            {
                // finding class index with maximum posterior probability
                float maxPosterior = 0;
                byte mapClass = 0;
                for (byte classPos = 0; classPos < classIndeces_.size(); classPos++)
                {
                    int lpos4D = ((classPos * volDims_.Depth + z) * volDims_.Height + y) * volDims_.Width + x;    // linear position into array representing 4D matrix
                    ushort post = posterior->postArray_[lpos4D];
                    if (post > maxPosterior)
                    {
                        maxPosterior = post;
                        mapClass = classIndeces_[classPos]; // getting the correct index into the array of organ classes
                    }
                }

                // assigning class index to voxel
                *p = mapClass;                          // using pointers for efficiency: (int lpos3D = (z * volDims_.Height + y) * volDims_.Width + x;)
                p++;
            }
        }
    }
    cout << "Done.\n";        
    return outClassLabels;
}



	// *****************************************************
    // threading
	// *****************************************************

/// <summary>
/// ThreadCore ... for all test threads .... Each thread applies multiple trees to the dataset 
/// </summary>
void DecisionForestTester::threadCore(int objThreadID)
{


    int nThreadID = objThreadID;

    // debug threads		
    // cout << "|   Thread  " << nThreadID << "  started.\n";

    if (testIndeces_[nThreadID].size()  == 0)
    {
        return;
    }


    for (int i = 0; i < testIndeces_[nThreadID].size(); i++)  // For each tree to test with this thread.    TAG_Candidate_for_OpenMP_parallelization, but have to deal with merging posteriors from each tree.. See the other candidate in testNode_and_SendDataToChildren_NodeList()
    {
        int treeIndex = testIndeces_[nThreadID][i];
        ptrsmartDecisionTree psDecisionTree = decisionForest_[treeIndex];         // The current, fully trained, decision tree

        // Before labeling with the trees we make sure bLeaf is true for all leaves and false otherwise
        // This need only be done once, for each tree, no matter how many datasets we have.
        // TAG_LevelAtATime_BASED_PROCESSING    and      TAG_NodeAtATime_NodeList_BASED_PROCESSING
        for (int lev = 0; lev < maxTestTreeDepth_; lev++)
        {
            // Testing all nodes at current level
            int numNodesAtLevel = (int)std::pow(2.0F, lev);
            int startNodeIndex = numNodesAtLevel - 1;
            for (int ni = 0; ni < numNodesAtLevel; ni++)
                if (psDecisionTree->nodes_[startNodeIndex + ni] != nullptr)
                {
					if (psDecisionTree->nodeIsLeaf(startNodeIndex + ni)) psDecisionTree->nodes_[startNodeIndex + ni]->bLeaf_ = true;
                    else psDecisionTree->nodes_[startNodeIndex + ni]->bLeaf_ = false;

						
		           psDecisionTree->nodes_[startNodeIndex + ni]->DeleteNodeLists();  // Release memory in voxel lists associated with the node which is becoming a leaf

                }
        }

		ptrsmartIndexVolume psIndexVolume(new IndexVolume(numTestingPoints_));

        ptrsmartDecisionTree psAuxDecisionTree;
        if (!auxDecisionForest_.empty()) psAuxDecisionTree=auxDecisionForest_[treeIndex];
	    ptrsmartFeatureComputer psFeatureComputer(new FeatureComputer(psAuxDecisionTree, treeIndex));  // Feature response computer, including setup for TAG_Autocontext

        psFeatureComputer->setDecisionTree(psDecisionTree);

		psFeatureComputer->setDataset(psDataset_,psIndexVolume); // Pass the predictors (X)of the current dataset and its nodeIndexArray mapping voxels in that datataset to tree nodes

        // Loop over all tree test levels
        cout << "| Testing tree " <<  (treeIndex+1) <<  "/" << numTestTrees_ << "\n";                // Console output

		// *****************************************************
        // Set up initial root node's list of pixels to be all pixels from the one dataset_
		// *****************************************************
        // TAG_NodeAtATime_NodeList_BASED_PROCESSING
        int nDepth;
        int nHeight;
        int nWidth;
        int nHeightWidth;
        int nNumDatasetsAtATimePerTreeDuringTesting = 1;

        vector<int>* pListDatasetsWithNodeVoxels = new vector<int>(nNumDatasetsAtATimePerTreeDuringTesting);  // Set size 
        for (int iDataset = 0; iDataset < nNumDatasetsAtATimePerTreeDuringTesting; iDataset++) (*pListDatasetsWithNodeVoxels)[iDataset]=iDataset;
        vector<vector<int>*>*  pListOfListsDatasetVoxelOffsets = new vector<vector<int>*>(nNumDatasetsAtATimePerTreeDuringTesting);

        nDepth = psDataset_->dims_.Depth;
        nHeight = psDataset_->dims_.Height;
        nWidth = psDataset_->dims_.Width;
        nHeightWidth = nHeight * nWidth;

        int nDatasetVoxelCount = nDepth * nHeight * nWidth;
        vector<int>* pListDatasetVoxelOffsets = new vector<int>(nDatasetVoxelCount);
        for (int iOffset = 0; iOffset < nDatasetVoxelCount; iOffset++) (*pListDatasetVoxelOffsets)[iOffset]=iOffset;
        (*pListOfListsDatasetVoxelOffsets)[0]=pListDatasetVoxelOffsets;


        int nRootNodeID = 0;
		psDecisionTree->nodes_[nRootNodeID]->pListDatasetsWithNodeVoxels_ = pListDatasetsWithNodeVoxels;
		psDecisionTree->nodes_[nRootNodeID]->pListOfListsDatasetVoxelOffsets_= pListOfListsDatasetVoxelOffsets;
                

        // TAG_LastLevelIsPreventedFromSplittingSoDoesntCount  AAM: There is a bug here: If we sendDataToChildren when lev=( TreeDepth_-1 )  then that level is nothing but leaves and therefore does nothing to change the accuracy.
        //          The solution is to either 1) ignore the last output of the class matrix in the loop over depth variation
        //                                        or  2) rewrite this loop to stop at (maxTestTreeDepth_-1)  . This would be more correct since a tree with N levels has really only N-1 levels of splitting. The last level wasnt allowed to split
        for (int lev = 0; lev < maxTestTreeDepth_; lev++)
        {


            // Testing all nodes at current level
			int numNodesAtLevel = (int)std::pow(2.0f, lev), startNodeIndex = numNodesAtLevel - 1;
            for (int ni = 0; ni < numNodesAtLevel; ni++)
				if ((psDecisionTree->nodes_[startNodeIndex + ni] != nullptr) && (!psDecisionTree->nodes_[startNodeIndex + ni]->bLeaf_))   // if internal node
                        testNode_and_SendDataToChildren_NodeList(psDecisionTree, startNodeIndex + ni, psIndexVolume, psFeatureComputer, nHeightWidth, nWidth);  // Test internal node
        }
        accumulatePosterior(psDecisionTree, psIndexVolume, posterior_);  // accumulating posterior

        cout << "|  ThreadID " << nThreadID << "  completed testing of tree " << treeIndex << "\n";

		double dProgress=0;
		if (numTestTrees_>0) dProgress=(treeIndex+1)/numTestTrees_;
		caster->UpdateProgress(dProgress);
    }


}

        

