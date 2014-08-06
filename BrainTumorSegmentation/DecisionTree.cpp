#include "basicDefs.h"
#include "DecisionTree.h"
#include <boost/assert.hpp>
#include <sstream>
#include <blitz/array.h>

using namespace std;


// Define statics
const float DecisionTree::log10_2_=std::log(2.0F);

/// <summary>
/// Constructor
/// </summary>
TreeNode::TreeNode(int numClasses, uint nNodeID)
{
    ACTIVE_ = false;   // field is obsolete 

    nNodeID_ = nNodeID;

    // Input to this node (when this node's parent is trained, this information is determined and sent to this node.)
    // =====================================================================
    numTrainPointsInNode_ = 0;
    histogram_ = new int[numClasses];
    entropy_ = 0;
	Modality_ = 0;
	Boxsize_ [0] = 1;
	Boxsize_ [1] = 1;
	Boxsize_ [2] = 1;

	// No voxels assigned from the datasets yet :
    pListDatasetsWithNodeVoxels_=nullptr;
    pListOfListsDatasetVoxelOffsets_=nullptr;

    // The splitting criteria chosen: (determines output of this node)
    // ========================
    featureType_ = FEATURE_TYPE_AbsoluteIntensity;
    dispFeature_ = new float[DISPLACEMENT_DIMS]; 
    threshold_ = 0;

    nFrontNode_ = -1; 
    bLeaf_ = false; // Assume non-leaf internal node, must be set after tree is loaded, built etc

}

TreeNode::TreeNode(const TreeNode& T)
{
    ACTIVE_ = false;   // field is obsolete 

    nNodeID_ = T.nNodeID_;

    // Input to this node (when this node's parent is trained, this information is determined and sent to this node.)
    // =====================================================================
    numTrainPointsInNode_ = T.numTrainPointsInNode_;
	int nSize=sizeof(T.histogram_)/sizeof(int);
    histogram_ = new int[nSize];
    entropy_ = T.entropy_;

	// No voxels assigned from the datasets yet :
    pListDatasetsWithNodeVoxels_=nullptr;
    pListOfListsDatasetVoxelOffsets_=nullptr;

    // The splitting criteria chosen: (determines output of this node)
    // ========================
    featureType_ = T.featureType_;
    // dispFeature_ = new float[3];
    dispFeature_ = new float[DISPLACEMENT_DIMS]; 
    threshold_ = T.threshold_;
	Modality_ = T.Modality_;
	Boxsize_ [0] = T.Boxsize_[0];
	Boxsize_ [1] = T.Boxsize_[1];
	Boxsize_ [2] = T.Boxsize_[2];
    nFrontNode_ = T.nFrontNode_; 
    bLeaf_ = T.bLeaf_; // Assume non-leaf internal node, must be set after tree is loaded, built etc    


}


void TreeNode::DeleteNodeLists()
{
	if (pListDatasetsWithNodeVoxels_!=nullptr) 
		{  pListDatasetsWithNodeVoxels_->clear();  // doesnt zero capacity 
	       delete pListDatasetsWithNodeVoxels_;    // zero's capacity and releases all memory
		   pListDatasetsWithNodeVoxels_=nullptr; 
	    }

	if (pListOfListsDatasetVoxelOffsets_!=nullptr)
		{
              if (! pListOfListsDatasetVoxelOffsets_->empty()) 
				for (uint i = 0; i < pListOfListsDatasetVoxelOffsets_->size(); i++) 
				{   if (   (*pListOfListsDatasetVoxelOffsets_)[i] != nullptr  )   // TAG_PossibleMemLeakFix
					{
						(*pListOfListsDatasetVoxelOffsets_)[i]->clear();
						delete ((*pListOfListsDatasetVoxelOffsets_)[i]);   
						(*pListOfListsDatasetVoxelOffsets_)[i]=nullptr;
					}
				}

			  pListOfListsDatasetVoxelOffsets_->clear(); //  
			  delete pListOfListsDatasetVoxelOffsets_; 
			  pListOfListsDatasetVoxelOffsets_=nullptr; 
		}

}

TreeNode::~TreeNode()
{
	// De-allocate the TreeNode
	if (histogram_!=nullptr)    { delete [] histogram_; histogram_=nullptr; }
	if (dispFeature_!=nullptr)  { delete [] dispFeature_; dispFeature_=nullptr; }

	DeleteNodeLists();

}

    
DecisionTree::DecisionTree()   // dumb tree initialization
{
    treeDepth_ = -1;    // This is a flag to indicate that the tree is not really initialized
    nodes_=nullptr;
}

/// <summary>
/// Constructing a tree of chosen max depth. 
/// </summary>
DecisionTree::DecisionTree(int treeDepth, int numClasses)
{
    treeDepth_ = treeDepth;                                                     // tree depth
    numNodes_ = (int)std::pow(2.0F,treeDepth_) - 1;                               // num nodes in balanced tree is 2^maxDepth-1
    numClasses_ = numClasses;                                                   // number of organ classes
    nodes_ = new TreeNode*[numNodes_];                                           // allocating table representation of tree
	for (int i = 0; i < numNodes_; i++) 
	{   // debug
		
		nodes_[i]=nullptr;                      // nothing allocated yet.
	}


	double dMaxElapsedTime=0;
	{
		boost::timer dummyTimer; 
		dMaxElapsedTime=dummyTimer.elapsed_max();    // Windows yields about 24 days (2,147,447 seconds)

	}

    dTrainingTimeInSeconds_ = dMaxElapsedTime; // dont want to bias timing tests so init to max 
    dTestingTimeInSeconds_ = dMaxElapsedTime;
}

DecisionTree::~DecisionTree()
{
    if (nodes_!=nullptr) 
	{   // delete each node individually
		for (int ii=0; ii<numNodes_; ii++)  
		{   if (nodes_[ii]!=nullptr) 
		     { 
                        delete nodes_[ii];
		        nodes_[ii]=nullptr;
		     }
		}

		// free the whole array of node pointers itself
		delete [] nodes_;  
		nodes_=nullptr;




	}
}

TreeNode* DecisionTree::getNode(int nodeIndex) 
{ return nodes_[nodeIndex]; 
}





void DecisionTree::getMinAndMaxEntropy(double& minEntropy, double& maxEntropy)
{
    minEntropy = numeric_limits<double>::max();
    maxEntropy = 0;
    bool bActive = false;

    for (int i = 0; i < numNodes_; i++)
    {
        bActive=(nodes_[i]!=nullptr);
        if (!bActive) continue;
        double entropy = nodes_[i]->entropy_;
        if (entropy < minEntropy) minEntropy = entropy;
        if (entropy > maxEntropy) maxEntropy = entropy;
    }
}




// First arg infileTree is a iostream  from which both fstream and stringstream inherit, thus making readTreeInternal work polymorphically regardless of whether we are reading the forest from 
// a file on disk or from an internally coded default decoded Base64 string 
bool DecisionTree::readTreeInternal(iostream& infileTree, bool bReturn, bool bDebug, string& strTreeName)
{
			double ver;
			infileTree.read(reinterpret_cast<char *>(&ver), sizeof(float64)); // READ version number
			if (ver > 1.4) { cout << "| ERROR: expecting version <= 1.4  DTF file for decision tree\n"; bReturn=false; return bReturn; }

			infileTree.read(reinterpret_cast<char *>(&treeDepth_), sizeof(int32));   // READ tree depth
			infileTree.read(reinterpret_cast<char *>(&numClasses_), sizeof(int32));  // READ num classes


			if (ver > 1.1)  // read in the training and testing time
			{
				infileTree.read(reinterpret_cast<char *>(&dTrainingTimeInSeconds_), sizeof(float64)); // READ training time
				infileTree.read(reinterpret_cast<char *>(&dTestingTimeInSeconds_), sizeof(float64)); // READ testing time 

			}
			else
			{
				double dMaxElapsedTime=0;
				{
					boost::timer dummyTimer; 
					dMaxElapsedTime=dummyTimer.elapsed_max();    // Windows yields about 24 days (2,147,447 seconds)
					// this block is to ensure dummyTimer goes out of scope 
				}

				dTrainingTimeInSeconds_ = dMaxElapsedTime; // dont want to bias timing tests so init to max 
				dTestingTimeInSeconds_ = dMaxElapsedTime;
			}


			numNodes_ = (int)std::pow(2.0F, treeDepth_) - 1;  // total number of nodes
			nodes_ = new TreeNode*[numNodes_];              // allocating table representation of tree


			if (bDebug)  cout << " max number of nodes is " << numNodes_ << "\n";


			// Initialize TreeNode pointers to null (inactive)
			//    Here w do not rely on any compiler initial value (platform dependent)
			for (uint ii=0; ii<(unsigned int)numNodes_; ii++) nodes_[ii] = nullptr;  

			int nNumActiveNodes = 0;
			uint nNewNodeID=0;


			unsigned int ni=0;
			unsigned int uintNumNodes=static_cast<unsigned int>(numNodes_);

			bool bActive;                   
			if (ver < 1.3) infileTree.read(reinterpret_cast<char *>(&bActive), sizeof(bool8));   // READ node ACTIVE
			else 
			{  
				infileTree.read(reinterpret_cast<char *>(&nNewNodeID), sizeof(uint32));   // READ node ACTIVE
				if (nNewNodeID>=numNodes_) 
					{  cout << "| ERROR while reading decision tree file " << strTreeName << " node ID encountered is larger than max num nodes\n"; 
					    bReturn=false; 
						return bReturn; 
					}
				else
				{  
                    if (bDebug)   cout << " read the " << nNumActiveNodes+1 << " active node with node id [" << nNewNodeID << "]\n";

					ni=nNewNodeID; 
					bActive=true;
				}
			}

			// for (uint ni = 0; ni < static_cast<unsigned int>(numNodes_); ni++)
			while ((ni<uintNumNodes) && (infileTree.good())  && (!infileTree.eof()))  // havent reached the last node and input stream is still valid, and havent reached end of file
			{

				if (ni==27167)
				{
					cout << "before null node \n";
				}

				if (bActive) // if node is inactive then we do not read anything else
				{
					nNumActiveNodes++;

					if (bDebug)   cout << " ni is " << ni << "\n";

					nodes_[ni] = new TreeNode(numClasses_, (uint) ni);
					nodes_[ni]->ACTIVE_ = bActive;

					// DTF Version 1.0
					// ======================================================
					infileTree.read(reinterpret_cast<char *>(&(nodes_[ni]->featureType_)), sizeof(int32));   // READ feature type

					
					infileTree.read(reinterpret_cast<char *>(&(nodes_[ni]->dispFeature_[0])), sizeof(float32));   // READ displacement X (probe1)
					infileTree.read(reinterpret_cast<char *>(&(nodes_[ni]->dispFeature_[1])), sizeof(float32));   // READ displacement Y
					infileTree.read(reinterpret_cast<char *>(&(nodes_[ni]->dispFeature_[2])), sizeof(float32));   // READ displacement Z

					infileTree.read(reinterpret_cast<char *>(&(nodes_[ni]->dispFeature_[3])), sizeof(float32));   // READ displacement X (probe2)
					infileTree.read(reinterpret_cast<char *>(&(nodes_[ni]->dispFeature_[4])), sizeof(float32));   // READ displacement Y
					infileTree.read(reinterpret_cast<char *>(&(nodes_[ni]->dispFeature_[5])), sizeof(float32));   // READ displacement Z

					infileTree.read(reinterpret_cast<char *>(&(nodes_[ni]->threshold_)), sizeof(float64));   // READ threshold value
					infileTree.read(reinterpret_cast<char *>(&(nodes_[ni]->numTrainPointsInNode_)), sizeof(int32));   // READ num training points value

					//1.4
					if(ver > 1.3){
						infileTree.read(reinterpret_cast<char *>(&(nodes_[ni]->Modality_)), sizeof(int));   // READ Modality type
						infileTree.read(reinterpret_cast<char *>(&(nodes_[ni]->Boxsize_[0])), sizeof(unsigned int));   // READ box width
						infileTree.read(reinterpret_cast<char *>(&(nodes_[ni]->Boxsize_[1])), sizeof(unsigned int));   // READ box height
						infileTree.read(reinterpret_cast<char *>(&(nodes_[ni]->Boxsize_[2])), sizeof(unsigned int));   // READ box depth
						
						int Hlength;
						infileTree.read(reinterpret_cast<char *>(&(Hlength)), sizeof(int)); // get size of histogram
						nodes_[ni]->refHist.resize(Hlength);
						for(int Histindex = 0; Histindex < Hlength; Histindex++){// READ histogram
							infileTree.read(reinterpret_cast<char *>(&(nodes_[ni]->refHist(Histindex))), sizeof(double));
						}
					}

					for (int classIndex = 0; classIndex < numClasses_; classIndex++)
						infileTree.read(reinterpret_cast<char *>(&(nodes_[ni]->histogram_[classIndex])), sizeof(int32));   // READ histogram value

					infileTree.read(reinterpret_cast<char *>(&(nodes_[ni]->entropy_)), sizeof(float64));   // READ entropy

					// DTF Version 1.1 
					// ======================================================
					if (ver > 1.0)
					{
						infileTree.read(reinterpret_cast<char *>(&(nodes_[ni]->nNodeID_)), sizeof(int32));   // DTF Version 1.1                     READ nodeID for debugging merging etc
						infileTree.read(reinterpret_cast<char *>(&(nodes_[ni]->uintFeatureParameter_)), sizeof(int32));   // DTF Version 1.1    READ entanglement feature parameters 
					}
					else
					{
						nodes_[ni]->nNodeID_ = ni;  
						nodes_[ni]->uintFeatureParameter_ = (uint)0;
					}

				}

				if (ver < 1.3) infileTree.read(reinterpret_cast<char *>(&bActive), sizeof(bool8));   // READ node ACTIVE
				else 
				{  
					infileTree.read(reinterpret_cast<char *>(&nNewNodeID), sizeof(uint32));   // READ node ACTIVE
					if ( !infileTree.eof() )
					{
						if (nNewNodeID>=numNodes_) 
						{  cout << "| ERROR while reading decision tree file " << strTreeName << " node ID encountered is larger than max num nodes\n"; 
							bReturn=false; 
							return bReturn; 
						}
						else
						{  
							if (bDebug)   cout << " read the " << nNumActiveNodes+1 << " active node with node id [" << nNewNodeID << "]\n";

							ni=nNewNodeID; 
							bActive=true;
						}
					}
				}

				// DEBUG
				// if (infileTree.eof())  cout << "EOF reached\n";
			}

			cout << "    Num active nodes: "<< nNumActiveNodes <<" of "<< numNodes_ <<"  or " << 100 * ((float)nNumActiveNodes / (float) numNodes_) << " percent\n";

			if (      (!infileTree.good())  && (!infileTree.eof())) throw; 
			cout << "Done.\n\n";
			
			bReturn=true;
			return bReturn; 
}







/// <summary>
/// returns true if successfully loaded the decision tree from the specified file 
///  For speed and compactness, just reads elements in same order as written
/// </summary>
bool DecisionTree::readFromBinaryStream(unsigned char *strTree, unsigned int nTreeLengthInBytes)
{
	bool bReturn=true;
	bool bDebug=false;

	string strDecisionTreeName="internal default character tree stream";

    // Reading the decision tree xml data from stringstream in memory 
    try
    {
		stringstream infileTree(ios_base::in | ios_base::out | ios::binary); // If we set ios::binary flag the other flags default to 0 thus we must set ios_base::in and ios_base::out or the stream will do nothing (no IO)
        infileTree.write((const char *)strTree, nTreeLengthInBytes); // We use write since it block copies the data as is, it does not convert (change) any values
		// as does this next line which does NOT work since it calls a copy ctor which converts the characters from 0-255 to -127..128
		// stringstream infileTree = stringstream((const char *)strTree);
		// Also, do not use wstringstream since def of wchar is platform dependent.
        
		bReturn=readTreeInternal(infileTree,bReturn,bDebug, strDecisionTreeName);
    }
    catch (...) 
	{  cout << "Error: problem with reading from " << strDecisionTreeName << "\n";
		bReturn=false; 
    };

	return bReturn;
}

/// <summary>
/// returns true if successfully loaded the decision tree from the specified file 
///  For speed and compactness, just reads elements in same order as written
/// </summary>
bool DecisionTree::readFromBinaryDTFFile(string& strDecisionTreeFilename)
{
	bool bReturn=true;

	bool bDebug=false;
    // Reading the decision tree xml file
    try
    {
        fstream infileTree(strDecisionTreeFilename.c_str(), ios::binary | ios::in); // open file
		if (infileTree.is_open())
		{
			cout << ">> Reading tree from " << strDecisionTreeFilename << "\n";

            bReturn=readTreeInternal(infileTree,bReturn,bDebug, strDecisionTreeFilename);

			infileTree.close();  // Close file
		}
		else
		{
			cout << "Error: could not open decision tree file: " << strDecisionTreeFilename << "\n";
			bReturn=false;
		}


    }
    catch (...) 
	{  cout << "Error: problem with reading the decision tree dtf file: " << strDecisionTreeFilename << "\n";
		bReturn=false; 
    };

	return bReturn;
}

