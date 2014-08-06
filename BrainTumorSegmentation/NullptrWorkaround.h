// On Linux, C++ compiler GCC for versions pre 4.6.0, it does not have nullptr as a keyword so we define a workaround here
//  Two things are needed: new GCC compiler. And mpiCC for openMPI that calls the new version of GCC
//
// On windows, visual studio2010 supports nullptr keyword, however VS2008 does not.


// Seems to not be defined for GCC 4.6.3 either
//#ifdef WIN32

//#if (_MSC_VER >= 1500 && _MSC_VER < 1600)
   // ... Do VC9/Visual Studio 2008 specific stuff
	//MSVC++ 11.0 _MSC_VER == 1700 (Visual Studio 2012)
	//MSVC++ 10.0 _MSC_VER == 1600 (Visual Studio 2010)
	//MSVC++ 9.0  _MSC_VER == 1500 (Visual Studio 2008)
	//MSVC++ 8.0  _MSC_VER == 1400 (Visual Studio 2005)
    // see : http://stackoverflow.com/questions/70013/how-to-detect-if-im-compiling-code-with-visual-studio-2008


	const // this is a const object... 
	// Define a workaround for using older gcc compiler. Version GCC 4.6.1   supports nullptr as a keyword in C++ but prior versions do not.
	class {
	public:
	 template<class T> // convertible to any type
	  operator T*() const // of null non-member
	   { return 0; } // pointer...

	  template<class C, class T> // or any type of null
	  operator T C::*() const // member pointer...
	  { return 0; }

	private:
	void operator&() const; // whose address can't be taken 

	} nullptr = {}; // and whose name is nullptr

//#endif

//#endif



