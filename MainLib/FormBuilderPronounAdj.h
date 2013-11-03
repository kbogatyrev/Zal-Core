#pragma once
#include "WordForm.h"
#include "FormBuilderBaseDecl.h"

using namespace std;
using namespace std::tr1;

class CFormBuilderPronounAdj : public CFormBuilderBaseDecl
{
public:
    CFormBuilderPronounAdj (CLexeme * pLexeme, const CEString& sLemma, ET_AccentType eAccentType) :
        CFormBuilderBaseDecl (pLexeme, SUBPARADIGM_LONG_ADJ), m_sLemma (sLemma), m_eAccentType (eAccentType)
    {
        m_pEndings = new CAdjLongEndings();
    }

protected:
    int iGramHash (ET_PartOfSpeech ePos, ET_Case eCase, ET_Number eNum, ET_Gender eGender, ET_Animacy eAnimacy);

    ET_ReturnCode eGetEndings();

    ET_ReturnCode eGetStressPositions (const CEString&, ET_StressLocation, vector<int>& vecStressPos);

    ET_ReturnCode eCreateFormTemplate (ET_Gender, ET_Number, ET_Case, ET_Animacy, const CEString& sEnding, CWordForm *&); 

    ET_ReturnCode eHandleCommonDeviations (CWordForm *);

public:
    ET_ReturnCode eBuild();

private:
    CEString m_sLemma;
    ET_AccentType m_eAccentType;
    ET_Subparadigm m_eSubparadigm;
    ET_Status m_eStatus;
    bool m_bIrregular;
};
