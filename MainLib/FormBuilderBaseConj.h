#ifndef FORMBUILDERBASECONJ_H_INCLUDED
#define FORMBUILDERBASECONJ_H_INCLUDED

#include "Enums.h"
#include "EString.h"

#include "FormBuilderBase.h"

using namespace std;
//using namespace std::tr1;

namespace Hlib
{

class CFormBuilderConj : public CFormBuilder
{
public:
    CFormBuilderConj (CLexeme * pLexeme, ET_Subparadigm eSubparadigm) : 
        CFormBuilder (pLexeme, eSubparadigm)
    {}

protected:
    ET_ReturnCode eBuildVerbStems();
    ET_ReturnCode eStandardAlternation (CEString& sPresentStem);
    ET_ReturnCode eGetEndingStressPosition (const CEString& sLemma, const CEString& sEnding, int& iPosition);
    ET_ReturnCode eBuildPastTenseStem (CEString& sLemma);
    ET_ReturnCode eFleetingVowelCheck (CEString& sVerbForm);
    ET_ReturnCode eHandleYoAlternation (int iStressPos, ET_Subparadigm eSubParadigm, CEString& sLemma);
    ET_ReturnCode eGetPastTenseStressTypes (ET_AccentType eAccentType, ET_Number eNumber, ET_Gender eGender, vector<ET_StressLocation>& vecStress);
    bool bHasIrregularPresent();
    bool bHasIrregularPast();
    bool bHasIrregularImperative();

    ET_ReturnCode eGetIrregularForms(CEString sHash, vector<CWordForm *>&); // get irregular form; fill stress pos for monosyll
};

}

#endif      //  FORMBUILDERBASE_H_INCLUDED

