#include <vector>
#include <string>
#include "tinyxml.h" 
#include <boost/shared_ptr.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
// #include <stdio.h>  // snprintf

#include <iostream>   // for Tinyxml
#include <sstream>   // for Tinyxml
#include <limits>
#include <blitz\array.h>
#include <blitz\memblock.h>


#include "DecisionTree.h"
#include "ForestParameters.h"  
#include "EnvironmentSettingsFileManager.h"
#include "FilenameManager.h"
#include "basicDefs.h"
#include "Dataset.h"
#include "FeatureVolumeType.h"
#include "Controller.h"
#include "PluginProcessWatcher.h" // version of itkPluginFilterWatcher.h for modules not written as an ITK filter
#include "itkPluginFilterWatcher.h"

#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkSmoothingRecursiveGaussianImageFilter.h"
#include "itkPluginUtilities.h"
#include "BrainTumorSegmentationCLP.h"   // The slicer include
#include "itkCastImageFilter.h" 

#include "itkOrientImageFilter.h"





// Compile options
// --------------------
// #define WIN32                    ... define for windows, undefine for linux



using namespace std;
using boost::lexical_cast;

#ifdef WIN32
char cFolderSeparator='\\';  // windows
char cNonFolderSeparator='/';
#else
char cFolderSeparator='/';  // Linux
char cNonFolderSeparator='\\';
#endif

struct PluginProcessWatcher gPluginProcessWatcher; // global process watcher - for reporting status on Slicer

void convertFilePath(string& strFilePath);


// slicer.exe  --launch  lib\Slicer-4.2\cli-modules\Debug\BrainTumorSegmentation.exe  G:\AnthonyBianchi_DataAndExperimentResults\Converted3\Dataset22\dataset.ds[2x2x2]T1m.nii G:\AnthonyBianchi_DataAndExperimentResults\Converted3\Dataset22\dataset.ds[2x2x2]T2m.nii G:\AnthonyBianchi_DataAndExperimentResults\Converted3\Dataset22\dataset.ds[2x2x2]T1Cm.nii G:\AnthonyBianchi_DataAndExperimentResults\Converted3\Dataset22\dataset.ds[2x2x2]FLAIRm.nii G:\AnthonyBianchi_DataAndExperimentResults\Converted3\Dataset22\dataset.ds[2x2x2]T1CGxy.nii G:\AnthonyBianchi_DataAndExperimentResults\Converted3\Dataset22\dataset.ds[2x2x2]T1CLBPxy.nii G:\AnthonyBianchi_DataAndExperimentResults\Converted3\Dataset22\dataset.ds[2x2x2]T2Gxz.nii G:\AnthonyBianchi_DataAndExperimentResults\Converted3\Dataset22\dataset.ds[2x2x2]T2LBPxz.nii G:\AnthonyBianchi_DataAndExperimentResults\Converted3\Dataset22\dataset.ds[2x2x2]FLAIRGxy.nii G:\AnthonyBianchi_DataAndExperimentResults\Converted3\Dataset22\dataset.ds[2x2x2]FLAIRLBPxy.nii G:\MyCopy\NeuroImagingData\BRATS\NIFTIFormat\Dataset22TissueSegmentation.nii G:\MyCopy\NeuroImagingData\BRATS\NIFTIFormat\Dataset22TumorProbability.nii G:\MyCopy\NeuroImagingData\BRATS\NIFTIFormat\Dataset22EdemaProbability.nii


// Using a global here so DecisionForestTester::threadcore() can report progress.
typedef itk::Image<unsigned short, 3> 	dummyInternalImageType;
typedef itk::Image<float, 3> 	dummyOutputImageType;
typedef itk::CastImageFilter<dummyInternalImageType, dummyOutputImageType> CastingFilterType;
CastingFilterType::Pointer caster = CastingFilterType::New();  // caster used to convey progress to Slicer GUI


// Use an anonymous namespace to keep class types and function names
// from colliding when module is used as shared object module.  Every
// thing should be in an anonymous namespace except for the module
// entry point, e.g. main()
//
namespace
{

} // end of anonymous namespace

int main( int argc, char * argv[] )
{
  PARSE_ARGS;

  if (strForestDescriptionFile=="..") strForestDescriptionFile="";  // .. is the default value which is translated to the empty string for this module

  bool bDebug=false;  // Set to true to write intermediate debug files out to hard coded directories

  itk::ImageIOBase::IOPixelType     pixelType;
  itk::ImageIOBase::IOComponentType componentType;

    std::cout << "\n Input files are: \n";
	std::cout << "strT1VolumeFilename =" << strT1VolumeFilename << "\n";
	std::cout << "strT2VolumeFilename =" << strT2VolumeFilename << "\n";
	std::cout << "strT1CVolumeFilename =" << strT1CVolumeFilename << "\n";
	std::cout << "strFLAIRVolumeFilename =" << strFLAIRVolumeFilename << "\n";
	std::cout << "\n Output files are: \n";
	std::cout << "strTissueSegmentationVolumeFilename =" << strTissueSegmentationVolumeFilename << "\n";


  try
    {
        Controller controller;

		// Load ea acquired channel into ITK image structure
		controller.loadITKVolume(strT1VolumeFilename, FEATURE_VOLUME_TYPE_T1);
		controller.loadITKVolume(strT2VolumeFilename,  FEATURE_VOLUME_TYPE_T2);
		controller.loadITKVolume(strT1CVolumeFilename,  FEATURE_VOLUME_TYPE_T1C);
		controller.loadITKVolume(strFLAIRVolumeFilename,  FEATURE_VOLUME_TYPE_FLAIR);


		// Reorientation algorithm:
		// ==========================
		// 1. outputDirection=volumeT1.getDirection() // Assume T1 volume is the orientation in which to return the results 
		// 2. Reorient all volumes to the internal orientation, currently ARS
		// 3. Generate segmentation 
		// 4. Reorient back to OrigOrientation


		//typedef itk::Image<ushort, 3> ImageUShortVolumeType;
		////ImageUShortVolumeType::Pointer myPtr=ImageUShortVolumeType::New();
		//// 1. outputDirection=volumeT1.getDirection() // Assume T1 volume is the orientation in which to return the results 
		ImageUShortVolumeType::DirectionType outputDirection=controller.ITKFeatureVolume_[FEATURE_VOLUME_TYPE_T1]->GetDirection();
        ImageUShortVolumeType::PointType outputOrigin=controller.ITKFeatureVolume_[FEATURE_VOLUME_TYPE_T1]->GetOrigin();

		vector<int> listInputVolumes;
		listInputVolumes.clear();
		listInputVolumes.push_back(FEATURE_VOLUME_TYPE_T1);
		listInputVolumes.push_back(FEATURE_VOLUME_TYPE_T2);
		listInputVolumes.push_back(FEATURE_VOLUME_TYPE_T1C);
		listInputVolumes.push_back(FEATURE_VOLUME_TYPE_FLAIR);


		// 2. Reorient all volumes to the internal orientation, currently RAI
		typedef itk::OrientImageFilter<ImageUShortVolumeType,ImageUShortVolumeType> OrientImageFilterType;


		for (int i=0; i<listInputVolumes.size(); i++) {
			int nInputChannel=listInputVolumes[i];

			ImageUShortVolumeType::DirectionType currentDirection=controller.ITKFeatureVolume_[nInputChannel]->GetDirection();
			// ImageUShortVolumeType::Pointer beforeImage=controller.ITKFeatureVolume_[nInputChannel]; // debug
			if (bDebug) { std::cout << "Input volume " << arrstrFEATURE_VOLUME_TYPE[nInputChannel] << " has orientation:\n" << currentDirection << "\n"; };
			
			OrientImageFilterType::Pointer orienter=OrientImageFilterType::New(); // Note: need a New() filter for each loop iteration, or all input volumes will be pointing to the same one in the single OrientFilter.
			orienter->SetInput(controller.ITKFeatureVolume_[nInputChannel]);
			orienter->UseImageDirectionOn();
			orienter->SetDesiredCoordinateOrientation(itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI);// internal orientation
			orienter->Update();
			// ImageUShortVolumeType::Pointer afterImage=orienter->GetOutput();
			controller.ITKFeatureVolume_[nInputChannel]=orienter->GetOutput();
			ImageUShortVolumeType::PointType imageOriginReoriented;   imageOriginReoriented[0]=0; imageOriginReoriented[1]=0; imageOriginReoriented[2]=0;  
	        controller.ITKFeatureVolume_[nInputChannel]->SetOrigin( imageOriginReoriented ); // internal processing assumes origin [0,0,0]

			currentDirection=controller.ITKFeatureVolume_[nInputChannel]->GetDirection();
			if (bDebug) { std::cout << "  After reorientation volume " << arrstrFEATURE_VOLUME_TYPE[nInputChannel] << " has orientation:\n" << currentDirection << "\n"; };

			if (bDebug) {
				// Serialize volume
				typedef itk::ImageFileWriter< ImageUShortVolumeType > ImageWriterType;
				ImageWriterType::Pointer psImageWriter = ImageWriterType::New();
				// char cstrBuff[500]; sprintf(cstrBuff, "D:/MyCopy/NeuroImagingData/BRATS/DebugReorientation/%s_ReorientedToRAI.nii", arrstrFEATURE_VOLUME_TYPE[nInputChannel]);
				string strOutputFilename =string("D:/MyCopy/NeuroImagingData/BRATS/DebugReorientation/") + string(arrstrFEATURE_VOLUME_TYPE[nInputChannel]) + string("_ReorientedToRAI.nii");
				psImageWriter->SetFileName(strOutputFilename );
				psImageWriter->SetInput( controller.ITKFeatureVolume_[nInputChannel] );
				try
				{
				  psImageWriter->Update();
				} 
				catch (itk::ExceptionObject& exp)
				{
					std::cerr << "Exception caught!" << std::endl;
					std::cerr << exp << std::endl;
				}
			}


		}



        // 3. Generate segmentation
		controller.bEnvironmentAndForestSettingsAreInSameFolder_=true;  

		convertFilePath(strForestDescriptionFile);
		controller.strForestSettingsFilename=strForestDescriptionFile;
		controller.LoadSettings();

		// How to tell if this code is beging run as a DLL 
		//    CLPProcessInformation is a pointer: "struct ModuleProcessInformation*"   ,  It is provided from PARSE_ARGS when this is a DLL, otherwise it is false (0)

		//dFraction is the fraction of the overall job (0-1.0) that is being performed by the watched filter this->GetProcess(), which could be one of several filters
		double dFraction=1.0;
		//dStart is the fraction of the overall job that was completed before the watched object, this->GetProcess() started
		double dStart=0.0;
        float fFractionCompleted=0.1; 		string strMessage="Images and forest settings loaded.";

		//typedef itk::Image<unsigned short, 3> 	dummyInternalImageType;
  //      typedef itk::Image<float, 3> 	dummyOutputImageType;
		//typedef itk::CastImageFilter<dummyInternalImageType, dummyOutputImageType> CastingFilterType;
		//CastingFilterType::Pointer caster = CastingFilterType::New();  // caster used to convey progress to Slicer GUI

		// register filter as an observer
		itk::PluginFilterWatcher watcher1(caster, strMessage.c_str(), CLPProcessInformation, dFraction, dStart);  // smoothing obj smooths the image yielding another image, reports progress

		itk::StartEvent startEvent;
		

		// v0.0: Using this order, an exception was thrown: cannot stop an object that has not started
		// caster->UpdateProgress(0);
		// caster->InvokeEvent(startEvent);
		//
		// v0.1:  Now try this order:
		caster->InvokeEvent(startEvent);
		caster->UpdateProgress(0);

		// gPluginProcessWatcher.init(CLPProcessInformation, dFraction, dStart);
		// gPluginProcessWatcher.displayProgress(dFractionCompleted, strMessage);

        

		// Derive channels from ITKtest data then apply forest to it and write segmentation results
		int nReturn=controller.LoadForest_DeriveChannelsFromITKTest_ThenTestAndWriteOutput(strT1VolumeFilename, 
			strTissueSegmentationVolumeFilename, outputDirection, outputOrigin);

		fFractionCompleted=1.0;  
		if (nReturn!=EXIT_FAILURE) strMessage="Segmentation completed.";
		else strMessage="Error during segmentation.";
		// gPluginProcessWatcher.displayProgress(dFractionCompleted, strMessage);

		caster->UpdateProgress(fFractionCompleted);

		itk::EndEvent endEvent;
		caster->InvokeEvent(endEvent);



    }

  catch( itk::ExceptionObject & excep )
    {
    std::cerr << argv[0] << "ERROR: exception caught !" << std::endl;
    std::cerr << excep << std::endl;
    return EXIT_FAILURE;
    }
  return EXIT_SUCCESS;
}


void convertFilePath(string& strFilePath)
{
		
    int len=(int)strFilePath.size();
	for (int ii=0; ii<len; ii++) {
		if (strFilePath[ii]==cNonFolderSeparator)  strFilePath[ii]=cFolderSeparator;
		else strFilePath[ii]=strFilePath[ii];
	}
}
