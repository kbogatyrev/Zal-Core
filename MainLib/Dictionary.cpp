// Dictionary.cpp : Implementation of CDictionary

#include "stdafx.h"
#include "Dictionary.h"
#include "Lexeme.h"
#include "Verifier.h"

static CEString sQueryBase (L"SELECT \
headword.source, \
headword.id, \
headword.comment, \
descriptor.id, \
descriptor.graphic_stem, \
descriptor.is_variant, \
descriptor.main_symbol, \
descriptor.part_of_speech, \
descriptor.is_plural_of, \
descriptor.is_intransitive, \
descriptor.is_reflexive, \
descriptor.main_symbol_plural_of, \
descriptor.alt_main_symbol, \
descriptor.inflection_type, \
descriptor.comment, \
descriptor.alt_main_symbol_comment, \
descriptor.alt_inflection_comment, \
descriptor.verb_stem_alternation, \
descriptor.part_past_pass_zhd, \
descriptor.section, \
descriptor.no_comparative, \
descriptor.assumed_forms, \
descriptor.yo_alternation, \
descriptor.o_alternation, \
descriptor.second_genitive, \
descriptor.has_aspect_pair, \
descriptor.has_questionable_forms, \
descriptor.has_irregular_forms, \
descriptor.has_deficiencies, \
descriptor.restricted_forms, \
descriptor.contexts, \
descriptor.trailing_comment, \
inflection.id, \
inflection.is_primary, \
inflection.inflection_type, \
inflection.accent_type1, \
inflection.accent_type2, \
inflection.short_form_restrictions, \
inflection.past_part_restrictions, \
inflection.no_short_form, \
inflection.no_past_part, \
inflection.fleeting_vowel, \
inflection.stem_augment \
FROM \
headword INNER JOIN descriptor ON descriptor.word_id = headword.id ");

HRESULT CDictionary::put_DbPath (BSTR bstrDbPath)
{
    USES_CONVERSION;

    m_sDbPath = OLE2W (bstrDbPath);

    m_pDb = new CSqlite (m_sDbPath);
    if (!m_pDb)
    {
        return E_FAIL;
    }

    return S_OK;

}   //  put_DbPath (...)

HRESULT CDictionary::GetLexeme (long iId, ILexeme ** ppLexeme)
{
    HRESULT hr = S_OK;

    CEString sQuery (sQueryBase);
    sQuery += L"LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id ";
    sQuery += L"WHERE descriptor.id = ";
    sQuery += CEString::sToString (iId);
    sQuery += L";";

    hr = hGetData (sQuery);
    if (S_OK == hr)
    {
        if (m_coll.size() < 1)
        {
            *ppLexeme = NULL;
            ERROR_LOG (L"No lexemes in dictionary object.");
            return E_FAIL;
        }

        CComVariant spVar = m_coll[0];
        if (VT_UNKNOWN == spVar.vt || VT_DISPATCH == spVar.vt)
        {
            CComQIPtr<ILexeme> spQiLex = spVar.pdispVal;
            *ppLexeme = spQiLex.Detach();
        }
        else
        {
            *ppLexeme = NULL;
            ERROR_LOG (L"Unexpected VARIANT data type.");
            return E_UNEXPECTED;
        }
    }

    return hr;

}   //  GetLexeme (...)

HRESULT CDictionary::GetLexemeByHash (ULONG ulLexemeHash, ILexeme ** ppLexeme)
{
    USES_CONVERSION;

    HRESULT hr = S_OK;

    CEString sQuery (sQueryBase);
    sQuery += L"INNER JOIN lexeme_hash_to_descriptor as l on l.descriptor_id=descriptor.id ";
    sQuery += L"LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id ";
    sQuery += L"WHERE lexeme_hash = ";
    sQuery += CEString::sToString (ulLexemeHash);
    sQuery += L";";

    hr = hGetData (sQuery);
    if (S_OK == hr)
    {
        if (m_coll.size() < 1)
        {
            *ppLexeme = NULL;
            ERROR_LOG (L"No lexemes in dictionary object.");
            return E_FAIL;
        }

        CComVariant spVar = m_coll[0];
        if (VT_UNKNOWN == spVar.vt || VT_DISPATCH == spVar.vt)
        {
            CComQIPtr<ILexeme> spQiLex = spVar.pdispVal;
            *ppLexeme = spQiLex.Detach();
        }
        else
        {
            *ppLexeme = NULL;
            ERROR_LOG (L"Unexpected VARIANT data type.");
            return E_UNEXPECTED;
        }
    }

    return hr;

}   //  GetLexemeByHash (...)

HRESULT CDictionary::GetLexemesByGraphicStem (BSTR bsHeadword)
{
    USES_CONVERSION;

    HRESULT hr = S_OK;

    CEString sQuery (sQueryBase);
    sQuery += L"LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id ";
    sQuery += L"WHERE descriptor.graphic_stem = \"";
    sQuery += OLE2W (bsHeadword);
    sQuery += L"\";";
    hr = hGetData (sQuery);

    return hr;
}

HRESULT CDictionary::GetLexemesByInitialForm (BSTR bsStem)
{
    USES_CONVERSION;

    HRESULT hr = S_OK;

    CEString sQuery (sQueryBase);
    sQuery += L"LEFT OUTER JOIN inflection ON descriptor.id = inflection.descriptor_id ";
    sQuery += L"WHERE headword.source = \"";
    sQuery += OLE2W (bsStem);
    sQuery += L"\";";
    hr = hGetData (sQuery);

    return hr;
}

HRESULT CDictionary::Clear ()
{
    m_coll.clear();
    return S_OK;
}


//
// Helpers
//

HRESULT CDictionary::hGetData (const CEString& sSelect)
{
    HRESULT hr = S_OK;

    if (NULL == m_pDb)
    {
        ATLASSERT(0);
        ERROR_LOG (L"DB pointer is NULL.");
        return E_POINTER;
    }

    m_coll.clear();

    try
    {
        m_pDb->PrepareForSelect (sSelect);

        while (m_pDb->bGetRow())
        {
            CComObject<CLexeme> * spLexeme;
            hr = CComObject<CLexeme>::CreateInstance (&spLexeme);
            if (S_OK != hr)
            {
                ERROR_LOG (L"CreateInstance failed on CLexeme.");
                return hr;
            }

            spLexeme->SetDb (m_sDbPath);

            m_pDb->GetData (0, spLexeme->m_sSourceForm);
            int iHeadwordId = -1;
            m_pDb->GetData (1, iHeadwordId);
            m_pDb->GetData (2, spLexeme->m_sHeadwordComment);
            m_pDb->GetData (3, spLexeme->m_iDbKey);
            m_pDb->GetData (4, spLexeme->m_sGraphicStem);
            m_pDb->GetData (5, spLexeme->m_bIsVariant);
            m_pDb->GetData (6, spLexeme->m_sMainSymbol);
            m_pDb->GetData (7, (int&)spLexeme->m_ePartOfSpeech);
            m_pDb->GetData (8, spLexeme->m_bIsPluralOf);
            m_pDb->GetData (9, spLexeme->m_bTransitive);
            spLexeme->m_bTransitive = !spLexeme->m_bTransitive;     // "нп" == !b_transitive
            bool bReflexive = false;
            m_pDb->GetData (10, bReflexive);
            if (POS_VERB == spLexeme->m_ePartOfSpeech)
            {
                spLexeme->m_eReflexive = bReflexive ? REFL_YES : REFL_NO;
            }
            else
            {
                spLexeme->m_eReflexive = REFL_UNDEFINED;
            }
            m_pDb->GetData (11, spLexeme->m_sMainSymbolPluralOf);
            m_pDb->GetData (12, spLexeme->m_sAltMainSymbol);
            m_pDb->GetData (13, spLexeme->m_sInflectionType);
            m_pDb->GetData (14, spLexeme->m_sComment);
            m_pDb->GetData (15, spLexeme->m_sAltMainSymbolComment);
            m_pDb->GetData (16, spLexeme->m_sAltInflectionComment);
            m_pDb->GetData (17, spLexeme->m_sVerbStemAlternation);
            m_pDb->GetData (18, spLexeme->m_bPartPastPassZhd);
            m_pDb->GetData (19, spLexeme->m_iSection);
            m_pDb->GetData (20, spLexeme->m_bNoComparative);
            m_pDb->GetData (21, spLexeme->m_bAssumedForms);
            m_pDb->GetData (22, spLexeme->m_bYoAlternation);
            m_pDb->GetData (23, spLexeme->m_bOAlternation);
            m_pDb->GetData (24, spLexeme->m_bSecondGenitive);
            m_pDb->GetData (25, spLexeme->m_bHasAspectPair);
//            m_pDb->GetData (25, spLexeme->m_iAspectPairType);
//            m_pDb->GetData (26, spLexeme->m_sAspectPairComment);
            m_pDb->GetData (26, spLexeme->m_sQuestionableForms);
            m_pDb->GetData (27, spLexeme->m_bHasIrregularForms);
            m_pDb->GetData (28, spLexeme->m_bHasDeficiencies);
            m_pDb->GetData (29, spLexeme->m_sRestrictedFroms);
            m_pDb->GetData (30, spLexeme->m_sContexts);
            m_pDb->GetData (31, spLexeme->m_sTrailingComment);
            int iInflectionId = -1;
            m_pDb->GetData (32, iInflectionId);
            m_pDb->GetData (33, spLexeme->m_bPrimaryInflectionGroup);
            m_pDb->GetData (34, spLexeme->m_iType);
            m_pDb->GetData (35, (int&)spLexeme->m_eAccentType1);
            m_pDb->GetData (36, (int&)spLexeme->m_eAccentType2);
            m_pDb->GetData (37, spLexeme->m_bShortFormsRestricted);
            m_pDb->GetData (38, spLexeme->m_bPastParticipleRestricted);
            m_pDb->GetData (39, spLexeme->m_bNoShortForms);
            m_pDb->GetData (40, spLexeme->m_bNoPastParticiple);
            m_pDb->GetData (41, spLexeme->m_bFleetingVowel);
            m_pDb->GetData (42, spLexeme->m_iStemAugment);
            
            CEString sStressQuery (L"SELECT stress_position, is_primary FROM stress WHERE headword_id = ");
            sStressQuery += CEString::sToString (iHeadwordId);

            unsigned int uiStressHandle = m_pDb->uiPrepareForSelect (sStressQuery);
            while (m_pDb->bGetRow (uiStressHandle))
            {
                int iPos = -1;
                bool bPrimary = false;
                m_pDb->GetData (0, iPos, uiStressHandle);
                m_pDb->GetData (1, bPrimary, uiStressHandle);
                int iStressedSyll = spLexeme->m_sSourceForm.uiGetSyllableFromVowelPos (iPos);
                if (bPrimary)
                {
                    spLexeme->m_vecSourceStressPos.push_back (iStressedSyll);
                }
                else
                {
                    spLexeme->m_vecSecondaryStressPos.push_back (iStressedSyll);
                }
            }
            m_pDb->Finalize (uiStressHandle);

            if (spLexeme->m_vecSourceStressPos.empty())
            {
                ATLASSERT (spLexeme->m_vecSecondaryStressPos.empty());
                CEString sHwCommentQuery (L"SELECT comment FROM headword WHERE id = ");
                sHwCommentQuery += CEString::sToString (iHeadwordId);

                unsigned int uiHwCommentHandle = m_pDb->uiPrepareForSelect (sHwCommentQuery);
                while (m_pDb->bGetRow (uiHwCommentHandle))
                {
                    CEString sHwComment;
                    m_pDb->GetData (0, sHwComment, uiHwCommentHandle);
                    if (L"_без удар._" == sHwComment)
                    {
                        spLexeme->m_bIsUnstressed = true;
                    }
                }
                m_pDb->Finalize (uiHwCommentHandle);

                if (!spLexeme->m_bIsUnstressed && !(POS_PARTICLE == spLexeme->m_ePartOfSpeech))
                {
                    ATLASSERT (1 == spLexeme->m_sSourceForm.uiGetNumOfSyllables());
                    spLexeme->m_vecSourceStressPos.push_back (0);
                }
            }

            CEString sHomonymsQuery (L"SELECT homonym_number FROM homonyms WHERE headword_id = ");
            sHomonymsQuery += CEString::sToString (iHeadwordId);

            unsigned int uiHomonymsHandle = m_pDb->uiPrepareForSelect (sHomonymsQuery);
            while (m_pDb->bGetRow (uiHomonymsHandle))
            {
                int iHomonym = -1;
                m_pDb->GetData (0, iHomonym, uiHomonymsHandle);
                spLexeme->m_vecHomonyms.push_back (iHomonym);
            }
            m_pDb->Finalize (uiHomonymsHandle);

            CEString sDeviationQuery 
                (L"SELECT deviation_type, is_optional FROM common_deviation WHERE inflection_id = ");
            sDeviationQuery += CEString::sToString (iInflectionId);

            unsigned int uiDeviationHandle = m_pDb->uiPrepareForSelect (sDeviationQuery);
            while (m_pDb->bGetRow (uiDeviationHandle))
            {
                int iType = -1;
                bool bOptional = false;
                m_pDb->GetData (0, iType, uiDeviationHandle);
                m_pDb->GetData (1, bOptional, uiDeviationHandle);
                spLexeme->m_mapCommonDeviations[iType] = bOptional;
            }
            m_pDb->Finalize (uiDeviationHandle);

            m_coll.push_back (CComVariant (spLexeme));
        
        }   // while (m_pDb->b_GetRow())

        m_pDb->Finalize();
    }
    catch (...)
    {
        CEString sMsg;
        try
        {
            CEString sError;
            m_pDb->GetLastError (sError);
            sMsg = L"DB error: ";
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }
    
        sMsg += CEString::sToString (m_pDb->iGetLastError());
        ERROR_LOG (sMsg);
 
        return E_FAIL;
    }

    return S_OK;

}   //  bGetdata (...)
