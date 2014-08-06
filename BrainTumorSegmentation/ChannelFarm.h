// Each function converts one itk image into another itk image with specific channel characteristics, e.g. mode shifted (intensity normalized), gradient, LBP
#include "FeatureVolumeType.h"
#include "itkScalarImageToHistogramGenerator.h"
#include "itkImage.h"
#include "itkImageRegionIterator.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include <itkCastImageFilter.h>
#include "itkImageDuplicator.h"
#include "itkRandomImageSource.h"
#include <climits>
#include <cfloat>
#include <string.h>
#include "math.h"
#include "basicDefs.h"

#define BOUND(x, lowerbound, upperbound)  { (x) = (x) > (lowerbound) ? (x) : (lowerbound); \
    (x) = (x) < (upperbound) ? (x) : (upperbound); };

#define POW(nBit)   (1 << (nBit))



struct ChannelFarmManager  // space of functions that derive channels from acquired intensity volumes
{
    static const int nNumBins=60;


    // returns the bin number (using 1-based index) that given value resides within. Each bin is defined by its min and max edge (given array).
	// If value is < min edge or > max edge then 0 is returned
	// Mimics Matlab [~,bin]=histc(arrayToProcess, edges)
	// Note: The edges can be those provided by this class, eg. ChannelFarmManager::linspace0_700
	static unsigned int GetBinFromValue(float* edges, const float value) 
	{
	  // Mimic matlab histc
	  // If the value is lower than any of min value in the Histogram, return 0
	  if ( value < edges[0] ) return 0;
	  // If the value is > max edge, return 0
	  if ( value > edges[ChannelFarmManager::nNumBins-1] ) return 0;
	    

	  // If the value is equal to max edge, return highest bin, use 1-based index
	  if ( value == edges[ChannelFarmManager::nNumBins-1] ) return ChannelFarmManager::nNumBins;


	  int binMinFromValue = 0;
	  for ( unsigned int i = 0; i < (ChannelFarmManager::nNumBins-1); i++ )
		{
		if ( ( value >= edges[i] )
			 && ( value <  edges[i+1] ) )
		  {
			  binMinFromValue = i+1;  // use 1 based indexing like matlab histc
			  break;
		  }
		}

	  return binMinFromValue;
	}

    
	// For given volume, compute 2D image gradient along image plane, gradImagePlane, then quantize to normalize the result
	static ImageUShortVolumeType::Pointer gradientQuantized(ImageUShortVolumeType::Pointer psImageVolume, 
		   int featureVolumeEnumBaseChannel, const string& gradImagePlane, string& strDerivedChannelName)
	 {
		 bool bDebug=false;
         string strDebugOutputFilenameFolder = "D:\\dev\\decisionForest\\LesionSeg\\decisionForestSolnTrunk\\OneFoldWorkingVS2008_withITK\\testDebug\\";
		 string strDebugOutputFilename;

		 // equivalent to matlab linspace(0,700,60)
			float linspace0_700[60]={
						 0,   11.8644,   23.7288,   35.5932,   47.4576,   59.3220,   71.1864,   83.0508,   94.9153,
				  106.7797,  118.6441,  130.5085,  142.3729,  154.2373,  166.1017,  177.9661,  189.8305,  201.6949,
				  213.5593,  225.4237,  237.2881,  249.1525,  261.0169,  272.8814,  284.7458,  296.6102,  308.4746,
				  320.3390,  332.2034,  344.0678,  355.9322,  367.7966,  379.6610,  391.5254,  403.3898,  415.2542,
				  427.1186,  438.9831,  450.8475,  462.7119,  474.5763,  486.4407,  498.3051,  510.1695,  522.0339,
				  533.8983,  545.7627,  557.6271,  569.4915,  581.3559,  593.2203,  605.0847,  616.9492,  628.8136,
				  640.6780,  652.5424,  664.4068,  676.2712,  688.1356,  700.0000							  
					};


		 // equivalent to matlab linspace(0,500,60)
			float linspace0_500[60]={
						 0,    8.4746,   16.9492,   25.4237,   33.8983,   42.3729,   50.8475,   59.3220,   67.7966,
				   76.2712,   84.7458,   93.2203,  101.6949,  110.1695,  118.6441,  127.1186,  135.5932,  144.0678,
				  152.5424,  161.0169,  169.4915,  177.9661,  186.4407,  194.9153,  203.3898,  211.8644,  220.3390,
				  228.8136,  237.2881,  245.7627,  254.2373,  262.7119,  271.1864,  279.6610,  288.1356,  296.6102,
				  305.0847,  313.5593,  322.0339,  330.5085,  338.9831,  347.4576,  355.9322,  364.4068,  372.8814,
				  381.3559,  389.8305,  398.3051,  406.7797,  415.2542,  423.7288,  432.2034,  440.6780,  449.1525,
				  457.6271,  466.1017,  474.5763,  483.0508,  491.5254,  500.0000
					};                                            
		    
			/* K>> format long
		K>> linspace(0,700,60)

		ans =

		   1.0e+02 *

		  Columns 1 through 4

						   0   0.118644067796610   0.237288135593220   0.355932203389831

		  Columns 5 through 8

		   0.474576271186441   0.593220338983051   0.711864406779661   0.830508474576271

		  Columns 9 through 12

		   0.949152542372881   1.067796610169492   1.186440677966102   1.305084745762712

		  Columns 13 through 16

		   1.423728813559322   1.542372881355932   1.661016949152542   1.779661016949153

		  Columns 17 through 20

		   1.898305084745763   2.016949152542373   2.135593220338983   2.254237288135593

		  Columns 21 through 24

		   2.372881355932203   2.491525423728814   2.610169491525424   2.728813559322034

		  Columns 25 through 28

		   2.847457627118644   2.966101694915254   3.084745762711864   3.203389830508475

		  Columns 29 through 32

		   3.322033898305085   3.440677966101695   3.559322033898305   3.677966101694915

		  Columns 33 through 36

		   3.796610169491526   3.915254237288136   4.033898305084746   4.152542372881356

		  Columns 37 through 40

		   4.271186440677966   4.389830508474576   4.508474576271187   4.627118644067797

		  Columns 41 through 44

		   4.745762711864407   4.864406779661017   4.983050847457627   5.101694915254237

		  Columns 45 through 48

		   5.220338983050848   5.338983050847458   5.457627118644068   5.576271186440677

		  Columns 49 through 52

		   5.694915254237288   5.813559322033898   5.932203389830509   6.050847457627119

		  Columns 53 through 56

		   6.169491525423728   6.288135593220339   6.406779661016949   6.525423728813560

		  Columns 57 through 60

		   6.644067796610170   6.762711864406779   6.881355932203389   7.000000000000000

		   K>> linspace(0,500,60)

		ans =

		   1.0e+02 *

		  Columns 1 through 4

						   0   0.084745762711864   0.169491525423729   0.254237288135593

		  Columns 5 through 8

		   0.338983050847458   0.423728813559322   0.508474576271186   0.593220338983051

		  Columns 9 through 12

		   0.677966101694915   0.762711864406780   0.847457627118644   0.932203389830509

		  Columns 13 through 16

		   1.016949152542373   1.101694915254237   1.186440677966102   1.271186440677966

		  Columns 17 through 20

		   1.355932203389831   1.440677966101695   1.525423728813559   1.610169491525424

		  Columns 21 through 24

		   1.694915254237288   1.779661016949153   1.864406779661017   1.949152542372882

		  Columns 25 through 28

		   2.033898305084746   2.118644067796610   2.203389830508475   2.288135593220339

		  Columns 29 through 32

		   2.372881355932203   2.457627118644068   2.542372881355932   2.627118644067797

		  Columns 33 through 36

		   2.711864406779661   2.796610169491526   2.881355932203390   2.966101694915254

		  Columns 37 through 40

		   3.050847457627119   3.135593220338983   3.220338983050847   3.305084745762712

		  Columns 41 through 44

		   3.389830508474576   3.474576271186440   3.559322033898305   3.644067796610170

		  Columns 45 through 48

		   3.728813559322034   3.813559322033898   3.898305084745763   3.983050847457627

		  Columns 49 through 52

		   4.067796610169491   4.152542372881356   4.237288135593221   4.322033898305085

		  Columns 53 through 56

		   4.406779661016949   4.491525423728813   4.576271186440678   4.661016949152542

		  Columns 57 through 60

		   4.745762711864407   4.830508474576272   4.915254237288136   5.000000000000000*/

		 // =======================================================
         // Compute gradient magnitude image (float)
		 // =======================================================
		 ImageUShortVolumeType::RegionType region=psImageVolume->GetLargestPossibleRegion();
		 ImageUShortVolumeType::SizeType imageExtents=region.GetSize();
		 ImageUShortVolumeType::IndexType imageIndex=region.GetIndex();
		 ImageUShortVolumeType::SpacingType imageSpacing=psImageVolume->GetSpacing();
		 ImageUShortVolumeType::PointType imageOrigin=psImageVolume->GetOrigin();
         ImageUShortVolumeType::DirectionType imageDirection=psImageVolume->GetDirection();

		 ImageUShortVolumeType::IndexType pixelIndex0;
		 ImageUShortVolumeType::IndexType pixelIndex1;
		 ImageUShortVolumeType::IndexType pixelIndex2;


        // Partial gradients volumes
		typedef itk::Image<int, 3> ImageIntType;
		
		typedef itk::CastImageFilter<ImageUShortVolumeType, ImageIntType > CastToIntFilter;
		CastToIntFilter::Pointer toIntCaster = CastToIntFilter::New();
		//DuplicatorType::Pointer duplicatorX = DuplicatorType::New();
		//duplicatorX->SetInputImage(psImageVolume);
		//duplicatorX->Update();
		//ImageUShortVolumeType::Pointer psOutImageX = duplicatorX->GetOutput();
		toIntCaster ->SetInput( psImageVolume );
		toIntCaster ->Update();
		ImageIntType::Pointer psOutImageX=toIntCaster ->GetOutput();
		psOutImageX->FillBuffer(0);

		typedef itk::ImageDuplicator< ImageIntType > DuplicatorTypeInt;
		DuplicatorTypeInt::Pointer duplicatorInt = DuplicatorTypeInt::New();
		duplicatorInt->SetInputImage(psOutImageX);
		duplicatorInt->Update();
		ImageIntType::Pointer psOutImageY = duplicatorInt->GetOutput();

		ImageIntType::PixelType pixelValueA;
		ImageIntType::PixelType pixelValueB;

        // gradient magnitude volume
		typedef itk::Image<float, 3> ImageFloatVolumeType;
		ImageFloatVolumeType::Pointer psOutImageGradMag=ImageFloatVolumeType::New();
		ImageFloatVolumeType::IndexType startIndexFloat;  	startIndexFloat[0]=imageIndex[0]; startIndexFloat[1]=imageIndex[1]; startIndexFloat[2]=imageIndex[2];
		ImageFloatVolumeType::SizeType  imageExtentFloat;   imageExtentFloat[0]=imageExtents[0]; 	imageExtentFloat[1]=imageExtents[1]; 	imageExtentFloat[2]=imageExtents[2];
		ImageFloatVolumeType::RegionType imageRegionFloat;  imageRegionFloat.SetSize(imageExtentFloat); imageRegionFloat.SetIndex(startIndexFloat);
		ImageFloatVolumeType::SpacingType spacingFloat;     spacingFloat[0]=imageSpacing[0]; spacingFloat[1]=imageSpacing[1]; spacingFloat[2]=imageSpacing[2]; 
		ImageFloatVolumeType::PointType imageOriginFloat;   imageOriginFloat[0]=imageOrigin[0]; imageOriginFloat[1]=imageOrigin[1]; imageOriginFloat[2]=imageOrigin[2];  
		psOutImageGradMag->SetRegions( imageRegionFloat);  psOutImageGradMag->SetSpacing( spacingFloat );   psOutImageGradMag->SetOrigin( imageOriginFloat );
		psOutImageGradMag->Allocate();   			    
		psOutImageGradMag->FillBuffer(0); 
		psOutImageGradMag->SetDirection(imageDirection);
		// psOutImageGradMag->SetDirection(itk::SpatialOrientationAdapter().ToDirectionCosines(itk::SpatialOrientation::ITK_COORDINATE_ORIENTATION_ARS)); // Set volume's orientation

		 if (gradImagePlane=="xy")
		 {

			for (int z0 = 1; z0 < (imageExtents[2]-1); z0++)
			{
				for (int y0 = 1; y0 < (imageExtents[1]-1); y0++)
				{
					for (int x0 = 1; x0 < (imageExtents[0]-1); x0++)
					{
						pixelIndex0[0]=x0; pixelIndex0[1]=y0; pixelIndex0[2]=z0;

						// a=kernel1 sum of products from psImageVolume;
						pixelIndex1[0]=x0-1; pixelIndex1[1]=y0; pixelIndex1[2]=z0;
						pixelIndex2[0]=x0+1; pixelIndex2[1]=y0; pixelIndex2[2]=z0;
						pixelValueA=((int)(psImageVolume->GetPixel(pixelIndex2))) - ((int)(psImageVolume->GetPixel(pixelIndex1)));

						// b=kernel2 sum of products from psImageVolume;
						pixelIndex1[0]=x0; pixelIndex1[1]=y0-1; pixelIndex1[2]=z0;
						pixelIndex2[0]=x0; pixelIndex2[1]=y0+1; pixelIndex2[2]=z0;
						pixelValueB=((int)(psImageVolume->GetPixel(pixelIndex2))) - ((int)(psImageVolume->GetPixel(pixelIndex1)));

						psOutImageX->SetPixel(pixelIndex0, pixelValueA);
						psOutImageY->SetPixel(pixelIndex0, pixelValueB);
						double aDouble=(double)pixelValueA;
						double bDouble=(double)pixelValueB;
						psOutImageGradMag->SetPixel(pixelIndex0, (float)(sqrt(aDouble*aDouble + bDouble*bDouble)));
					}
				}
			}

		 }
		 else if (gradImagePlane=="xz") // Here we mimic orthoginalHOG.m
		 {
			for (int z0 = 1; z0 < (imageExtents[2]-1); z0++)
			{
				for (int y0 = 1; y0 < (imageExtents[1]-1); y0++)
				{
					for (int x0 = 1; x0 < (imageExtents[0]-1); x0++)
					{
						pixelIndex0[0]=x0; pixelIndex0[1]=y0; pixelIndex0[2]=z0;

						// a=kernel1 sum of products from psImageVolume;
						pixelIndex1[0]=x0-1; pixelIndex1[1]=y0; pixelIndex1[2]=z0;
						pixelIndex2[0]=x0+1; pixelIndex2[1]=y0; pixelIndex2[2]=z0;
						pixelValueA=((int)(psImageVolume->GetPixel(pixelIndex2))) - ((int)(psImageVolume->GetPixel(pixelIndex1)));

						// b=kernel2 sum of products from psImageVolume;
						pixelIndex1[0]=x0; pixelIndex1[1]=y0; pixelIndex1[2]=z0-1;
						pixelIndex2[0]=x0; pixelIndex2[1]=y0; pixelIndex2[2]=z0+1;
						pixelValueB=((int)(psImageVolume->GetPixel(pixelIndex2))) - ((int)(psImageVolume->GetPixel(pixelIndex1)));

						psOutImageX->SetPixel(pixelIndex0, pixelValueA);
						psOutImageY->SetPixel(pixelIndex0, pixelValueB);
						double aDouble=(double)pixelValueA;
						double bDouble=(double)pixelValueB;
						psOutImageGradMag->SetPixel(pixelIndex0, (float)(sqrt(aDouble*aDouble + bDouble*bDouble)));
					}
				}
			}
		 }
		 else if (gradImagePlane=="yz")  // AAM: Here we mimic orthoginalHOG.m 
		 {
			for (int z0 = 1; z0 < (imageExtents[2]-1); z0++)
			{
				for (int y0 = 1; y0 < (imageExtents[1]-1); y0++)
				{
					for (int x0 = 1; x0 < (imageExtents[0]-1); x0++)
					{
						pixelIndex0[0]=x0; pixelIndex0[1]=y0; pixelIndex0[2]=z0;

						// a=kernel1 sum of products from psImageVolume;
						pixelIndex1[0]=x0; pixelIndex1[1]=y0-1; pixelIndex1[2]=z0;
						pixelIndex2[0]=x0; pixelIndex2[1]=y0+1; pixelIndex2[2]=z0;
						pixelValueA=((int)(psImageVolume->GetPixel(pixelIndex2))) - ((int)(psImageVolume->GetPixel(pixelIndex1)));

						// b=kernel2 sum of products from psImageVolume;
						pixelIndex1[0]=x0; pixelIndex1[1]=y0; pixelIndex1[2]=z0-1;
						pixelIndex2[0]=x0; pixelIndex2[1]=y0; pixelIndex2[2]=z0+1;
						pixelValueB=((int)(psImageVolume->GetPixel(pixelIndex2))) - ((int)(psImageVolume->GetPixel(pixelIndex1)));

						psOutImageX->SetPixel(pixelIndex0, pixelValueA);
						psOutImageY->SetPixel(pixelIndex0, pixelValueB);
						double aDouble=(double)pixelValueA;
						double bDouble=(double)pixelValueB;
						psOutImageGradMag->SetPixel(pixelIndex0, (float)(sqrt(aDouble*aDouble + bDouble*bDouble)));
					}
				}
			}
		 }


		 
if (bDebug)
{
        // DEBUG, find min & max value of psOutImageX
		typedef itk::ImageRegionIterator<ImageIntType> ImageIteratorTypeInt;
		ImageIteratorTypeInt::RegionType regionOutImageX=psOutImageX->GetBufferedRegion();
		ImageIteratorTypeInt itOutImageX(psOutImageX,regionOutImageX);  
		int myOutImageXMin=INT_MAX;
		int myOutImageXMax=INT_MIN;
		for (itOutImageX.GoToBegin(); !itOutImageX.IsAtEnd(); ++itOutImageX)
		{
          int pixelValue=itOutImageX.Get();
		  if (pixelValue>myOutImageXMax) myOutImageXMax=pixelValue;
		  if (pixelValue<myOutImageXMin) myOutImageXMin=pixelValue;
		}
            // DEBUG, dump out image
			typedef itk::ImageFileWriter< ImageIntType > ImageIntWriterType;
			ImageIntWriterType::Pointer psImageIntWriter = ImageIntWriterType::New();
			strDebugOutputFilename = strDebugOutputFilenameFolder  + strDerivedChannelName + "_intermediateOutImageX" + ".nii";
			psImageIntWriter->SetFileName(strDebugOutputFilename);
			psImageIntWriter->SetInput( psOutImageX );
			try { 
               psImageIntWriter->Update();
			} 
			catch (itk::ExceptionObject& exp) {
			 	std::cerr << "Exception caught!" << std::endl;
				std::cerr << exp << std::endl;
			}


		// DEBUG, find min & max value of psOutImageY
		ImageIteratorTypeInt::RegionType regionOutImageY=psOutImageY->GetBufferedRegion();
		ImageIteratorTypeInt itOutImageY(psOutImageY,regionOutImageY);  
		int myOutImageYMin=INT_MAX;
		int myOutImageYMax=INT_MIN;
		for (itOutImageY.GoToBegin(); !itOutImageY.IsAtEnd(); ++itOutImageY)
		{
          int pixelValue=itOutImageY.Get();
		  if (pixelValue>myOutImageYMax) myOutImageYMax=pixelValue;
		  if (pixelValue<myOutImageYMin) myOutImageYMin=pixelValue;
		}
            // DEBUG, dump out image
			strDebugOutputFilename = strDebugOutputFilenameFolder  + strDerivedChannelName + "_intermediateOutImageY" + ".nii";
			psImageIntWriter->SetFileName(strDebugOutputFilename);
			psImageIntWriter->SetInput( psOutImageY );
			try { 
               psImageIntWriter->Update();
			} 
			catch (itk::ExceptionObject& exp) {
			 	std::cerr << "Exception caught!" << std::endl;
				std::cerr << exp << std::endl;
			}

		// DEBUG: find grad mag volume min & max and verify correct
		typedef itk::ImageRegionIterator<ImageFloatVolumeType> ImageIteratorTypeFloat;
		ImageFloatVolumeType::RegionType regionGradMag=psOutImageGradMag->GetBufferedRegion();
		ImageIteratorTypeFloat itGradMag(psOutImageGradMag,regionGradMag);       
		double dGradMagMin=FLT_MAX ;
		double dGradMagMax=-1.0 ;  // grad mag is always >=0
		for (itGradMag.GoToBegin(); !itGradMag.IsAtEnd(); ++itGradMag)
		{
          float pixelValueGradMag=itGradMag.Get();
		  if (pixelValueGradMag>dGradMagMax) dGradMagMax=pixelValueGradMag;
		  if (pixelValueGradMag<dGradMagMin) dGradMagMin=pixelValueGradMag;
		}
            // DEBUG, dump out image
			typedef itk::ImageFileWriter< ImageFloatVolumeType > ImageFloatWriterType;
			ImageFloatWriterType::Pointer psImageFloatWriter = ImageFloatWriterType::New();
			strDebugOutputFilename = strDebugOutputFilenameFolder  + strDerivedChannelName + "_intermediateOutImageGradMag" + ".nii";
			psImageFloatWriter->SetFileName(strDebugOutputFilename);
			psImageFloatWriter->SetInput( psOutImageGradMag );
			try { 
               psImageFloatWriter->Update();
			} 
			catch (itk::ExceptionObject& exp) {
			 	std::cerr << "Exception caught!" << std::endl;
				std::cerr << exp << std::endl;
			}

}

        // =======================================================
        // Quantize gradient magnitude image into 0-60 bins
		//    ITs possible the source matlab code should have produced 0-59 (to correspond to LBP uniform codes (rotationally invariant), hwoever it did produce 0-60 so this code does too.
		//    Note bin 0 means out of range (just like matlab histc)
		// =======================================================
		 float* edges=linspace0_700;
		 switch  (featureVolumeEnumBaseChannel)
		 {
			case FEATURE_VOLUME_TYPE_T1: edges=   linspace0_500; break;
			case FEATURE_VOLUME_TYPE_T2: edges=   linspace0_700; break;
			case FEATURE_VOLUME_TYPE_T1C: edges=  linspace0_700; break;
			case FEATURE_VOLUME_TYPE_FLAIR: edges=linspace0_500; break;
		 };

		// Quantize the gradiment magnitude
		// Make a copy of original image to transfer all properties, then fill with zeros 
		typedef itk::ImageDuplicator< ImageUShortVolumeType > DuplicatorTypeUshort;
		DuplicatorTypeUshort::Pointer duplicatorUshort = DuplicatorTypeUshort::New();
		duplicatorUshort->SetInputImage(psImageVolume);
		duplicatorUshort->Update();
		ImageUShortVolumeType::Pointer psDerivedChannel = duplicatorUshort->GetOutput();
		psDerivedChannel->FillBuffer(0);
		
		typedef itk::ImageRegionIterator<ImageUShortVolumeType> ImageIteratorTypeUShort;
		ImageUShortVolumeType::RegionType regionIO=psImageVolume->GetBufferedRegion();

		typedef itk::ImageRegionIterator<ImageFloatVolumeType> ImageIteratorTypeFloat;
		ImageIteratorTypeFloat itGradMag2(psOutImageGradMag, regionIO);
        ImageIteratorTypeUShort itDerived(psDerivedChannel,regionIO);

		for (itGradMag2.GoToBegin(), itDerived.GoToBegin(); !itGradMag2.IsAtEnd() && !itDerived.IsAtEnd(); ++itGradMag2, ++itDerived)
		{
          itDerived.Set(ChannelFarmManager::GetBinFromValue(edges,itGradMag2.Get()));
		}

if (bDebug) {
        // DEBUG, find min & max value of psDerivedChannel
		unsigned int myDerivedChannelMin=65535;
		unsigned int myDerivedChannelMax=0;
		for (itDerived.GoToBegin(); !itDerived.IsAtEnd(); ++itDerived)
		{
          unsigned short pixelValue=itDerived.Get();
		  if (pixelValue>myDerivedChannelMax) myDerivedChannelMax=pixelValue;
		  if (pixelValue<myDerivedChannelMin) myDerivedChannelMin=pixelValue;
		}
            // DEBUG, dump out image
			typedef itk::ImageFileWriter< ImageUShortVolumeType > ImageUShortWriterType;
			ImageUShortWriterType::Pointer psImageUShortWriter = ImageUShortWriterType::New();
			strDebugOutputFilename = strDebugOutputFilenameFolder  + strDerivedChannelName + ".nii";
			psImageUShortWriter->SetFileName(strDebugOutputFilename);
			psImageUShortWriter->SetInput( psDerivedChannel );
			try { 
               psImageUShortWriter->Update();
			} 
			catch (itk::ExceptionObject& exp) {
			 	std::cerr << "Exception caught!" << std::endl;
				std::cerr << exp << std::endl;
			}
}

       return psDerivedChannel;
	 }

	static ImageUShortVolumeType::Pointer modeShift(ImageUShortVolumeType::Pointer psImageVolume, int featureVolumeEnum, string& strDerivedChannelName)
	{
        bool bDebug=false;
        string strDebugOutputFilenameFolder = "D:\\dev\\decisionForest\\LesionSeg\\decisionForestSolnTrunk\\OneFoldWorkingVS2008_withITK\\testDebug\\";
		string strDebugOutputFilename;

	    // Find the max image value so we can set the histogram parameters
		typedef itk::ImageRegionIterator<ImageUShortVolumeType> ImageIteratorType;
		ImageUShortVolumeType::RegionType region=psImageVolume->GetBufferedRegion();

		ImageIteratorType it1(psImageVolume,region);
        
		int myMin=65535;
		int myMax=0;

		for (it1.GoToBegin(); !it1.IsAtEnd(); ++it1)
		{
          unsigned short pixelValue=it1.Get();
		  if (pixelValue>myMax) myMax=pixelValue;
		  if (pixelValue<myMin) myMin=pixelValue;
		}

	  typedef itk::Statistics::ScalarImageToHistogramGenerator<
									 ImageUShortVolumeType >   HistogramGeneratorType;

	  HistogramGeneratorType::Pointer histogramGenerator =
											HistogramGeneratorType::New();
	  histogramGenerator->SetInput(  psImageVolume );

      // Using this approach I would need to add 1 to the resulting bin
	  //histogramGenerator->SetNumberOfBins( myMax );  // USHRT_MAX = 65535  typically
	  //histogramGenerator->SetMarginalScale( 1.0 );
	  //histogramGenerator->SetHistogramMin(  -0.5 );
	  //histogramGenerator->SetHistogramMax( ((double)(myMax)) + 0.5 );


	  histogramGenerator->SetNumberOfBins( myMax+1 );  // USHRT_MAX = 65535  typically
	  histogramGenerator->SetMarginalScale( 1.0 );
	  histogramGenerator->SetHistogramMin(  -0.5 );
	  histogramGenerator->SetHistogramMax( ((double)(myMax)) + 0.5 );

	  histogramGenerator->Compute();

	  // The resulting histogram can be obtained from the generator by invoking its
	  // \code{GetOutput()} method. It is also convenient to get the Histogram type
	  // from the traits of the generator type itself as shown in the code below.
	  typedef HistogramGeneratorType::HistogramType  HistogramType;

	  const HistogramType * histogram = histogramGenerator->GetOutput();


	  const unsigned int histogramSize = histogram->Size();
      if (bDebug) std::cout << "Histogram size " << histogramSize << std::endl;

	  // find largest count > 20 
	  unsigned int bin;

	  unsigned int nStartIntensity=21;  // dont want to count the background around the brain as the mode, so we dont start from 0, assumption is that 21 is sufficient to avoid background
	  unsigned int nModeIntensity=nStartIntensity;
	  unsigned int nModeFrequency=0;
	  unsigned int nCurrentFrequency;

	  for( bin=nStartIntensity; bin < histogramSize; bin++ )
		{
			nCurrentFrequency= histogram->GetFrequency( bin, 0 );
			if (nCurrentFrequency>nModeFrequency)
			{  nModeIntensity=bin;
			   nModeFrequency=nCurrentFrequency;
			}

			//if (bDebug)
			//{ 
			//  std::cout << "bin = " << bin << " frequency = ";
			//  std::cout <<  nCurrentFrequency << std::endl;
			//}
		}

		// Figure out what acquisition source was for this image, so we can add the appropriate pedastal value. To some degree this is for backwards compatibility (better pedastal values may be appropriate)
		unsigned nShift=0;

		switch (featureVolumeEnum)
		{
			case FEATURE_VOLUME_TYPE_T1m: nShift=2000; break;
			case FEATURE_VOLUME_TYPE_T2m: nShift=1000; break;
			case FEATURE_VOLUME_TYPE_T1Cm: nShift=2000; break;
			case FEATURE_VOLUME_TYPE_FLAIRm: nShift=1000; break;
		};

        // Copy the image, subtract mode & add shift
		typedef itk::ImageDuplicator< ImageUShortVolumeType > DuplicatorType;
		DuplicatorType::Pointer duplicator = DuplicatorType::New();
		duplicator->SetInputImage(psImageVolume);
		duplicator->Update();
		ImageUShortVolumeType::Pointer psDerivedChannel = duplicator->GetOutput();


		
		//typedef itk::ImageRegionIterator<ImageUShortVolumeType> ImageIteratorType;
		//ImageUShortVolumeType::RegionType region=psImageVolume->GetBufferedRegion();

		//ImageIteratorType it1(psImageVolume,region);
        ImageIteratorType it2(psDerivedChannel,region);

		for (it1.GoToBegin(), it2.GoToBegin(); !it1.IsAtEnd() && !it2.IsAtEnd(); ++it1, ++it2)
		{
          it2.Set(it1.Get() - nModeIntensity + nShift);
		}

		if (bDebug) {
			// DEBUG 
			myMin=65535;
			myMax=0;
			for (it2.GoToBegin(); !it2.IsAtEnd(); ++it2)
			{
			  unsigned short pixelValue=it2.Get();
			  if (pixelValue>myMax) myMax=pixelValue;
			  if (pixelValue<myMin) myMin=pixelValue;
			}
				// DEBUG, dump out image
				typedef itk::ImageFileWriter< ImageUShortVolumeType > ImageUShortWriterType;
				ImageUShortWriterType::Pointer psImageUShortWriter = ImageUShortWriterType::New();
				strDebugOutputFilename = strDebugOutputFilenameFolder  + strDerivedChannelName + ".nii";
				psImageUShortWriter->SetFileName(strDebugOutputFilename);
				psImageUShortWriter->SetInput( psDerivedChannel );
				try { 
				   psImageUShortWriter->Update();
				} 
				catch (itk::ExceptionObject& exp) {
			 		std::cerr << "Exception caught!" << std::endl;
					std::cerr << exp << std::endl;
				}
		}


       return psDerivedChannel;
	}



	
		   // Fixed two bugs, z slices should start and sto p based on the zRadius and leave the boundary slices zero
		   // The order of the Z slices sis reversed - so the Z direction is flipped , hence I am searching in the opposite direction (clockwise instead of counter clockwise) 
		   // Thus even though there are rotationally invariant patterns this doesnt take into account the search direction
	static ImageUShortVolumeType::Pointer LBP(ImageUShortVolumeType::Pointer psImageVolume, 
		   int featureVolumeEnum, const string& gradImagePlane, string& strDerivedChannelName)
	{
        bool bDebug=false;
        string strDebugOutputFilenameFolder = "D:\\dev\\decisionForest\\LesionSeg\\decisionForestSolnTrunk\\OneFoldWorkingVS2008_withITK\\testDebug\\";
		string strDebugOutputFilename;

	    unsigned int UniformPattern59[256]={    
				 1,   2,   3,   4,   5,   0,   6,   7,   8,   0,   0,   0,   9,   0,  10,  11,
				12,   0,   0,   0,   0,   0,   0,   0,  13,   0,   0,   0,  14,   0,  15,  16,
				17,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
				18,   0,   0,   0,   0,   0,   0,   0,  19,   0,   0,   0,  20,   0,  21,  22,
				23,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
				0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
				24,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
				25,   0,   0,   0,   0,   0,   0,   0,  26,   0,   0,   0,  27,   0,  28,  29,
				30,  31,   0,  32,   0,   0,   0,  33,   0,   0,   0,   0,   0,   0,   0,  34,
				0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  35,
				0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
				0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  36,
				37,  38,   0,  39,   0,   0,   0,  40,   0,   0,   0,   0,   0,   0,   0,  41,
				0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,  42,
				43,  44,   0,  45,   0,   0,   0,  46,   0,   0,   0,   0,   0,   0,   0,  47,
				48,  49,   0,  50,   0,   0,   0,  51,  52,  53,   0,  54,  55,  56,  57,  58
		};
		float sins[8] = { 0, 0.707, 1, 0.708, 0.002, -0.706,-1, -0.709};
		float coss[8] = { 1, 0.707, 0.001,-0.706,-1,-0.709,-0.002, 0.705};


// Algorithm
// Create duplicate image 
// fill with zeros
// for ea pixel, compute LBP for specified image plane, 
// if bDebug, comput min, max ushort and dump out all image to disk

		typedef itk::ImageDuplicator< ImageUShortVolumeType > DuplicatorTypeUshort;
		DuplicatorTypeUshort::Pointer duplicatorUshort = DuplicatorTypeUshort::New();
		duplicatorUshort->SetInputImage(psImageVolume);
		duplicatorUshort->Update();
		ImageUShortVolumeType::Pointer psDerivedChannel = duplicatorUshort->GetOutput();
		psDerivedChannel->FillBuffer(0);

   	    ImageUShortVolumeType::RegionType region=psImageVolume->GetLargestPossibleRegion();
		ImageUShortVolumeType::SizeType imageExtents=region.GetSize();

		ImageUShortVolumeType::IndexType pixelIndex0;
		ImageUShortVolumeType::IndexType pixelIndex1;

		ImageUShortVolumeType::PixelType pixelValueCenter;
		ImageUShortVolumeType::PixelType pixelValueRing;

        int X,Y,Z;
		int BasicLBP = 0;	
		int FeaBin = 0;	
		float xRadius=1.0;
		float yRadius=1.0;
		float zRadius=2.0; // to mimic LBPTop.m, specifies the radius in the Z direction. Though TBPTop.m did this, this could alternatively be set to the same radius in X and Y directions

		 if (gradImagePlane=="xy") {
			for (int z0 = zRadius; z0 < (imageExtents[2]-zRadius); z0++) {
				for (int y0 = yRadius; y0 < (imageExtents[1]-yRadius); y0++) {
					for (int x0 = xRadius; x0 < (imageExtents[0]-xRadius); x0++) {
						pixelIndex0[0]=x0; pixelIndex0[1]=y0; pixelIndex0[2]=z0;
					    pixelValueCenter=psImageVolume->GetPixel(pixelIndex0);

						BasicLBP = 0;	FeaBin = 0;
						for(int p = 0; p < 8; p++) {
							//X = int (xc + coss[p] + 0.5);
							//Y = int (yc - sins[p] + 0.5);
							//BOUND(X,0,width); BOUND(Y,0,width);
							//if(fg[i][Y][X] >= CenterByte) BasicLBP += POW ( FeaBin); 
							//FeaBin++;
							X = floor((float)x0 + xRadius * coss[p] + 0.5);
							Y = floor((float)y0  - yRadius * sins[p] + 0.5);
							BOUND(X,0,(imageExtents[0]-1)); BOUND(Y,0,(imageExtents[1]-1));
							pixelIndex1[0]=X; pixelIndex1[1]=Y; pixelIndex1[2]=z0;
                            pixelValueRing=psImageVolume->GetPixel(pixelIndex1);
							if(pixelValueRing >= pixelValueCenter) BasicLBP += POW ( FeaBin); 
							FeaBin++;
						}
						psDerivedChannel->SetPixel(pixelIndex0, UniformPattern59[BasicLBP]);
					}
				}
			}
		 } else if (gradImagePlane=="xz") {
			for (int z0 = zRadius; z0 < (imageExtents[2]-zRadius); z0++) {
				for (int y0 = yRadius; y0 < (imageExtents[1]-yRadius); y0++) {
					for (int x0 = xRadius; x0 < (imageExtents[0]-xRadius); x0++) {
						pixelIndex0[0]=x0; pixelIndex0[1]=y0; pixelIndex0[2]=z0;
					    pixelValueCenter=psImageVolume->GetPixel(pixelIndex0);

						BasicLBP = 0;	FeaBin = 0;
						for(int p = 0; p < 8; p++) {
							//X = int (xc + coss[p] + 0.5);
							//Z = int (i + sins[p] + 0.5);
							//BOUND(X,0,width); BOUND(Z,0,width);
							//if(fg[Z][yc][X] >= CenterByte) BasicLBP += POW ( FeaBin);
							//FeaBin++;
							X = floor((float)x0 + xRadius * coss[p] + 0.5);
							// Z = floor((float)z0 + zRadius * sins[p] + 0.5);
							Z = floor((float)z0 - zRadius * sins[p] + 0.5);  // change sign since since z slice order is reversed relative to matlab codes
							BOUND(X,0,(imageExtents[0]-1)); BOUND(Z,0,(imageExtents[2]-1));
							pixelIndex1[0]=X; pixelIndex1[1]=y0; pixelIndex1[2]=Z;
                            pixelValueRing=psImageVolume->GetPixel(pixelIndex1);
							if(pixelValueRing >= pixelValueCenter) BasicLBP += POW ( FeaBin); 
							FeaBin++;
						}
						psDerivedChannel->SetPixel(pixelIndex0, UniformPattern59[BasicLBP]);
					}
				}
			}
		 } else if (gradImagePlane=="yz") {
			for (int z0 = zRadius; z0 < (imageExtents[2]-zRadius); z0++) {
				for (int y0 = yRadius; y0 < (imageExtents[1]-yRadius); y0++) {
					for (int x0 = xRadius; x0 < (imageExtents[0]-xRadius); x0++) {
						pixelIndex0[0]=x0; pixelIndex0[1]=y0; pixelIndex0[2]=z0;
					    pixelValueCenter=psImageVolume->GetPixel(pixelIndex0);

						BasicLBP = 0;	FeaBin = 0;
						for(int p = 0; p < 8; p++) {
							//Y = int (yc - sins[p]+ 0.5);
							//Z = int (i + coss[p] + 0.5);
							//BOUND(Y,0,width); BOUND(Z,0,width);
							//if(fg[Z][Y][xc] >= CenterByte) BasicLBP += POW ( FeaBin);
							//FeaBin++;
							Y = floor((float)y0 - yRadius * sins[p] + 0.5);
							// Z = floor((float)z0 + zRadius * coss[p] + 0.5);
							Z = floor((float)z0 - zRadius * coss[p] + 0.5);   // change sign since since z slice order is reversed relative to matlab code
							BOUND(Y,0,(imageExtents[1]-1)); BOUND(Z,0,(imageExtents[2]-1));
							pixelIndex1[0]=x0; pixelIndex1[1]=Y; pixelIndex1[2]=Z;
                            pixelValueRing=psImageVolume->GetPixel(pixelIndex1);
							if(pixelValueRing >= pixelValueCenter) BasicLBP += POW ( FeaBin); 
							FeaBin++;
						}
						psDerivedChannel->SetPixel(pixelIndex0, UniformPattern59[BasicLBP]);
					}
				}
			}
		}

if (bDebug) {
		 // if bDebug, comput min, max ushort and dump out all image to disk
        // DEBUG, find min & max value of psDerivedChannel
		typedef itk::ImageRegionIterator<ImageUShortVolumeType> ImageIteratorTypeUShort;
		ImageUShortVolumeType::RegionType regionIO=psDerivedChannel->GetBufferedRegion();
        ImageIteratorTypeUShort itDerived(psDerivedChannel,regionIO);
		unsigned int myDerivedChannelMin=65535;
		unsigned int myDerivedChannelMax=0;
		for (itDerived.GoToBegin(); !itDerived.IsAtEnd(); ++itDerived)
		{
          unsigned short pixelValue=itDerived.Get();
		  if (pixelValue>myDerivedChannelMax) myDerivedChannelMax=pixelValue;
		  if (pixelValue<myDerivedChannelMin) myDerivedChannelMin=pixelValue;
		}
            // DEBUG, dump out image
			typedef itk::ImageFileWriter< ImageUShortVolumeType > ImageUShortWriterType;
			ImageUShortWriterType::Pointer psImageUShortWriter = ImageUShortWriterType::New();
			strDebugOutputFilename = strDebugOutputFilenameFolder  + strDerivedChannelName + ".nii";
			psImageUShortWriter->SetFileName(strDebugOutputFilename);
			psImageUShortWriter->SetInput( psDerivedChannel );
			try { 
               psImageUShortWriter->Update();
			} 
			catch (itk::ExceptionObject& exp) {
			 	std::cerr << "Exception caught!" << std::endl;
				std::cerr << exp << std::endl;
			}
}


        return psDerivedChannel;

	 } 

}; // end class def