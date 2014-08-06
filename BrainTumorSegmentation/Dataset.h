#pragma once
#include <vector>
#include <string>
// #include "DecisionTreeTrainer.h"  // remove circular #include on IndexVolume
#include "IndexVolume.h"
#include "xvdFileManager.h"
#include "BvdFileManager.h"
#include "basicDefs.h"
#include "WLmapper.h"
#include "FeatureVolumeType.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/shared_ptr.hpp>
#include <blitz/array.h>

using namespace std;
using boost::lexical_cast;



struct VolumeDimensions
{
        int Depth;
        int Height;
        int Width;
};

class Dataset
{   
  public:
	VolumeDimensions dims_;
	ushort* intensVolume_;
	ushort** AdditionalIntensVolumes_;

	vector<double> SymAxis_;

	blitz::Array<ushort,4> Avolume;//array volume for multi-modal
	blitz::Array<long long,4> AIvolume;//array volume for multi-modal integral image
	blitz::Array<int,5> AHvolume;

	double* dintensVolume_;
	double** dAdditionalIntensVolumes_;

	xvdTags_ver1 xvdtags_;

	byte* gtStructuresVolume_;              // this volume stores ground-truth, hard segmentation labels for different organs; AAM: public for direct access when training

    vector<ptrsmartIndexVolume> arrIndexVolumesForAllAuxTrees_;    // Trees regenerated for autocontext


	vector<double> scales_;                               // pixel scales (in mm)
	vector<double> invScales_;                            // inverse of pixel scales (in mm)
	RescaleParams rescaleParams_;                   // line parameters for computing HU values
	string stringXvdDataFileName_;                  // Single file denoting the general header information that pertains to all channels (as of May 2013)
	string gtStructuresFileName_;                   // Ground truth volume filename
    vector<string> lstModalFilenames_;              // Filename for each channel, currently these are only set when loading image volumes into Blitz++ not older legacy code

    /// <summary>
    ///  Performs multitude of functions, including:
    ///  1.loads image header (xvd), image data (bvd)  at the requested downsampling
    ///  2. loads GT labels at the requested downsampling
    ///  Note: for both 1 & 2, if downsampling version isnt already present it is created and saved to disk
    /// </summary>
    /// <param name="xvdDataFileName"></param>
    /// <param name="gtStructuresFileName"></param>
    Dataset(string& xvdDataFileName, string& gtStructuresFileName, float sampleFactor, vector<int>& arrKnownGTLabelToPosteriorBinLUT);
	Dataset(string& xvdDataFileName, string& gtStructuresFileName, float sampleFactor, vector<int>& arrKnownGTLabelToPosteriorBinLUT,vector<string>& modalNames, bool sym);
	Dataset(string& xvdDataFileName, string& gtStructuresFileName, float sampleFactor, vector<int>& arrKnownGTLabelToPosteriorBinLUT,vector<string>& modalNames,vector<string>& histNames,unsigned int hlvls,bool sym);

	// Construct a dataset for a given set of acquired images loaded from disk (or slicer 3D)
	Dataset(vector<ImageUShortVolumeType::Pointer>& ITKFeatureVolume, string& gtStructuresFileName, float sampleFactor, 
				 vector<int>& arrKnownGTLabelToPosteriorBinLUT, vector<string>& modalNames,bool sym);
		// helper function that Inserts the given ITK image into the arrays Avolume and AIvolume as the iDerivedChannel image location
		void insertITKVolumeIntoDataset(ImageUShortVolumeType::Pointer psImageVolume, int iDerivedChannel);

	// construct a dataset as a subset of the rows of an existing one called datasetMaster; assumes 1D data (e.g. timeseries data)
	Dataset(Dataset& datasetMaster, int nStartRowBlock1, int nStopRowBlock1, int nStartRowBlock2, int nStopRowBlock2);

	int volume() 
	{ return dims_.Width * dims_.Height * dims_.Depth; };


	~Dataset()
	{
		if (intensVolume_!=nullptr)  { delete [] intensVolume_; intensVolume_=nullptr; }
		if (dintensVolume_!=nullptr)  { delete [] dintensVolume_; dintensVolume_=nullptr; }
        if (gtStructuresVolume_!=nullptr)  { delete [] gtStructuresVolume_; gtStructuresVolume_=nullptr; }

		// Note these next two deletions are not correct. since they are arrays of arrays, we have to loop through the first dim (pointers to arrays) and delete each array
		// Since the functionality this supported is not really used any longer we are not spending time to make it correct
		if (AdditionalIntensVolumes_!=nullptr)  { delete [] AdditionalIntensVolumes_; AdditionalIntensVolumes_=nullptr; }
		if (dAdditionalIntensVolumes_!=nullptr)  { delete [] dAdditionalIntensVolumes_; dAdditionalIntensVolumes_=nullptr; }

	}


  private:
    // bool attemptToLoadFiles(string& xvdDataFileName, string& gtStructuresFileName, xvdTags_ver1& xvdtags, BvdFileHeader& bvdGTHeader, vector<int>& arrKnownGTLabelToPosteriorBinLUT);
	bool attemptToLoadFiles(string& xvdDataFileName, string& xmlFilenameWithoutModal, string& gtStructuresFileName, xvdTags_ver1& xvdtags, BvdFileHeader& bvdGTHeader, vector<int>& arrKnownGTLabelToPosteriorBinLUT,  vector<string>& modalNames);
	bool attemptToLoadFiles(string& xvdDataFileName, string& gtStructuresFileName, xvdTags_ver1& xvdtags, BvdFileHeader& bvdGTHeader, vector<int>& arrKnownGTLabelToPosteriorBinLUT,  vector<string>& modalNames, vector<string>& histNames,unsigned int hlvls);
	bool attemptToLoadSym(string& symFileName);

};

typedef boost::shared_ptr<Dataset> ptrsmartDataset;


