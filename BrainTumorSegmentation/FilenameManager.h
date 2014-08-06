#pragma once

#include "basicDefs.h"
#include <string>
using std::string;

extern char cFolderSeparator;

struct FilenameManager
{

	static string ExtractDirectory( const string& path )
	  { return path.substr( 0, path.find_last_of( cFolderSeparator ) +1 );   }

	static string ExtractFilename( const string& path )
	  { 	  return path.substr( path.find_last_of( cFolderSeparator ) +1 ); 	  }

	static string ChangeExtension( const string& path, const std::string& ext )
	  { string filename = ExtractFilename( path );
	    return ExtractDirectory( path ) +filename.substr( 0, filename.find_last_of( '.' ) ) +ext;
	  }


	//  goal is to convert a path like this : F:\TrainingData\GEDataset52\
	// into this:  GEDataset52
	static string ExtractDatasetname( const string& path )
	{
		// remove the trailing directory slash, if any
		int q3=(int)path.size()-1;
		int jj=0;
		for (jj = q3; jj >= 0; jj--)  { if (path[jj] != cFolderSeparator ) { break; } };
		string newString = path.substr( 0, jj+1);

            
			int q1 = (int)newString.size() - 2;
            for (int j = (int)newString.size() - 2; j >= 0; j--) 
			   { if (newString[j] == cFolderSeparator ) { break; } 
			     q1--; 
			    }

			q1++;
            return newString.substr(q1, newString.size() - q1 + 1);
	}

};
