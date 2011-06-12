#include "stdafx.h"
#include "Analyzer.h"
#include "Dictionary.h"
#include "Lexeme.h"

void CAnalyzer::v_Init()
{
    // Fill map_MainSymbol
    wstring arr_strMainSymbol[ ] = { L"м", L"мо", L"ж", L"жо", L"с", L"со", L"мо-жо", L"мн.",
            L"мн. неод.", L"мн. одуш.", L"мн. _от_", L"п", L"мс", L"мс-п", L"мс-предик.", L"числ.", L"числ.-п", 
            L"св", L"нсв", L"св-нсв", L"н", L"предл.", L"союз", L"предик.", L"вводн.", L"сравн.", 
            L"част.", L"межд.", L"NULL"  };
    for (ET_MainSymbol eo_ms = MS_START; eo_ms < MS_END; ++eo_ms)
    {
        map_MainSymbol[arr_strMainSymbol[eo_ms]] = eo_ms; 
    }
}

HRESULT CAnalyzer::put_DbPath(BSTR bstr_Path)
{
    USES_CONVERSION;

    if (pco_db)
    {
        delete pco_db;
    }

    str_DbPath = OLE2W(bstr_Path);
    pco_db = new CSqlite(str_DbPath);
    if (pco_db == NULL)
    {
        ERROR_LOG (L"Failed to instantiate CSqlite.");
        return E_FAIL;
    }

    // Fill umap_freq_stems
    umap_freq_stems.clear();
    wstring str_query = L"Select * from stems_for_analysis"; // TODO: frequencies
    pco_db->v_PrepareForSelect(str_query);
    struct_stem_links s_links;
    wstring str_stem = L"", str_stem_without_yo = L"";
    while (pco_db->b_GetRow())
    {
        pco_db->v_GetData(0, s_links.i_stem_id);
        pco_db->v_GetData(1, s_links.ll_lexeme_id);
        pco_db->v_GetData(2, str_stem);
        pco_db->v_GetData(3, s_links.i_stress_pos);
        pco_db->v_GetData(4, s_links.i_endings_link);
        pco_db->v_GetData(5, s_links.str_lemma);
        umap_freq_stems.insert(unordered_multimap<wstring, struct_stem_links>::value_type(str_stem, s_links));
        str_stem_without_yo = str_stem;
        replace(str_stem_without_yo.begin(), str_stem_without_yo.end(), L'ё', L'е');
        if (str_stem_without_yo != str_stem)
        {
            umap_freq_stems.insert(unordered_multimap<wstring, struct_stem_links>::value_type(str_stem_without_yo, s_links));
        }
    }
    pco_db->v_Finalize();

    // Fill arr_freq_endings
    int i_subtable_id, i_cur_subtable_id = -1, i_hash, i_stress_pos;
    wstring str_ending = L"", str_ending_without_yo = L"";
    if (arr_freq_endings != NULL)
    {
        delete[] arr_freq_endings;
    }
    int i_last_subtable = pco_db->i_LastID(L"endings_for_analysis");
    if (i_last_subtable < -1)
    {
        return E_FAIL;
    }
    arr_freq_endings = new CEndingsTable[i_last_subtable + 1];
    str_query = L"Select * from endings_for_analysis order by subtable_id asc";
    pco_db->v_PrepareForSelect(str_query);
    CEndingsTable co_et;
    while (pco_db->b_GetRow())
    {
        pco_db->v_GetData(1, i_subtable_id);
        pco_db->v_GetData(2, str_ending);
        pco_db->v_GetData(3, i_hash);
        pco_db->v_GetData(4, i_stress_pos);
        if (i_subtable_id != i_cur_subtable_id)
        {
            // Push the current subtable to the array
            if (i_cur_subtable_id >= 0)
            {
                arr_freq_endings[i_cur_subtable_id] = co_et;
            }
            i_cur_subtable_id = i_subtable_id;
            co_et.Clear();
        }
        co_et.i_AddEnding(i_hash, str_ending, i_stress_pos);
        str_ending_without_yo = str_ending;
        replace(str_ending_without_yo.begin(), str_ending_without_yo.end(), L'ё', L'е');
        if (str_ending_without_yo != str_ending)
        {
            co_et.i_AddEnding(i_hash, str_ending_without_yo, i_stress_pos);
        }
    }
    pco_db->v_Finalize();

    // Fill umap_endings2subtbl
    umap_endings2subtbl.clear();
    str_query = L"Select * from subtables_for_heuristics";
    pco_db->v_PrepareForSelect(str_query);
    int i_lexemes_count = 0, i_cut_right = 0;
    wstring str_stem_ending = L"", str_lemma_ending = L"";
    while (pco_db->b_GetRow())
    {
        pco_db->v_GetData(1, i_subtable_id);
        pco_db->v_GetData(2, i_lexemes_count);
        pco_db->v_GetData(3, str_stem_ending);
        pco_db->v_GetData(4, i_cut_right);
        pco_db->v_GetData(5, str_lemma_ending);
        arr_freq_endings[i_subtable_id].i_lexemes_count = i_lexemes_count;
        arr_freq_endings[i_subtable_id].i_cut_right = i_cut_right;
        arr_freq_endings[i_subtable_id].str_stem_ending = str_stem_ending;
        arr_freq_endings[i_subtable_id].str_lemma_ending = str_lemma_ending;
        for (vector<wstring>::iterator iter_ending = arr_freq_endings[i_subtable_id].vec_Endings.begin();
             iter_ending != arr_freq_endings[i_subtable_id].vec_Endings.end();
             ++iter_ending)
        {
            umap_endings2subtbl.insert(unordered_multimap<wstring, int>::value_type(*iter_ending, i_subtable_id));
        }
    }
    pco_db->v_Finalize();
    
    return S_OK;
}

HRESULT CAnalyzer::Analyze(BSTR bstr_Wordform)
{
    USES_CONVERSION;

    if (pco_db == NULL)
    {
        ERROR_LOG (L"DB pointer is NULL.");
        return E_FAIL;
    }
    
    m_coll.clear();
    vector<CT_Hasher> vec_possible_analyses;
    int i_wf_count = i_Analyze(OLE2W(bstr_Wordform), &vec_possible_analyses, TRUE);
    if (i_wf_count < 0)
    {
        ERROR_LOG (L"Negative number returned from i_Analyze.");
        return E_FAIL;
    }
    if (i_wf_count == 0)
    {
        return S_FALSE;
    }
    // Otherwise, i_wf_count > 0
    // Then, for every wordform found, put the data to the collection
    for (vector<CT_Hasher>::iterator iter_wf = vec_possible_analyses.begin();
        iter_wf != vec_possible_analyses.end();
        iter_wf++)
    {
        CComObject<CWordForm> *co_wf = NULL;
        h_Hasher2Wordform(OLE2W(bstr_Wordform), *iter_wf, co_wf);
        m_coll.push_back(CComVariant(co_wf));
    }
    vec_possible_analyses.clear();
    return S_OK;
} // Analyze(BSTR bstr_Wordform)

int CAnalyzer::i_Analyze(wstring str_wordform,
                           vector<CT_Hasher>* pvec_possible_wordforms,
                           BOOL b_guess)
{
    // Be careful: it changes the input
    if (str_wordform.length() <= 0 || pco_db == NULL || pvec_possible_wordforms == NULL)
    {
        return -1;
    }

    int i_StressPos = -1, i_StressPosStem = -1, i_StressPosEnding = -1;
    wsmatch result;
    bool b_match = regex_match(str_wordform, result, (const wregex)L"^([^<\u0301]*)([<\u0301])(.*)$");
    if (b_match == true)
    {
        wstring str_left = (wstring)result[1];
        wstring str_delimiter = (wstring)result[2];
        wstring str_right = (wstring)result[3];
        str_wordform = str_left + str_right;
        if (str_delimiter[0] == L'<')       // кор<ова
        {
            i_StressPos = str_left.length();
        }
        else                                // коро\u0301ва
        {
            i_StressPos = str_left.length() - 1;
        }
    }
    else
    {
        i_StressPos = -1;
    }

    wstring str_left, str_right;
    vector<struct_stem_links>* pvec_stems;
    pvec_possible_wordforms->clear();
    pvec_stems = new vector<struct_stem_links>;
    for (int i_left = str_wordform.length(); i_left >= 0; i_left--)
    {
        str_left = str_wordform.substr(0, i_left);
        str_right = str_wordform.substr(i_left, str_wordform.length() - i_left);

        // Stress positions for the stem and the ending
        if (i_StressPos == -1)
        {
            i_StressPosStem = i_StressPosEnding = -2;
        }
        else if (i_StressPos >= str_left.length())
        {
            i_StressPosStem = -1;
            i_StressPosEnding = i_StressPos - str_left.length();
        }
        else
        {
            i_StressPosStem = i_StressPos;
            i_StressPosEnding = -1;
        }

        pvec_stems->clear();
        i_LookUpStems(pvec_stems, str_left, i_StressPosStem);
        if (pvec_stems->empty())
        {
            continue;
        }
        i_CheckEndings(pvec_possible_wordforms, pvec_stems, str_left, str_right, i_StressPosEnding);
    }

    // Now, if we haven't found anything, we may guess the lexeme
    if (pvec_possible_wordforms->empty() && b_guess == TRUE)
    {
        for (int i_left = 0; i_left <= str_wordform.length(); ++i_left)
        {
            str_left = str_wordform.substr(0, i_left);
            str_right = str_wordform.substr(i_left, str_wordform.length() - i_left);

            // Stress positions for the stem and the ending
            if (i_StressPos == -1)
            {
                i_StressPosStem = i_StressPosEnding = -2;
            }
            else if (i_StressPos >= str_left.length())
            {
                i_StressPosStem = -1;
                i_StressPosEnding = i_StressPos - str_left.length();
            }
            else
            {
                i_StressPosStem = i_StressPos;
                i_StressPosEnding = -1;
            }
            pvec_stems->clear();
            i_CheckEndings(pvec_possible_wordforms, pvec_stems, str_left, str_right, i_StressPosEnding);
            if ((b_ContainsPlausibleVariants(pvec_possible_wordforms) && str_right.length() <= 3) ||
                pvec_possible_wordforms->size() >= 4)
            {
                break;
            }
        }
        if (pvec_possible_wordforms->size() > 4)
        {
            v_LeaveMostPlausible(pvec_possible_wordforms);
        }
    }
    return pvec_possible_wordforms->size();
}

int CAnalyzer::i_LookUpStems(vector<struct_stem_links>* pvec_stems,
                               wstring str_left,
                               int i_StressPosStem)
{
    // Search the DB for graphic stems equal to str_left;
    // write their IDs to vec_stems_id
    if (pco_db == NULL || pvec_stems == NULL)
    {
        return -1;
    }
    // First, try to find it among the frequent stems:
    pair<unordered_multimap<wstring, struct_stem_links>::iterator,
         unordered_multimap<wstring, struct_stem_links>::iterator> pair_search_result = umap_freq_stems.equal_range(str_left);
    for (; pair_search_result.first != pair_search_result.second; ++pair_search_result.first)
    {
        struct_stem_links s_links = pair_search_result.first->second;
        if (i_StressPosStem >= -1 && s_links.i_stress_pos != i_StressPosStem)
        {
            continue;
        }
        pvec_stems->push_back(s_links);
    }
    //if (!pvec_stems_id->empty())
    //{
        return pvec_stems->size();
    //}

    // Then, if we haven't got anything, look it up in the DB.
        /*
    wstring str_query = L"Select * from stems_for_analysis where stem = \"" + str_left + L"\"";
    if (i_StressPosStem >= -1)
    {
        str_query += L" and stress_pos = " + str_ToString<int>(i_StressPosStem);
    }
    int i_stem_id;
    pco_db->v_PrepareForSelect(str_query);
    while (pco_db->b_GetRow())
    {
        pco_db->v_GetData(0, i_stem_id);
        pvec_stems_id->push_back(i_stem_id);
    }
    return pvec_stems_id->size();
    */
}

int CAnalyzer::i_CheckEndings(vector<CT_Hasher>* pvec_possible_wordforms,
                                vector<struct_stem_links>* pvec_stems,
                                wstring str_left,
                                wstring str_right,
                                int i_StressPosEnding)
// If pvec_stems_id IS NOT empty:
// For every stem in pvec_stems_id, take the corresponding endings table
// and look whether it contains an ending equal to str_right;
// for every such ending, add a wordform to pvec_possible_wordforms.
//
// If pvec_stems_id IS empty:
// Look for an ending equal to str_right; for every such ending,
// build a wordform and store it in pvec_possible_wordforms.
// (Identical wordforms are stored as one wordform.)
{
    if (pco_db == NULL || pvec_stems == NULL)
    {
        return -1;
    }
    static vector<int> vec_gram;
    wstring str_query, str_lemma;
    vector<wstring> vec_lemma;

    for (vector<struct_stem_links>::iterator iter_stems = pvec_stems->begin();
        iter_stems != pvec_stems->end(); iter_stems++)
    {
        // For each *iter_stems look up the endings table ID in DB, then in this table try to find
        // endings which are equal to str_right. For each ending found, write the parameters
        // to co_tmp_wf and then push_back co_tmp_wf to pvec_possible_wordforms:
        vec_gram.clear();
        vec_gram = arr_freq_endings[(*iter_stems).i_endings_link].vec_Find(str_right, i_StressPosEnding);
        if (vec_gram.empty())
        {
            continue;
        }
        for (vector<int>::iterator iter_endings = vec_gram.begin();
            iter_endings != vec_gram.end(); iter_endings++)
        {
            CT_Hasher co_tmp_wf;
            co_tmp_wf.h_DecodeHash(*iter_endings);
            co_tmp_wf.ll_Lexeme_id = (*iter_stems).ll_lexeme_id;
            co_tmp_wf.str_Lemma = (*iter_stems).str_lemma;
            //co_tmp_wf.str_WordForm = str_left + str_right;
            //h_AddClassifyingCategories(&co_tmp_wf);
            pvec_possible_wordforms->push_back(co_tmp_wf);
        }
        vec_lemma.clear(); // that vector is different for every stem found
    }

    if (pvec_stems->empty())
    // Try to guess the lexeme
    {
        if (str_left.length() <= 2)
        {
            return 0;
        }
        vector<int> vec_i_possible_ETs;
        pair<unordered_multimap<wstring, int>::iterator,
             unordered_multimap<wstring, int>::iterator> pair_search_result = umap_endings2subtbl.equal_range(str_right);
        for (; pair_search_result.first != pair_search_result.second; ++pair_search_result.first)
        {
            vec_i_possible_ETs.push_back(pair_search_result.first->second);
        }
        for (vector<int>::iterator iter_ET = vec_i_possible_ETs.begin();
             iter_ET != vec_i_possible_ETs.end();
             ++iter_ET)
        {
            if (arr_freq_endings[*iter_ET].str_stem_ending.length() > 0 &&
                !regex_match(str_left, (const wregex)(L"^.*(" + arr_freq_endings[*iter_ET].str_stem_ending + L")$")))
            {
                continue;
            }
            if (str_left.length() <= arr_freq_endings[*iter_ET].i_cut_right)
            {
                continue;
            }
            vec_gram.clear();
            vec_gram = arr_freq_endings[*iter_ET].vec_Find(str_right, -2);
            if (vec_gram.empty())
            {
                continue;
            }
            for (vector<int>::iterator iter_hash = vec_gram.begin();
                 iter_hash != vec_gram.end(); ++iter_hash)
            {
                CT_Hasher co_tmp_wf;
                co_tmp_wf.str_Lemma = str_left.substr(0, str_left.length() - arr_freq_endings[*iter_ET].i_cut_right) + arr_freq_endings[*iter_ET].str_lemma_ending;
                if (!b_IsValidLemma(co_tmp_wf.str_Lemma))
                {
                    continue;
                }
                // Check if what we've found is a new wordform
                bool b_exists = false;
                for (vector<CT_Hasher>::iterator iter_wf = pvec_possible_wordforms->begin();
                     iter_wf != pvec_possible_wordforms->end();
                     ++iter_wf)
                {
                    if ((*iter_wf).str_Lemma == co_tmp_wf.str_Lemma &&
                        (*iter_wf).i_GramHash() == *iter_hash)
                    {
                        b_exists = true;
                    }
                }
                if (!b_exists)
                {
                    co_tmp_wf.h_DecodeHash(*iter_hash);
                    co_tmp_wf.ll_Lexeme_id = 0;
                    pvec_possible_wordforms->push_back(co_tmp_wf);
                }
            }
        }
    }
    vec_gram.clear();
    return 0;
}

HRESULT CAnalyzer::h_AddClassifyingCategories(CT_Hasher *pco_wf)
{
    if (pco_wf == NULL || pco_db == NULL)
    {
        return E_FAIL;
    }
    wstring str_query = L"select * from descriptor where id = " + str_ToString<__int64>(pco_wf->ll_Lexeme_id);
    wstring str_MainSymbol = L"";
    pco_db->v_PrepareForSelect(str_query);
    pco_db->b_GetRow();
    pco_db->v_GetData(4, str_MainSymbol);

    ET_MainSymbol eo_ms = map_MainSymbol[str_MainSymbol];
    switch (eo_ms)
    {
        case MS_M:
            pco_wf->eo_Gender = GENDER_M;
            pco_wf->eo_Animacy = ANIM_NO;
            break;
        case MS_MO:
            pco_wf->eo_Gender = GENDER_M;
            pco_wf->eo_Animacy = ANIM_YES;
            break;
        case MS_ZH:
            pco_wf->eo_Gender = GENDER_F;
            pco_wf->eo_Animacy = ANIM_NO;
            break;
        case MS_ZHO:
            pco_wf->eo_Gender = GENDER_F;
            pco_wf->eo_Animacy = ANIM_YES;
            break;
        case MS_S:
            pco_wf->eo_Gender = GENDER_N;
            pco_wf->eo_Animacy = ANIM_NO;
            break;
        case MS_SO:
            pco_wf->eo_Gender = GENDER_N;
            pco_wf->eo_Animacy = ANIM_YES;
            break;
        case MS_MO_ZHO:
            pco_wf->eo_Animacy = ANIM_YES;
            break;
        case MS_MN_NEOD:
            pco_wf->eo_Animacy = ANIM_NO;
            break;
        case MS_MN_ODUSH:
            pco_wf->eo_Animacy = ANIM_YES;
            break;

        case MS_SV:
            pco_wf->eo_Aspect = ASPECT_PERFECTIVE;
            break;
        case MS_NSV:
            pco_wf->eo_Aspect = ASPECT_IMPERFECTIVE;
            break;
    }    // switch
    return S_OK;
}

void CAnalyzer::v_DeleteRepeats(vector<wstring>& vec_strings)
{
    if (vec_strings.empty())
    {
        return;
    }
    wstring str_previous = L"";
    sort(vec_strings.begin(), vec_strings.end());
    for (vector<wstring>::iterator iter = vec_strings.begin();
        iter != vec_strings.end();
        ++iter)
    {
        if (*iter == str_previous)
        {
            vector<wstring>::iterator iter_erase = iter;
            --iter;
            if (iter == vec_strings.begin())
            {
                str_previous = L"";
            }
            else
            {
                --iter;
                str_previous = *iter;
                ++iter;
            }
            vec_strings.erase(iter_erase);
        }
        else
        {
            str_previous = *iter;
        }
    }
}

HRESULT CAnalyzer::h_Hasher2Wordform (const wstring& str_wordform,
                                        CT_Hasher co_from,
                                        CComObject<CWordForm> *& pco_to)
{
    HRESULT h_r = S_OK;

    h_r = CComObject<CWordForm>::CreateInstance (&pco_to);
    if (S_OK != h_r)
    {
        ATLASSERT(0);
        CString cs_msg;
        cs_msg.Format (L"CreateInstance() failed, error %x.", h_r);
        ERROR_LOG ((LPCTSTR)cs_msg);
        throw CT_Exception (E_INVALIDARG, (LPCTSTR)cs_msg);
    }

    pco_to->eo_POS = co_from.eo_POS;
    pco_to->eo_Subparadigm = co_from.eo_Subparadigm;
    pco_to->str_Lemma = co_from.str_Lemma;
    pco_to->eo_Animacy = co_from.eo_Animacy;
    pco_to->eo_Aspect = co_from.eo_Aspect;
    pco_to->eo_Case = co_from.eo_Case;
    pco_to->eo_Gender = co_from.eo_Gender;
    pco_to->eo_Number = co_from.eo_Number;
    pco_to->eo_Person = co_from.eo_Person;
    pco_to->eo_Reflexive = co_from.eo_Reflexive;
    pco_to->ll_Lexeme_id = co_from.ll_Lexeme_id;
    pco_to->str_WordForm = str_wordform;
    // TODO: stress position
    return S_OK;
}

bool CAnalyzer::b_IsValidLemma(wstring str_wf)
{
    if (!regex_search(str_wf, (const wregex)(L"[аеёиоуыэюяАЕЁИОУЫЭЮЯ]")))
    {
        return false;
    }
    if (regex_search(str_wf, (const wregex)(L"[аеёиоуыэюяъь][ьъ]")))
    {
        return false;
    }
    if (regex_search(str_wf, (const wregex)(L"ъ[аоуыэи]")))
    {
        return false;
    }
    if (regex_search(str_wf, (const wregex)(L"[аоэуе][аоэуы]ть$")))
    {
        return false;
    }
    if (regex_search(str_wf, (const wregex)(L"[кгхц]ь$")))
    {
        return false;
    }
    if (regex_search(str_wf, (const wregex)(L"[кгх]ый$")))
    {
        return false;
    }
    if (regex_search(str_wf, (const wregex)(L"[жчшщ]ы")))
    {
        return false;
    }
    if (regex_search(str_wf, (const wregex)(L"ы$")))
    {
        return false;
    }
    if (regex_search(str_wf, (const wregex)(L"[бвгджзклмнпрстфхцчшщ](й|ъ$)")))
    {
        return false;
    }
    if (regex_search(str_wf, (const wregex)(L"[бвгджзклмнпрстфхцчшщ]{4}$")))
    {
        return false;
    }
    return true;
}

bool CAnalyzer::b_ContainsPlausibleVariants(vector<CT_Hasher>* pvec_possible_wordforms)
{
    if (pvec_possible_wordforms == NULL)
    {
        return false;
    }
    for (vector<CT_Hasher>::iterator iter_wf = pvec_possible_wordforms->begin();
         iter_wf != pvec_possible_wordforms->end();
         ++iter_wf)
    {
        if (i_Plausibility(*iter_wf) >= 4)
        {
            return true;
        }
    }
    return false;
}

int CAnalyzer::i_Plausibility(CT_Hasher co_wf)
{
    if (co_wf.eo_POS == POS_PRONOUN ||
        co_wf.eo_POS == POS_PRONOUN_ADJ ||
        co_wf.eo_POS == POS_PRONOUN_PREDIC ||
        co_wf.eo_POS == POS_NUM ||
        co_wf.eo_POS == POS_NUM_ADJ ||
        co_wf.eo_POS == POS_PREDIC ||
        co_wf.eo_POS == POS_PREP ||
        co_wf.eo_POS == POS_CONJ ||
        co_wf.eo_POS == POS_PARTICLE ||
        co_wf.eo_POS == POS_INTERJ ||
        co_wf.eo_POS == POS_PARENTH ||
        co_wf.eo_POS == POS_INTERJ ||
        co_wf.eo_POS == POS_NULL ||
        co_wf.eo_POS == POS_UNDEFINED)
    {
        return 0;
    }
    else if (co_wf.eo_Subparadigm == SUBPARADIGM_COMPARATIVE ||
             co_wf.eo_POS == POS_COMPAR)
    {
        return 1;
    }
    else if (co_wf.eo_Subparadigm == SUBPARADIGM_PART_PRES_PASS_SHORT ||
             co_wf.eo_Subparadigm == SUBPARADIGM_PART_PAST_PASS_SHORT ||
             co_wf.eo_Subparadigm == SUBPARADIGM_SHORT_ADJ)
    {
        return 2;
    }
    else if (co_wf.eo_POS == POS_ADV ||
             co_wf.eo_Subparadigm == SUBPARADIGM_IMPERATIVE ||
             co_wf.eo_Subparadigm == SUBPARADIGM_ADVERBIAL_PAST ||
             co_wf.eo_Subparadigm == SUBPARADIGM_ADVERBIAL_PRESENT ||
             (co_wf.eo_POS == POS_NOUN && co_wf.eo_Animacy == ANIM_YES))
    {
        return 3;
    }
    else if (co_wf.eo_Subparadigm == SUBPARADIGM_PRESENT_TENSE ||
             co_wf.eo_Subparadigm == SUBPARADIGM_PAST_TENSE ||
             co_wf.eo_Subparadigm == SUBPARADIGM_PART_PAST_PASS_LONG)
    {
        return 4;
    }
    else if (co_wf.eo_POS == POS_NOUN && co_wf.eo_Animacy == ANIM_NO && co_wf.eo_Gender == GENDER_N)
    {
        return 5;
    }
    else if (co_wf.eo_POS == POS_NOUN && co_wf.eo_Animacy == ANIM_NO && co_wf.eo_Gender == GENDER_M)
    {
        return 6;
    }
    else if (co_wf.eo_POS == POS_NOUN && co_wf.eo_Animacy == ANIM_NO && co_wf.eo_Gender == GENDER_F)
    {
        return 7;
    }
    return 8;
}

void CAnalyzer::v_LeaveMostPlausible(vector<CT_Hasher>* pvec_possible_wordforms)
{
    if (pvec_possible_wordforms == NULL || pvec_possible_wordforms->size() <= 4)
    {
        return;
    }
    vector<CT_Hasher> vec_plausible_wordforms;
    int i_plausibility_level = 8;
    while (vec_plausible_wordforms.size() < 4 && i_plausibility_level > 0)
    {
        for (int i_wf = pvec_possible_wordforms->size() - 1; i_wf >= 0; --i_wf)
        {
            if (i_Plausibility((*pvec_possible_wordforms)[i_wf]) == i_plausibility_level)
            {
                vec_plausible_wordforms.push_back((*pvec_possible_wordforms)[i_wf]);
            }
        }
        --i_plausibility_level;
    }
    pvec_possible_wordforms->clear();
    *pvec_possible_wordforms = vec_plausible_wordforms;
}
