#pragma once

// ModuleDescriptionParser includes
#include <ModuleProcessInformation.h>


struct PluginProcessWatcher  // See similar functionality in #include "itkPluginFilterWatcher.h"
{

  ModuleProcessInformation *m_ProcessInformation;
  double m_Fraction;
  double m_Start;

	void init(struct ModuleProcessInformation* CLPProcessInformation, double dFraction, double dStart)
	{
		m_ProcessInformation=CLPProcessInformation;
		m_Fraction=dFraction;
		m_Start=dStart;

	}

   void displayProgress(double dFractionComplete, string& strMessage)  // See similar functionality in #include "itkPluginFilterWatcher.h"
   {
       
      if (m_ProcessInformation)  // if running as a DLL
      {
        strncpy(m_ProcessInformation->ProgressMessage,strMessage.c_str(), 1023);
        m_ProcessInformation->Progress = (dFractionComplete * m_Fraction + m_Start);

		m_ProcessInformation->StageProgress = m_ProcessInformation->Progress;
        //if (m_Fraction != 1.0)
        //  { m_ProcessInformation->StageProgress = this->GetProcess()->GetProgress();
        //  }

		m_ProcessInformation->ElapsedTime=0.1;
        //this->GetTimeProbe().Stop();
        //m_ProcessInformation->ElapsedTime = this->GetTimeProbe().GetMean() * this->GetTimeProbe().GetNumberOfStops();
        //this->GetTimeProbe().Start();

     //   if (m_ProcessInformation->Abort)
     //     {
			  //this->GetProcess()->AbortGenerateDataOn();
			  //m_ProcessInformation->Progress = 0;
			  //m_ProcessInformation->StageProgress = 0;
     //     }

        if (m_ProcessInformation->ProgressCallbackFunction && 
			m_ProcessInformation->ProgressCallbackClientData)
          {
             (*(m_ProcessInformation->ProgressCallbackFunction))(m_ProcessInformation->ProgressCallbackClientData);
          }
      }
      else  // if running as a EXE
      {
        std::cout << "<filter-progress>"
                  << (dFractionComplete * m_Fraction) + m_Start
                  << "</filter-progress>"
                  << std::endl;
        if (m_Fraction != 1.0)
          {
            std::cout << "<filter-stage-progress>"
                    << dFractionComplete 
                    << "</filter-stage-progress>"
                    << std::endl;
          }
            std::cout << std::flush;
      }
      
   }
};