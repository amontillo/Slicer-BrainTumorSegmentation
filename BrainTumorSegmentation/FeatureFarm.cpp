#include <vector>
#include <iomanip>
#include "FeatureFarm.h"
#include <boost/assert.hpp>
#include <sstream>
#include <algorithm>
#include <blitz/array.h>
#include <numeric>

using namespace std;
using boost::lexical_cast;


 

    
        /// <summary>
        /// Constructor
        /// </summary>
        FeatureComputer::FeatureComputer(ptrsmartDecisionTree psDecisionTreeForAutocontext, int treeIndex)
        {
            treeIndex_= treeIndex;
			psAuxDecisionTree_=psDecisionTreeForAutocontext;
        }



		        // psFeatComp_->setDataset(psDataset->intensVolume, psDataset->volDims, psDataset->invScales_, psDataset->rescaleParameters, indexVolumes_[datasetIndex]); // set dataset within feature farm
		//psFeatComp_->setDatasetWithChannels(psDataset->dintensVolume_, psDataset->dAdditionalIntensVolumes_, psDataset->dims_, psDataset->invScales_, psDataset->rescaleParams_, indexVolumes_[datasetIndex]); // set dataset within feature farm
  //      psFeatComp_->setDatasetForAuxUsage(psDataset);  // need full dataset instance to access datasets's aux channels (e.g. for autocontext)  TAG_Autocontext

        /// <summary>
        /// Set dataset
        /// </summary>
        void FeatureComputer::setDataset(ptrsmartDataset psDataset, ptrsmartIndexVolume psIndexVolume) 
        {
			psDataset_ = psDataset;  // need full dataset instance to access datasets's aux channels (e.g. for autocontext)  TAG_Autocontext

			// Note: the volumes not in use are nullptrs. e.g. dintensVolume_ and dAdditionalIntensVolumes_ are nullptrs when we are not processing columnar (1D) data 
            intensVolume_ =  psDataset->intensVolume_;   // ushort
			dintensVolume_ = psDataset->dintensVolume_; // double
			dAdditionalIntensVolumes_ = psDataset->dAdditionalIntensVolumes_; // double channels
			pAvolume = &(psDataset->Avolume);//multimodal array
			pAIvolume = &(psDataset->AIvolume);//multimodal integral image
			pAHvolume = &(psDataset->AHvolume);//multimodal intergral histogram
			AxisSym = psDataset->SymAxis_;//symmetrical axis
			scales_ = psDataset->scales_;

			volDims_ =  psDataset->dims_;
            invScales_ = psDataset->invScales_;
			rescaleParams_ =  psDataset->rescaleParams_;

            psIndexVolume_ = psIndexVolume;
        }


        /// <summary>
        /// Setting dataset
        /// </summary>
        /// <summary>
        /// Setting decision tree to allow computing probabilistic feature responses
        /// </summary>
        void FeatureComputer::setDecisionTree(ptrsmartDecisionTree psDecisionTree)
        {
            psDecisionTree_ = psDecisionTree;
        }

        /// <summary>
        /// Feature response
        /// </summary>
        double FeatureComputer::featureResponse(int x0, int y0, int z0, float disp_mm[], FEATURE_TYPE featureType, uint uintFeatureParameter)
        {
            double output = 0;
            switch (featureType)
            {
                case FEATURE_TYPE_AbsoluteIntensity: { output = featureResponse_AbsoluteIntensity(x0, y0, z0, disp_mm); break; }
                case FEATURE_TYPE_RelativeIntensity: { output = featureResponse_RelativeIntensity(x0, y0, z0, disp_mm); break; }
                case FEATURE_TYPE_RelativePosterior: { output = featureResponse_RelativePosterior(x0, y0, z0, disp_mm); break; }
                case FEATURE_TYPE_RatioIntensity: { output = featureResponse_RatioIntensity(x0, y0, z0, disp_mm); break; }
                case FEATURE_TYPE_DiffOf2Probes: { output = featureResponse_DiffOf2Probes(x0, y0, z0, disp_mm); break; }
            }
            return output;
        }

		double FeatureComputer::featureResponse(int x0, int y0, int z0, float disp_mm[], FEATURE_TYPE featureType, uint uintFeatureParameter, int dim, unsigned int boxsize[])
        {
            double output = 0;
            switch (featureType)
            {
                case FEATURE_TYPE_AbsoluteIntensity: { output = featureResponse_AbsoluteIntensity(x0, y0, z0, disp_mm,dim); break; }
                case FEATURE_TYPE_RelativeIntensity: { output = featureResponse_RelativeIntensity(x0, y0, z0, disp_mm,dim,boxsize); break; }
                case FEATURE_TYPE_RelativePosterior: { output = featureResponse_RelativePosterior(x0, y0, z0, disp_mm); break; }
                case FEATURE_TYPE_RatioIntensity: { output = featureResponse_RatioIntensity(x0, y0, z0, disp_mm); break; }
                case FEATURE_TYPE_DiffOf2Probes: { output = featureResponse_DiffOf2Probes(x0, y0, z0, disp_mm, dim, boxsize); break; }
				case FEATURE_TYPE_AbsSym: { output = featureResponse_AbsSym(x0, y0, z0,dim,boxsize); break; }
            }

            return output;
        }
		
		double FeatureComputer::featureResponseTrain(int x0, int y0, int z0, float disp_mm[], FEATURE_TYPE featureType, uint uintFeatureParameter, int dim, unsigned int boxsize[], blitz::Array<double,2> &histProbe, int featindex)
        {
			double output = 0;
            switch (featureType)
            {
                case FEATURE_TYPE_AbsoluteIntensity: { output = featureResponse_AbsoluteIntensity(x0, y0, z0, disp_mm,dim); break; }
                case FEATURE_TYPE_RelativeIntensity: { output = featureResponse_RelativeIntensity(x0, y0, z0, disp_mm,dim,boxsize); break; }
                case FEATURE_TYPE_RelativePosterior: { output = featureResponse_RelativePosterior(x0, y0, z0, disp_mm); break; }
                case FEATURE_TYPE_RatioIntensity: { output = featureResponse_RatioIntensity(x0, y0, z0, disp_mm); break; }
                case FEATURE_TYPE_DiffOf2Probes: { output = featureResponse_DiffOf2Probes(x0, y0, z0, disp_mm, dim, boxsize); break; }
				case FEATURE_TYPE_AbsSym: { output = featureResponse_AbsSym(x0, y0, z0,dim,boxsize); break; }
            }
            return output;
        }

		// *****************************************************
        // private methods - The actual feature computation
		// *****************************************************


        /// <summary>
        /// Feature response computation: Type: ChannelIntensityRef
        /// Extract channel intensity at the ref 
        /// 
        /// </summary>
        /// <param name="x0"></param>
        /// <param name="y0"></param>
        /// <param name="z0"></param>
        /// <param name="disp_mm"></param>
        /// <param name="uintFeatureParameter"></param>
        /// <returns></returns>

        /// <summary>
        /// Feature response computation: Type 0 -- Absolute Intensity of probe in HU
        /// </summary>
        double FeatureComputer::featureResponse_AbsoluteIntensity(int x0, int y0, int z0, float disp_mm[])
        {
            // compute position of context voxel
            int x1, y1, z1;
            bool INSIDE_VOLUME = positionOfContextVoxel(x0, y0, z0, disp_mm, x1, y1, z1);
            
            // compute feature response in HU
            int intensContextPoint = 0;             // air outside volume
            int W = volDims_.Width, H = volDims_.Height, D = volDims_.Depth;
            if (INSIDE_VOLUME) intensContextPoint = intensVolume_[(z1 * H + y1) * W + x1]; // proper value of context pixel, inside volume

            return rescaleParams_.rescaleSlope * intensContextPoint + rescaleParams_.rescaleIntercept; // return feature response (in HU units)
        }
		double FeatureComputer::featureResponse_AbsoluteIntensity(int x0, int y0, int z0, float disp_mm[], int dim)
        {
            // compute position of context voxel
            int x1, y1, z1;
            bool INSIDE_VOLUME = positionOfContextVoxel(x0, y0, z0, disp_mm, x1, y1, z1);
            
            // compute feature response in HU
            int intensContextPoint = 0;             // air outside volume
            if (INSIDE_VOLUME){
				intensContextPoint = (*pAvolume)(x1,y1,z1,dim);} // proper value of context pixel, inside volume

            return intensContextPoint; // return feature response (in HU units)
        }

        /// <summary>
        /// Feature response computation: Type 1 -- Relative Intensity ...  this is the difference of intensities: probe - ref in HU.  (poorly named? should be intensity difference)
        /// </summary>
        double FeatureComputer::featureResponse_RelativeIntensity(int x0, int y0, int z0, float disp_mm[])
        {
            // compute position of context voxel
            int x1, y1, z1;

			// edit by Anthony including a box
			int Bwidth(2), Bhieght(2), Bdepth(2);//size of the mean box in each dimension

			//check boundary
            bool INSIDE_VOLUME_Left = positionOfContextVoxel(x0, y0, z0, disp_mm, x1, y1, z1);
			bool INSIDE_VOLUME_Right = positionOfContextVoxelR(x0, y0, z0, x1+Bwidth, y1+Bhieght, z1+Bdepth);
            
            int intensContextPoint = 0;         // outside the volume
            int W = volDims_.Width+1, H = volDims_.Height+1, D = volDims_.Depth+1;

            // when the input is a volume
			/*if (INSIDE_VOLUME_Right && INSIDE_VOLUME_Left) { // get the sum of the neighborhood
				for(int i = 0; i < Bwidth; i++){
					for(int j = 0; j < Bhieght; j++){
						for(int k = 0; k < Bdepth; k++){
							intensContextPoint += intensVolume_[((z1+k) * H + (y1+j)) * W + (x1+i)]; // proper value of context pixel, inside volume
						}
					}
				}
				intensContextPoint /= Bwidth*Bhieght*Bdepth; // get the mean value
			}

            int intensRefPoint = intensVolume_[(z0 * H + y0) * W + x0];

            return (intensContextPoint - intensRefPoint); // return feature response (in HU units)
			*/
		
			// when the input is an integral image
			//calculate the mean around the realtive voxel, x1,y1,z1 is the top left hand corner of the box
			double nhood [8];
			if (INSIDE_VOLUME_Right && INSIDE_VOLUME_Left) {
				nhood[0] = dintensVolume_[((z1) * H + (y1)) * W + (x1)];//-
				nhood[1] = dintensVolume_[((z1+Bdepth) * H + (y1+Bhieght)) * W + (x1+Bwidth)] ;//+
				nhood[2] = dintensVolume_[((z1+Bdepth) * H + (y1)) * W + (x1)];//+
				nhood[3] = dintensVolume_[((z1) * H + (y1)) * W + (x1+Bwidth)];//+
				nhood[4] = dintensVolume_[((z1) * H + (y1+Bhieght)) * W + (x1)];//+
				nhood[5] = dintensVolume_[((z1+Bdepth) * H + (y1)) * W + (x1+Bwidth)];//-
				nhood[6] = dintensVolume_[((z1+Bdepth) * H + (y1+Bhieght)) * W + (x1)];//-
				nhood[7] = dintensVolume_[((z1) * H + (y1+Bhieght)) * W + (x1+Bwidth)];//-
				intensContextPoint = (int)((nhood[1]+nhood[2]+nhood[3]+nhood[4])-(nhood[0]+nhood[5]+nhood[6]+nhood[7]))/(Bwidth*Bhieght*Bdepth);//get the mean
			}

			//mean around observed voxel
			int intensRefPoint = 0;
			INSIDE_VOLUME_Left = positionOfContextVoxelR(x0, y0, z0, x0, y0, z0);
			INSIDE_VOLUME_Right = positionOfContextVoxelR(x0, y0, z0, x0+Bwidth+1, y0+Bhieght+1, z0+Bdepth+1);

			if (INSIDE_VOLUME_Right && INSIDE_VOLUME_Left) {
				//get the 8 corners in the insensity volume to calculate the mean
				nhood[0] = dintensVolume_[((z0) * H + (y0)) * W + (x0)];//-
				nhood[1] = dintensVolume_[((z0+Bdepth) * H + (y0+Bhieght)) * W + (x0+Bwidth)] ;//+
				nhood[2] = dintensVolume_[((z0+Bdepth) * H + (y0)) * W + (x0)];//+
				nhood[3] = dintensVolume_[((z0) * H + (y0)) * W + (x0+Bwidth)];//+
				nhood[4] = dintensVolume_[((z0) * H + (y0+Bhieght)) * W + (x0)];//+
				nhood[5] = dintensVolume_[((z0+Bdepth) * H + (y0)) * W + (x0+Bwidth)];//-
				nhood[6] = dintensVolume_[((z0+Bdepth) * H + (y0+Bhieght)) * W + (x0)];//-
				nhood[7] = dintensVolume_[((z0) * H + (y0+Bhieght)) * W + (x0+Bwidth)];//-
				intensRefPoint = (int)((nhood[1]+nhood[2]+nhood[3]+nhood[4])-(nhood[0]+nhood[5]+nhood[6]+nhood[7]))/(Bwidth*Bhieght*Bdepth);//get the mean
			}


			
			return (intensContextPoint-intensRefPoint);
        }
		double FeatureComputer::featureResponse_RelativeIntensity(int x0, int y0, int z0, float disp_mm[],int dim, unsigned int boxsize[])
        {
            // compute position of context voxel
            int x1, y1, z1;

			// edit by Anthony including a box
			int Bwidth=boxsize[0]*invScales_[0], Bhieght=boxsize[1]*invScales_[1], Bdepth=boxsize[2]*invScales_[2];//size of the mean box in each dimension
			if (Bwidth == 0) Bwidth = 1;
			if (Bhieght == 0) Bhieght = 1;
			if (Bdepth == 0) Bdepth = 1;

			//check boundary
            bool INSIDE_VOLUME_Left = positionOfContextVoxel(x0, y0, z0, disp_mm, x1, y1, z1);
			bool INSIDE_VOLUME_Right = positionOfContextVoxelR(x0, y0, z0, x1+Bwidth, y1+Bhieght, z1+Bdepth);
            
            double intensContextPoint = 0;         // outside the volume
            int W = volDims_.Width+1, H = volDims_.Height+1, D = volDims_.Depth+1;

            // when the input is a volume
			/*if (INSIDE_VOLUME_Right && INSIDE_VOLUME_Left) { // get the sum of the neighborhood
				for(int i = 0; i < Bwidth; i++){
					for(int j = 0; j < Bhieght; j++){
						for(int k = 0; k < Bdepth; k++){
							intensContextPoint += intensVolume_[((z1+k) * H + (y1+j)) * W + (x1+i)]; // proper value of context pixel, inside volume
						}
					}
				}
				intensContextPoint /= Bwidth*Bhieght*Bdepth; // get the mean value
			}

            int intensRefPoint = intensVolume_[(z0 * H + y0) * W + x0];

            return (intensContextPoint - intensRefPoint); // return feature response (in HU units)
			*/
		
			// when the input is an integral image
			//calculate the mean around the realtive voxel, x1,y1,z1 is the top left hand corner of the box
			long long nhood [8];
			if (INSIDE_VOLUME_Right && INSIDE_VOLUME_Left) {
				nhood[0] = (*pAIvolume)(x1,y1,z1,dim);//-
				nhood[1] = (*pAIvolume)(x1+Bwidth,y1+Bhieght,z1+Bdepth,dim);//+
				nhood[2] = (*pAIvolume)(x1,y1,z1+Bdepth,dim);//+
				nhood[3] = (*pAIvolume)(x1+Bwidth,y1,z1,dim);//+
				nhood[4] = (*pAIvolume)(x1,y1+Bhieght,z1,dim);//+
				nhood[5] = (*pAIvolume)(x1+Bwidth,y1,z1+Bdepth,dim);//-
				nhood[6] = (*pAIvolume)(x1,y1+Bhieght,z1+Bdepth,dim);//-
				nhood[7] = (*pAIvolume)(x1+Bwidth,y1+Bhieght,z1,dim);//-
				intensContextPoint = ((nhood[1]+nhood[2]+nhood[3]+nhood[4])-(nhood[0]+nhood[5]+nhood[6]+nhood[7]))/(Bwidth*Bhieght*Bdepth);//get the mean
			}

			//mean around observed voxel
			double intensRefPoint = 0;
			INSIDE_VOLUME_Left = positionOfContextVoxelR(x0, y0, z0, x0, y0, z0);
			INSIDE_VOLUME_Right = positionOfContextVoxelR(x0, y0, z0, x0+Bwidth+1, y0+Bhieght+1, z0+Bdepth+1);

			if (INSIDE_VOLUME_Right && INSIDE_VOLUME_Left) {
				//get the 8 corners in the insensity volume to calculate the mean
				nhood[0] = (*pAIvolume)(x0,y0,z0,dim);//-
				nhood[1] = (*pAIvolume)(x0+Bwidth,y0+Bhieght,z0+Bdepth,dim);//+
				nhood[2] = (*pAIvolume)(x0,y0,z0+Bdepth,dim);//+
				nhood[3] = (*pAIvolume)(x0+Bwidth,y0,z0,dim);//+
				nhood[4] = (*pAIvolume)(x0,y0+Bhieght,z0,dim);//+
				nhood[5] = (*pAIvolume)(x0+Bwidth,y0,z0+Bdepth,dim);//-
				nhood[6] = (*pAIvolume)(x0,y0+Bhieght,z0+Bdepth,dim);//-
				nhood[7] = (*pAIvolume)(x0+Bwidth,y0+Bhieght,z0,dim);//-
				intensRefPoint = ((nhood[1]+nhood[2]+nhood[3]+nhood[4])-(nhood[0]+nhood[5]+nhood[6]+nhood[7]))/(Bwidth*Bhieght*Bdepth);//get the mean
			}


			
			return (intensContextPoint-intensRefPoint);
        }
        /// <summary>
        /// double featureResponse_RatioIntensityUN_SHIFTED(int x0, int y0, int z0, float disp_mm[])
        /// Feature response computation: Type 3: Ratio of intensities: probe / ref  (dimensionless quanitity, in the range [-1024 .. +1024]
        /// </summary>
        /// <param name="x0"></param> 
        /// <param name="y0"></param>
        /// <param name="z0"></param>
        /// <param name="disp_mm"></param>
        /// <returns></returns>
        /// Currently this way gives lower test and train error, and the feature is actually used
        /// 
        /// ==> TAG_SWITCH_BETWEEN_RATIOINTENSITY    Uncomment to select this one
        double FeatureComputer::featureResponse_RatioIntensity(int x0, int y0, int z0, float disp_mm[])
        {
            // compute position of context voxel
            int x1, y1, z1;
            bool INSIDE_VOLUME = positionOfContextVoxel(x0, y0, z0, disp_mm, x1, y1, z1);

            // compute feature response in HU
            int intensContextPoint = 0;         // air outside volume
            int W = volDims_.Width, H = volDims_.Height, D = volDims_.Depth;
            if (INSIDE_VOLUME) intensContextPoint = intensVolume_[(z1 * H + y1) * W + x1]; // proper value of context pixel, inside volume

            double dIntensContextPointHU = rescaleParams_.rescaleSlope * intensContextPoint + rescaleParams_.rescaleIntercept; // convert to HU


            int intensRefPoint = intensVolume_[(z0 * H + y0) * W + x0];
            double dIntensRefPointHU = rescaleParams_.rescaleSlope * intensRefPoint + rescaleParams_.rescaleIntercept; // convert to HU

            if (dIntensRefPointHU > -1 && dIntensRefPointHU < 1) dIntensRefPointHU = 1; // so that thresholds are generated from finite range


            double dRatio = 0;
            try
            {
                dRatio = dIntensContextPointHU / dIntensRefPointHU;
            }
            catch  (...)  // div by zero, etc
            {
                dRatio = 0;
            }

            return dRatio; // return feature response (dimensionless )
        }



        
        /// <summary>
        /// Feature response computation: Type 4: difference of intensities: probe1 - probe2  
        /// </summary>
        /// <param name="x0"></param> 
        /// <param name="y0"></param>
        /// <param name="z0"></param>
        /// <param name="disp_mm"></param>
        /// <returns></returns>
        double FeatureComputer::featureResponse_DiffOf2Probes(int x0, int y0, int z0, float disp_mm[])
        {
            // compute position of probe1
            int x1, y1, z1;
            bool bProbe1InsideVol = positionOfContextVoxel(x0, y0, z0, disp_mm, x1, y1, z1);

            int x2, y2, z2;
            bool bProbe2InsideVol = false;
            float* arrDisplacementProbe2_mm = &disp_mm[3];
            bProbe2InsideVol = positionOfContextVoxel2(x0, y0, z0, arrDisplacementProbe2_mm, x2, y2, z2);


            // compute feature response in HU
            int nIntensityProbe1 = 0;         // air outside volume
            int nIntensityProbe2 = 0;
            int W = volDims_.Width, H = volDims_.Height, D = volDims_.Depth;
            if (bProbe1InsideVol) nIntensityProbe1 = intensVolume_[(z1 * H + y1) * W + x1]; // proper value of context pixel, inside volume
            if (bProbe2InsideVol) nIntensityProbe2 = intensVolume_[(z2 * H + y2) * W + x2]; // proper value of context pixel, inside volume

            double huDiff = rescaleParams_.rescaleSlope * (nIntensityProbe1 - nIntensityProbe2);

            return huDiff; // return feature response (in HU units)
        }

		double FeatureComputer::featureResponse_DiffOf2Probes(int x0, int y0, int z0, float disp_mm[], int dim, unsigned int boxsize[])
        {	// set box size
			int Bwidth=boxsize[0]*invScales_[0], Bhieght=boxsize[1]*invScales_[1], Bdepth=boxsize[2]*invScales_[2];//size of the mean box in each dimension
			if (Bwidth == 0) Bwidth = 1;
			if (Bhieght == 0) Bhieght = 1;
			if (Bdepth == 0) Bdepth = 1;

            // compute position of probe1
            int x1, y1, z1;
            bool INSIDE_VOLUME_Left_1 = positionOfContextVoxel(x0, y0, z0, disp_mm, x1, y1, z1);
			bool INSIDE_VOLUME_Right_1 = positionOfContextVoxelR(x0, y0, z0, x1+Bwidth, y1+Bhieght, z1+Bdepth);

            int x2, y2, z2;
            bool bProbe2InsideVol = false;
            float* arrDisplacementProbe2_mm = &disp_mm[3];
			bool INSIDE_VOLUME_Left_2 = positionOfContextVoxel(x0, y0, z0, arrDisplacementProbe2_mm, x2, y2, z2);
			bool INSIDE_VOLUME_Right_2 = positionOfContextVoxelR(x0, y0, z0, x2+Bwidth, y2+Bhieght, z2+Bdepth);

            // compute feature response in HU
            int nIntensityProbe1 = 0;         // air outside volume
            int nIntensityProbe2 = 0;
            
            long long nhood [8];
			if (INSIDE_VOLUME_Right_1 && INSIDE_VOLUME_Left_1) {
				nhood[0] = (*pAIvolume)(x1,y1,z1,dim);//-
				nhood[1] = (*pAIvolume)(x1+Bwidth,y1+Bhieght,z1+Bdepth,dim);//+
				nhood[2] = (*pAIvolume)(x1,y1,z1+Bdepth,dim);//+
				nhood[3] = (*pAIvolume)(x1+Bwidth,y1,z1,dim);//+
				nhood[4] = (*pAIvolume)(x1,y1+Bhieght,z1,dim);//+
				nhood[5] = (*pAIvolume)(x1+Bwidth,y1,z1+Bdepth,dim);//-
				nhood[6] = (*pAIvolume)(x1,y1+Bhieght,z1+Bdepth,dim);//-
				nhood[7] = (*pAIvolume)(x1+Bwidth,y1+Bhieght,z1,dim);//-
				nIntensityProbe1 = (int)((nhood[1]+nhood[2]+nhood[3]+nhood[4])-(nhood[0]+nhood[5]+nhood[6]+nhood[7]))/(Bwidth*Bhieght*Bdepth);//get the mean
			}

            if (INSIDE_VOLUME_Right_2 && INSIDE_VOLUME_Left_2) {
				nhood[0] = (*pAIvolume)(x2,y2,z2,dim);//-
				nhood[1] = (*pAIvolume)(x2+Bwidth,y2+Bhieght,z2+Bdepth,dim);//+
				nhood[2] = (*pAIvolume)(x2,y2,z2+Bdepth,dim);//+
				nhood[3] = (*pAIvolume)(x2+Bwidth,y2,z2,dim);//+
				nhood[4] = (*pAIvolume)(x2,y2+Bhieght,z2,dim);//+
				nhood[5] = (*pAIvolume)(x2+Bwidth,y2,z2+Bdepth,dim);//-
				nhood[6] = (*pAIvolume)(x2,y2+Bhieght,z2+Bdepth,dim);//-
				nhood[7] = (*pAIvolume)(x2+Bwidth,y2+Bhieght,z2,dim);//-
				nIntensityProbe2 = (int)((nhood[1]+nhood[2]+nhood[3]+nhood[4])-(nhood[0]+nhood[5]+nhood[6]+nhood[7]))/(Bwidth*Bhieght*Bdepth);//get the mean
			}

            double huDiff = rescaleParams_.rescaleSlope * (nIntensityProbe1 - nIntensityProbe2);

            return huDiff; // return feature response (in HU units)
        }

        /// <summary>
        /// Feature response computation: Type 2 -- Relative Posterior
        /// </summary>
        double FeatureComputer::featureResponse_RelativePosterior(int x0, int y0, int z0, float disp_mm[])
        {
            // compute position of context voxel
            int x1, y1, z1;
            bool INSIDE_VOLUME = positionOfContextVoxel(x0, y0, z0, disp_mm, x1, y1, z1);

            double Output = 0;

            return Output; // return feature response \in [0.0,1.0]
        }

        bool FeatureComputer::positionOfContextVoxel(int x0, int y0, int z0, float disp_mm[], int& x1, int& y1, int& z1)
        {
            // compute position of context pixel
            int dx_pix = (int)(disp_mm[0] * invScales_[0]), dy_pix = (int)(disp_mm[1] * invScales_[1]), dz_pix = (int)(disp_mm[2] * invScales_[2]);     // convert displacement from mm to pixels
            x1 = x0 + dx_pix; y1 = y0 + dy_pix; z1 = z0 + dz_pix;                                                                                // compute position of context pixel

            // check that inside volume
            bool INSIDE_VOLUME = true;
            int W = volDims_.Width, H = volDims_.Height, D = volDims_.Depth;  // AAM These are fast structure accesses

			if (!(x1 > -1  &&    x1 < W     &&    y1 > -1   &&   y1 < H    &&   z1 >  -1   &&  z1 < D ) )  INSIDE_VOLUME = false;

            return INSIDE_VOLUME;
        }

		bool FeatureComputer::positionOfContextVoxelR(int x0, int y0, int z0, const int& x1, const int& y1, const int& z1)
        {
            // check that inside volume
            bool INSIDE_VOLUME = true;
            int W = volDims_.Width, H = volDims_.Height, D = volDims_.Depth;  // AAM These are fast structure accesses

			if (!(x1 > -1  &&    x1 < W     &&    y1 > -1   &&   y1 < H    &&   z1 >  -1   &&  z1 < D ) )  INSIDE_VOLUME = false;

            return INSIDE_VOLUME;
        }

        bool FeatureComputer::positionOfContextVoxel2(int x0, int y0, int z0, float* disp_mm, int& x1, int& y1, int& z1)
        {
            // compute position of context pixel
            int dx_pix = (int)(disp_mm[0] * invScales_[0]), dy_pix = (int)(disp_mm[1] * invScales_[1]), dz_pix = (int)(disp_mm[2] * invScales_[2]);     // convert displacement from mm to pixels
            x1 = x0 + dx_pix; y1 = y0 + dy_pix; z1 = z0 + dz_pix;                                                                                // compute position of context pixel

            // check that inside volume
            bool INSIDE_VOLUME = true;
            int W = volDims_.Width, H = volDims_.Height, D = volDims_.Depth;  // AAM These are fast structure accesses

			if (!(x1 > -1 && x1 < W && y1 > -1 && y1 < H && z1 > -1 && z1 < D)) INSIDE_VOLUME = false;

            return INSIDE_VOLUME;
        }

        /// <summary>
        /// Normalizes the histogram stored at each tree node and returns it out via the argument posterior
        /// </summary>
        void FeatureComputer::computeNodePosterior(TreeNode& node, float posterior[], int numClasses)
        {
            
            float sum = 0;
            for (int c = 0; c < numClasses; c++)
            {
                float val = (float) node.histogram_[c];
                posterior[c] = val;
                sum += val;
            }

            float invSum = 1.0f / sum;
            for (int c = 0; c < numClasses; c++)
                posterior[c] *= invSum;


        }

        /// <summary>
        /// Dot product of posteriors. This is only one of the possible distance functions to measrue difference/similarity between distributions
        /// </summary>
        double FeatureComputer::posteriorDotProduct(float posterior0[], float posterior1[], int numClasses)
        {
            double dotProd = 0;
            for (int c = 0; c < numClasses; c++)
                dotProd += posterior0[c] * posterior1[c];
            return dotProd;
        }

        double FeatureComputer::featureResponse_AbsSym(int x0, int y0, int z0, int dim, unsigned int boxsize[])
		{	

			int Bwidth=boxsize[0]*invScales_[0]+1, Bhieght=boxsize[1]*invScales_[1]+1, Bdepth=boxsize[2]*invScales_[2]+1;//size of the mean box in each dimension
			if (Bwidth <= 1) Bwidth = 2;
			if (Bhieght <= 1) Bhieght = 2;
			if (Bdepth <= 1) Bdepth = 2;
            
			int x1 = !(Bwidth%2) ? x0-(Bwidth/2-1) : x0-((Bwidth-1)/2);//left
			int y1 = !(Bhieght%2) ? y0-(Bhieght/2-1) : y0-((Bhieght-1)/2);
			int z1 = !(Bdepth%2) ? z0-(Bdepth/2-1) : z0-((Bdepth-1)/2);
			
			int x2 = !(Bwidth%2) ? x0+(Bwidth/2) : x0+((Bwidth-1)/2);//right
			int y2 = !(Bhieght%2) ? y0+(Bhieght/2) : y0+((Bhieght-1)/2);
			int z2 = !(Bdepth%2) ? z0+(Bdepth/2) : z0+((Bdepth-1)/2);

			bool INSIDE_VOLUME_Right = positionOfContextVoxelR(x2, y2, z2, x2, y2, z2);
			bool INSIDE_VOLUME_Left = positionOfContextVoxelR(x1, y1, z1, x1, y1, z1);
			
			double Obsmean = 0;
			if (INSIDE_VOLUME_Right && INSIDE_VOLUME_Left) {
				long long nhood = 0;
				nhood -= (*pAIvolume)(x1,y1,z1,dim);//-
				nhood += (*pAIvolume)(x2,y2,z2,dim);//+
				nhood += (*pAIvolume)(x1,y1,z2,dim);//+
				nhood += (*pAIvolume)(x2,y1,z1,dim);//+
				nhood += (*pAIvolume)(x1,y2,z1,dim);//+
				nhood -= (*pAIvolume)(x2,y1,z2,dim);//-
				nhood -= (*pAIvolume)(x1,y2,z2,dim);//-
				nhood -= (*pAIvolume)(x2,y2,z1,dim);//-

				Obsmean = nhood;
			}
			

			//find the symmectric point assumes that the symmetric axis was found with 1x1x1 voxels size
			double x0s = (x0 + 1)*scales_[0];
			double y0s = (y0 + 1 - (*pAvolume).extent(1)/ 2)*scales_[1];
			double z0s = (z0 + 1 - (*pAvolume).extent(2)/ 2)*scales_[2];

			double dist = (AxisSym[0]*z0s + AxisSym[1]*y0s + AxisSym[2]*x0s + AxisSym[3]) / AxisSym[4];
			x0s = ((x0s - (2*dist*AxisSym[2]/AxisSym[4]) - 1) / scales_[0]);
			y0s = ((y0s - (2*dist*AxisSym[1]/AxisSym[4]) - 1 + (*pAvolume).extent(1)*scales_[1] / 2) / scales_[1]);
			z0s = ((z0s - (2*dist*AxisSym[0]/AxisSym[4]) - 1 + (*pAvolume).extent(2)*scales_[2] / 2) / scales_[2]);

			x1 = !(Bwidth%2) ? x0s-(Bwidth/2-1) : x0s-((Bwidth-1)/2);//left
			y1 = !(Bhieght%2) ? y0s-(Bhieght/2-1) : y0s-((Bhieght-1)/2);
			z1 = !(Bdepth%2) ? z0s-(Bdepth/2-1) : z0s-((Bdepth-1)/2);
			
			x2 = !(Bwidth%2) ? x0s+(Bwidth/2) : x0s+((Bwidth-1)/2);//right
			y2 = !(Bhieght%2) ? y0s+(Bhieght/2) : y0s+((Bhieght-1)/2);
			z2 = !(Bdepth%2) ? z0s+(Bdepth/2) : z0s+((Bdepth-1)/2);

			INSIDE_VOLUME_Right = positionOfContextVoxelR(x2, y2, z2, x2, y2, z2);
			INSIDE_VOLUME_Left = positionOfContextVoxelR(x1, y1, z1, x1, y1, z1);

			double Symmean = 0;
			if (INSIDE_VOLUME_Right && INSIDE_VOLUME_Left) {
				long long nhood2 = 0;
				nhood2 -= (*pAIvolume)(x1,y1,z1,dim);//-
				nhood2 += (*pAIvolume)(x2,y2,z2,dim);//+
				nhood2 += (*pAIvolume)(x1,y1,z2,dim);//+
				nhood2 += (*pAIvolume)(x2,y1,z1,dim);//+
				nhood2 += (*pAIvolume)(x1,y2,z1,dim);//+
				nhood2 -= (*pAIvolume)(x2,y1,z2,dim);//-
				nhood2 -= (*pAIvolume)(x1,y2,z2,dim);//-
				nhood2 -= (*pAIvolume)(x2,y2,z1,dim);//-
				Symmean = nhood2;
			}
			
			return Obsmean-Symmean;
		}