#pragma once
#include <string>

using namespace std;


// <summary>
// xvd tags are a subset of DICOM tags. Used for volumetric medical datasets
// </summary>
class xvdTags_ver1  {
	public:
		bool formatOK;
		string binaryFileName, patientName, patientSex, modality, contrastAgent;
		int volumeWidth, volumeHeight, volumeDepth;
		double pixScaleX, pixScaleY, pixScaleZ;
		double rescaleSlope, rescaleIntercept;

		// for multi-channel support:
		int nNumChannels;
		int channelDisplayWidth, channelDisplayHeight, channelDisplayDepth;
		string strChannelInterpretation;

		xvdTags_ver1()
		{
			formatOK = false;
			binaryFileName = patientName = patientSex = modality = contrastAgent = "";
			volumeWidth = volumeHeight = volumeDepth = -1;
			pixScaleX = pixScaleY = pixScaleZ = rescaleSlope = rescaleIntercept = -1;
			nNumChannels=channelDisplayWidth = channelDisplayHeight = channelDisplayDepth = -1;
			strChannelInterpretation = "SingleChannel";   // AAM 10-7-2011  Prior default was "". Changed to "SingleChannel" for use with image data
		}



};

