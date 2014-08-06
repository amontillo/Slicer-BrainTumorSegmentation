#include "IndexVolume.h"

using namespace std;


IndexVolume::IndexVolume(int numPointsInScan)
{
	numPointsInScan_=numPointsInScan;
    nodeIndexArray_ = new int[numPointsInScan_];
	reset();  // make all nodes zero (root)
}

void IndexVolume::reset()
{
    int* P = nodeIndexArray_;
    int* p = P;
    for (int i = 0; i < numPointsInScan_; i++) { *p = 0; p++; }
}

IndexVolume::~IndexVolume()
{
	if (nodeIndexArray_!=nullptr) { delete nodeIndexArray_; nodeIndexArray_=nullptr; };
}
