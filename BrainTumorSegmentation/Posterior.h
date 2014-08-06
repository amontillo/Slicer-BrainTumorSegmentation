#pragma once

#include "basicDefs.h"
#include "BvdFileManager.h" // for  VoxelFormat
#include "Dataset.h"  // for VolumeDimensions



    /// <summary>
    /// Header for Posterior4D binary volume file format
    /// </summary>
    struct Posterior4DFileHeader
    {
         float fVersion_; // = 1
         VoxelFormat VoxelFormat_;
         int Width_, Height_, Depth_;
		 int numClasses_;
         
    };


/// <summary>
/// Posterior is a 4D float array which stores the voxel-wise class posterior
/// </summary>
struct Posterior
{

	public:
		ushort* postArray_;     // an enormous 4D array [class,z,y,x]. This should be a float array but we quantize it to avoid memory issues.
        int nSizePostArray_;
		VolumeDimensions dims_;
		int numClasses_;



		Posterior(VolumeDimensions& dims, int numClasses);
		~Posterior();   // dtor deletes postArray_
		ushort* extract3DClassPosterior(int classIndex, VolumeDimensions volDims);  // Caller is responsible for deleting the returned 3D array
	
		void ZeroThePosterior();
		bool loadAndAccumulate_4DPosterior(string& fileName);
		bool save_4DPosterior(string& fileName);

};

