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
    CFormBuilderConj (shared_ptr<CLexeme> spLexeme, shared_ptr<CInflection> spInflection, ET_Subparadigm eSubparadigm) : 
        CFormBuilder (spLexeme, spInflection, eSubparadigm)
    {
        const wchar_t* arrPreverbs[] = { L"в", L"над", L"об", L"от", L"под", L"пред", L"с" };
        for (int i_ap = 0; i_ap < (int)(sizeof(arrPreverbs) / sizeof(wchar_t*)); ++i_ap)
        {
            m_vecAlternatingPreverbs.push_back(arrPreverbs[i_ap]);
        }

        const wchar_t* arrPreverbsV[] = { L"вс", L"вз", L"вос", L"воз", L"ис", L"из", L"нис", L"низ", L"рас", L"раз" };
        for (int i_ap = 0; i_ap < (int)(sizeof(arrPreverbsV) / sizeof(wchar_t*)); ++i_ap)
        {
            m_vecAlternatingPreverbsWithVoicing.push_back(arrPreverbsV[i_ap]);
        }
    }

protected:
    ET_ReturnCode eBuildVerbStems();
    ET_ReturnCode eStandardAlternation (CEString& sPresentStem);
    ET_ReturnCode eGetEndingStressPosition (const CEString& sLemma, const CEString& sEnding, int& iPosition);
    ET_ReturnCode eBuildPastTenseStem (CEString& sLemma);
    ET_ReturnCode eFleetingVowelCheck (CEString& sVerbForm);
    ET_ReturnCode eHandleYoAlternation (int iStressPos, ET_Subparadigm eSubParadigm, CEString& sLemma);
    ET_ReturnCode eGetAlternatingPreverb(const CEString& sVerbForm, CEString& sPreverb, bool& bVoicing);
    ET_ReturnCode eGetPastTenseStressTypes (ET_AccentType eAccentType, ET_Number eNumber, ET_Gender eGender, vector<ET_StressLocation>& vecStress);
    bool bHasIrregularPresent();
    bool bHasIrregularPast();
    bool bHasIrregularImperative();

    ET_ReturnCode eGetIrregularForms(CEString sHash, vector<shared_ptr<CWordForm>>&); // get irregular form; fill stress pos for monosyll

protected:
    vector<CEString> m_vecAlternatingPreverbs, m_vecAlternatingPreverbsWithVoicing;

};

}

#endif      //  FORMBUILDERBASE_H_INCLUDED

