#pragma once
#include "FormBuilderBase.h"

using namespace std;
using namespace std::tr1;

class CT_FormBuilderBaseConj : public CT_FormBuilderBase
{
public:
    CT_FormBuilderBaseConj (CT_Lexeme * pco_lexeme, ET_Subparadigm eo_subparadigm) : 
        CT_FormBuilderBase (pco_lexeme, eo_subparadigm)
    {}

protected:
    HRESULT h_BuildVerbStems();
    HRESULT h_StandardAlternation (CT_ExtString& xstr_presentStem);
    HRESULT h_GetStemStressPositions (const wstring& str_lemma, 
                                      vector<int>& vec_iPosition);
    HRESULT h_GetEndingStressPosition (const wstring& str_lemma, 
                                       const wstring& str_ending,
                                       int& i_position);
    HRESULT h_BuildPastTenseStem (wstring& xstr_lemma);
    HRESULT h_FleetingVowelCheck (wstring& str_verbForm);
    HRESULT h_HandleYoAlternation (int i_stressPos,
                                   ET_Subparadigm eo_subParadigm, 
                                   wstring& str_lemma);
    HRESULT h_GetPastTenseStressTypes (ET_AccentType eo_accentType,
                                       ET_Number eo_number, 
                                       ET_Gender eo_gender,
                                       vector<ET_StressLocation>& vec_eoStress);
    bool b_HasIrregularPresent();
    bool b_HasIrregularPast();
    bool b_HasIrregularImperative();
    bool b_HasIrregularActivePartPast();
    bool b_HasIrregularPassivePartPast();

    HRESULT h_Get1SgIrregular (ST_IrregularForm& st_if);
    HRESULT h_Get3SgIrregular (ST_IrregularForm& st_if);
    HRESULT h_Get3PlIrregular (ST_IrregularForm& st_if);

    HRESULT h_GetPastMIrregular (ST_IrregularForm& st_if);
    HRESULT h_GetPastFIrregular (ST_IrregularForm& st_if);

    HRESULT h_GetPastPartActIrregular (ST_IrregularForm& st_if);

    HRESULT h_GetPastPartPassNSgIrregular (ST_IrregularForm& st_if);
};
