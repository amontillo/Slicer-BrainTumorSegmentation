#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>  // for log2
#include <limits>  // for numeric_limits, may have to compile with: /EHsc   see http://msdn.microsoft.com/en-us/library/s086ab1z%28v=vs.71%29.aspx
#include "FeatureType.h"
#include "ForestParameters.h"
#include "FilenameManager.h"
#include "basicDefs.h"
#include <boost/shared_ptr.hpp>
#include <boost/timer.hpp>
#include <blitz/array.h>
using boost::shared_ptr;



// 6 bc we alloc as if 2 probes per node were chosen for every features (quick and dirty for now)
#define DISPLACEMENT_DIMS 6

/// <summary>
/// Tree node structure
/// </summary>
struct TreeNode
{
    /// <summary>
    /// Fields
    /// </summary>
    bool ACTIVE_;                   // true if node has been trained. false if node has been skipped. A node with two inactive children is a leaf node.
    FEATURE_TYPE featureType_;      // feature type which has been assigned to the node
    float* dispFeature_;           // displacement between reference pixel and context pixel (in mm)
    double threshold_;              // learned threshold for node test
    int Modality_;				  // learned modality for node test
	unsigned int Boxsize_ [3];				  // learned box size for a node test
	blitz::Array<double,1> refHist;

    int numTrainPointsInNode_;      // percentage of training points reaching that node
    int* histogram_;               // unnormalized histogram of classes is stored here
                                                            
    double entropy_;                // entropy of distribution stored at node
    uint nNodeID_;  // AAM: for debugging merging

    uint uintFeatureParameter_; // AAM: entanglement feature parameters 

    vector<int>* pListDatasetsWithNodeVoxels_; // the list of datasets that contain at least one voxel at the node
    vector<vector<int>* >* pListOfListsDatasetVoxelOffsets_; // the list of voxels (by offset) from each dataset that are at the node
    int nFrontNode_;
    bool bLeaf_;  // true if leaf, false ow.

    /// <summary>
    /// Constructor
    /// </summary>
    TreeNode(int numClasses, uint nNodeID);


	TreeNode(const TreeNode& T);  // Copy ctor ... HACK . this should be disallowed
	void DeleteNodeLists();
		
	~TreeNode();
	


};





struct DecisionTree
{
	public: 
		
        int numNodes_;              // number of all nodes in tree
        int treeDepth_;             // max tree depth
        int numClasses_;            // number of label classes
		TreeNode** nodes_;          // Tabular representation of tree (a linear array of tree nodes)
		
        double dTrainingTimeInSeconds_;
        double dTestingTimeInSeconds_;

		static const float log10_2_;  /// precompute log_10(2) for speed when computing log_base2
		

        /// <summary>
        /// Default initialization. Uses treeDepth=-1 to indicate a fake initialization
        /// </summary>
        DecisionTree();   // dumb tree initialization

        /// <summary>
        /// Constructing a tree of chosen max depth. 
        /// </summary>
        DecisionTree(int treeDepth, int numClasses);

		~DecisionTree();

        TreeNode* getNode(int nodeIndex);

		// static version when no tree is available 
		static int getTreeLevelFromNodeIndexStatic(int nodeIndex) { return (int)std::floor(std::log((float)(nodeIndex + 1))/DecisionTree::log10_2_ ); }; 

        int getLeftChildIndex(int parentNodeIndex) { return (2 * parentNodeIndex + 1); }
        int getRighChildIndex(int parentNodeIndex) { return (2 * parentNodeIndex + 2); }
        int getTreeLevelFromNodeIndex(int nodeIndex) { return (int)std::floor(std::log((float)(nodeIndex + 1))/DecisionTree::log10_2_ ); }   // log_base2
        int getParentFromNodeIndex(int childNodeIndex) { return (int)std::floor((double)((childNodeIndex - 1) / 2)); }
        bool nodeIsAtBottomLevel(int nodeIndex, int treeDepth) { return ((1 + getTreeLevelFromNodeIndex(nodeIndex)) == treeDepth); }
        bool nodeIsLeaf(int nodeIndex)
        {
            if (nodes_[nodeIndex] == nullptr)
            {
                return true;
            }
            else  // must be active, so we can check the following conditions:
            {
                int nLeftNode = getLeftChildIndex(nodeIndex);
                int nRightNode = getRighChildIndex(nodeIndex);
                return (nodeIsAtBottomLevel(nodeIndex, treeDepth_) || ((nodes_[nLeftNode] == nullptr) && (nodes_[nRightNode] == nullptr)));

            }
        }

     public:


        void getMinAndMaxEntropy(double& minEntropy, double& maxEntropy);

        /// <summary>
        /// returns true if successfully loaded the decision tree from the specified character stream
        ///  For speed and compactness, just reads elements in same order as written
        /// </summary>
        bool readFromBinaryStream(unsigned char *strTree, unsigned int nTreeLengthInBytes);

        /// <summary>
        /// returns true if successfully loaded the decision tree from the specified file 
        ///  For speed and compactness, just reads elements in same order as written
        /// </summary>
        bool readFromBinaryDTFFile(string& strDecisionTreeFilename);

		// internal helper function for above two methods
		bool readTreeInternal(iostream& infileTree, bool bReturn, bool bDebug, string& strTreeName);
};

typedef boost::shared_ptr<DecisionTree> ptrsmartDecisionTree;


