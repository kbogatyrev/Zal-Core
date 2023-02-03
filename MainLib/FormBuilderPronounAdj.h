#ifndef FORMBUILDERPRONOUNADJ_H_INCLUDED
#define FORMBUILDERPRONOUNADJ_H_INCLUDED

#include "Enums.h"
#include "EString.h"

#include "WordForm.h"
#include "FormBuilderBaseDecl.h"

using namespace std;

namespace Hlib
{

class CFormBuilderPronounAdj : public CFormBuilderDecl
{
public:
    CFormBuilderPronounAdj (shared_ptr<CLexeme> spLexeme, shared_ptr<CInflection> spInflection, const CEString& sStem, ET_AccentType eAccentType) :
        CFormBuilderDecl (spLexeme, spInflection, SUBPARADIGM_ADVERBIAL_PAST), m_sStem(sStem), m_eAccentType (eAccentType)
    {}

protected:
    CEString sGramHash (ET_PartOfSpeech ePos, ET_Case eCase, ET_Number eNum, ET_Gender eGender, ET_Animacy eAnimacy);
    ET_ReturnCode eGetStressPositions (CEString& sStem, const CEString& sEnding, ET_StressLocation, vector<int>& vecStressPos);
    ET_ReturnCode eCreateFormTemplate (ET_Gender, ET_Number, ET_Case, ET_Animacy, const CEString& sStem, 
                                       const CEString& sEnding, int64_t llEndingKey, shared_ptr<CWordForm>&);
    ET_ReturnCode eHandleCommonDeviations (shared_ptr<CWordForm>);

public:
    ET_ReturnCode eBuild();

private:
    CEString m_sStem;
    ET_AccentType m_eAccentType;
    ET_Subparadigm m_eSubparadigm;
    ET_Status m_eStatus;
    bool m_bIrregular;
};

}

#endif      //  FORMBUILDERPRONOUNADJ_H_INCLUDED
