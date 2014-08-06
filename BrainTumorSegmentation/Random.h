#pragma once
#include <boost/random.hpp>                // for random numbers
#include "boost/generator_iterator.hpp"    // for random numbers
#include <ctime>                           // for seeding with current time via std::time
#include <iomanip>



// This class consolidates the generation of random ints and reals 
// Requires modified Boost so that a request for a random # can be accompanied by a range for that request
struct RandomIntAndReal
{
	typedef boost::mt19937 RandomNumberGenerator; // Mersenne Twister 

	
	typedef boost::uniform_real<> DistributionReal; 
	typedef boost::variate_generator<RandomNumberGenerator&, DistributionReal> GeneratorReal; 
	const double dRangeMin_;
	const double dRangeMax_;
	RandomNumberGenerator generatorForReals_;
	DistributionReal* pDistributionReal_;
	GeneratorReal* pNextDoubleInRange_;

	
	typedef boost::uniform_int<> DistributionInt; 
	typedef boost::variate_generator<RandomNumberGenerator&, DistributionInt> GeneratorInt; 
	const int nRangeMin_; 
	const int nRangeMax_; 
	RandomNumberGenerator generatorForInts_;
	DistributionInt* pDistributionInt_; 
	GeneratorInt* pNextIntInRange_; 


	RandomIntAndReal()  // default is random
		: dRangeMin_(0), dRangeMax_(1), nRangeMin_(0), nRangeMax_(1)
	{   
		int nNewSeed=(int)std::time(0);

		cout << "==============================\n";
		cout << "\n\n\n Random number generator seed to " << nNewSeed << "\n\n\n";

		// for reals
		generatorForReals_.seed(static_cast<unsigned int>(nNewSeed)); // random seed based on current time 
		pDistributionReal_=new DistributionReal(dRangeMin_, dRangeMax_); 
		pNextDoubleInRange_=new GeneratorReal(generatorForReals_, *pDistributionReal_); 

		// for ints
		pDistributionInt_=new DistributionInt(nRangeMin_, nRangeMax_); 
		generatorForInts_.seed(static_cast<unsigned int>(nNewSeed)); // random seed based on current time 
		pNextIntInRange_=new GeneratorInt(generatorForInts_, *pDistributionInt_); 

		cout << "==============================\n";
        cout << "\n\n\n";
	}

	RandomIntAndReal(unsigned int nSeed)   // specify seed for deterministic behavior
		: dRangeMin_(0), dRangeMax_(1), nRangeMin_(0), nRangeMax_(1)
	{

		// for reals
		generatorForReals_.seed(nSeed); // seed with fixed seed 
		pDistributionReal_=new DistributionReal(dRangeMin_, dRangeMax_); 
		pNextDoubleInRange_=new GeneratorReal(generatorForReals_, *pDistributionReal_); 

		// for ints
		pDistributionInt_=new DistributionInt(nRangeMin_, nRangeMax_); 
		generatorForInts_.seed(nSeed); // seed with fixed seed
		pNextIntInRange_=new GeneratorInt(generatorForInts_, *pDistributionInt_); 

	}


	~RandomIntAndReal()
	{
		if (pDistributionReal_!=nullptr) { delete pDistributionReal_; pDistributionReal_=nullptr; }
		if (pDistributionReal_!=nullptr) { delete pNextDoubleInRange_; pNextDoubleInRange_=nullptr; }
		if (pDistributionInt_!=nullptr) { delete pDistributionInt_; pDistributionInt_=nullptr; }
		if (pNextIntInRange_!=nullptr) { delete pNextIntInRange_; pNextIntInRange_=nullptr; }

	}

	void setSeedBasedOnCurrentTime(int nUnitOfWorkID)
	{

		   // Automatically vary the seed value for each process in MPI distributed parallelism
		   // unsigned int mseconds= (unsigned int) (pMPIMngr->nThisProcessID * 550 + (*rng_.pNextDoubleInRange_)() * 100); // sleep for a random amount of msec based on this ProcessID
		   unsigned int mseconds= (unsigned int) (nUnitOfWorkID * 550); // sleep for a random amount of msec based on this ProcessID
		   clock_t goal = mseconds + clock();
		   while (goal > clock())  {  };

		   int nNewSeed=nUnitOfWorkID + (int)std::time(0);

		#ifdef USE_MPI
			cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
			cout << "\n\n\nResetting seed for process " << pMPIMngr->nThisProcessID << " to " << nNewSeed << "\n\n\n";
        #endif 
		// Reset the seed in the engine:
		pNextDoubleInRange_->engine().seed(nNewSeed);
		pNextDoubleInRange_ ->distribution().reset();

		pNextIntInRange_->engine().seed(nNewSeed);
		pNextIntInRange_ ->distribution().reset();



	}

	void setSeed(unsigned int nSeed)  // 
	{
		// for reals
		generatorForReals_.seed(nSeed); // seed with fixed seed 

		// for ints
		generatorForInts_.seed(nSeed); // seed with fixed seed
	}


	void testReal()
	{
		std::cout << "Generate random reals using preset range [0..1)  .. Half open interval for reals\n";
		  for (int i=0; i<100; i++)
			  std::cout << (*pNextDoubleInRange_)() << std::endl; 
  
		std::cout << "..now generate from newly specified range [10,30)..\n";
		for (int i=0; i<100; i++)
			  std::cout << (*pNextDoubleInRange_)(10,30) << std::endl; 

		
		// get 1 million random real numbers ... test for speed of generator
		for (int i=0; i<1000000; i++)
			  (*pNextDoubleInRange_)(10,40); 

		std::cout << "confirm still in the range [0..1) begin initialized range..\n";
		  for (int i=0; i<100; i++)
			  std::cout << (*pNextDoubleInRange_)() << std::endl; 

		// get 1 million random reals
		for (int i=0; i<1000000; i++) (*pNextDoubleInRange_)(10,40); 

		std::cout << "done.\n";
		

	}

	void testInt()
	{
		std::cout << "\n ********\n Begin integer random sequences:\n";

		std::cout << "Generate random ints in the preset range [0,1] ... Closed interval for ints \n";
		for (int i=0; i<100; i++)
			  std::cout << (*pNextIntInRange_)() << std::endl; 
  
		std::cout << "..now generate from newly specified range [10,30] inclusive\n";
		for (int i=0; i<100; i++)
			  std::cout << (*pNextIntInRange_)(10,30) << std::endl; 

		std::cout << "..now generate from newly specified range [0,5] inclusive where min is implicit\n";
		for (int i=0; i<100; i++)
			  std::cout << (*pNextIntInRange_)(5) << std::endl; 

		std::cout << "begin initialized range..at the set range [0,1] \n";
		for (int i=0; i<100; i++)
			  std::cout << (*pNextIntInRange_)() << std::endl; 

		// get 1 million random ints
		for (int i=0; i<1000000; i++) (*pNextIntInRange_)(10,40); 
	}
};
