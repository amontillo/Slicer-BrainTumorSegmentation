// This file contains Controller methods that use ITK

// decisionForest
#include "Controller.h"
#include "DecisionForestTester.h"
#include "FilenameManager.h"
#include "BvdFileManager.h"
#include "FileConverter.h"
#include "basicDefs.h"
//#include "AccuracyEvaluator.h"
#include "Dataset.h"

// Std stuff
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <iostream>               // for std::cout
// #include <boost/filesystem.hpp>   // Commented out to avoid building boost on every platform for now just to do a mkdir 
#ifdef WIN32
   #include <direct.h>   // for mkdir
#else
   #include <sys/stat.h> // for mkdir
#endif


using namespace std;
using boost::lexical_cast;



int Controller::loadITKVolume(string& strVolumeFilename, FEATURE_VOLUME_TYPE iFeatureVolumeType )
{   bool bReturn=true; 
	typedef itk::ImageFileReader<ImageUShortVolumeType>  ReaderType;
	ReaderType::Pointer reader = ReaderType::New();

	reader->SetFileName( strVolumeFilename );
	try 
	{ reader->Update();
	  ITKFeatureVolume_[iFeatureVolumeType]=reader->GetOutput();
	}
	catch (itk::ExceptionObject& err)
	{
		std::cout << "ExceptionObject caught!\n";
		std::cout << err << std::endl;
		bReturn=false;
	}

	return bReturn;
}


// this version transforms the outputs into RAI orientation from the internally used orientation
// Save GT or Auto segmentation labels to ITK volume
int Controller::saveByteVolumeToITK(ptrsmartDataset& psDataset, byte* labelVolume, 
									string& strTissueSegmentationVolumeFilename,
									ImageUShortVolumeType::DirectionType& outputDirection, 
									ImageUShortVolumeType::PointType& outputOrigin)
{
	// 1. Create in memory copy of the volume of segmentation output in an ITK image
	// 2. Write out to disk 
	VolumeDimensions volDims = psDataset->dims_;
    string strOutputFilenameLabel=strTissueSegmentationVolumeFilename;

	typedef itk::Image<byte, 3> LabelImageType;
	LabelImageType::Pointer psLabelImage=LabelImageType::New();

	LabelImageType::IndexType pixelIndexLabelImage;
	LabelImageType::PixelType pixelValueLabelImage;

	// Define and alloc an itk image for the feature volume 
	LabelImageType::IndexType startIndexLabel;  	startIndexLabel[0]=0; startIndexLabel[1]=0; startIndexLabel[2]=0;
	LabelImageType::SizeType  imageExtentLabel;   imageExtentLabel[0]=volDims.Height; 	imageExtentLabel[1]=volDims.Width; 	imageExtentLabel[2]=volDims.Depth;
	LabelImageType::RegionType imageRegionLabel;  imageRegionLabel.SetSize(imageExtentLabel); imageRegionLabel.SetIndex(startIndexLabel);
	LabelImageType::SpacingType spacingLabel;     spacingLabel[0]=psDataset->scales_[1]; spacingLabel[1]=psDataset->scales_[0]; spacingLabel[2]=psDataset->scales_[2]; 
	// Bug fix: use origin from input data specified as argument to this function 
	LabelImageType::PointType imageOriginLabel;   imageOriginLabel[0]=outputOrigin[0]; imageOriginLabel[1]=outputOrigin[1]; imageOriginLabel[2]=outputOrigin[2];  
	psLabelImage->SetRegions( imageRegionLabel);  psLabelImage->SetSpacing( spacingLabel );   psLabelImage->SetOrigin( imageOriginLabel );
	psLabelImage->Allocate();   			    
	psLabelImage->FillBuffer(0); 

	// Bug fix: internal orientation output is RAI
	// psGTImage->SetDirection(itk::SpatialOrientationAdapter().ToDirectionCosines(itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI)); // Set volume's orientation, use default RAI


	byte* PLab = labelVolume;                      // labels are stored as a flat 1D array (byte)
	byte* pLab = PLab;  // ptr to label		

	int maxDepth=volDims.Depth-1;
	for (int z0 = 0; z0 < volDims.Depth; z0++)
	{
		for (int y0 = 0; y0 < volDims.Height; y0++)
		{
			for (int x0 = 0; x0 < volDims.Width; x0++)
			{
				pixelValueLabelImage=*pLab;
				pixelIndexLabelImage[0]=y0; pixelIndexLabelImage[1]=x0; pixelIndexLabelImage[2]=maxDepth-z0;
				psLabelImage->SetPixel( pixelIndexLabelImage, pixelValueLabelImage );
				pLab++;
			}
		}
	}


	// Reorient the volume to the desired outputDirection
    typedef itk::OrientImageFilter<LabelImageType,LabelImageType> OrientImageFilterType;
	OrientImageFilterType::Pointer orienter=OrientImageFilterType::New();
	orienter->SetInput(psLabelImage);
	orienter->UseImageDirectionOn();
	orienter->SetDesiredCoordinateDirection(outputDirection);// original orientation direction cosines
	try
	{
	  orienter->Update();
	} 
	catch (itk::ExceptionObject& exp)
	{
		std::cerr << "Exception caught during reorientation of auto-segmentation image!" << std::endl;
		std::cerr << exp << std::endl;
		return EXIT_FAILURE;
	}
	psLabelImage=orienter->GetOutput();


	// Serialize volume
	typedef itk::ImageFileWriter< LabelImageType > LabelImageWriterType;
	LabelImageWriterType::Pointer psLabelImageWriter = LabelImageWriterType::New();
	// string strOutputFilenameLabel = FilenameManager::ChangeExtension(psDataset->gtStructuresFileName_, strOutputImageFileExtension); // e.g. strOutputImageFileExtension ".nii"
	psLabelImageWriter->SetFileName(strOutputFilenameLabel);
	psLabelImageWriter->SetInput( psLabelImage );
	try
	{
	  psLabelImageWriter->Update();
	} 
	catch (itk::ExceptionObject& exp)
	{
		std::cerr << "Exception caught!" << std::endl;
		std::cerr << exp << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;

}


// Save GT or Auto segmentation labels to ITK volume



//int Controller::saveByteVolumeToITK(ptrsmartDataset& psDataset, byte* labelVolume, 
//									string& strTissueSegmentationVolumeFilename,
//									ImageUShortVolumeType::DirectionType& outputDirection)
//{
//	// 1. Create in memory copy of the volume of segmentation output in an ITK image
//	// 2. Write out to disk 
//	VolumeDimensions volDims = psDataset->dims_;
//    string strOutputFilenameLabel=strTissueSegmentationVolumeFilename;
//
//	typedef itk::Image<byte, 3> LabelImageType;
//	LabelImageType::Pointer psLabelImage=LabelImageType::New();
//
//	LabelImageType::IndexType pixelIndexLabelImage;
//	LabelImageType::PixelType pixelValueLabelImage;
//
//	// Define and alloc an itk image for the feature volume 
//	LabelImageType::IndexType startIndexLabel;  	startIndexLabel[0]=0; startIndexLabel[1]=0; startIndexLabel[2]=0;
//	LabelImageType::SizeType  imageExtentLabel;   imageExtentLabel[0]=volDims.Width; 	imageExtentLabel[1]=volDims.Height; 	imageExtentLabel[2]=volDims.Depth;
//	LabelImageType::RegionType imageRegionLabel;  imageRegionLabel.SetSize(imageExtentLabel); imageRegionLabel.SetIndex(startIndexLabel);
//	LabelImageType::SpacingType spacingLabel;     spacingLabel[0]=psDataset->scales_[0]; spacingLabel[1]=psDataset->scales_[1]; spacingLabel[2]=psDataset->scales_[2]; 
//	LabelImageType::PointType imageOriginLabel;   imageOriginLabel[0]=0; imageOriginLabel[1]=0; imageOriginLabel[2]=0;  
//	psLabelImage->SetRegions( imageRegionLabel);  psLabelImage->SetSpacing( spacingLabel );   psLabelImage->SetOrigin( imageOriginLabel );
//	psLabelImage->Allocate();   			    
//	psLabelImage->FillBuffer(0); 
//
//	// Bug fix: internal orientation output is RAI
//	// psLabelImage->SetDirection(itk::SpatialOrientationAdapter().ToDirectionCosines(itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_RAI)); // Set volume's orientation
//
//
//	byte* PLab = labelVolume;                      // labels are stored as a flat 1D array (byte)
//	byte* pLab = PLab;  // ptr to label		
//
//	for (int z0 = 0; z0 < volDims.Depth; z0++)
//	{
//		for (int y0 = 0; y0 < volDims.Height; y0++)
//		{
//			for (int x0 = 0; x0 < volDims.Width; x0++)
//			{
//				pixelValueLabelImage=*pLab;
//				pixelIndexLabelImage[0]=x0; pixelIndexLabelImage[1]=y0; pixelIndexLabelImage[2]=z0;
//				psLabelImage->SetPixel( pixelIndexLabelImage, pixelValueLabelImage );
//				pLab++;
//			}
//		}
//	}
//
//
//	// Reorient the volume to the desired outputDirection
//    typedef itk::OrientImageFilter<LabelImageType,LabelImageType> OrientImageFilterType;
//	OrientImageFilterType::Pointer orienter=OrientImageFilterType::New();
//	orienter->SetInput(psLabelImage);
//	orienter->UseImageDirectionOn();
//	orienter->SetDesiredCoordinateDirection(outputDirection);// original orientation direction cosines
//	try
//	{
//	  orienter->Update();
//	} 
//	catch (itk::ExceptionObject& exp)
//	{
//		std::cerr << "Exception caught during reorientation of auto-segmentation image!" << std::endl;
//		std::cerr << exp << std::endl;
//		return EXIT_FAILURE;
//	}
//	psLabelImage=orienter->GetOutput();
//
//
//	// Serialize volume
//	typedef itk::ImageFileWriter< LabelImageType > LabelImageWriterType;
//	LabelImageWriterType::Pointer psLabelImageWriter = LabelImageWriterType::New();
//	// string strOutputFilenameLabel = FilenameManager::ChangeExtension(psDataset->gtStructuresFileName_, strOutputImageFileExtension); // e.g. strOutputImageFileExtension ".nii"
//	psLabelImageWriter->SetFileName(strOutputFilenameLabel);
//	psLabelImageWriter->SetInput( psLabelImage );
//	try
//	{
//	  psLabelImageWriter->Update();
//	} 
//	catch (itk::ExceptionObject& exp)
//	{
//		std::cerr << "Exception caught!" << std::endl;
//		std::cerr << exp << std::endl;
//		return EXIT_FAILURE;
//	}
//
//	return EXIT_SUCCESS;
//
//}
//
//
//

// returns 0 upon success and 1 upon failure
int Controller::LoadForest_DeriveChannelsFromITKTest_ThenTestAndWriteOutput(string& strT1VolumeFilename,
																			string& strTissueSegmentationVolumeFilename,
																			ImageUShortVolumeType::DirectionType& outputDirection, 
																			ImageUShortVolumeType::PointType& outputOrigin)
{
    if (FOREST_PARAMETERS_LOADED_)
    {
        bool bForestLoaded = LoadForest();
        if (bForestLoaded)  
        {
			if (!TESTING_DATASETS_LOADED_) loadOneCommandLineSpecifiedDataset();  // Load the commandline specified dataset
			if (TESTING_DATASETS_LOADED_) 
			{
				// Apply forest to segment comamnd line dataset  and serialize the output to volume files 
				// Loading all scans ---------------------------------------------------------------
				if (!forestInMemory()) { cout << "Error: Need to load the decision forest first.\n";  return EXIT_FAILURE; };
				if (!TESTING_DATASETS_LOADED_) { cout << "Error: Need to load test dataset first.\n";  return EXIT_FAILURE; };

				string outputFolder = FilenameManager::ExtractDirectory(forestParams_.strForestParametersFilename_);
				ptrsmartDecisionForestTester psForestTester(new DecisionForestTester(decisionForest_, forestParams_, structEnvironmentParameters_, auxDecisionForest_));   // Instantiating the forest tester class

				cout << " ********************** " + strT1VolumeFilename + " ********************** \n";

				// generate the predictions
			   psForestTester->parallelTest(testingDatasets_[0], outputFolder);


			   bool bSaveAutoLabels=true; 

				if (bSaveAutoLabels)
				{
					// Save segmentation output to binary volume file(s)

					// 3. write labels out
					saveByteVolumeToITK(testingDatasets_[0], psForestTester->outClassLabels_, strTissueSegmentationVolumeFilename, outputDirection, outputOrigin);

					
					displayStatemachineStatus();
				}				

			}
			else
			{
				cout << "Error: Could not load test data.\n";
				return EXIT_FAILURE;
			}
        }
        else
        {
           cout << "Error: Could not load trained forest\n";
		   return EXIT_FAILURE;
        }
                    
    }
    else
    {
       cout << "Error: settings not loaded.\n";
	   return EXIT_FAILURE;
    }

	return EXIT_SUCCESS;
}
  
