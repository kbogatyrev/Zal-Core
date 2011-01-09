#include "stdafx.h"
#include "Analyzer.h"
#include "Dictionary.h"
#include "Lexeme.h"

using namespace std;
using namespace std::tr1;

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

    // Create and initialize an IDictionary object to work with
    HRESULT h_r = S_OK;
    h_r = sp_dict.CoCreateInstance(__uuidof(ZalDictionary));
    if (h_r != S_OK)
    {
        ERROR_LOG (L"CreateInstance failed for ZalDictionary.");
        return h_r;
    }
    sp_dict->put_DbPath(bstr_Path);

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
    int i_last_subtable = i_LastID(L"endings_for_analysis");
    if (i_last_subtable <= 0)
    {
        return E_FAIL;
    }
    arr_freq_endings = new CT_EndingsTable[i_last_subtable];
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
        /*CComQIPtr<IWordForm> spqi_wf &(*iter_wf);
        if (!spqi_wf)
        {
            ATLASSERT(0);
            ERROR_LOG (L"QI for IWordForm failed.");
            return E_NOINTERFACE;
        }*/
        m_coll.push_back(CComVariant(co_wf));
    }
    vec_possible_analyses.clear();
    return S_OK;
} // Analyze(BSTR bstr_Wordform)

HRESULT CT_Analyzer::PrepareLexeme(__int64 ll_lexeme_id, BOOL b_Stress)
{
    if (ll_lexeme_id < 0)
    {
        ERROR_LOG (L"Bad lexeme id.");
        return E_FAIL;
    }
    HRESULT h_r = PrepareLexemes(ll_lexeme_id, ll_lexeme_id, b_Stress);
    return h_r;
}

HRESULT CT_Analyzer::PrepareLexemes(__int64 ll_start, __int64 ll_end, BOOL b_Stress)
{
    USES_CONVERSION;
    
    if (ll_start < 0 || ll_end < 0 || pco_db == NULL || sp_dict == NULL)
    {
        ERROR_LOG (L"Illegal argument.");
        return E_INVALIDARG;
    }

    if (ll_start == 0 && ll_end == 0)
    {
        // Call the i_ClassifyStems function instead of preparing lexemes
        i_ClassifyStems();
        return S_OK;
    }

    // TEST
    b_Stress = TRUE;

    wstring str_Stem;
    int i_MaxStems = 0;
    long l_Items = 0, l_Id = 0, l_Wordforms = 0;
    __int64 ll_lexeme_id;
    HRESULT h_r;
    wsmatch result;
    wstring *arr_str_Paradigm, **parr_str_Stems, str_ending;
    parr_str_Stems = new wstring*;
    *parr_str_Stems = new wstring[1];
    bool b_match;

    pco_db->v_BeginTransaction();
    
    for (ll_lexeme_id = ll_start; 
        ll_lexeme_id <= ll_end; 
        ll_lexeme_id++) // for every lexeme within the range
    {
        int *arr_i_subtable_id, *arr_i_Hash;
        parr_str_Stems = new wstring*;
        *parr_str_Stems = new wstring[1];
        int i_wf = 0;
        CComPtr<ILexeme> sp_lex;

        // Find the lexeme in the db; generate its paradigm
        // (i_th element of arr_str_Paradigm has form arr_l_Hash[i])
        try
        {
            h_r = sp_dict->GetLexeme((LONG)ll_lexeme_id, &sp_lex);
            if (h_r != S_OK)
            {
                ERROR_LOG (L"GetLexeme() failed.");
                return h_r;
            }
            CComBSTR bstr_Lemma;
            sp_lex->get_InitialForm(&bstr_Lemma);
            ET_PartOfSpeech eo_pos = POS_UNDEFINED;
            h_r = sp_lex->get_PartOfSpeech(&eo_pos);
            // --------- Test ----------
            if (eo_pos != POS_NOUN && eo_pos != POS_ADJ && eo_pos != POS_VERB)
            {
                continue;
            }
            // --------- /Test ----------
            if (eo_pos == POS_VERB)
            {
                i_MaxStems = 5;
            }
            else
            {
                i_MaxStems = 3;
            }
            h_r = sp_lex->GenerateWordForms();
            if (h_r == E_UNEXPECTED) // Just skip it
            {
                continue;
            }
            if (h_r != S_OK)
            {
                ERROR_LOG (L"GenerateWordForms() failed.");
                continue;
                //return h_r;
            }
            h_r = sp_lex->get_Count(&l_Wordforms);
            if (l_Wordforms <= 0)
            {
                ERROR_LOG (L"l_Wordforms = " + str_ToString<long>(l_Wordforms) + L".");
                //return E_FAIL;
                continue;
            }

            arr_str_Paradigm = new wstring[l_Wordforms];
            arr_i_Hash = new int[l_Wordforms];
            for (long l_Item = 1; l_Item <= l_Wordforms; ++l_Item)
            {
                CComVariant sp_disp = NULL;
                h_r = sp_lex->get_Item(l_Item, &sp_disp);
                if (h_r != S_OK)
                {
                    ERROR_LOG (L"get_Item failed.");
                    continue;
                    //return h_r;
                }
                CComQIPtr<IWordForm> sp_wf = sp_disp.pdispVal;
                CComBSTR bstr_WordForm;
                wstring str_WordForm;
                long l_Hash = -1;
                sp_wf->get_Wordform(&bstr_WordForm);
                if (bstr_WordForm.Length() <= 0)
                {
                    ERROR_LOG (L"Empty wordform!");
                }
                sp_wf->get_Hash(&l_Hash);
                arr_i_Hash[l_Item - 1] = (int)l_Hash; // TODO: it should be of the same type throughout the whole project
                str_WordForm = OLE2W(bstr_WordForm);
                if (b_Stress)                   // TODO: What if there are several stresses?
                {
                    long l_Stress = -1;
                    sp_wf->get_Stress(&l_Stress);
                    arr_str_Paradigm[l_Item - 1] = str_InsertStress(l_Stress,
                                                              OLE2W(bstr_WordForm));
                }
                else
                {
                    arr_str_Paradigm[l_Item - 1] = OLE2W(bstr_WordForm);
                }

                sp_disp.Clear();
            }
            //sp_lex.Release(); // We don't need the lexeme anymore
            
            // Create several gr_stems
            int i_Stems = i_LCP(arr_str_Paradigm, parr_str_Stems, (int)l_Wordforms, i_MaxStems);
            
            // Split every wordform into stem and ending,
            // then for every gr_stem create a table of endings usable with it
            CT_EndingsTable *arr_co_endings;
            arr_co_endings = new CT_EndingsTable[i_Stems];
            arr_i_subtable_id = new int[i_Stems];
            for (i_wf = 0; i_wf < l_Wordforms; i_wf++)
            {
                for (int i_s = 0; i_s < i_Stems; i_s++)
                {
                    b_match = regex_match(arr_str_Paradigm[i_wf],
                        result, (const wregex)((*parr_str_Stems)[i_s] + L"(.*)"));
                    if (b_match == true)
                    {
                        str_ending = (wstring)result[1];
                        // TODO: Several stress positions
                        int i_StressPos = i_DeleteStress(str_ending);
                        if ((*parr_str_Stems)[i_s].substr((*parr_str_Stems)[i_s].length() - 1, 1) == L"<")
                        {
                            i_StressPos = 0;
                        }
                        // Write the ending and its grammatical meaning 
                        // to the temporary "endings table" no. i_s
                        arr_co_endings[i_s].i_AddEnding(arr_i_Hash[i_wf],
                            str_ending, i_StressPos);
                        break;
                    }
                }
            }
            // Write the "endings tables" to the db if necessary;
            // get their subtable_id's
            for (int i_s = 0; i_s < i_Stems; i_s++)
            {
                arr_i_subtable_id[i_s] = arr_co_endings[i_s].i_db_Write(pco_db);
            }
            // Add the stems with links to the corresponding subtables to the db
            v_InsertStemsAndLinks(parr_str_Stems, arr_i_subtable_id, i_Stems, ll_lexeme_id);

            // Delete the arrays
            for (int i_ = 0; i_ < i_Stems; ++i_)
            {
                arr_co_endings[i_].~CT_EndingsTable();
            }
            delete[] arr_str_Paradigm;
            delete[] arr_i_Hash;
            delete[] arr_i_subtable_id;
            delete[] (*parr_str_Stems);
            delete parr_str_Stems;

            // Commit transactions approximately every 200 entries
            if (ll_lexeme_id % 200 == 0)
            {
                pco_db->v_CommitTransaction();
                pco_db->v_BeginTransaction();
            }
        }
        catch (...)
        {
            ERROR_LOG (L"Exception thrown.");
            //return E_FAIL;
            continue;
        }
    } // for (ll_lexeme_id = ll_start; ll_lexeme_id <= ll_end; ll_lexeme_id++)...
    pco_db->v_CommitTransaction();

    sp_dict->Clear();

    CoUninitialize();

    return S_OK;
} // PrepareLexemes(__int64 ll_start, __int64 ll_end)

int CT_Analyzer::i_ClassifyStems()
// For every endings subtable, looks for the stems usable with it and
// stores up to NUM_SFX their longest common suffixes in the database
{
    if (pco_db == NULL)
    {
        return -1;
    }
    const int MIN_NUMBER_OF_STEMS = 11;
    const int NUM_SFX = 10;

    wstring str_query, str_stem;
    wstring *arr_str_stems;
    wstring **parr_str_sfx;
    vector<wstring> vec_stems;
    int i_last_subtable = 0, i_stem;

    i_last_subtable = i_LastID(L"endings_meta");
    for (int i_subtable = 1; i_subtable < i_last_subtable; ++i_subtable)
    {
        v_LongStemsBySubtable(i_subtable, 2, vec_stems);
        if (vec_stems.size() < MIN_NUMBER_OF_STEMS)
        {
            continue;
        }
        i_stem = 0;
        arr_str_stems = new wstring[vec_stems.size()];
        parr_str_sfx = new wstring*;
        *parr_str_sfx = new wstring[1];
        for (vector<wstring>::iterator iter_stems = vec_stems.begin();
            iter_stems != vec_stems.end();
            ++iter_stems, ++i_stem)
        {
            // We reverse the stem so that i_LCP could find suffixes
            // instead of prefixes
            reverse((*iter_stems).begin(), (*iter_stems).end());
            arr_str_stems[i_stem] = *iter_stems;
        }
        int i_sfx = i_LCP(arr_str_stems, parr_str_sfx, vec_stems.size(), NUM_SFX);
        v_InsertCommonSfx(parr_str_sfx, i_sfx, i_subtable);

        delete[] arr_str_stems;
        delete[] *parr_str_sfx;
        delete parr_str_sfx;
        vec_stems.clear();

        // TEST
        if (i_subtable > 100)
        {
            break;
        }
    }
    return 0;
}

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
        */

        str_query = L"Select * from headword as a0 where exists (select * from descriptor as a1 where id = " +
                    str_ToString<__int64>((*iter_stems).ll_lexeme_id) + L" and a0.id = a1.word_id)";
        pco_db->v_PrepareForSelect(str_query);
        while (pco_db->b_GetRow())
        {
            pco_db->v_GetData(1, str_lemma);
            vec_lemma.push_back(str_lemma);
        }
        /*
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
        if (vec_lemma.empty())
        {
            continue;
        }
        for (vector<int>::iterator iter_endings = vec_gram.begin();
            iter_endings != vec_gram.end(); iter_endings++)
        {
            CT_Hasher co_tmp_wf;
            co_tmp_wf.h_DecodeHash(*iter_endings);
            co_tmp_wf.ll_Lexeme_id = (*iter_stems).ll_lexeme_id;
            co_tmp_wf.str_Lemma = *(vec_lemma.begin());
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

wstring CT_Analyzer::str_InsertStress(int i_letter, wstring str_)
{
    if (i_letter >= 0 && i_letter < (int)(str_.length()))
    {
        return str_.substr(0, i_letter) + L"<" + str_.substr(i_letter, str_.length() - i_letter);
    }
    return str_;
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

int CT_Analyzer::i_DeleteStress(wstring& str_)
// Deletes the stress and returns the number of the letter it follows
{
    int i_StressPos = str_.find(L"<");
    if ((unsigned int)i_StressPos >= str_.length())
    {
        return -1;
    }
    str_ = str_.substr(0, i_StressPos) 
        + str_.substr(i_StressPos + 1, str_.length() - i_StressPos - 1);
    return i_StressPos;
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

int CT_Analyzer::i_LCP(wstring* str_words, wstring** pstr_pfx, int i_words, int i_pfx)
{
    // Divides the input into up to i_pfx groups and finds their longest common prefixes.
    // They may differ in length, but don't exceed the length of the shortest word.
    if (str_words == NULL || i_words <= 0 || i_pfx < 0)
    {
        return -1;
    }

    bool b_class_changed = false, b_try_again = false, *b_disabled;
    int i_len, i_minlen = 65536, i_numpfx = 1, i_numpfx_prev = 1, i_w_finished = 0, *i_pfxclass, *i_based_on;
    wstring *str_curpfx, *str_cursymb;
    wstring str_s;
    str_curpfx = new wstring[i_pfx];
    str_cursymb = new wstring[i_pfx];
    str_curpfx[0] = str_cursymb[0] = L"";
    i_pfxclass = new int[i_words]; // Stores current prefix class number of the i_th string or -1 if corresponding prefix is finished
    i_based_on = new int[i_words]; // Temporarily stores the number of class i_th class is based on
    b_disabled = new bool[i_words];
    memset(i_pfxclass, 0, i_words * sizeof(int)); // Initially, there is only one prefix class
    memset(i_based_on, 0, i_words * sizeof(int));
    memset(b_disabled, 0, i_words * sizeof(bool)); // All the words are enabled

    // Find minimal word length
    for (int i_ = 0; i_ < i_words; i_++)
    {
        i_len = str_words[i_].length();
        if (i_minlen > i_len)
        {
            i_minlen = i_len;
        }
    }

    // Find the prefixes
    for (int i_l = 0; i_l < i_minlen; i_l++)
    {
        for (int i_w = 0; i_w < i_words; i_w++)
        {
            if (b_disabled[i_w] == true)
            {
                continue; // The word is disabled
            }
            str_s = str_words[i_w].substr(i_l, 1); //current symbol of the current word
            if (str_cursymb[i_pfxclass[i_w]].length() == 0)
            {
                str_cursymb[i_pfxclass[i_w]] = str_s;
            }
            else
            {
                if (str_s != str_cursymb[i_pfxclass[i_w]])
                {
                    // Maybe we can just change its class to one of those recently introduced?
                    for (int i_ = i_pfxclass[i_w] + 1; i_ < i_numpfx; i_++)
                    {
                        if (str_cursymb[i_] == str_s && i_based_on[i_] == i_pfxclass[i_w])
                        {
                            i_pfxclass[i_w] = i_;
                            b_class_changed = true;
                            break;
                        }
                    }

                    if (b_class_changed == true)
                    {
                        b_class_changed = false;
                        continue;
                    }
                    //if not:
                    if (++i_numpfx > i_pfx)
                    {
                        //Finalize this prefix and initialize variables
                        for (int i_ = 0; i_ < i_words; i_++)
                        {
                            if (i_based_on[i_] == i_pfxclass[i_w])
                            {
                                b_disabled[i_] = true;
                                i_w_finished++; //One more word finished
                            }
                            i_pfxclass[i_] = i_based_on[i_];
                        }
                        for (int i_ = 0; i_ < i_pfx - 1; i_++)
                        {
                            str_cursymb[i_] = L"";
                        }
                        i_numpfx = i_numpfx_prev;
                        i_l--;
                        b_try_again = true; // Try again with the same symbol position, but more words disabled
                        break;
                    }
                    else
                    {
                        // Introduce a new prefix class
                        str_curpfx[i_numpfx - 1] = str_curpfx[i_pfxclass[i_w]];
                        str_cursymb[i_numpfx - 1] = str_s;
                        i_based_on[i_w] = i_pfxclass[i_w];
                        i_pfxclass[i_w] = i_numpfx - 1;
                    }
                }
            }
        }
        // Maybe we need to try the same position in the word again
        if (b_try_again == true)
        {
            b_try_again = false;
            if (i_w_finished == i_words)
            {
                break;
            }
            else
            {
                continue;
            }
        }
        // If not, add recently obtained symbols to the prefixes
        for (int i_ = 0; i_ < i_numpfx; i_++)
        {
            str_curpfx[i_] += str_cursymb[i_];
            str_cursymb[i_] = L"";
        }
        // Let recently formed classes become independent
        memcpy(i_based_on, i_pfxclass, i_words * sizeof(int));
        i_numpfx_prev = i_numpfx;
    }

    delete[] *pstr_pfx;
    *pstr_pfx = new wstring[i_numpfx_prev];
    for (int i_ = 0; i_ < i_numpfx_prev; i_++)
    {
        (*pstr_pfx)[i_] = str_curpfx[i_];
    }
    delete[] str_curpfx;
    delete[] str_cursymb;
    delete[] i_pfxclass;
    return i_numpfx_prev;
}
