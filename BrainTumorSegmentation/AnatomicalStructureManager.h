#pragma once

    #include <vector>
	#include <string>
    #include "WLmapper.h"
    #include "basicDefs.h"
    using namespace std;

	    // the IDs of these structure names must be sequential 0,1,2,3 
    enum ANATOMICAL_STRUCTURE   
    {
        BACKGROUND, 
		EDEMA,
		TUMOR
    };

    class AnatomicalStructureManager  // do not use variables named asm since that is a keyword !!
    {  
	public: 
        // Purpose: define a table (correspondence) of class names, unique ID, window level, ground truth structure colors

        const int numStructures_ ;
        vector<string> structNames_;
		vector<vector<byte> > structColors_;




        AnatomicalStructureManager() : 
		numStructures_(256)   // Assumption max number of class labels is 256. i.e. 0...255  since that is that can fit into a byte rep
		{
            // initializing structure names 
            structNames_.resize(numStructures_,"");


			#ifdef COLUMNAR_INTERPRETATION
			for (int nStructureID=0; nStructureID<numStructures_; nStructureID++)
			{
				structNames_[nStructureID] = lexical_cast<string>(nStructureID);
			}
            #else
            // initializing structure names  modified for bratz
            structNames_[0] = "Background"; 
            structNames_[1] = "Edema";
            structNames_[2] = "Tumor";
            #endif

            // initializing structure colours
            structColors_.assign(numStructures_, vector<byte>(3,0));  // RGB only for now  .. All 255 structures initially have RGB=[0,0,0], some are updated below
            structColors_[0][0] = 0; structColors_[0][1] = 0; structColors_[0][2] = 0;          // Background
            structColors_[1][0] = 255; structColors_[1][1] = 255; structColors_[1][2] = 255;    // Edema
            structColors_[2][0] = 255; structColors_[2][1] = 0; structColors_[2][2] = 0;        // Tumor
            

            
        };

	public:
        


        string anatStructIdentifier_To_StructName(int structIdentifier)
        {
            string str;
            if (structIdentifier > numStructures_ - 1) str = "error";
            else str = structNames_[structIdentifier];
            return str;
        }

        int anatStructName_To_StructIdentifier(const string& structName)
        {
            int output = -1;
            for (int i = 0; i < numStructures_; i++)
                if (structName == structNames_[i]) output = i;
            return output;
        }




    };

