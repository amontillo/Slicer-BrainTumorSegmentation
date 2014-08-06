#include  "Posterior.h"

using namespace std;
using boost::lexical_cast;

Posterior::Posterior(VolumeDimensions& dims, int numClasses)
{
	nSizePostArray_=0;
	dims_=dims;
	numClasses_=numClasses;

	int datasetVolume=dims_.Depth * dims_.Height * dims_.Width;

    try
    {   int nDesiredSize=numClasses * datasetVolume;
        postArray_ = new ushort[nDesiredSize];
		ushort uZero=(ushort)0;
		for (int i=0; i<nDesiredSize; i++) postArray_[i]=uZero;
		nSizePostArray_=nDesiredSize;
    }
    catch (...)
    {
        postArray_ = nullptr;
		nSizePostArray_=0;
		dims_.Depth=0;
		dims_.Height=0;
		dims_.Width=0;
		numClasses_=0;
        cout << "\n>> ERROR: cannot allocate 4D ushort posterior array. Insufficient memory? \n";
    }
}

void Posterior::ZeroThePosterior()
{
	if (postArray_!=nullptr) 
	{
		ushort uZero=(ushort)0;
		for (int i=0; i<nSizePostArray_; i++) postArray_[i]=uZero;
	}
}

Posterior::~Posterior()
{
	if (postArray_!=nullptr) { delete [] postArray_; postArray_=nullptr; };

}
  

// Load the 4D posterior from disk and sum it to the posterior in this objects memory
bool Posterior::loadAndAccumulate_4DPosterior(string& fileName)
{
    bool bReturn=true;

    try
    {
		// Open 4D file soft posterior labels
		fstream infilePosteriorLabels(fileName.c_str(), ios::binary | ios::in); // Open file
		if (infilePosteriorLabels.is_open())
		{
			cout << "| Loading predicted posterior labels ...";


			// Read in the header
			Posterior4DFileHeader headerPosterior4D;

			// Write header to posterior volume
			infilePosteriorLabels.read(reinterpret_cast<char *>(&headerPosterior4D), sizeof(Posterior4DFileHeader));   // READ header

			float fExpectedMaxFileVersion = 1.0;
			if (headerPosterior4D.fVersion_ > fExpectedMaxFileVersion ) { cout << " ERROR: incorrect file version" << headerPosterior4D.fVersion_  << " in Posterior4D file header, this code handles to version " << fExpectedMaxFileVersion << "  \n"; return false; }
			if (headerPosterior4D.VoxelFormat_ != VOXEL_FORMAT_Y_16U) { cout << " ERROR: incorrect voxel format in Posterior4D file header.\n"; return false; }

			if  ( ( headerPosterior4D.Depth_ !=  dims_.Depth) ||   ( headerPosterior4D.Height_ !=  dims_.Height)  ||   ( headerPosterior4D.Width_ !=  dims_.Width)  )
				{ cout << " ERROR: dimensions of volume in Posterior4D file header do not match in memory posterior object.\n"; return false; }

			if  ( ( headerPosterior4D.Depth_ !=  dims_.Depth) ||   ( headerPosterior4D.Height_ !=  dims_.Height)  ||   ( headerPosterior4D.Width_ !=  dims_.Width)  )
				{ cout << " ERROR: dimensions of volume in Posterior4D file header do not match in memory posterior object.\n"; return false; }

			// Read predicted posterior labels 
			ushort buf;
			// ushort* p = postArray_;
			for (int classIndex=0; classIndex < numClasses_; classIndex++)
			{
				for (int z = 0; z < dims_.Depth; z++)
				{
					for (int y = 0; y < dims_.Height; y++)
					{
						for (int x = 0; x < dims_.Width; x++)
						{
							infilePosteriorLabels.read(reinterpret_cast<char *>(&buf), sizeof(ushort));
							postArray_[((classIndex * dims_.Depth + z) * dims_.Height + y) * dims_.Width + x] += buf;  // add rather than assign so we can using this function to read and sum more than one file  
							// p++;
						}
					}
				}
			}

			cout << "Done.\n\n";
			infilePosteriorLabels.close();  // Close file
			bReturn=true;

		}
		else
		{
			cout << "Error: could not open posterior label file: " << fileName << "\n";
			bReturn=false;
		}
    }
	catch (...)
	{
		cout << "Error: could not load some data from 4D posterior label file: " << fileName << "\n";
		bReturn=false; 
	}

 	return bReturn;

}

bool Posterior::save_4DPosterior(string& fileName)
{
    bool bReturn=true;

    try
    {
		// Create file for soft posterior labels
		fstream outfilePosteriorLabels(fileName.c_str(), ios::binary | ios::out);

		if (outfilePosteriorLabels.is_open())
		{
			cout << "| Saving predicted posterior labels ...";

			Posterior4DFileHeader headerPosterior4D;
			headerPosterior4D.fVersion_=1.0;  // Starting with 1.0 of posterior file format, we save the nNumClasses in the VoxelDepth field
			headerPosterior4D.VoxelFormat_=VOXEL_FORMAT_Y_16U;  // ushort values 
			headerPosterior4D.Depth_=dims_.Depth;
			headerPosterior4D.Height_=dims_.Height;
			headerPosterior4D.Width_=dims_.Width;
			headerPosterior4D.numClasses_=numClasses_;

			// Write header to posterior volume
			outfilePosteriorLabels.write(reinterpret_cast<char *>(&headerPosterior4D), sizeof(Posterior4DFileHeader));   // WRITE header

			// Write predicted posterior labels 
			// ushort* p = postArray_;
			ushort ushortSingle;
			for (int classIndex=0; classIndex < numClasses_; classIndex++)
			{
				for (int z = 0; z < dims_.Depth; z++)
				{
					for (int y = 0; y < dims_.Height; y++)
					{
						for (int x = 0; x < dims_.Width; x++)
						{
							ushortSingle = postArray_[((classIndex * dims_.Depth + z) * dims_.Height + y) * dims_.Width + x];
							outfilePosteriorLabels.write(reinterpret_cast<char *>(&ushortSingle), sizeof(ushort));
							// p++;
						}
					}
				}
			}

			cout << "Done.\n\n";
			outfilePosteriorLabels.close();  // Close file
			bReturn=true;

		}
		else
		{
			cout << "Error: could not open posterior label file: " << fileName << "\n";
			bReturn=false;
		}
    }
	catch (...)
	{
		cout << "Error: could not save predicted posterior label file: " << fileName << "\n";
		bReturn=false; 
	}

 	return bReturn;
}


/// <summary>
/// Select a 3D array out of the 4D array for the specified organ class
/// Caller is responsible for deleteing the returned 3D array 
/// </summary>
ushort* Posterior::extract3DClassPosterior(int classIndex, VolumeDimensions volDims)
{
    ushort* outClassPosterior = new ushort[volDims.Width * volDims.Height * volDims.Depth];    // allocating the output 3D array
    ushort* P = outClassPosterior;

    ushort* p = P;
    for (int z = 0; z < volDims.Depth; z++)
    {
        for (int y = 0; y < volDims.Height; y++)
        {
            for (int x = 0; x < volDims.Width; x++)
            {
                *p = postArray_[((classIndex * volDims.Depth + z) * volDims.Height + y) * volDims.Width + x];
                p++;
            }
        }
    }

    return outClassPosterior;
}

