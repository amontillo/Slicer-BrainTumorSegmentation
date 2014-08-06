#pragma once




// these numbers must be sequential 0,1,2,3 
enum FEATURE_TYPE { 
	FEATURE_TYPE_min=0,
	FEATURE_TYPE_AbsoluteIntensity=FEATURE_TYPE_min, 
	FEATURE_TYPE_RelativeIntensity=1, 
	FEATURE_TYPE_RelativePosterior=2, 
	FEATURE_TYPE_RatioIntensity=3, 
	FEATURE_TYPE_DiffOf2Probes=4, 
	FEATURE_TYPE_AbsoluteSpatialAppearanceEntanglement1a=5, 
	FEATURE_TYPE_SimilarityOfAppearancesEntanglement1b=6, 
	FEATURE_TYPE_AbsoluteMAPLabelEntanglement2a=7,
	FEATURE_TYPE_PosteriorBinEntanglement2b=8,
	FEATURE_TYPE_Top2Classes=9,
	FEATURE_TYPE_Top3Classes = 10,
	FEATURE_TYPE_Top4Classes = 11,
	FEATURE_TYPE_AutocontextMAPLabel=12,
	FEATURE_TYPE_ChannelIntensityRef=13,
	FEATURE_TYPE_HistoDiff=14,
	FEATURE_TYPE_HistoMean=15,
	FEATURE_TYPE_HistoVar=16,
	FEATURE_TYPE_HistoSkew=17,
	FEATURE_TYPE_HistoKurt=18,
	FEATURE_TYPE_HistoEnt=19,
	FEATURE_TYPE_AbsSym=20,
	FEATURE_TYPE_max=FEATURE_TYPE_AbsSym,   // <== Adding a new enum? MUST update  FEATURE_TYPE_max too 
	FEATURE_TYPE_count=(FEATURE_TYPE_max-FEATURE_TYPE_min)+1
};

static const char* arrstrFEATURE_TYPE[]={
	"AbsoluteIntensity",
	"RelativeIntensity",
	"RelativePosterior", 
	"RatioIntensity", 
	"DiffOf2Probes", 
	"AbsoluteSpatialAppearanceEntanglement1a", 
	"SimilarityOfAppearancesEntanglement1b", 
	"AbsoluteMAPLabelEntanglement2a",
	"PosteriorBinEntanglement2b",
	"Top2Classes",
	"Top3Classes",
	"Top4Classes",
	"AutocontextMAPLabel",
	"ChannelIntensityRef",
	"HistoDiff",
	"HistoMean",
	"HistoVar",
	"HistoSkew",
	"HistoKurt",
	"HistoEnt",
	"HistoSym"
};

