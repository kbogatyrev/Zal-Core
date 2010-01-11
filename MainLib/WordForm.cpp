#include "StdAfx.h"
#include "WordForm.h"

HRESULT CT_WordForm::get_Wordform (BSTR * pbstr_wordForm)
{
    if (str_WordForm.empty())
    {
        return S_FALSE;
    }
    CComBSTR sp_bstr (str_WordForm.c_str());
    *pbstr_wordForm = sp_bstr.Detach();

    return S_OK;
}

HRESULT CT_WordForm::get_Lemma (BSTR * pbstr_lemma)
{
    if (str_Lemma.empty())
    {
        return S_FALSE;
    }
    CComBSTR sp_bstr (str_Lemma.c_str());
    *pbstr_lemma = sp_bstr.Detach();

    return S_OK;
}

HRESULT CT_WordForm::get_LexemeId (LONG *)
{
    return E_NOTIMPL;
}

HRESULT CT_WordForm::get_PartOfSpeech (ET_PartOfSpeech * pe_partOfSpeech)
{
    *pe_partOfSpeech = (ET_PartOfSpeech)eo_POS;
    return S_OK;
}

HRESULT CT_WordForm::get_AdjForm (ET_AdjForm * pe_adjForm)
{
    *pe_adjForm = (ET_AdjForm)eo_AdjForm;
    return S_OK;
}

HRESULT CT_WordForm::get_VerbForm (ET_VerbForm * pe_verbForm)
{
    *pe_verbForm = (ET_VerbForm)eo_VerbForm;
    return S_OK;
}

HRESULT CT_WordForm::get_Case (ET_Case * pe_case)
{
    *pe_case = (ET_Case)eo_Case;
    return S_OK;
}

HRESULT CT_WordForm::get_Number (ET_Number * pe_number)
{
    *pe_number = (ET_Number)eo_Number;
    return S_OK;
}

HRESULT CT_WordForm::get_Gender (ET_Gender * pe_gender)
{
    *pe_gender = (ET_Gender)eo_Gender;
    return S_OK;
}

HRESULT CT_WordForm::get_Tense (ET_Tense * pe_tense)
{
    *pe_tense = (ET_Tense)eo_Tense;
    return S_OK;
}

HRESULT CT_WordForm::get_Person (ET_Person * pe_person)
{
    *pe_person = (ET_Person)eo_Person;
    return S_OK;
}

HRESULT CT_WordForm::get_IsReflexive (ET_Reflexive * pe_reflexive)
{
    *pe_reflexive = eo_Reflexive;
    return S_OK;
}

HRESULT CT_WordForm::get_Animacy (ET_Animacy * pe_animacy)
{
    *pe_animacy = (ET_Animacy)eo_Animacy;
    return S_OK;
}

HRESULT CT_WordForm::get_Voice (ET_Voice * pe_voice)
{
    *pe_voice = (ET_Voice)eo_Voice;
    return S_OK;
}

HRESULT CT_WordForm::get_StressPos (long * pl_stressPos)
{
    *pl_stressPos = i_Stress;
    return S_OK;
}

/*
HRESULT CT_WordForm::get_SpecialForm (ET_SpecialForm * pe_specialForm)
{
    *pe_specialForm = (ET_SpecialForm)eo_Special;
    return S_OK;
}
*/
