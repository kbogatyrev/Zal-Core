#include "stdafx.h"
#include "Analyzer.h"
#include "Dictionary.h"
#include "Lexeme.h"

void CT_Analyzer::v_Init()
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

HRESULT CT_Analyzer::put_DbPath(BSTR bstr_Path)
{
    USES_CONVERSION;

    if (pco_db)
    {
        delete pco_db;
    }

    str_DbPath = OLE2W(bstr_Path);
    pco_db = new CT_Sqlite(str_DbPath);
    if (pco_db == NULL)
    {
        ERROR_LOG (L"Failed to instantiate CT_Sqlite.");
        return E_FAIL;
    }

    // Fill umap_freq_stems
    umap_freq_stems.clear();
    wstring str_query = L"Select * from stems_for_analysis"; // TODO: frequencies
    pco_db->v_PrepareForSelect(str_query);
    struct_stem_links s_links;
    wstring str_stem;
    while (pco_db->b_GetRow())
    {
        pco_db->v_GetData(0, s_links.i_stem_id);
        pco_db->v_GetData(1, s_links.ll_lexeme_id);
        pco_db->v_GetData(2, str_stem);
        pco_db->v_GetData(3, s_links.i_stress_pos);
        pco_db->v_GetData(4, s_links.i_endings_link);
        pco_db->v_GetData(5, s_links.str_lemma);
        umap_freq_stems.insert(unordered_multimap<wstring, struct_stem_links>::value_type(str_stem, s_links));
    }
    pco_db->v_Finalize();

    // Fill arr_freq_endings
    int i_subtable_id, i_cur_subtable_id = -1, i_hash, i_stress_pos;
    wstring str_ending;
    if (arr_freq_endings != NULL)
    {
        delete[] arr_freq_endings;
    }
    int i_last_subtable = pco_db->i_LastID(L"endings_for_analysis");
    if (i_last_subtable < -1)
    {
        return E_FAIL;
    }
    arr_freq_endings = new CT_EndingsTable[i_last_subtable + 1];
    str_query = L"Select * from endings_for_analysis order by subtable_id asc"; // TODO: frequencies
    pco_db->v_PrepareForSelect(str_query);
    CT_EndingsTable co_et;
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
    }
    pco_db->v_Finalize();
    
    return S_OK;
}

HRESULT CT_Analyzer::Analyze(BSTR bstr_Wordform)
{
    USES_CONVERSION;

    if (pco_db == NULL)
    {
        ERROR_LOG (L"DB pointer is NULL.");
        return E_FAIL;
    }
    
    m_coll.clear();
    vector<CT_Hasher> vec_possible_analyses;
    int i_wf_count = i_Analyze(OLE2W(bstr_Wordform), &vec_possible_analyses, FALSE); // FALSE for now
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
        CComObject<CT_WordForm> *co_wf = NULL;
        h_Hasher2Wordform(OLE2W(bstr_Wordform), *iter_wf, co_wf);
        m_coll.push_back(CComVariant(co_wf));
    }
    vec_possible_analyses.clear();
    return S_OK;
} // Analyze(BSTR bstr_Wordform)

int CT_Analyzer::i_Analyze(wstring str_wordform,
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
            i_CheckEndings(pvec_possible_wordforms, pvec_stems, str_left, str_right, i_StressPosEnding);
            if (!pvec_possible_wordforms->empty() && str_right.length() <= 3)
            {
                // The longer the ending, the more accurate the prediction
                // (But we exclude any predictions based on endings longer than 4.)
                break;
            }
        }
    }
    return pvec_possible_wordforms->size();
}

int CT_Analyzer::i_LookUpStems(vector<struct_stem_links>* pvec_stems,
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

int CT_Analyzer::i_CheckEndings(vector<CT_Hasher>* pvec_possible_wordforms,
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

        /*
        str_query = L"Select * from stems_for_analysis where id = " + str_ToString<int>(*iter_stems);
        pco_db->v_PrepareForSelect(str_query);
        while (pco_db->b_GetRow())
        {
            pco_db->v_GetData(4, i_subtable_id);
            pco_db->v_GetData(1, ll_lexeme_id);
            vec_subtable_id.push_back(i_subtable_id);
            vec_lexeme_id.push_back(ll_lexeme_id);
        }
        if (vec_subtable_id.empty() || vec_lexeme_id.empty())
        {
            return -2;
        }
        str_query = L"Select * from headword as a0 where exists (select * from descriptor as a1 where id = " +
                    str_ToString<__int64>((*iter_stems).ll_lexeme_id) + L" and a0.id = a1.word_id)";
        pco_db->v_PrepareForSelect(str_query);
        while (pco_db->b_GetRow())
        {
            pco_db->v_GetData(1, str_lemma);
            vec_lemma.push_back(str_lemma);
        }
        str_query = L"Select * from endings_for_analysis where subtable_id = " + str_ToString<int>(*(vec_subtable_id.begin())) 
            + L" and ending = \"" + str_right + L"\"";
        if (i_StressPosEnding >= -1)
        {
            str_query += L" and stress_pos = " + str_ToString<int>(i_StressPosEnding);
        }
        pco_db->v_PrepareForSelect(str_query);
        while (pco_db->b_GetRow())
        {
            int i_hash;
            pco_db->v_GetData(3, i_hash);
            vec_gram.push_back(i_hash);
        }
        */
        for (vector<int>::iterator iter_endings = vec_gram.begin();
            iter_endings != vec_gram.end(); iter_endings++)
        {
            CT_Hasher co_tmp_wf;
            co_tmp_wf.h_DecodeHash(*iter_endings);
            co_tmp_wf.ll_Lexeme_id = (*iter_stems).ll_lexeme_id;
            co_tmp_wf.str_Lemma = (*iter_stems).str_lemma;
            //co_tmp_wf.str_WordForm = str_left + str_right;
            h_AddClassifyingCategories(&co_tmp_wf);
            pvec_possible_wordforms->push_back(co_tmp_wf);
        }
        vec_lemma.clear(); // that vector is different for every stem found
    }

    if (pvec_stems->empty())
        // Try to guess the lexeme
    {
        /* --     Temporarily switched off     -- */


        /*
        // Initialization
        vec_subtable_id.clear();
        vec_gram.clear();

        wstring str_stem = L"";
        __int64 ll_lexemes_guessed = -1, i_endings_link;
        vector<wstring> vec_stems, vec_possible_lemmas;
        vector<int> vec_lexeme_id;
        vector<int>::iterator iter_hash, iter_subtable, iter_id;
        vector<wstring>::iterator iter_stems;

        // Find endings equal to str_right
        str_query = L"Select * from endings_for_analysis where ending = \"" 
            + str_right + L"\"";
        if (i_StressPosEnding >= -1)
        {
            str_query += L" and stress_pos = " + str_ToString<int>(i_StressPosEnding);
        }
        pco_db->v_PrepareForSelect(str_query);
        while (pco_db->b_GetRow())
        {
            int i_hash;
            pco_db->v_GetData(1, i_subtable_id);
            pco_db->v_GetData(3, i_hash);
            vec_gram.push_back(i_hash);
            vec_subtable_id.push_back(i_subtable_id);
        }
        if (vec_subtable_id.empty() || vec_gram.empty() 
            || vec_subtable_id.size() != vec_gram.size())
        {
            return -2;
        }
        // For every ending found, build a wordform
        for (iter_hash = vec_gram.begin(),
            iter_subtable = vec_subtable_id.begin();
            iter_hash != vec_gram.end();
            ++iter_hash, ++iter_subtable)
        {
            int i_descriptor_id;
            vec_lexeme_id.clear();
            vec_stems.clear();
            CComObject<CT_WordForm>* co_tmp_wf;
            CComObject<CT_WordForm>::CreateInstance(&co_tmp_wf);
            co_tmp_wf->h_DecodeHash(*iter_hash);

            int i_LemmaHash;
            if (co_tmp_wf->eo_POS == POS_NOUN)
            {
                i_LemmaHash = 4; // Nom. Sg.
            }
            else if (co_tmp_wf->eo_POS == POS_ADJ)
            {
                i_LemmaHash = 1413; // Plen. M. Nom. Sg.
            }
            else if (co_tmp_wf->eo_POS == POS_VERB)
            {
                if (co_tmp_wf->eo_Reflexive == REFL_NO)
                {
                    i_LemmaHash = 5982; // Inf. Nonrefl.
                }
                else
                {
                    i_LemmaHash = 5981; // Inf. Refl.
                }
            }
            else
            {
                continue; // Other Parts of speech: not yet implemented
            }

            // Find lexemes which have such an ending
            str_query = L"Select descriptor_id, stem from stems_for_analysis where endings_link = " 
                + str_ToString<int>(*iter_subtable);
            pco_db->v_PrepareForSelect(str_query);
            while (pco_db->b_GetRow())
            {
                pco_db->v_GetData(0, i_descriptor_id);
                pco_db->v_GetData(1, str_stem);
                vec_lexeme_id.push_back(i_descriptor_id);
                vec_stems.push_back(str_stem);
            }
            if (vec_lexeme_id.empty() || vec_stems.empty()
                || vec_lexeme_id.size() != vec_stems.size())
            {
                return -2;
            }

            vec_possible_lemmas.clear();
            int i_found_simple, i_found_tricky;
            i_found_simple = i_found_tricky = 0;
            for (iter_id = vec_lexeme_id.begin(),
                iter_stems = vec_stems.begin();
                iter_id != vec_lexeme_id.end();
                ++iter_id, ++iter_stems)
            // For every such lexeme, find the first its stem in the list
            // (because it is the stem of the lemma)
            {
                wstring str_stem_lemma = L"", str_ending_lemma = L"";
                str_query = L"Select * from stems_for_analysis as a1 where descriptor_id = " 
                    + str_ToString<int>(*iter_id) + L" and not exists (select * from stems_for_analysis as a2 where descriptor_id = "
                    + str_ToString<int>(*iter_id) + L" and a2.id < a1.id)";
                pco_db->v_PrepareForSelect(str_query);
                while (pco_db->b_GetRow())
                {
                    pco_db->v_GetData(2, str_stem_lemma);
                    pco_db->v_GetData(4, i_endings_link);
                }
                pco_db->v_Finalize();

                // Find the ending of the lemma
                str_query = L"Select * from endings_for_analysis where subtable_id = "
                    + str_ToString<int>(i_endings_link) + L" and gram_hash = "
                    + str_ToString<int>(i_LemmaHash);
                pco_db->v_PrepareForSelect(str_query);
                while (pco_db->b_GetRow())
                {
                    pco_db->v_GetData(2, str_ending_lemma);
                }
                pco_db->v_Finalize();

                if (i_endings_link == *iter_subtable)
                // If the ending of the lemma lies in the same endings table,
                // then just attach it to str_left
                {
                    vec_possible_lemmas.push_back(str_left + str_ending_lemma);
                    if (i_found_simple++ > 10)
                    {
                        break;
                    }
                    //co_tmp_wf->str_Lemma = str_left + str_ending_lemma;
                    //break;
                }
                else
                // If not, look in what way the stems differ
                {
                    wstring arr_str_stems[2];   // the lemma stem and the oblique stem
                    arr_str_stems[0] = *iter_stems;
                    arr_str_stems[1] = str_stem_lemma;
                    wstring **parr_str_pfx;     // their longest common prefix
                    parr_str_pfx = new wstring*;
                    *parr_str_pfx = new wstring[1];
                    i_LCP(arr_str_stems, parr_str_pfx, 2, 1);
                    if (parr_str_pfx == NULL || *parr_str_pfx == NULL)
                    {
                        return -2;
                    }
                    wstring str_pfx = (*parr_str_pfx)[0]; // their longest common prefix
                    wstring str_middle_wf, str_middle_stem;
                    // Suppose "королёк" is in the dictionary,
                    // but "кренделёк" isn't, and we need to parse "кренделька".
                    // We see:      корол|ёк|.    -       ?
                    //              корол|ьк|а    -   крендельк|а
                    // The stem "крендельк" could be transformed similarly,
                    // that is, by replacing "ьк" with "ёк". Thus, the lemma stem
                    // will be "кренделёк" (and the ending is the empty one).
                    // Here:
                    // str_middle_lemma = "ёк",
                    // str_middle_wf = "ьк",
                    // str_ending_lemma = "".
                    
                    str_middle_wf = (*iter_stems).substr(str_pfx.length(), 
                        (*iter_stems).length() - str_pfx.length());
                    str_middle_stem = str_stem_lemma.substr(str_pfx.length(), 
                        str_stem_lemma.length() - str_pfx.length());
                    if (str_left.substr(str_left.length() - str_middle_wf.length(),
                        str_middle_wf.length()) == str_middle_wf)
                    {
                        vec_possible_lemmas.push_back(str_left.substr(0,
                            str_left.length() - str_middle_wf.length()) 
                            + str_middle_stem + str_ending_lemma);
                        if (i_found_tricky++ > 10)
                        {
                            break;
                        }
                        //co_tmp_wf->str_Lemma = str_left.substr(0, str_left.length() - str_middle_wf.length())
                        //    + str_middle_stem + str_ending_lemma;
                        //break;
                    }
                }
            }
            //
            //if (co_tmp_wf->str_Lemma == L"")
            //{
            //    continue;
            //}
            //
            if (vec_possible_lemmas.empty())
            {
                continue;
            }
            v_DeleteRepeats(vec_possible_lemmas);
            for (vector<wstring>::iterator iter_lemma = vec_possible_lemmas.begin();
                iter_lemma != vec_possible_lemmas.end();
                ++iter_lemma)
            {
                // First, check for repeats
                bool b_exists = false;
                for (vector<CT_Hasher>::iterator iter_possible_wf = pvec_possible_wordforms->begin();
                    iter_possible_wf != pvec_possible_wordforms->end();
                    ++iter_possible_wf)
                {
                    if ((*iter_possible_wf).i_GramHash() == *iter_hash
                        && (*iter_possible_wf).str_Lemma == *iter_lemma)
                    {
                        b_exists = true;
                    }
                }
                if (b_exists)
                {
                    continue;
                }
                else
                // We've found a new possibility
                {
                    CComObject<CT_WordForm>* co_possible_wf;
                    CComObject<CT_WordForm>::CreateInstance(&co_possible_wf);
                    co_possible_wf->h_DecodeHash(*iter_hash);
                    co_possible_wf->str_Lemma = *iter_lemma;
                    co_possible_wf->ll_Lexeme_id = --ll_lexemes_guessed;
                    co_possible_wf->str_WordForm = str_left + str_right;
                    pvec_possible_wordforms->push_back(co_possible_wf);
                }
            }
        }
        */
    }
    vec_gram.clear();
    return 0;
}

HRESULT CT_Analyzer::h_AddClassifyingCategories(CT_Hasher *pco_wf)
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

void CT_Analyzer::v_DeleteRepeats(vector<wstring>& vec_strings)
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

HRESULT CT_Analyzer::h_Hasher2Wordform (const wstring& str_wordform,
                                        CT_Hasher co_from,
                                        CComObject<CT_WordForm> *& pco_to)
{
    HRESULT h_r = S_OK;

    h_r = CComObject<CT_WordForm>::CreateInstance (&pco_to);
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
