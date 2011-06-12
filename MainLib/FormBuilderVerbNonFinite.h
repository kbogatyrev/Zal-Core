#pragma once
#include "resource.h"       // main symbols
#include "MainLib_i.h"
#include "WordForm.h"
#include "SqliteWrapper.h"
#include "Endings.h"
#include "FormBuilderBaseConj.h"

using namespace std;
using namespace std::tr1;

class CFormBuilderNonFinite : public CFormBuilderBaseConj
{
public:
    CFormBuilderNonFinite (CLexeme * pco_lexeme) : 
        CFormBuilderBaseConj (pco_lexeme, SUBPARADIGM_UNDEFINED)
    {}

public:
    HRESULT h_Build();

private:
    HRESULT h_BuildInfinitive();
    HRESULT h_BuildPresentActiveParticiple();
    HRESULT h_BuildPresentAdverbial();
    HRESULT h_BuildPresentPassiveParticiple();
    HRESULT h_BuildPastActiveParticiple();
    HRESULT h_BuildPastAdverbial();
    HRESULT h_BuildPastPassiveParticiple();

    void v_GetParticipleStressPos (ET_Subparadigm eo_subparadigm, vector<int>& vec_positions);

    int i_StressPosFromTemplate (const CWordForm& co_template, const wstring& str_lemma, int i_at = 0);

    HRESULT h_PresAdvType13();
    HRESULT h_PresAdvGeneral();

    //
    // Regular derivation off of irregularly derived stem
    //
    HRESULT h_DeriveIrregPresAdverbial();
    HRESULT h_DeriveIrregPresActiveParticiple();
    HRESULT h_DeriveIrregPresPassiveParticiple();

    HRESULT h_DeriveIrregPastActPartAndAdverbial();
    
    HRESULT h_BuildIrregParticipialFormsLong (ET_Subparadigm eo_sp);
    
    HRESULT h_DeriveIrregPartPresPassShort (ET_Status eo_status);
    HRESULT h_BuildIrregPartPastPassShort (bool& b_isVariant);
    HRESULT h_DeriveIrregPartPastPassShort();

    bool b_HasIrregPartPastPassShort();
};
