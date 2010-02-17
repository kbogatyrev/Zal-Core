#pragma once
#include "resource.h"       // main symbols
#include "MainLib_i.h"
#include "Lexeme.h"
#include "WordForm.h"
#include "SqliteWrapper.h"
#include "Endings.h"

using namespace std;
using namespace std::tr1;

class CT_FormBuilderBase
{
protected:
    CT_Lexeme * pco_Lexeme;
    CT_Sqlite * pco_Db;
    CT_Endings * pco_Endings;

    void v_ReporDbError();

    HRESULT h_GetCommonDeviation (int i_cdNum, St_CommonDeviation& st_data);

    HRESULT h_HandleDeclFleetingVowel (ET_Number eo_number,              // in
                                       ET_Case eo_case,                  // in
                                       ET_Gender eo_gender,              // in
                                       ET_EndingStressType eo_stressType,// in
                                       ET_Subparadigm eo_subparadigm,    // in
                                       const wstring& str_ending,        // in
                                       wstring& str_lemma);              // out

    HRESULT h_GetDeclStemStressPosition (const wstring& str_lemma, 
                                         vector<int>& vec_iPositions, 
                                         ET_Subparadigm eo_subParadigm);

    HRESULT h_GetDeclEndingStressPosition (const wstring& str_lemma, 
                                           const wstring& str_ending,
                                           int& i_position);

    HRESULT h_HandleDeclYoAlternation (ET_EndingStressType eo_stressType, 
                                       int i_stressPos,
                                       ET_Subparadigm eo_subParadigm, 
                                       wstring& str_lemma);

    HRESULT h_GetVerbStemStressPosition (const wstring& str_lemma, 
                                         vector<int>& vec_iPosition);

    HRESULT h_GetVerbEndingStressPosition (const wstring& str_lemma, 
                                           const wstring& str_ending,
                                           int& i_position);

    HRESULT h_BuildPastTenseStem (CT_ExtString& xstr_lemma);

    HRESULT h_StressOnPastTenseEnding (ET_AccentType eo_accentType,
                                       ET_Number eo_number, 
                                       ET_Gender eo_gender);

    HRESULT h_CloneWordForm (CComObject<CT_WordForm> * p_source, 
                             CComObject<CT_WordForm> *& p_clone);

private:
    CT_FormBuilderBase()
    {
        pco_Endings = NULL;
    }

public:
    CT_FormBuilderBase (CT_Lexeme * pco_lexeme) : pco_Lexeme (pco_lexeme), pco_Endings (NULL)
    {
        pco_Db = pco_lexeme->pco_Db;
    }

    virtual ~CT_FormBuilderBase()
    {
        //
        // Delete a member that may have been created by a subclass
        //
        if (pco_Endings)
        {
            delete pco_Endings;
            pco_Endings = NULL;
        }
    };

};
