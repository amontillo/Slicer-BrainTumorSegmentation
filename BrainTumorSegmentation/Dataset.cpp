#include "Dataset.h"
#include "FeatureVolumeType.h"
#include "ChannelFarm.h"
#include <blitz/array.h>

using namespace std;
using boost::lexical_cast;

/// <summary>
///  Construct a dataset by de-serializing data from disk: 
///  1. Loads image header (xvd), image data (bvd)  at the requested downsampling
///  2. loads GT labels at the requested downsampling
///  Note: for steps 1 & 2, if downsampling version isnt already present it is created and saved to disk
/// </summary>
/// <param name="xvdDataFileName"></param>
/// <param name="gtStructuresFileName"></param>
//when there are multiple modalities

// Inserts the given ITK image into the arrays Avolume and AIvolume as the iDerivedChannel image location
void Dataset::insertITKVolumeIntoDataset(ImageUShortVolumeType::Pointer psImageVolume, int iDerivedChannel)
{

	ImageUShortVolumeType::IndexType pixelIndex;
	ImageUShortVolumeType::PixelType pixelValue;
	//1. copy the pixel values from psImageVolume into Avolume
	//2. Compute the integral image in AIvolume

	int maxDepth=dims_.Depth-1;
	for (int z0 = 0; z0 < dims_.Depth; z0++)
	{
		for (int y0 = 0; y0 < dims_.Height; y0++)
		{
			for (int x0 = 0; x0 < dims_.Width; x0++)
			{

				//pixelIndex[0]=x0; pixelIndex[1]=y0; pixelIndex[2]=z0;
				pixelIndex[0]=y0; pixelIndex[1]=x0; pixelIndex[2]=maxDepth-z0; // Swap first two dimensions and reverse the direction of last dim, to map RAI to internally used orientation. 
				pixelValue = psImageVolume->GetPixel( pixelIndex);

				Avolume(x0,y0,z0,iDerivedChannel)=pixelValue;
			}
		}
	}


	//convert image to the integral image 
	int W = dims_.Width+1, H = dims_.Height+1, D = dims_.Depth+1;
	long long nhood [8];
	
	for (int k = 0; k < (dims_.Depth+1); k++)
		for (int j = 0; j < (dims_.Height+1); j++)
			for (int i = 0; i < (dims_.Width+1); i++)
			{   
				if ((i==0)||(j==0)||(k==0)) //pad the array
				{
					AIvolume(i,j,k,iDerivedChannel) = 0;
				}
				else
				{
					nhood[0] = (long long)Avolume(i-1,j-1,k-1,iDerivedChannel);

					//get neighborhood
					nhood[1] = AIvolume(i-1,j-1,k-1,iDerivedChannel);
					nhood[2] = AIvolume(i,j,k-1,iDerivedChannel);
					nhood[3] = AIvolume(i-1,j,k,iDerivedChannel);
					nhood[4] = AIvolume(i,j-1,k,iDerivedChannel);
					nhood[5] = AIvolume(i-1,j,k-1,iDerivedChannel);
					nhood[6] = AIvolume(i,j-1,k-1,iDerivedChannel);
					nhood[7] = AIvolume(i-1,j-1,k,iDerivedChannel);

					AIvolume(i,j,k,iDerivedChannel) = nhood[0] + nhood[1] + nhood[2] + nhood[3] + nhood[4] - nhood[5] - nhood[6] - nhood[7];	
					//cout << AIvolume(i,j,k,iDerivedChannel) << endl; //for debuging
				}
			}

}

Dataset::Dataset(vector<ImageUShortVolumeType::Pointer>& ITKFeatureVolume, string& gtStructuresFileName, float sampleFactor, 
				 vector<int>& arrKnownGTLabelToPosteriorBinLUT, vector<string>& modalNames,bool sym)
{
    // set these to nullptr so destructor works properly
	intensVolume_=nullptr; dintensVolume_=nullptr; gtStructuresVolume_=nullptr;  AdditionalIntensVolumes_=nullptr;  dAdditionalIntensVolumes_=nullptr;

	// 1. Create xvtags structure with information from the first image
	// Find the first non-null ITKFeatureVolume

	int featureVolumeEnumFirstImage=FeatureVolumeTypeManager::featureVolumeName_To_EnumIdentifier(modalNames[0]);
	int featureVolumeEnumFirstImageBaseChannel=FeatureVolumeTypeManager::featureEnumToBaseChannel(featureVolumeEnumFirstImage);

	ImageUShortVolumeType::Pointer q=ITKFeatureVolume[featureVolumeEnumFirstImageBaseChannel];
	ImageUShortVolumeType::SpacingType spacing=q->GetSpacing();
	ImageUShortVolumeType::RegionType  region=q->GetBufferedRegion();


	//xvdtags_.pixScaleX=spacing[0]; 
	//xvdtags_.pixScaleY=spacing[1]; 
	xvdtags_.pixScaleX=spacing[1];  // Swap first two dimensions to map RAI to internally used orientation. 
	xvdtags_.pixScaleY=spacing[0]; 
    //
	xvdtags_.pixScaleZ=spacing[2];
	xvdtags_.rescaleSlope=1;
	xvdtags_.rescaleIntercept=0;
	//xvdtags_.volumeWidth=region.GetSize()[0];
	//xvdtags_.volumeHeight=region.GetSize()[1];
	xvdtags_.volumeWidth=region.GetSize()[1];  // Swap first two dimensions to map RAI to internally used orientation. 
	xvdtags_.volumeHeight=region.GetSize()[0];
    //
	xvdtags_.volumeDepth=region.GetSize()[2];
	xvdtags_.nNumChannels=1;
	xvdtags_.channelDisplayWidth = -1; 
	xvdtags_.channelDisplayHeight = -1;
	xvdtags_.channelDisplayDepth = -1;
	xvdtags_.binaryFileName="?";
	xvdtags_.patientName="?";
	xvdtags_.patientSex="?";
	xvdtags_.modality="?";
	xvdtags_.contrastAgent="?";
	xvdtags_.formatOK=true;

	// 2. Extract Dataset fields from xvdtags structure 
	stringXvdDataFileName_ = "dummy";

	// setting volume dimensions
	dims_.Width = xvdtags_.volumeWidth;
	dims_.Height = xvdtags_.volumeHeight;
	dims_.Depth = xvdtags_.volumeDepth;

	// setting pixel scales
	scales_.resize(3);
	scales_[0] = xvdtags_.pixScaleX;
	scales_[1] = xvdtags_.pixScaleY;
	scales_[2] = xvdtags_.pixScaleZ;

	// inverse of pixel scale are stored for efficiency
	invScales_.resize(3);
	for (int i = 0; i < 3; i++) invScales_[i] = 1.0 / scales_[i];

	// setting pixel<->HU conversion parameters
	rescaleParams_.rescaleSlope = (float)xvdtags_.rescaleSlope;
	rescaleParams_.rescaleIntercept = (float)xvdtags_.rescaleIntercept;


	// Assumption, command line will not provide the ground truth file 
	gtStructuresFileName_="dummy";

	// 3.    Allocate space for all channels in Avolume and AIvolume
	Avolume.resize(xvdtags_.volumeWidth,xvdtags_.volumeHeight,xvdtags_.volumeDepth,modalNames.size()); //store volume 
	AIvolume.resize(xvdtags_.volumeWidth+1,xvdtags_.volumeHeight+1,xvdtags_.volumeDepth+1,modalNames.size());//store integral image

	// 4.    Then load them all from the images into the channels 
    ImageUShortVolumeType::Pointer psDerivedChannelVolume;

	for (int iDerivedChannel=0; iDerivedChannel<modalNames.size(); iDerivedChannel++)
	{
		
		string case_=modalNames[iDerivedChannel];

		int featureVolumeEnum=FeatureVolumeTypeManager::featureVolumeName_To_EnumIdentifier(case_);
		int featureVolumeEnumBaseChannel=FeatureVolumeTypeManager::featureEnumToBaseChannel(featureVolumeEnum);

		switch (featureVolumeEnum)
		{
			case FEATURE_VOLUME_TYPE_T1m:
			case FEATURE_VOLUME_TYPE_T2m:
			case FEATURE_VOLUME_TYPE_T1Cm:
			case FEATURE_VOLUME_TYPE_FLAIRm:
				// ModeShift
				psDerivedChannelVolume=ChannelFarmManager::modeShift(ITKFeatureVolume[featureVolumeEnumBaseChannel], featureVolumeEnum, case_);
				insertITKVolumeIntoDataset(psDerivedChannelVolume, iDerivedChannel);
			   break;

			case FEATURE_VOLUME_TYPE_T1LBPxy:
			case FEATURE_VOLUME_TYPE_T2LBPxy:
			case FEATURE_VOLUME_TYPE_T1CLBPxy:
			case FEATURE_VOLUME_TYPE_FLAIRLBPxy:
				psDerivedChannelVolume=ChannelFarmManager::LBP(ITKFeatureVolume[featureVolumeEnumBaseChannel], featureVolumeEnum, "xy",case_);
				insertITKVolumeIntoDataset(psDerivedChannelVolume, iDerivedChannel);
 			   break;

			case FEATURE_VOLUME_TYPE_T1LBPxz:
			case FEATURE_VOLUME_TYPE_T2LBPxz:
			case FEATURE_VOLUME_TYPE_T1CLBPxz:
			case FEATURE_VOLUME_TYPE_FLAIRLBPxz:
				psDerivedChannelVolume=ChannelFarmManager::LBP(ITKFeatureVolume[featureVolumeEnumBaseChannel], featureVolumeEnum,"xz",case_);
				insertITKVolumeIntoDataset(psDerivedChannelVolume, iDerivedChannel);
			   break;

			case FEATURE_VOLUME_TYPE_T1LBPyz:
			case FEATURE_VOLUME_TYPE_T2LBPyz:
			case FEATURE_VOLUME_TYPE_T1CLBPyz:
			case FEATURE_VOLUME_TYPE_FLAIRLBPyz:
				psDerivedChannelVolume=ChannelFarmManager::LBP(ITKFeatureVolume[featureVolumeEnumBaseChannel], featureVolumeEnum,"yz",case_);
				insertITKVolumeIntoDataset(psDerivedChannelVolume, iDerivedChannel);
			   break;


			case FEATURE_VOLUME_TYPE_T1Gxy:
			case FEATURE_VOLUME_TYPE_T2Gxy:
			case FEATURE_VOLUME_TYPE_T1CGxy:
			case FEATURE_VOLUME_TYPE_FLAIRGxy:
				psDerivedChannelVolume=ChannelFarmManager::gradientQuantized(ITKFeatureVolume[featureVolumeEnumBaseChannel], featureVolumeEnumBaseChannel,"xy",case_);
				insertITKVolumeIntoDataset(psDerivedChannelVolume, iDerivedChannel);
			   break;


			case FEATURE_VOLUME_TYPE_T1Gxz:
			case FEATURE_VOLUME_TYPE_T2Gxz:
			case FEATURE_VOLUME_TYPE_T1CGxz:
			case FEATURE_VOLUME_TYPE_FLAIRGxz:
				psDerivedChannelVolume=ChannelFarmManager::gradientQuantized(ITKFeatureVolume[featureVolumeEnumBaseChannel], featureVolumeEnumBaseChannel,"xz",case_);
				insertITKVolumeIntoDataset(psDerivedChannelVolume, iDerivedChannel);
			   break;


			case FEATURE_VOLUME_TYPE_T1Gyz:
			case FEATURE_VOLUME_TYPE_T2Gyz:
			case FEATURE_VOLUME_TYPE_T1CGyz:
			case FEATURE_VOLUME_TYPE_FLAIRGyz:
				psDerivedChannelVolume=ChannelFarmManager::gradientQuantized(ITKFeatureVolume[featureVolumeEnumBaseChannel],featureVolumeEnumBaseChannel, "yz",case_);
				insertITKVolumeIntoDataset(psDerivedChannelVolume, iDerivedChannel);
			   break;


			default: 
				std::cout << "Error: unknown feature volume type [" << case_ << "]\n";

		};
	}




}


Dataset::Dataset(string& xvdDataFileName, string& gtStructuresFileName, float sampleFactor, vector<int>& arrKnownGTLabelToPosteriorBinLUT, 
				 vector<string>& modalNames,bool sym)
{   // set these to nullptr so destructor works properly
	intensVolume_=nullptr; dintensVolume_=nullptr; gtStructuresVolume_=nullptr;  AdditionalIntensVolumes_=nullptr;  dAdditionalIntensVolumes_=nullptr;

    string xvdDataDownsampledFileName = xvdDataFileName;
    string gtStructuresDownsampledFileName = gtStructuresFileName;
	string xmlFilenameWithoutModal = xvdDataFileName;

    if (sampleFactor != 1.0)
    {
        // Assume that the downsampled versions exist on disk
        // 1. Modify xvdDataFileName & gtStructuresFileName to include downsample suffix
        // 2. try to load the [sampleFactor].xvd & bvd image files and GT bvd file
		int nSampleFactor=(int)sampleFactor;
        string strDownsampleSuffix = ".ds[" + lexical_cast<string>(nSampleFactor) + "x" + lexical_cast<string>(nSampleFactor) + "x" + lexical_cast<string>(nSampleFactor) + "]";
        
		// AAM 20130525 Change so that the xvd is read from the first channel rather than from some abstract dataset.xvd, since some volumes like the uncropped file dataset3\dataset.ds[2x2x2]T1.bvd does not correspond in size to dataset.ds[2x2x2].bvd or dataset.ds[2x2x2]T1cr.bvd (which are both themselves the same size)
		xvdDataDownsampledFileName = FilenameManager::ChangeExtension(xvdDataFileName, strDownsampleSuffix + modalNames[0] + ".xvd");
		// xvdDataDownsampledFileName = FilenameManager::ChangeExtension(xvdDataFileName, strDownsampleSuffix + ".xvd");
        xmlFilenameWithoutModal = FilenameManager::ChangeExtension(xvdDataFileName, strDownsampleSuffix + ".xvd");
  
        gtStructuresDownsampledFileName = FilenameManager::ChangeExtension(gtStructuresFileName, strDownsampleSuffix + ".bvd");
    }

        
    BvdFileHeader bvdGTHeader;

	attemptToLoadFiles(xvdDataDownsampledFileName, xmlFilenameWithoutModal, gtStructuresDownsampledFileName, xvdtags_, bvdGTHeader, arrKnownGTLabelToPosteriorBinLUT, modalNames);

	if (sym)
	{	string SymFilename; //also do calculation here for the downsampleing
		SymFilename = FilenameManager::ChangeExtension(xvdDataFileName,"symaxis.txt");
		attemptToLoadSym(SymFilename);
	}
	// TODO Implement the "downsample and save if not found" functionality
    // if (!attemptToLoadFiles(xvdDataDownsampledFileName, gtStructuresDownsampledFileName, ref xvdtags_, ref bvdGTHeader, arrKnownGTLabelToPosteriorBinLUT))
}

// for cross-validation
// construct a dataset as a subset of the rows of an existing one called datasetMaster; assumes 1D data (e.g. timeseries data)
// The Start and stop rows are zero based indices
// ASSUMPTION:  the datasetMaster was loaded "byColumn"
//for multiple modalities Anthony 
bool Dataset::attemptToLoadFiles(string& xvdDataFileName, string& xmlFilenameWithoutModal, string& gtStructuresFileName, xvdTags_ver1& xvdtags, BvdFileHeader& bvdGTHeader, vector<int>& arrKnownGTLabelToPosteriorBinLUT, vector<string>& modalNames)
{
    bool bReturn = true;

    try
    {
        // Load intensity values (predictors)
		lstModalFilenames_.resize(modalNames.size());
		bReturn=XvdFileManager::readDataFromFile_ver1(lstModalFilenames_, xvdtags, xvdDataFileName, xmlFilenameWithoutModal, modalNames, Avolume, AIvolume );  // this also initializes data volume and loads bvd file
			if (!bReturn) throw;
			stringXvdDataFileName_ = xvdDataFileName;	

			// setting volume dimensions
			dims_.Width = xvdtags.volumeWidth;
			dims_.Height = xvdtags.volumeHeight;
			dims_.Depth = xvdtags.volumeDepth;

			// setting pixel scales
			scales_.resize(3);
			scales_[0] = xvdtags.pixScaleX;
			scales_[1] = xvdtags.pixScaleY;
			scales_[2] = xvdtags.pixScaleZ;

			// inverse of pixel scale are stored for efficiency
			invScales_.resize(3);
			for (int i = 0; i < 3; i++)
				invScales_[i] = 1.0 / scales_[i];

			// setting pixel<->HU conversion parameters
			rescaleParams_.rescaleSlope = (float)xvdtags.rescaleSlope;
			rescaleParams_.rescaleIntercept = (float)xvdtags.rescaleIntercept;
        

        // Load labels (target)
		// Assumption, single-output problem; not multi-output 
        BvdFileManager::loadVolume_labels(gtStructuresFileName, bvdGTHeader, gtStructuresVolume_);
		gtStructuresFileName_=gtStructuresFileName;
    }
    catch (...)
    {
        bReturn = false;
    }

    return bReturn;

}

//bool Dataset::attemptToLoadFiles(string& xvdDataFileName, string& gtStructuresFileName, xvdTags_ver1& xvdtags, BvdFileHeader& bvdGTHeader, vector<int>& arrKnownGTLabelToPosteriorBinLUT)
//{
//    bool bReturn = true;
//
//    try
//    {
//        // Load intensity values (predictors)
//		bReturn=XvdFileManager::readDataFromFile_ver1(xvdtags, xvdDataFileName, intensVolume_, AdditionalIntensVolumes_, dintensVolume_, dAdditionalIntensVolumes_);  // this also initializes data volume and loads bvd file
//			if (!bReturn) throw;
//
//			stringXvdDataFileName_ = xvdDataFileName;	
//
//			// setting volume dimensions
//			dims_.Width = xvdtags.volumeWidth;
//			dims_.Height = xvdtags.volumeHeight;
//			dims_.Depth = xvdtags.volumeDepth;
//
//			// setting pixel scales
//			scales_.resize(3);
//			scales_[0] = xvdtags.pixScaleX;
//			scales_[1] = xvdtags.pixScaleY;
//			scales_[2] = xvdtags.pixScaleZ;
//
//			// inverse of pixel scale are stored for efficiency
//			invScales_.resize(3);
//			for (int i = 0; i < 3; i++)
//				invScales_[i] = 1.0 / scales_[i];
//
//			// setting pixel<->HU conversion parameters
//			rescaleParams_.rescaleSlope = (float)xvdtags.rescaleSlope;
//			rescaleParams_.rescaleIntercept = (float)xvdtags.rescaleIntercept;
//        
//
//        // Load labels (target)
//		// Assumption, single-output problem; not multi-output 
//        BvdFileManager::loadVolume_labels(gtStructuresFileName, bvdGTHeader, gtStructuresVolume_);
//		gtStructuresFileName_=gtStructuresFileName;
//    }
//    catch (...)
//    {
//        bReturn = false;
//    }
//
//    return bReturn;
//
//}

bool Dataset::attemptToLoadSym(string& symFileName)
{	
	bool bReturn = true;
	SymAxis_.resize(5);
	try{
		ifstream symfile(symFileName.c_str());
		if(!symfile) throw; //check if file is open

		for (int i = 0; i < 3; i++){
			symfile >> SymAxis_[i];
		}

		SymAxis_[4] = sqrt( SymAxis_[0]*SymAxis_[0] + SymAxis_[1]*SymAxis_[1] + SymAxis_[2]*SymAxis_[2]);//calculated the magnitude here to save computation
	}
	catch(...)
	{
		bReturn = false;
	}
	return bReturn;
}
