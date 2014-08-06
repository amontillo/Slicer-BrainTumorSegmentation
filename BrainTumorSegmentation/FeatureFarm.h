#pragma once
#include "Dataset.h"
#include "DecisionTree.h"
#include "IndexVolume.h"
#include <vector>
#include "basicDefs.h"
#include "Random.h"
#include "FeatureType.h"
#include <boost/shared_ptr.hpp>
#include <blitz/array.h>

using namespace std;
using boost::shared_ptr;


	

    /// <summary>
    /// This class computes feature responses, deals with different types of features, initializes threshold ranges etc.
    /// </summary>
    struct FeatureComputer
    {
		// Borrowed pointers (FeatureComputer doesnt own this memory, kept as native rather than smartptr so no chance of performance penalty)
        ushort* intensVolume_;                                 // CT volume
        ushort** AdditionalIntensVolumes_;
        double* dintensVolume_;                                 // double version of intensity data
        double** dAdditionalIntensVolumes_;
		blitz::Array<ushort,4>* pAvolume;
		blitz::Array<long long,4>* pAIvolume;
		blitz::Array<int,5>* pAHvolume;
		vector<double> AxisSym;
		vector<double> scales_;

        VolumeDimensions volDims_;            // volume dimensions
        vector<double> invScales_;                                    // inverse of pixel scales
        RescaleParams rescaleParams_;                           // rescale parameters for photometric calibration

        ptrsmartDecisionTree psDecisionTree_;
        ptrsmartIndexVolume psIndexVolume_;

        ptrsmartDataset psDataset_;
        ptrsmartDecisionTree psAuxDecisionTree_;
        int treeIndex_;



		FeatureComputer(ptrsmartDecisionTree psDecisionTreeForAutocontext, int treeIndex);

		void setDataset(ptrsmartDataset psDataset, ptrsmartIndexVolume psIndexVolume);
		// These were merged with above

		void setDecisionTree(ptrsmartDecisionTree psDecisionTree);
		double featureResponse(int x0, int y0, int z0, float disp_mm[], FEATURE_TYPE featureType, uint uintFeatureParameter);
		double featureResponse(int x0, int y0, int z0, float disp_mm[], FEATURE_TYPE featureType, uint uintFeatureParameter, int dim, unsigned int boxsize[]);
		double featureResponseH(int x0, int y0, int z0, float disp_mm[], blitz::Array<double,2> &Histref,blitz::Array<double,2> &histProbe,  FEATURE_TYPE featureType, int dim, unsigned int boxsize[],int threshIndex);
		double featureResponseHtest(int x0, int y0, int z0, float disp_mm[], blitz::Array<double,1> &Histref,blitz::Array<double,1> &histProbe,  FEATURE_TYPE featureType, int dim, unsigned int boxsize[]);
		double featureResponseTrain(int x0, int y0, int z0, float disp_mm[], FEATURE_TYPE featureType, uint uintFeatureParameter, int dim, unsigned int boxsize[], blitz::Array<double,2> &histProbe, int featindex);

		blitz::Array<double,1> pointHist(int x0, int y0, int z0, int dim, unsigned int boxsize[]);

	private:
		double featureResponse_AbsoluteIntensity(int x0, int y0, int z0, float disp_mm[]);
		double featureResponse_AbsoluteIntensity(int x0, int y0, int z0, float disp_mm[], int dim);
		double featureResponse_RelativeIntensity(int x0, int y0, int z0, float disp_mm[]);
		double featureResponse_RelativeIntensity(int x0, int y0, int z0, float disp_mm[], int dim, unsigned int boxsize[]);
		double featureResponse_RatioIntensity(int x0, int y0, int z0, float disp_mm[]);
		double featureResponse_DiffOf2Probes(int x0, int y0, int z0, float disp_mm[]);
		double featureResponse_DiffOf2Probes(int x0, int y0, int z0, float disp_mm[], int dim, unsigned int boxsize[]);
		double featureResponse_RelativePosterior(int x0, int y0, int z0, float disp_mm[]);
		bool positionOfContextVoxel(int x0, int y0, int z0, float disp_mm[], int& x1, int& y1, int& z1);
		bool positionOfContextVoxel2(int x0, int y0, int z0, float* disp_mm, int& x1, int& y1, int& z1);
		bool positionOfContextVoxelR(int x0, int y0, int z0, const int& x1, const int& y1, const int& z1);
		void computeNodePosterior(TreeNode& node, float posterior[], int numClasses);
		double posteriorDotProduct(float posterior0[], float posterior1[], int numClasses);
        double featureResponse_AbsSym(int x0, int y0, int z0, int dim, unsigned int boxsize[]);

    };

	
    typedef boost::shared_ptr<FeatureComputer> ptrsmartFeatureComputer;

