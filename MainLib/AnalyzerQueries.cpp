#include "stdafx.h"
#include "Analyzer.h"
#include "Dictionary.h"
#include "Lexeme.h"

using namespace std;
using namespace std::tr1;

int CT_Analyzer::i_LastID(wstring str_TableName)
{
    int i_last_id = 0;
    wstring str_query = L"Select * from " + str_TableName 
        + L" as a0 where not exists (select * from " + str_TableName 
        + L" as a1 where a1.id > a0.id)";
    pco_db->v_PrepareForSelect(str_query);
    pco_db->b_GetRow();
    pco_db->v_GetData(0, i_last_id);
    pco_db->v_Finalize();
    return i_last_id;
}

void CT_Analyzer::v_LongStemsBySubtable(int i_subtable,
                                        int i_min_len,
                                        vector<wstring>& vec_stems)
{
    wstring str_query = L"Select * from stems_for_analysis where endings_link = "
            + str_ToString<int>(i_subtable);
    wstring str_stem;
    pco_db->v_PrepareForSelect(str_query);
    while (pco_db->b_GetRow())
    {
        pco_db->v_GetData(2, str_stem);
        if (str_stem.length() > i_min_len)
        {
            vec_stems.push_back(str_stem);
        }
    }
    pco_db->v_Finalize();
}

void CT_Analyzer::v_InsertCommonSfx(wstring **parr_str_sfx,
                                    int i_sfx,
                                    int i_subtable)
{
    wstring str_sfx;
    for (int i_s = 0; i_s < i_sfx; ++i_s)
    {
        str_sfx = (*parr_str_sfx)[i_s];
        reverse(str_sfx.begin(), str_sfx.end());
        pco_db->v_PrepareForInsert(L"longest_common_suffixes", 2);
        pco_db->v_Bind(1, i_subtable);
        pco_db->v_Bind(2, str_sfx);
        pco_db->v_InsertRow();
        pco_db->v_Finalize();
    }
}

void CT_Analyzer::v_InsertStemsAndLinks(wstring **parr_str_stems,
                                        int *arr_i_subtable_id,
                                        int i_stems,
                                        __int64 ll_lexeme_id)
{
    for (int i_s = 0; i_s < i_stems; i_s++)
    {
        wstring str_stem = (*parr_str_stems)[i_s];
        int i_StressPos = i_DeleteStress(str_stem);
        if (i_StressPos >= str_stem.length())
        {
            // This in fact means that the ending is stressed
            i_StressPos = -1;
        }
        pco_db->v_PrepareForInsert(L"stems_for_analysis", 4);
        pco_db->v_Bind(1, ll_lexeme_id);
        pco_db->v_Bind(2, str_stem);
        pco_db->v_Bind(3, i_StressPos);
        pco_db->v_Bind(4, arr_i_subtable_id[i_s]);
        pco_db->v_InsertRow();
        pco_db->v_Finalize();
    }
}