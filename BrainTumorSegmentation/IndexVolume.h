#pragma once
#include "basicDefs.h"
#include <boost/shared_ptr.hpp>

/// <summary>
/// Node indeces for each voxel
/// </summary>
struct IndexVolume
{
    int* nodeIndexArray_;    // 
	int numPointsInScan_;
    IndexVolume(int numPointsInScan);

    void reset();

	~IndexVolume();
};

typedef boost::shared_ptr<IndexVolume> ptrsmartIndexVolume;

