#pragma once
#include "Lexeme.h"
#include "WordForm.h"
#include "SqliteWrapper.h"
#include "Endings.h"

using namespace std;
using namespace std::tr1;

struct ST_IrregularForm
{
    wstring str_Form;
    map<int, ET_StressType> map_stress;
};

class CT_FormBuilderBase
{
public:
    CT_FormBuilderBase (CT_Lexeme * pco_lexeme, ET_Subparadigm eo_subparadigm) : 
      pco_Lexeme (pco_lexeme), eo_Subparadigm (eo_subparadigm), pco_Endings (NULL)
    {
        pco_Db = pco_lexeme->pco_Db;
        if (pco_Lexeme->b_HasIrregularForms)
        {
            h_GetIrregularForms();
        }
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
    }

protected:
    void v_ReportDbError();

    void v_AssignSecondaryStress (CComObject<CT_WordForm> * pco_wf);

    bool b_MultiStress (const wstring& str_lemma, vector<int>& vec_iStressPos);

    HRESULT h_CloneWordForm (const CComObject<CT_WordForm> * p_source, 
                             CComObject<CT_WordForm> *& p_clone);

    HRESULT h_CloneWordForm (const  CT_WordForm * co_source, 
                             CComObject<CT_WordForm> *& p_clone);

    HRESULT h_WordFormFromHash (int i_hash, int i_at, CT_WordForm *& pco_wf);

    int i_NForms (int i_hash);

    bool b_HasCommonDeviation (int i_cd);

    bool b_DeviationOptional (int i_cd);

    HRESULT h_RetractStressToPreverb (CT_WordForm *, bool b_isOptional);

    HRESULT h_GetIrregularForms();

protected:
    CT_Lexeme * pco_Lexeme;
    ET_Subparadigm eo_Subparadigm;
    CT_Sqlite * pco_Db;
    CT_Endings * pco_Endings;

//    map<int, wstring> map_IrregularForms;    // <hash, wordform>
    map<int, ST_IrregularForm> map_IrregularForms;  

private:
    CT_FormBuilderBase()
    {
        pco_Endings = NULL;
    }

};
