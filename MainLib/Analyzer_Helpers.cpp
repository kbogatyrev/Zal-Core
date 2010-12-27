#include "stdafx.h"
#include "Analyzer_Helpers.h"

CT_EndingsTable::CT_EndingsTable()
{
    /*
    pvec_Endings = new vector<wstring>;
    pvec_Gram = new vector<int>;
    pvec_Stress = new vector<int>;
    */
}

CT_EndingsTable::~CT_EndingsTable()
{
    Clear();
    /*
    delete pvec_Endings;
    delete pvec_Gram;
    delete pvec_Stress;
    */
}

void CT_EndingsTable::Clear()
{
    vec_Endings.clear();
    vec_Gram.clear();
    vec_Stress.clear();
}

int CT_EndingsTable::i_AddEnding(int i_Hash, wstring str_Ending, int i_StressPos)
{
    vec_Endings.push_back(str_Ending);
    vec_Gram.push_back(i_Hash);
    vec_Stress.push_back(i_StressPos);
    return 0;
}

int CT_EndingsTable::i_AddEnding(long l_Hash, wstring str_Ending, int i_StressPos)
{
    // TODO: replace int with long in the whole class
    vec_Endings.push_back(str_Ending);
    vec_Gram.push_back((int)l_Hash);
    vec_Stress.push_back(i_StressPos);
    return 0;
}

vector<int> CT_EndingsTable::vec_db_id(CT_Sqlite* pco_dbHandle)
// Tries to find the table in the db, returns -1 if there's no such table.
// If everything goes well, the output vector will contain just 1 element. But who knows what can happen.
{
    vector<int> vec_subtable_id, vec_entries_count, vec_result;
    if (pco_dbHandle == NULL ||
        vec_Endings.empty() || vec_Gram.empty() || vec_Stress.empty() ||
        vec_Endings.size() != vec_Gram.size() || vec_Endings.size() != vec_Stress.size())
    {
        return vec_result;
    }
    wstring str_query, str_count, str_gram, str_stress;
    int i_entries_count, i_subtable_id;

    vector<wstring>::iterator iter_endings = vec_Endings.begin();
    vector<int>::iterator iter_gram = vec_Gram.begin();
    vector<int>::iterator iter_stress = vec_Stress.begin();
    str_gram = str_ToString<int>(*iter_gram);
    str_stress = str_ToString<int>(*iter_stress);
    str_query += L"Select * from endings_for_analysis as a0 where ending = \"" 
        + *iter_endings + L"\" and gram_hash = " + str_gram 
        + L" and stress_pos = " + str_stress;
    ++iter_gram;
    ++iter_endings;
    ++iter_stress;
    if (vec_Endings.size() > 1)
    {
        for (int i_ = 1;
            iter_endings != vec_Endings.end();
            ++iter_endings, ++iter_gram, ++iter_stress, ++i_)
        {
            str_count = str_ToString<int>(i_);
            str_gram = str_ToString<int>(*iter_gram);
            str_stress = str_ToString<int>(*iter_stress);
            str_query += L" and exists (select * from endings_for_analysis as a" 
                + str_count + L" where ending = \"" + *iter_endings 
                + L"\" and gram_hash = " + str_gram 
                + L" and stress_pos = " + str_stress
                + L" and a" + str_count + L".subtable_id = a0.subtable_id)";
        }
    }
    pco_dbHandle->v_PrepareForSelect(str_query);
    while (pco_dbHandle->b_GetRow())
    {
        pco_dbHandle->v_GetData(1, i_subtable_id);
        vec_subtable_id.push_back(i_subtable_id);
    }
    pco_dbHandle->v_Finalize();

    // Now that we've learned the ids of possible subtables we must ensure they 
    // don't contain any entries except those specified by the input vectors.
    for (vector<int>::iterator iter_subtable_id = vec_subtable_id.begin();
        iter_subtable_id != vec_subtable_id.end();
        ++iter_subtable_id)
    {
        str_query = L"Select entries_count from endings_meta where subtable_id = " 
            + str_ToString<int>(*iter_subtable_id);
        pco_dbHandle->v_PrepareForSelect(str_query);
        while (pco_dbHandle->b_GetRow())
        {
            pco_dbHandle->v_GetData(0, i_entries_count);
            vec_entries_count.push_back(i_entries_count);
        }
        pco_dbHandle->v_Finalize();
        if (vec_entries_count.empty() == false
            && *(vec_entries_count.begin()) == vec_Gram.size())
        {
            vec_result.push_back(*iter_subtable_id);
            break;
        }
        vec_entries_count.clear();
    }
    return vec_result;
}

int CT_EndingsTable::i_db_Write(CT_Sqlite* pco_dbHandle)
// If a table already exists, return its subtable_id.
// If not, write the whole table to the db, then return its subtable_id.
// Return -1 or -2 on error.
{
    if (pco_dbHandle == NULL)
    {
        return -1;
    }
    int i_inserted = 0, i_subtable_id, i_search_result;
    vector<int> vec_search_result;
    vec_search_result = vec_db_id(pco_dbHandle);
    if (!vec_search_result.empty())
    {
        return *(vec_search_result.begin());
        // TODO: What if it contains more than one element?
    }

    // If we've reached this mark, it means that there's no such table in the db yet.
    // We should get the number of the last table in the db and save our table there.
    vector<wstring>::iterator iter_Ending;
    vector<int>::iterator iter_Gram;
    vector<int>::iterator iter_Stress;
    wstring str_query = L"Select * from endings_meta as a0 where not exists (select * from endings_meta as a1 where a1.id > a0.id)";
    pco_dbHandle->v_PrepareForSelect(str_query);
    while (pco_dbHandle->b_GetRow())
    {
        pco_dbHandle->v_GetData(0, i_search_result);
        vec_search_result.push_back(i_search_result);
    }
    pco_dbHandle->v_Finalize();

    if (vec_search_result.empty())
    {
        i_subtable_id = 0;
    }
    else if (vec_search_result.size() == 1)
    {
        i_subtable_id = *(vec_search_result.begin()) + 1;
    }
    else
    {
        return -2;      // Something wrong with the DB
    }

    for (iter_Ending = vec_Endings.begin(),
            iter_Gram = vec_Gram.begin(),
            iter_Stress = vec_Stress.begin();
        (iter_Ending != vec_Endings.end()) &&
            (iter_Gram != vec_Gram.end()) &&
            (iter_Stress != vec_Stress.end());
        ++iter_Ending, ++iter_Gram, ++iter_Stress, ++i_inserted)
    {
        // For each tuple <ending, grammatical parameters, stress position>,
        // insert it into the endings table.
        pco_dbHandle->v_PrepareForInsert(L"endings_for_analysis", 4);
        pco_dbHandle->v_Bind(1, i_subtable_id);  // 0-based
        pco_dbHandle->v_Bind(2, *iter_Ending);
        pco_dbHandle->v_Bind(3, *iter_Gram);
        pco_dbHandle->v_Bind(4, *iter_Stress);
        pco_dbHandle->v_InsertRow();
        pco_dbHandle->v_Finalize();
    }
    // Now, write auxiliary information to endings_meta
    pco_dbHandle->v_PrepareForInsert(L"endings_meta", 2);
    pco_dbHandle->v_Bind(1, i_subtable_id);
    pco_dbHandle->v_Bind(2, i_inserted);
    pco_dbHandle->v_InsertRow();
    pco_dbHandle->v_Finalize();
    return i_subtable_id;
}

vector<int> CT_EndingsTable::vec_Find(wstring str_ending, int i_stress_pos)
{
    vector<int> vec_i_result;
    if (vec_Endings.empty() || vec_Gram.empty() || vec_Stress.empty())
    {
        return vec_i_result;
    }
    vector<wstring>::iterator iter_endings;
    vector<int>::iterator iter_stress, iter_hash;
    for (iter_endings = vec_Endings.begin(),
            iter_stress = vec_Stress.begin(),
            iter_hash = vec_Gram.begin();
         iter_endings != vec_Endings.end();
         ++iter_endings, ++iter_stress, ++iter_hash)
    {
        if ((i_stress_pos < -1 || (i_stress_pos >= -1 && *iter_stress == i_stress_pos)) &&
            *iter_endings == str_ending)
        {
            vec_i_result.push_back(*iter_hash);
        }
    }
    return vec_i_result;
}

