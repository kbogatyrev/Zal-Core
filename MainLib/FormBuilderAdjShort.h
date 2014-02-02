#ifndef FORMBUILDERADJSHORT_H_INCLUDED
#define FORMBUILDERADJSHORT_H_INCLUDED

#include "FormBuilderBaseDecl.h"

using namespace std;
using namespace std::tr1;

namespace Hlib
{

class CFormBuilderShortAdj : public CFormBuilderBaseDecl
{
public:

    // Use with adjectives
    CFormBuilderShortAdj (CLexeme * pLexeme);

    // Use with participles
    CFormBuilderShortAdj (CLexeme * pLexeme, 
                          bool bYoAlternation, 
                          const CEString& sLemma,
                          ET_Subparadigm eSubparadigm,
                          ET_AccentType eAccentType1,
                          ET_AccentType eAccentType2,
                          vector<int>& vecStressPos,
                          bool bFleetingVowel,
                          int iDeviation = -1,
                          ET_Status eStatus = STATUS_COMMON);

    ET_ReturnCode eStressOnEnding (ET_Number eNumber, ET_Gender eGender);

    ET_ReturnCode eGetStressTypes (ET_Number eNumber, 
                             ET_Gender eGender, 
                             vector<ET_StressLocation>& vecStressType);

    ET_ReturnCode eCreateFormTemplate (const CEString& sLemma, 
                                       const CEString& sEnding,
                                       ET_Gender eGender,
                                       ET_Number eNumber,
                                       CWordForm *& pWordForm);

    ET_ReturnCode eCreateWordForm (ET_Number eNumber, 
                                   ET_Gender eGender, 
                                   ET_StressLocation eStressType,
                                   const CEString& sEnding,
                                   CEString& sLemma);

    ET_ReturnCode eHandleDeviations (CWordForm *);

    ET_ReturnCode eBuild();

private:
    CEString m_sLemma;
    int m_iType;
    vector<int> m_vecStressPos;        // Participles only
    ET_AccentType m_eAccentType1, m_eAccentType2;
    bool m_bFleetingVowel;
    map<int, bool> m_mapCommonDeviations;
    ET_Status m_eStatus;
};

}   // Namespace Hlib

#endif  //  FORMBUILDERADJSHORT_H_INCLUDED
