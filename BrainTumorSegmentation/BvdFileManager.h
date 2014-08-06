#pragma once
#pragma once

#include <iostream>
#include <sstream>
#include <string> 
#include <fstream>
#include <vector>
#include "basicDefs.h"
#include "xvdTags.h"
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include <blitz/array.h>

using namespace std;
using boost::lexical_cast;
using boost::algorithm::to_lower;


    /// <summary>
    /// Voxel format for binary volume file
    /// </summary>
    enum VoxelFormat { VOXEL_FORMAT_Y_8U = 0, 
        VOXEL_FORMAT_Y_16U = 2, 
        VOXEL_FORMAT_ARGB_32U = 3, 
        VOXEL_FORMAT_ARGB_64U = 4,
        VOXEL_FORMAT_64DOUBLE = 5 
    };


    /// <summary>
    /// Header for binary volume file format
    /// </summary>
    struct BvdFileHeader
    {
         int Version; // = 1
         VoxelFormat Format;
         int Width, Height, Depth;
         float VoxelWidth, VoxelHeight, VoxelDepth;
    };



	struct BvdFileManager 
	{

		// returns true if successfully loaded the features 
        		
		static bool loadVolume_originalCT(string& fileName, xvdTags_ver1& xvdtags, BvdFileHeader& header, blitz::Array<ushort,4>& Avolume, blitz::Array<long long,4>& AIvolume, int dim )
        {
			bool bReturn=false;


  

            try
            {
				fstream infileFeatures(fileName.c_str(), ios::binary | ios::in); // Open file
				if (infileFeatures.is_open())
				{
					cout << "\n>> Reading volume " << fileName << "...";

					infileFeatures.read(reinterpret_cast<char *>(&header.Version), sizeof(int));
					infileFeatures.read(reinterpret_cast<char *>(&header.Format), sizeof(int));
					infileFeatures.read(reinterpret_cast<char *>(&header.Width), sizeof(int));
					infileFeatures.read(reinterpret_cast<char *>(&header.Height), sizeof(int));
					infileFeatures.read(reinterpret_cast<char *>(&header.Depth), sizeof(int));
					infileFeatures.read(reinterpret_cast<char *>(&header.VoxelWidth), sizeof(float));
					infileFeatures.read(reinterpret_cast<char *>(&header.VoxelHeight), sizeof(float));
					infileFeatures.read(reinterpret_cast<char *>(&header.VoxelDepth), sizeof(float));

					if (!infileFeatures.good())  throw; 

					if ((header.Format != VOXEL_FORMAT_Y_16U) && (header.Format != VOXEL_FORMAT_Y_8U)
						&& (header.Format != VOXEL_FORMAT_64DOUBLE))
					{ cout << " ERROR: incorrect file header only 16 and 8 bit and double voxels are supported.\n"; throw; }
                
					string strChannelInterpreationLower=xvdtags.strChannelInterpretation; to_lower(strChannelInterpreationLower);
								
					// Load whole volume (note: stride == 4*W and leap == 4*W*H)
								for (int k = 0; k < header.Depth; k++)
									for (int j = 0; j < header.Height; j++)
										for (int i = 0; i < header.Width; i++)
										{
											infileFeatures.read(reinterpret_cast<char *>(&(Avolume(i,j,k,dim))), sizeof(ushort));
											//cout << Avolume(i,j,k) << endl;
										}

								//convert image to the integral image Anthony
								int W = header.Width+1, H = header.Height+1, D = header.Depth+1;
								long long nhood [8];
								
								for (int k = 0; k < (header.Depth+1); k++)
									for (int j = 0; j < (header.Height+1); j++)
										for (int i = 0; i < (header.Width+1); i++)
										{   
											if ((i==0)||(j==0)||(k==0)) //pad the array
											{
												AIvolume(i,j,k,dim) = 0;
											}
											else
											{
												nhood[0] = (long long)Avolume(i-1,j-1,k-1,dim);

												//get neighborhood
												nhood[1] = AIvolume(i-1,j-1,k-1,dim);
												nhood[2] = AIvolume(i,j,k-1,dim);
												nhood[3] = AIvolume(i-1,j,k,dim);
												nhood[4] = AIvolume(i,j-1,k,dim);
												nhood[5] = AIvolume(i-1,j,k-1,dim);
												nhood[6] = AIvolume(i,j-1,k-1,dim);
												nhood[7] = AIvolume(i-1,j-1,k,dim);

												AIvolume(i,j,k,dim) = nhood[0] + nhood[1] + nhood[2] + nhood[3] + nhood[4] - nhood[5] - nhood[6] - nhood[7];	
												//cout << AIvolume(i,j,k,dim) << endl; //for debuging
											}
										}
								
						if (      (!infileFeatures.good())  && (!infileFeatures.eof())) throw; 
						cout << "Done.\n\n";
						infileFeatures.close();  // Close file
						bReturn=true;

				}
				else
				{
					cout << "Error: could not open feature file: " << fileName << "\n";
					bReturn=false;
				}
            }
            catch (...)
			{   cout << "Error: could not load some elements from feature file: " << fileName << "\n";
				bReturn=false;  
			}; 

			return bReturn;
        }


		
	   static bool loadVolume_labels(string& fileName, BvdFileHeader& header, byte*& volume)  // CALLER is responsible for deleting memory for volume allocated herein
        {
            bool bReturn=true;

 			volume=nullptr;  // Assume not already allocated


            try
            {	
				fstream infileLabels(fileName.c_str(), ios::binary | ios::in); // Open file
				if (infileLabels.is_open())
				{
					cout << ">> Reading ground-truth labels " + fileName + "...";

					infileLabels.read(reinterpret_cast<char *>(&header.Version), sizeof(int));
					infileLabels.read(reinterpret_cast<char *>(&header.Format), sizeof(int));
					infileLabels.read(reinterpret_cast<char *>(&header.Width), sizeof(int));
					infileLabels.read(reinterpret_cast<char *>(&header.Height), sizeof(int));
					infileLabels.read(reinterpret_cast<char *>(&header.Depth), sizeof(int));
					infileLabels.read(reinterpret_cast<char *>(&header.VoxelWidth), sizeof(float));
					infileLabels.read(reinterpret_cast<char *>(&header.VoxelHeight), sizeof(float));
					infileLabels.read(reinterpret_cast<char *>(&header.VoxelDepth), sizeof(float));


					if (header.Format != VOXEL_FORMAT_Y_8U) { cout << " ERROR: incorrect file header.\n"; return false; }

					// Allocate the space for the label which the caller owns
					volume = new byte[header.Width * header.Height * header.Depth];

					// Load whole volume (note: stride == 4*W and leap == 4*W*H)
					byte* P = volume;

					byte* p = P;
					byte aux;

					for (int k = 0; k < header.Depth; k++)
						for (int j = 0; j < header.Height; j++)
							for (int i = 0; i < header.Width; i++)
							{
								infileLabels.read(reinterpret_cast<char *>(&aux), sizeof(byte));

								*p = aux;
								p++;
							}

					if (      (!infileLabels.good())  && (!infileLabels.eof())) throw; 
					cout << "Done.\n\n";
					infileLabels.close();  // Close file
					bReturn=true;

				}
				else
				{
					cout << "Error: could not open ground truth label file: " << fileName << "\n";
					bReturn=false;
				}
            }
            catch (...)
			{   cout << "Error: could not load some elements from ground truth label file: " << fileName << "\n";
				bReturn=false;  
			}; 

			
			return bReturn;


        }


        


	   static bool saveVolume_labels(string& fileName, byte* volume, BvdFileHeader& header)
        {
			bool bReturn=true;

            try
            {
				// Create file in which labels are saved
				fstream outfileLabels(fileName.c_str(), ios::binary | ios::out);

				if (outfileLabels.is_open())
				{
					cout << "| Saving predicted labels ...";

					// Write header to label volume
					outfileLabels.write(reinterpret_cast<char *>(&header), sizeof(BvdFileHeader));   // WRITE header

					// Write predicted labels 
					byte* p = volume;
					for (int k = 0; k < header.Depth; k++)
						for (int j = 0; j < header.Height; j++)
							for (int i = 0; i < header.Width; i++)
							{
								outfileLabels.write(reinterpret_cast<char *>(p), sizeof(byte));
								p++;
							}

					cout << "Done.\n\n";
					outfileLabels.close();  // Close file
					bReturn=true;


				}
				else
				{
					cout << "Error: could not open predicted label file: " << fileName << "\n";
					bReturn=false;
				}
            }
			catch (...)
			{
				cout << "Error: could not save predicted label file: " << fileName << "\n";
				bReturn=false; 
			}

 	        return bReturn;
		}
};

