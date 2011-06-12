#include "stdafx.h"
#include "LexPreprocessor.h"
#include "Analyzer_Helpers.h"
#include "Dictionary.h"
#include "Lexeme.h"

void CT_LexPreprocessor::v_Init()
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

HRESULT CT_LexPreprocessor::put_DbPath(BSTR bstr_Path)
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

    // Create and initialize an IDictionary object to work with
    HRESULT h_r = S_OK;
    h_r = sp_dict.CoCreateInstance(__uuidof(ZalDictionary));
    if (h_r != S_OK)
    {
        ERROR_LOG (L"CreateInstance failed for ZalDictionary.");
        return h_r;
    }
    sp_dict->put_DbPath(bstr_Path);
    
    return S_OK;
}

HRESULT CT_LexPreprocessor::PrepareLexeme(__int64 ll_lexeme_id, BOOL b_Stress)
{
    if (ll_lexeme_id < 0)
    {
        ERROR_LOG (L"Bad lexeme id.");
        return E_FAIL;
    }
    HRESULT h_r = PrepareLexemes(ll_lexeme_id, ll_lexeme_id, b_Stress);
    return h_r;
}

HRESULT CT_LexPreprocessor::PrepareLexemes(__int64 ll_start, __int64 ll_end, BOOL b_Stress)
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
    const int MAX_NUM_STEMS = 9;
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
            //if (eo_pos != POS_NOUN && eo_pos != POS_ADJ && eo_pos != POS_VERB && eo_pos != POS_PRONOUN_ADJ)
            //if (eo_pos == POS_NUM_ADJ)
            //{
            //    continue;
            //}
            // --------- /Test ----------
            BOOL b_HasIrregularForms = 0;
            sp_lex->get_HasIrregularForms(&b_HasIrregularForms);
            if (eo_pos == POS_VERB && b_HasIrregularForms != 0)
            {
                i_MaxStems = 5;
            }
            else if ((eo_pos == POS_NOUN && b_HasIrregularForms == 0) ||
                     (eo_pos == POS_ADJ && b_HasIrregularForms == 0) ||
                     eo_pos == POS_PRONOUN_ADJ ||
                     eo_pos == POS_NUM || eo_pos == POS_NUM_ADJ)
            {
                i_MaxStems = 2;
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
                if (b_Stress)
                {
                    long l_StressPos = -1;
                    long l_StressCount = 0;
                    BOOL b_Primary = FALSE;
                    sp_wf->get_StressCount(&l_StressCount);
                    for (long l_Stress = 0; l_Stress < l_StressCount; ++l_Stress)
                    {
                        // Use only the first primary stress
                        sp_wf->GetStressPos(l_Stress, &l_StressPos, &b_Primary);
                        if (b_Primary == TRUE)
                        {
                            break;
                        }
                        else
                        {
                            l_StressPos = -1;
                        }
                    }
                    wstring str_wf_stressed = str_InsertStress(l_StressPos, OLE2W(bstr_WordForm));
                    arr_str_Paradigm[l_Item - 1] = str_wf_stressed;
                }
                else
                {
                    arr_str_Paradigm[l_Item - 1] = OLE2W(bstr_WordForm);
                }

                sp_disp.Clear();
            }
            
            // Create several gr_stems
            int i_Stems = 0;
            while (i_Stems <= 0 && i_MaxStems <= MAX_NUM_STEMS)
            {
                delete[] *parr_str_Stems;
                delete parr_str_Stems;
                parr_str_Stems = new wstring*;
                *parr_str_Stems = new wstring[1];
                i_Stems = i_LCP(arr_str_Paradigm, parr_str_Stems, (int)l_Wordforms, i_MaxStems);
                if (i_Stems == 1 && (*parr_str_Stems)[0].length() <= 0)
                {
                    i_Stems = 0;
                }
                i_MaxStems += 2;
            }
            
            // Split every wordform into stem and ending,
            // then for every gr_stem create a table of endings usable with it
            CEndingsTable *arr_co_endings;
            arr_co_endings = new CEndingsTable[i_Stems];
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
            v_InsertStemsAndLinks(parr_str_Stems, OLE2W(bstr_Lemma), arr_i_subtable_id, i_Stems, ll_lexeme_id);

            // Delete the arrays
            for (int i_ = 0; i_ < i_Stems; ++i_)
            {
                arr_co_endings[i_].~CEndingsTable();
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

int CT_LexPreprocessor::i_ClassifyStems()
// For every endings subtable, looks for the stems usable with it and
// stores up to NUM_SFX their longest common suffixes in the database
{
    if (pco_db == NULL)
    {
        return -1;
    }
    const int MIN_NUMBER_OF_STEMS = 70;
    const int NUM_SFX = 5;
    const int MAX_NUM_SFX = 24;

    wstring str_query, str_stem;
    wstring *arr_str_stems;
    wstring **parr_str_sfx;
    vector<wstring> vec_stems;
    int i_last_subtable = 0, i_stem;

    i_last_subtable = pco_db->i_LastID(L"endings_meta");
    for (int i_subtable = 0; i_subtable <= i_last_subtable; ++i_subtable)
    {
        vec_stems.clear();
        wstring str_first_lemma = L"";
        int i_cut_right = 0;
        wstring str_lemma_ending = L"";

        v_LongStemsBySubtable(i_subtable, 2, vec_stems, str_first_lemma);
        if (vec_stems.size() < MIN_NUMBER_OF_STEMS)
        {
            continue;
        }

        // Find the longest common prefix of the first stem and the corresponding lemma
        wstring* arr_str_stem_and_lemma;
        wstring** parr_str_pfx;
        arr_str_stem_and_lemma = new wstring[2];
        arr_str_stem_and_lemma[0] = vec_stems[0];
        arr_str_stem_and_lemma[1] = str_first_lemma;
        parr_str_pfx = new wstring*;
        *parr_str_pfx = new wstring[1];
        int i_pfx = i_LCP(arr_str_stem_and_lemma, parr_str_pfx, 2, 1);
        if (i_pfx <= 0)
        {
            continue;
        }
        wstring str_common_pfx = (*parr_str_pfx)[0];
        i_cut_right = vec_stems[0].length() - str_common_pfx.length();
        if (i_cut_right >= 4)
        {
            continue;
        }
        str_lemma_ending = str_first_lemma.substr(str_common_pfx.length(), str_first_lemma.length() - str_common_pfx.length());

        // Find longest common suffixes of the stems found
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

        // several attemps
        int i_sfx = 0;
        int i_max_sfx = NUM_SFX;
        while (i_sfx <= 0 && i_max_sfx <= MAX_NUM_SFX)
        {
            delete[] *parr_str_sfx;
            delete parr_str_sfx;
            parr_str_sfx = new wstring*;
            *parr_str_sfx = new wstring[1];
            i_sfx = i_LCP(arr_str_stems, parr_str_sfx, vec_stems.size(), i_max_sfx);
            if (i_sfx == 1 && (*parr_str_sfx)[0].length() <= 0)
            {
                i_sfx = 0;
            }
            i_max_sfx += 2;
        }
        v_InsertCommonSfx(parr_str_sfx, i_sfx, i_subtable, vec_stems.size(), i_cut_right, str_lemma_ending);

        delete[] arr_str_stems;
        delete[] *parr_str_sfx;
        delete parr_str_sfx;

        // TEST
        //if (i_subtable > 100)
        //{
        //    break;
        //}
    }
    return 0;
}

wstring CT_LexPreprocessor::str_InsertStress(int i_letter, wstring str_)
{
    if (i_letter >= 0 && i_letter < (int)(str_.length()))
    {
        return str_.substr(0, i_letter) + L"<" + str_.substr(i_letter, str_.length() - i_letter);
    }
    return str_;
}

void CT_LexPreprocessor::v_DeleteRepeats(vector<wstring>& vec_strings)
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

int CT_LexPreprocessor::i_DeleteStress(wstring& str_)
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

int CT_LexPreprocessor::i_LCP(wstring* str_words, wstring** pstr_pfx, int i_words, int i_pfx)
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
            str_s = str_words[i_w][i_l]; // current symbol of the current word
            if (str_cursymb[i_pfxclass[i_w]].length() == 0)
            {
                str_cursymb[i_pfxclass[i_w]] = str_s;
            }
            else
            {
                if (str_s != str_cursymb[i_pfxclass[i_w]])  // The current prefix needs to be split or finalized
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

                    // if not:
                    if (++i_numpfx > i_pfx)
                    {
                        // Finalize this prefix and initialize variables
                        for (int i_ = 0; i_ < i_words; i_++)
                        {
                            if (i_based_on[i_] == i_pfxclass[i_w])
                            {
                                b_disabled[i_] = true;
                                i_w_finished++; // One more word finished
                            }
                            i_pfxclass[i_] = i_based_on[i_];
                        }
                        for (int i_ = 0; i_ < i_pfx; i_++)
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
