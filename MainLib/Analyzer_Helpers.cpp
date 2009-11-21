#include "stdafx.h"
#include "Analyzer_Helpers.h"

CT_GramHelper::CT_GramHelper()
{
    eo_POS = POS_UNDEFINED;
    eo_Case = CASE_UNDEFINED;
    eo_Number = NUM_UNDEFINED;
    eo_AdjForm = ADJ_FORM_UNDEFINED;
    eo_Gender = GENDER_UNDEFINED;
    eo_Tense = TENSE_UNDEFINED;
    eo_Person = PERSON_UNDEFINED;
    eo_Reflexive = REFL_UNDEFINED;
    eo_Voice = VOICE_UNDEFINED;
    eo_Animacy = ANIM_UNDEFINED;
    eo_Special = SPECIAL_UNDEFINED;
}

CT_GramHelper::CT_GramHelper (wstring str_wf)
{
    eo_POS = POS_UNDEFINED;
    eo_Case = CASE_UNDEFINED;
    eo_Number = NUM_UNDEFINED;
    eo_AdjForm = ADJ_FORM_UNDEFINED;
    eo_Gender = GENDER_UNDEFINED;
    eo_Tense = TENSE_UNDEFINED;
    eo_Person = PERSON_UNDEFINED;
    eo_Reflexive = REFL_UNDEFINED;
    eo_Voice = VOICE_UNDEFINED;
    eo_Animacy = ANIM_UNDEFINED;
    eo_Special = SPECIAL_UNDEFINED;
    str_wordform = str_wf;
}

CT_GramHelper::CT_GramHelper (wstring str_wf, __int64 ll_id)
{
    eo_POS = POS_UNDEFINED;
    eo_Case = CASE_UNDEFINED;
    eo_Number = NUM_UNDEFINED;
    eo_AdjForm = ADJ_FORM_UNDEFINED;
    eo_Gender = GENDER_UNDEFINED;
    eo_Tense = TENSE_UNDEFINED;
    eo_Person = PERSON_UNDEFINED;
    eo_Reflexive = REFL_UNDEFINED;
    eo_Voice = VOICE_UNDEFINED;
    eo_Animacy = ANIM_UNDEFINED;
    eo_Special = SPECIAL_UNDEFINED;
    str_wordform = str_wf;
    ll_lexeme_id = ll_id;
}

CT_GramHelper::~CT_GramHelper() {}

void CT_GramHelper::GramClear()
{
    eo_Case = CASE_UNDEFINED;
    eo_Number = NUM_UNDEFINED;
    eo_AdjForm = ADJ_FORM_UNDEFINED;
    eo_Gender = GENDER_UNDEFINED;
    eo_Tense = TENSE_UNDEFINED;
    eo_Person = PERSON_UNDEFINED;
    eo_Reflexive = REFL_UNDEFINED;
    eo_Voice = VOICE_UNDEFINED;
    eo_Animacy = ANIM_UNDEFINED;
    eo_Special = SPECIAL_UNDEFINED;   
    return;
}

void CT_GramHelper::Clear()
{
    GramClear();
    eo_POS = POS_UNDEFINED;
    str_wordform = L"";
    str_lemma = L"";
    ll_lexeme_id = -1;   
    return;
}

int CT_GramHelper::i_GramNumber()
{
    if (eo_POS == POS_UNDEFINED || eo_POS == POS_ADV || eo_POS == POS_PREP ||
        eo_POS == POS_CONJ || eo_POS == POS_PARTICLE || eo_POS == POS_COMPAR ||
        eo_POS == POS_INTERJ || eo_POS == POS_PARENTH)
    {
        return 0;
    }
    int i_Case;
    if (eo_POS == POS_NOUN) // 40 possibilities
    {
        return eo_Case * NUM_COUNT + eo_Number;
    }
    if (eo_POS == POS_ADJ || eo_POS == POS_NUM || eo_POS == POS_PRONOUN) // 264 possibilities
    {
        if (eo_Case != CASE_ACC || eo_Animacy != ANIM_YES)
        {
            i_Case = eo_Case;
        }
        else
        {
            i_Case = CASE_COUNT; // In fact, it means Accusative case of an animate adjective
        }
        return eo_AdjForm * (CASE_COUNT + 1) * NUM_COUNT * GENDER_COUNT
                + i_Case * NUM_COUNT * GENDER_COUNT + eo_Number * GENDER_COUNT + eo_Gender;
    }
    if (eo_POS == POS_VERB)
    {
        if (eo_Special == SPECIAL_UNDEFINED)
        {
            return -1;
        }
        if (eo_Special == SPECIAL_NORMAL) // 144 possibilities
        {
            // It is assumed that GENDER_COUNT == PERSON_COUNT
            if (eo_Tense != TENSE_PAST)
            {
                return eo_Tense * NUM_COUNT * PERSON_COUNT * REFL_COUNT
                        + eo_Number * PERSON_COUNT * REFL_COUNT + eo_Person * REFL_COUNT + eo_Reflexive;
            }
            else
            {
                return TENSE_PAST * NUM_COUNT * GENDER_COUNT * REFL_COUNT 
                        + eo_Number * GENDER_COUNT * REFL_COUNT + eo_Gender * REFL_COUNT + eo_Reflexive;
            }
        }
        if (eo_Special == SPECIAL_IMPERATIVE) // 36 possibilities
        {
            return eo_Number * PERSON_COUNT * REFL_COUNT + eo_Person * REFL_COUNT + eo_Reflexive
                    + TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT;
        }
        if (eo_Special == SPECIAL_PARTICIPLE) // 4752 possibilities
        {
            if (eo_Case != CASE_ACC || eo_Animacy != ANIM_YES)
            {
                i_Case = eo_Case;
            }
            else           
            {
                i_Case = CASE_COUNT; // In fact, it means Accusative case of an animate participle
            }
            return eo_Tense * (CASE_COUNT + 1) * NUM_COUNT * GENDER_COUNT * VOICE_COUNT * REFL_COUNT + 
                    i_Case * NUM_COUNT * GENDER_COUNT * VOICE_COUNT * REFL_COUNT + 
                    eo_Number * GENDER_COUNT * VOICE_COUNT * REFL_COUNT + 
                    eo_Gender * VOICE_COUNT * REFL_COUNT + 
                    eo_Voice * REFL_COUNT + 
                    eo_Reflexive + 
                    TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT + NUM_COUNT * PERSON_COUNT * REFL_COUNT;
        }
        if (eo_Special == SPECIAL_CONVERB) // 12 possibilities
        {
            return eo_Tense * REFL_COUNT + eo_Reflexive 
                    + TENSE_COUNT * (CASE_COUNT + 1) * NUM_COUNT * GENDER_COUNT * VOICE_COUNT * REFL_COUNT
                    + TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                    + NUM_COUNT * PERSON_COUNT * REFL_COUNT;
        }
        if (eo_Special == SPECIAL_INFINITIVE) // 2 possibilities
        {
            return eo_Reflexive + TENSE_COUNT * REFL_COUNT 
                    + TENSE_COUNT * (CASE_COUNT + 1) * NUM_COUNT * GENDER_COUNT * VOICE_COUNT * REFL_COUNT
                    + TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                    + NUM_COUNT * PERSON_COUNT * REFL_COUNT;
        }
    }
    if (eo_Special == SPECIAL_COMPARATIVE) // 1 possibility
    {
        return REFL_COUNT + TENSE_COUNT * REFL_COUNT 
                + TENSE_COUNT * (CASE_COUNT + 1) * NUM_COUNT * GENDER_COUNT * VOICE_COUNT * REFL_COUNT
                + TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                + NUM_COUNT * PERSON_COUNT * REFL_COUNT;
        //In original version, this used to give 4947;
    }
    return -1;   
}   //  i_GramNumber()

   
int CT_GramHelper::i_DecodeNumber (int i_gram)
{
    int i_gram_max = REFL_COUNT + TENSE_COUNT * REFL_COUNT 
                + TENSE_COUNT * (CASE_COUNT + 1) * NUM_COUNT * GENDER_COUNT * VOICE_COUNT * REFL_COUNT
                + TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                + NUM_COUNT * PERSON_COUNT * REFL_COUNT;
    if (i_gram < 0 || i_gram > i_gram_max)
    {
        return -1;
    }
    GramClear();
    if (eo_POS == POS_UNDEFINED || eo_POS == POS_ADV || eo_POS == POS_PREP ||
        eo_POS == POS_CONJ || eo_POS == POS_PARTICLE || eo_POS == POS_COMPAR ||
        eo_POS == POS_INTERJ || eo_POS == POS_PARENTH)
    {
        return 0;
    }
    if (eo_POS == POS_NOUN) // 40 possibilities
    {
        eo_Number = (ET_Number)(i_gram % NUM_COUNT);
        eo_Case = (ET_Case)((i_gram - eo_Number) / NUM_COUNT);
        return 0;
    }
    if (eo_POS == POS_ADJ || eo_POS == POS_NUM || eo_POS == POS_PRONOUN) // 264 possibilities
    {
        eo_Gender = (ET_Gender)(i_gram % GENDER_COUNT);
        i_gram = (i_gram - eo_Gender) / GENDER_COUNT;
        eo_Number = (ET_Number)(i_gram % NUM_COUNT);
        i_gram = (i_gram - eo_Number) / NUM_COUNT;
        int i_tmp_case = i_gram % (CASE_COUNT + 1);
        if (i_tmp_case == CASE_COUNT)
        {
            eo_Case = CASE_ACC;
            eo_Animacy = ANIM_YES;
        }
        else
        {
            eo_Case = (ET_Case)i_tmp_case;
            if (eo_Case != CASE_UNDEFINED)
            {
                eo_Animacy = ANIM_NO;
            }           
        }
        i_gram = (i_gram - i_tmp_case) / (CASE_COUNT + 1);
        eo_AdjForm = (ET_AdjForm)i_gram;   
    }
    if (eo_POS == POS_VERB)
    {
        if (i_gram < TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT) // 144 possibilities
        {
            eo_Special = SPECIAL_NORMAL;
            eo_Reflexive = (ET_Reflexive)(i_gram % REFL_COUNT);
            i_gram = (i_gram - eo_Reflexive) / REFL_COUNT;
            if (i_gram < TENSE_PAST * NUM_COUNT * GENDER_COUNT * REFL_COUNT)
            {
                eo_Person = (ET_Person)(i_gram % PERSON_COUNT);
                i_gram = (i_gram - eo_Person) / PERSON_COUNT;
                eo_Number = (ET_Number)(i_gram % NUM_COUNT);
                i_gram = (i_gram - eo_Number) / NUM_COUNT;
                eo_Tense = (ET_Tense)i_gram;
                return 0;
            }           
            else
            {
                eo_Gender = (ET_Gender)(i_gram % GENDER_COUNT);
                i_gram = (i_gram - eo_Gender) / GENDER_COUNT;
                eo_Number = (ET_Number)(i_gram % NUM_COUNT);
                eo_Tense = TENSE_PAST;
                return 0;
            }
        }
       
        if (i_gram >= TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
            && i_gram < TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                        + NUM_COUNT * PERSON_COUNT * REFL_COUNT) // 36 possibilities
        {
            i_gram -= TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT;
            eo_Special = SPECIAL_IMPERATIVE;
            eo_Reflexive = (ET_Reflexive)(i_gram % REFL_COUNT);
            i_gram = (i_gram - eo_Reflexive) / REFL_COUNT;
            eo_Person = (ET_Person)(i_gram % PERSON_COUNT);
            i_gram = (i_gram - eo_Person) / PERSON_COUNT;
            eo_Number = (ET_Number)i_gram;
            return 0;
        }
        if (i_gram >= TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                    + NUM_COUNT * PERSON_COUNT * REFL_COUNT
                && i_gram < TENSE_COUNT * (CASE_COUNT + 1) * NUM_COUNT * GENDER_COUNT * VOICE_COUNT * REFL_COUNT
                    + TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                    + NUM_COUNT * PERSON_COUNT * REFL_COUNT) // 4752 possibilities
        {
            i_gram -= TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                        + NUM_COUNT * PERSON_COUNT * REFL_COUNT;
            eo_Special = SPECIAL_PARTICIPLE;
            eo_Reflexive = (ET_Reflexive)(i_gram % REFL_COUNT);
            i_gram = (i_gram - eo_Reflexive) / REFL_COUNT;
            eo_Voice = (ET_Voice)(i_gram % VOICE_COUNT);
            i_gram = (i_gram - eo_Voice) / VOICE_COUNT;
            eo_Gender = (ET_Gender)(i_gram % GENDER_COUNT);
            i_gram = (i_gram - eo_Gender) / GENDER_COUNT;
            eo_Number = (ET_Number)(i_gram % NUM_COUNT);
            i_gram = (i_gram - eo_Number) / NUM_COUNT;
            int i_tmp_case = i_gram % (CASE_COUNT + 1);
            if (i_tmp_case == CASE_COUNT)
            {
                eo_Case = CASE_ACC;
                eo_Animacy = ANIM_YES;
            }
            else

            {
                eo_Case = (ET_Case)i_tmp_case;
                if (eo_Case != CASE_UNDEFINED)
                {
                    eo_Animacy = ANIM_NO;
                }
            }
            i_gram = (i_gram - i_tmp_case) / (CASE_COUNT + 1);
            eo_Tense = (ET_Tense)i_gram;
            return 0;
        }
        if (i_gram >= TENSE_COUNT * (CASE_COUNT + 1) * NUM_COUNT * GENDER_COUNT * VOICE_COUNT * REFL_COUNT
                    + TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                    + NUM_COUNT * PERSON_COUNT * REFL_COUNT
                && i_gram < i_gram_max - REFL_COUNT) // 12 possibilities
        {
            i_gram -= TENSE_COUNT * (CASE_COUNT + 1) * NUM_COUNT * GENDER_COUNT * VOICE_COUNT * REFL_COUNT
                    + TENSE_COUNT * NUM_COUNT * GENDER_COUNT * REFL_COUNT
                    + NUM_COUNT * PERSON_COUNT * REFL_COUNT;
            eo_Special = SPECIAL_CONVERB;
            eo_Reflexive = (ET_Reflexive)(i_gram % REFL_COUNT);
            i_gram = (i_gram - eo_Reflexive) / REFL_COUNT;
            eo_Tense = (ET_Tense)i_gram;
            return 0;
        }
        if (i_gram >= i_gram_max - REFL_COUNT && i_gram < i_gram_max) // 2 possibilities
        {
            i_gram -= i_gram_max - REFL_COUNT;
            eo_Special = SPECIAL_INFINITIVE;
            eo_Reflexive = (ET_Reflexive)i_gram;
            return 0;
        }
    }
    if (i_gram == i_gram_max)
    {
        eo_Special = SPECIAL_COMPARATIVE;
        return 0;
    }
    
    return -1;
}   //  int i_DecodeNumber (int i_gram)

int CT_GramHelper::i_DecodeString (wstring str_gram)
{
    // Partially depends on POS.
   
    // Warning!
    // 1. The list may be incomplete.
    // 2. Verb reflexivity is recognized only if str_lemma is present.

    GramClear();
    if (str_gram.length() == 0)
    {
        return 0;
    }
    wsmatch result;
    wstring str_parameter;
    bool b_match;
    while (str_gram.length() > 0)
    {
        b_match = regex_match(str_gram, result, (const wregex)L"\\s*([^\\s\\.]*\\.?)\\s*(.*)");
        str_parameter = result[1];
        str_gram = result[2];
        if (str_parameter == L"1")
        {
            eo_Person = PERSON_1;
        }
        if (str_parameter == L"2")
        {
            eo_Person = PERSON_2;
        }
        if (str_parameter == L"3")
        {
            eo_Person = PERSON_3;
        }
        if (str_parameter == L"ед.")
        {
            eo_Number = NUM_SG;
        }
        if (str_parameter == L"мн.")
        {
            eo_Number = NUM_PL;
        }
        if (str_parameter == L"И.")
        {
            eo_Case = CASE_NOM;
        }
        if (str_parameter == L"Р.")
        {
            eo_Case = CASE_GEN;
        }
        if (str_parameter == L"Д.")
        {
            eo_Case = CASE_DAT;
        }
        if (str_parameter == L"В.")
        {
            eo_Case = CASE_ACC;
        }
        if (str_parameter == L"Т.")
        {
            eo_Case = CASE_INST;
        }
        if (str_parameter == L"П.")
        {
            eo_Case = CASE_PREP;
        }
        if (str_parameter == L"Р2")       
        {
            eo_Case = CASE_PART;
        }
        if (str_parameter == L"П2")
        {
            eo_Case = CASE_LOC;          
        }
        if (str_parameter == L"кф")
        {
            eo_AdjForm = ADJ_FORM_SHORT;
        }
        if (str_parameter == L"буд.")
        {
            eo_Tense = TENSE_FUTURE;
        }
        if (str_parameter == L"наст.")
        {
            eo_Tense = TENSE_PRESENT;
        }
        if (str_parameter == L"прош.")
        {
            eo_Tense = TENSE_PAST;
        }
        if (str_parameter == L"прич.")
        {
            eo_Special = SPECIAL_PARTICIPLE;
        }
        if (str_parameter == L"деепр.")
        {
            eo_Special = SPECIAL_CONVERB;
        }
        if (str_parameter == L"повел.")
        {
            eo_Special = SPECIAL_IMPERATIVE;
        }
        if (str_parameter == L"страд.")
        {
            eo_Voice = VOICE_PASSIVE;
        }
        if (str_parameter == L"м")
        {
            eo_Gender = GENDER_M;
        }
        if (str_parameter == L"ж")
        {
            eo_Gender = GENDER_F;
        }
        if (str_parameter == L"с")
        {
            eo_Gender = GENDER_N;
        }
        if (str_parameter == L"одуш.")
        {
            eo_Animacy = ANIM_YES;
        }
        if (str_parameter == L"сравн.")
        {
            eo_Special = SPECIAL_COMPARATIVE;
        }
    }
    if (eo_POS == POS_ADV)
    {
        if (eo_AdjForm == ADJ_FORM_UNDEFINED)
        {
            eo_AdjForm = ADJ_FORM_LONG;
        }
        if (eo_Case == CASE_ACC && eo_Animacy == ANIM_UNDEFINED)
        {
            eo_Animacy = ANIM_NO;
        }
    }
    if (eo_POS == POS_VERB)
    {
        if (eo_Special == SPECIAL_UNDEFINED)
        {
            eo_Special = SPECIAL_NORMAL;
        }
        if (eo_Special == SPECIAL_IMPERATIVE && eo_Person == PERSON_UNDEFINED)
        {
            eo_Person = PERSON_2;
        }
        if (eo_Special == SPECIAL_PARTICIPLE && eo_Case == CASE_ACC && eo_Animacy == ANIM_UNDEFINED)
        {
            eo_Animacy = ANIM_NO;
        }
        if (eo_Special == SPECIAL_PARTICIPLE && eo_Voice == VOICE_UNDEFINED)
        {
            eo_Voice = VOICE_ACTIVE;
        }
        b_match = regex_match(str_lemma, result, (const wregex)L".*(ся|сь)");
        if (b_match == true)
        {
            eo_Reflexive = REFL_YES;
        }
        else
        {
            eo_Reflexive = REFL_NO;
        }
    }
   
    return 0;
}


CT_EndingsTable::CT_EndingsTable()
{
    pvec_Endings = new vector<wstring>;
    pvec_Gram = new vector<int>;
}

CT_EndingsTable::~CT_EndingsTable()
{
    delete pvec_Endings;
    delete pvec_Gram;
}

void CT_EndingsTable::Clear()
{
    pvec_Endings->clear();
    pvec_Gram->clear();
    str_Stem = L"";
}

int CT_EndingsTable::i_AddEnding(int i_Gram, wstring str_Ending)
{
    pvec_Endings->push_back(str_Ending);
    pvec_Gram->push_back(i_Gram);
    return 0;
}

vector<int> CT_EndingsTable::vec_db_id(CT_Sqlite* pco_dbHandle)
// Tries to find the table in the db, returns -1 if there's no such table.
// If everything goes well the output vector will contain just 1 element. But who knows what can happen.
{
    vector<int> vec_subtable_id, vec_entries_count, vec_result;
    if (pco_dbHandle == NULL || pvec_Endings->empty() == true || pvec_Gram->empty() == true || pvec_Endings->size() != pvec_Gram->size())
    {
        return vec_result;
    }
    wstring str_query, str_count, str_gram;
    S_Field<int> so_subtable_id, so_entries_count;
    so_subtable_id.i_ColNumber = 1;     // Provided subtable_id occupies the 1st column in endings_for_analysis
    so_subtable_id.pvec_FieldData = &vec_subtable_id;
    so_entries_count.i_ColNumber = 2;	// Provided entries_count occupies the 2nd column in endings_meta
    so_entries_count.pvec_FieldData = &vec_entries_count;

    vector<wstring>::iterator iter_endings = pvec_Endings->begin();
    vector<int>::iterator iter_gram = pvec_Gram->begin();
    str_gram = str_ToString(*iter_gram);
    str_query += L"Select * from endings_for_analysis as a0 where ending = \"" 
        + *iter_endings + L"\" and gram = " + str_gram;
    iter_gram++;
    iter_endings++;
    for (int i_ = 1;
        iter_endings != pvec_Endings->end();
        iter_endings++, iter_gram++, i_++)
    {
        str_count = str_ToString(i_);
        str_gram = str_ToString(*iter_gram);
        str_query += L" and exists (select * from endings_for_analysis as a" + str_count 
            + L" where ending = \"" + *iter_endings + L"\" and gram_hash = " 
            + str_gram + L" and a" + str_count + L".subtable_id = a0.subtable_id)";
    }
    pco_dbHandle->v_Exec(str_query, v_GetTableField_int, &so_subtable_id);

    // Now that we've learned the ids of possible subtables we must ensure they don't contain
    // any entries except those specified by the input vectors.
    for (vector<int>::iterator iter_subtable_id = vec_subtable_id.begin();
        iter_subtable_id != vec_subtable_id.end();
        iter_subtable_id++)
    {
        str_query = L"Select * from endings_meta where subtable_id = " + str_ToString(*iter_subtable_id);
        pco_dbHandle->v_Exec(str_query, v_GetTableField_int, &so_entries_count);
        if (so_entries_count.pvec_FieldData->empty() == false
            && *(so_entries_count.pvec_FieldData->begin()) == pvec_Gram->size())
        {
            vec_result.push_back(*iter_subtable_id);
        }
    }
    //ERROR_LOG(L"*** CT_EndingsTable::vec_db_id works: str_query = '" + str_query + L"'.");
    /*
    so_subtable_id.pvec_FieldData = NULL;	// vec_subtable_id lives, so_subtable_id dies
    so_entries_count.pvec_FieldData = NULL;	// vec_entries_count lives, so_entries_count dies
    */
    return vec_result;
}

int CT_EndingsTable::i_db_Write(CT_Sqlite* pco_dbHandle)
// If a table already exists, return its subtable_id.
// If not, write the whole table to db, then return its subtable_id.
// Return -1 or -2 on error.
{
    if (pco_dbHandle == NULL)
    {
        return -1;
    }
    int i_inserted = 0, i_subtable_id;
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
    S_Field<int> so_search_result;
    so_search_result.i_ColNumber = 0; // In endings_meta
    so_search_result.pvec_FieldData = &vec_search_result;
    wstring str_query = L"Select * from endings_meta as a0 where not exists (select * from endings_meta as a1 where a1.id > a0.id)";
    // TODO: Check the query.
    pco_dbHandle->v_Exec(str_query, v_GetTableField_int, &so_search_result);
    i_subtable_id = *(vec_search_result.begin()) + 1;

    for (iter_Ending = pvec_Endings->begin(), iter_Gram = pvec_Gram->begin();
        (iter_Ending != pvec_Endings->end()) && (iter_Gram != pvec_Gram->end());
        iter_Ending++, iter_Gram++, i_inserted++)
    {
        // For each couple <ending, grammatical parameters>, insert it into the endings table.
        pco_dbHandle->v_PrepareForInsert(L"endings_for_analysis", 3);
        pco_dbHandle->v_Bind(1, i_subtable_id);  // 0-based
        pco_dbHandle->v_Bind(2, *iter_Ending);
        pco_dbHandle->v_Bind(3, *iter_Gram);
        pco_dbHandle->v_Finalize();
    }
    // Now, write auxiliary information to endings_meta
    pco_dbHandle->v_PrepareForInsert(L"endings_meta", 2);
    pco_dbHandle->v_Bind(1, i_subtable_id);
    pco_dbHandle->v_Bind(2, i_inserted);
    pco_dbHandle->v_Finalize();
    return i_inserted;
}

void v_GetTableField_int(sqlite3_stmt *po_statement, void *v_FieldData)
{
    if (v_FieldData == NULL)
    {
        return;
    }
    S_Field<int> *pso_FieldData = (S_Field<int> *)v_FieldData;
    if (pso_FieldData->pvec_FieldData == NULL)
    {
        return;
    }
    pso_FieldData->pvec_FieldData->push_back(sqlite3_column_int(po_statement, pso_FieldData->i_ColNumber)); // Get value from i_th column.
    ERROR_LOG(L"*** v_GetTableField_int works.");
    return;
}

void v_GetTableField_int64(sqlite3_stmt *po_statement, void *v_FieldData)
{
    if (v_FieldData == NULL)
    {
        return;
    }
    S_Field<__int64> *pso_FieldData = (S_Field<__int64> *)v_FieldData;
    if (pso_FieldData->pvec_FieldData == NULL)
    {
        return;
    }
    pso_FieldData->pvec_FieldData->push_back(sqlite3_column_int64(po_statement, pso_FieldData->i_ColNumber)); // Get value from i_th column.
    ERROR_LOG(L"*** v_GetTableField_int64 works.");
    return;
}

void v_GetTableField_str(sqlite3_stmt *po_statement, void *v_FieldData)
{
    if (v_FieldData == NULL)
    {
        return;
    }
    S_Field<wstring> *pso_FieldData = (S_Field<wstring> *)v_FieldData;
    if (pso_FieldData->pvec_FieldData == NULL)
    {
        return;
    }
    const void * p_ = sqlite3_column_text16(po_statement, pso_FieldData->i_ColNumber);
    if (p_)
    {
        pso_FieldData->pvec_FieldData->push_back(static_cast<wchar_t *>(const_cast<void *>(p_))); // Get value from i_th column.
    }
    ERROR_LOG(L"*** v_GetTableField_str works.");
    return;
}


template <typename WF>
CT_Wordform_Iterator<WF>::CT_Wordform_Iterator()
{
}

template <typename WF>
CT_Wordform_Iterator<WF>::~CT_Wordform_Iterator()
{
}

template <typename WF>
int CT_Wordform_Iterator<WF>::i_Initialize(ET_PartOfSpeech eo_in_POS)
{
    eo_POS = eo_in_POS;
    b_end = false;
    co_wf.eo_POS = eo_POS;
    co_wf.eo_Case = CASE_UNDEFINED;
    co_wf.eo_Number = NUM_UNDEFINED;
    co_wf.eo_AdjForm = ADJ_FORM_UNDEFINED;
    co_wf.eo_Gender = GENDER_UNDEFINED;
    co_wf.eo_Tense = TENSE_UNDEFINED;
    co_wf.eo_Person = PERSON_UNDEFINED;
    co_wf.eo_Reflexive = REFL_UNDEFINED;
    co_wf.eo_Voice = VOICE_UNDEFINED;
    co_wf.eo_Animacy = ANIM_UNDEFINED;
    //co_wf.eo_Special = SPECIAL_UNDEFINED;  

    if (eo_POS == POS_NOUN)
    {
        co_wf.eo_Case++;
        co_wf.eo_Number++;
    }
    if (eo_POS == POS_ADJ)
    {
        co_wf.eo_AdjForm = ADJ_FORM_LONG;
        co_wf.eo_Case++;
        co_wf.eo_Number++;
    }
    if (eo_POS == POS_VERB)
    {
        co_wf.eo_Special = SPECIAL_NORMAL;
        co_wf.eo_Tense = TENSE_PRESENT;
        co_wf.eo_Person++;
        co_wf.eo_Number++;
    }
    if (eo_POS == POS_ADV ||
        eo_POS == POS_PREP ||
        eo_POS == POS_CONJ || 
        eo_POS == POS_PARTICLE ||
        eo_POS == POS_INTERJ || 
        eo_POS == POS_PARENTH)
    {
        b_end = true;
    }
    return 0;
}

template <typename WF>
WF CT_Wordform_Iterator<WF>::get_Wordform()
{
    return co_wf;
}

template <typename WF>
CT_Wordform_Iterator<WF> CT_Wordform_Iterator<WF>::operator++()
{
    if (eo_POS == POS_NOUN)
    {
        co_wf.eo_Case++;
        if (co_wf.eo_Case == CASE_COUNT)
        {
            co_wf.eo_Number++;
            if (co_wf.eo_Number == NUM_COUNT)
            {
                b_end = true;
                return this;
            }
            co_wf.eo_Case = CASE_UNDEFINED++;
        }
    }

    if (eo_POS == POS_ADJ) // Including short forms with cases
    {
        if (co_wf.eo_Case == CASE_ACC && co_wf.eo_Animacy == ANIM_NO)
        {
            co_wf.eo_Animacy = ANIM_UNDEFINED;
        }
        co_wf.eo_Case++;
        if (co_wf.eo_Case == CASE_COUNT)
        {
            if (co_wf.eo_Number == NUM_SG)
            {
                co_wf.eo_Gender++;
                if (co_wf.eo_Gender == GENDER_COUNT)
                {
                    co_wf.eo_Gender = GENDER_UNDEFINED;
                    co_wf.eo_Number = NUM_PL;
                }
            }
            else
            {
                co_wf.AdjForm++;
                if (co_wf.AdjForm == ADJ_FORM_COUNT)
                {
                    b_end = true;
                    return this;
                }
                co_wf.eo_Number = NUM_SG;
                co_wf.eo_Gender = GENDER_UNDEFINED++;
            }
            co_wf.eo_Case = CASE_UNDEFINED++;
        }

        // TODO: Verbs and so on
    }
    return this;
}
