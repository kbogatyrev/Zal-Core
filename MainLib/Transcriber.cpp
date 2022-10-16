#define _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING

#include <memory>
#include <cassert>

#include "Analytics.h"
#include "Transcriber.h"
#include "SqliteWrapper.h"
#include "WordForm.h"

using namespace Hlib;

CTranscriber::CTranscriber(shared_ptr<CSqlite> pCSqlite) : m_pDb(pCSqlite)
{
    if (!m_pDb)
    {
        ERROR_LOG(L"No database.");
        return;
    }

    auto eRet = eLoadTranscriptionRules();
    if (H_NO_ERROR != eRet)
    {
        ERROR_LOG(L"Unable to load transcription rules.");
    }

    if (m_mapCharToRules.empty())
    {
        ERROR_LOG(L"No transcription rules.");
    }
}

ET_ReturnCode CTranscriber::eSplitSource(CEString& sSource, vector<CEString>& vecTarget)
{
    sSource.ResetSeparators();
    sSource.SetBreakChars(L", \t");

    int iNWords = sSource.uiNFields();
    if (iNWords < 1)
    {
        return H_ERROR_UNEXPECTED;
    }

    for (int iAt = 0; iAt < iNWords; ++iAt)
    {
        vecTarget.push_back(sSource.sGetField(iAt));
    }

    return H_NO_ERROR;
}

ET_ReturnCode CTranscriber::eParseContexts(CEString& sSource, vector<PhonemicContextAtom>& vecTarget)
{
    assert(!sSource.bIsEmpty());

    vector<CEString> vecContextStrings;
    ET_ReturnCode eRet = eSplitSource(sSource, vecContextStrings);
    if (eRet != H_NO_ERROR)
    {
        ERROR_LOG(L"unable to split context string.");
        return H_ERROR_INVALID_ARG;
    }

    vector<PhonemicContextAtom> vecContexts;
    for (auto sContext : vecContextStrings)
    {
        auto itContext = m_mapStringToContext.find(sContext);
        PhonemicContextAtom context = ET_PhonemicContext::PHONEMIC_CONTEXT_UNDEFINED;
        if (m_mapStringToContext.end() != itContext)
        {
            context = itContext->second;
        }
        else
        {
            if (CEString::bStringOverAlphabet(sContext, m_LowerCaseCyrillic))
            {
                context = sContext;
            }
            else if (m_mapStringToSound.find(sContext) != m_mapStringToSound.end())
            {
                context = m_mapStringToSound[sContext];
            }
            else
            {
                ERROR_LOG(L"Phonemic context characters not recognized.");
                continue;
            }
        }

        if (context != PhonemicContextAtom(ET_PhonemicContext::PHONEMIC_CONTEXT_UNDEFINED))
        {
            vecTarget.emplace_back(context);
        }
    }

    return H_NO_ERROR;
}

ET_ReturnCode CTranscriber::eParseBoundaries(CEString& sSource, vector<ET_Boundary>& vecTarget)
{
    assert(!sSource.bIsEmpty());

    vector<CEString> vecBoundaryStrings;
    ET_ReturnCode eRet = eSplitSource(sSource, vecBoundaryStrings);
    if (eRet != H_NO_ERROR)
    {
        ERROR_LOG(L"unable to split boundary string.");
        return H_ERROR_INVALID_ARG;
    }

    vector<ET_Boundary> vecBoundaries;
    for (auto sBoundary : vecBoundaryStrings)
    {
        auto itBoundary = m_mapStringToBoundary.find(sBoundary);
        auto eBoundary = ET_Boundary::BOUNDARY_UNDEFINED;
        if (m_mapStringToBoundary.end() != itBoundary)
        {
            eBoundary = itBoundary->second;
        }
        else
        {
            ERROR_LOG(L"Boundary not recognized.");
            continue;
        }

        if (eBoundary != ET_Boundary::BOUNDARY_UNDEFINED)
        {
            vecTarget.emplace_back(eBoundary);
        }
    }

    return H_NO_ERROR;
}

ET_ReturnCode CTranscriber::eParseTargets(CEString& sSource, vector<ET_Sound>& vecTargets)
{
    assert(!sSource.bIsEmpty());

    unsigned int uiStart{ 0 };
    auto uiLength = sSource.uiFind(L" + ", uiStart);
    bool bError = false;
    while (uiLength != ERelation::ecNotFound)
    {
        uiLength -= uiStart;
        auto itTarget = m_mapStringToSound.find(sSource.sSubstr(uiStart, uiLength));
        if (m_mapStringToSound.end() == itTarget)
        {
            ERROR_LOG(L"Target not recognized.");
            bError = true;
            break;
        }
        vecTargets.push_back((*itTarget).second);
        uiStart += uiLength + 3;
        uiLength = sSource.uiFind(L" + ", uiStart);
    }

    if (bError)
    {
        return H_ERROR_INVALID_ARG;
    }

    auto itTarget = m_mapStringToSound.find(sSource.sSubstr(uiStart));
    if (m_mapStringToSound.end() == itTarget)
    {
        ERROR_LOG(L"Target not recognized.");
        return H_ERROR_INVALID_ARG;
    }
    else
    {
        vecTargets.push_back((*itTarget).second);
    }

    return H_NO_ERROR;
}

ET_ReturnCode CTranscriber::eParseTransforms(CEString& sSource, vector<ET_Transform>& vecTransforms)
{
    assert(!sSource.bIsEmpty());

    unsigned int uiStart{ 0 };
    auto uiLength = sSource.uiFind(L" + ", uiStart);
    bool bError = false;
    while (uiLength != ERelation::ecNotFound)
    {
        uiLength -= uiStart;
        auto itTransform = m_mapStringToTransform.find(sSource.sSubstr(uiStart, uiLength));
        if (m_mapStringToTransform.end() == itTransform)
        {
            ERROR_LOG(L"Transform not recognized.");
            bError = true;
            break;
        }
        vecTransforms.push_back((*itTransform).second);
        uiStart += uiLength + 3;
        uiLength = sSource.uiFind(L" + ", uiStart);
    }

    if (bError)
    {
        return H_ERROR_INVALID_ARG;
    }
    auto itTransform = m_mapStringToTransform.find(sSource.sSubstr(uiStart));
    if (m_mapStringToTransform.end() == itTransform)
    {
        ERROR_LOG(L"Transform not recognized.");
        return H_ERROR_INVALID_ARG;
    }
    else
    {
        vecTransforms.push_back((*itTransform).second);
    }

    return H_NO_ERROR;
}

bool CTranscriber::bIsProclitic(CWordForm& wf)
{
    if (POS_PREPOSITION == wf.ePos() || POS_CONJUNCTION == wf.ePos())
    {
        return true;
    }
    return false;
}

bool CTranscriber::bIsEnclitic(CWordForm& wf)
{
    if (POS_PARTICLE == wf.ePos())
    {
        if (wf.sWordForm() == L"ка" || wf.sWordForm() == L"ли" || wf.sWordForm() == L"то"
            || wf.sWordForm() == L"же")
        {
            return true;
        }
    }
    return false;
}

ET_ReturnCode CTranscriber::eLoadTranscriptionRules()
{
    if (nullptr == m_pDb)
    {
        ERROR_LOG(L"Database not available.");
        return H_ERROR_DB;
    }

    ET_ReturnCode eRet = H_NO_ERROR;

    static const CEString sQuery
        (L"SELECT tr.id, ti.input_chars, tr.stress, tr.left_contexts, tr.left_boundary, tr.right_contexts, right_boundary, \
        tr.morpheme_type, tr.subparadigm, tr.gramm_gender, tr.gramm_number, tr.gramm_case, tr.strength, tr.target, \
        tr.transform, tr.comment \
        FROM transcription_inputs AS ti INNER JOIN transcription_rules as tr ON ti.id = tr.input_id");

    try
    {
        m_pDb->PrepareForSelect(sQuery);
        while (m_pDb->bGetRow())
        {
            StRule stRule;
            m_pDb->GetData(0, stRule.m_llRuleId);

            CEString sInputs, sKeys;
            m_pDb->GetData(1, sInputs);

            //
            //  E.g., "a+b" --> "ab" = 'these two characters together, "ab" --> vector of chars = 'any of the chars in the vector
            //
            if (sInputs.bRegexMatch(L"^([абвгдеёжзийклмнопрстуфхцчшщъыьэюя]+)\\s*\\+\\s*([абвгдеёжзийклмнопрстуфхцчшщъыьэюя]+)\\s*"))
            {
                if (sInputs.uiGetNumOfRegexMatches() != 2)
                {
                    ERROR_LOG(L"Unexpected number of \"+\"-separated strings");
                    return H_ERROR_UNEXPECTED;
                }

                sKeys = sInputs.sGetRegexMatch(0);
                stRule.m_sFollowedBy = sInputs.sGetRegexMatch(1);
            }
            else
            {
                if (sInputs.bRegexMatch(L"^([абвгдеёжзийклмнопрстуфхцчшщъыьэюя]+)\\s*"))
                {
                    sKeys = sInputs.sGetRegexMatch(0);
                }
                //                else if (sInputs.bRegexMatch(L"^([A-Z]+)"))
                //                {
                //                    sKeys = sInputs.sGetRegexMatch(0);
                //                }
                else
                {
                    ERROR_LOG(L"Unable to parse transcription rule inputs.");
                    continue;
                }
            }

            // Stress (a string)
            CEString sStressType;
            m_pDb->GetData(2, sStressType);
            if (!sStressType.bIsEmpty())
            {
                auto itStressType = m_mapStringToStressRelation.find(sStressType);
                if (m_mapStringToStressRelation.end() == itStressType)
                {
                    ERROR_LOG(L"Stress context not recognized.");
                    continue;
                }
                stRule.m_vecStressContexts.emplace_back((*itStressType).second);
            }
            sort(stRule.m_vecStressContexts.begin(), stRule.m_vecStressContexts.end());

            // Left contexts
            CEString sLeftContexts;
            m_pDb->GetData(3, sLeftContexts);
            if (!sLeftContexts.bIsEmpty())
            {
                eRet = eParseContexts(sLeftContexts, stRule.m_vecLeftContexts);
            }

            // Left boundaries
            CEString sLeftBoundaries;
            m_pDb->GetData(4, sLeftBoundaries);
            if (!sLeftBoundaries.bIsEmpty())
            {
                eRet = eParseBoundaries(sLeftBoundaries, stRule.m_vecLeftBoundaries);
            }

            // Right contexts
            CEString sRightContexts;
            m_pDb->GetData(5, sRightContexts);
            if (!sRightContexts.bIsEmpty())
            {
                eRet = eParseContexts(sRightContexts, stRule.m_vecRightContexts);
            }

            // Right boundaries
            CEString sRightBoundaries;
            m_pDb->GetData(6, sRightBoundaries);
            if (!sRightBoundaries.bIsEmpty())
            {
                eRet = eParseBoundaries(sRightBoundaries, stRule.m_vecRightBoundaries);
            }

            // Morphemic context
            CEString sMorphemeType;
            m_pDb->GetData(7, sMorphemeType);
            if (!sMorphemeType.bIsEmpty())
            {
                vector<CEString> vecMorphemeTypeStrings;
                eRet = eSplitSource(sMorphemeType, vecMorphemeTypeStrings);
                if (eRet != H_NO_ERROR)
                {
                    ERROR_LOG(L"unable to split morphemic contexts string.");
                    continue;
                }

                MorphemicContextAtom context;
                for (auto sMorphemeType : vecMorphemeTypeStrings)
                {
                    auto itMorphemeType = m_mapStringToMorphemicContext.find(sMorphemeType);
                    if (itMorphemeType != m_mapStringToMorphemicContext.end())
                    {
                        context = itMorphemeType->second;
                    }
                    else
                    {
                        if (CEString::bStringOverAlphabet(sMorphemeType, m_LowerCaseCyrillic))
                        {
                            context = sMorphemeType;
                        }
                        else
                        {
                            ERROR_LOG(L"Phonemic context characters not recognized.");
                            continue;
                        }
                    }
                    stRule.m_vecMorphemicContexts.emplace_back(context);
                }
            }

            // Subparadigm (a string)
            CEString sSubparadigm;
            m_pDb->GetData(8, sSubparadigm);
            if (!sSubparadigm.bIsEmpty())
            {
                auto itSubparadigm = m_mapStringToSubparadigm.find(sSubparadigm);
                if (m_mapStringToSubparadigm.end() == itSubparadigm)
                {
                    ERROR_LOG(L"Subparadigm not recognized.");
                    continue;
                }
                stRule.m_vecSubparadigms.emplace_back((*itSubparadigm).second);
            }

            // Gender
            CEString sGender;
            m_pDb->GetData(9, sGender);
            if (!sGender.bIsEmpty())
            {
                auto itGender = m_mapStringToGender.find(sGender);
                if (m_mapStringToGender.end() == itGender)
                {
                    ERROR_LOG(L"Gender not recognized.");
                    continue;
                }
                stRule.m_vecGenders.emplace_back((*itGender).second);
            }

            // Number 
            CEString sNumber;
            m_pDb->GetData(10, sNumber);
            if (!sNumber.bIsEmpty())
            {
                auto itNumber = m_mapStringToNumber.find(sNumber);
                if (m_mapStringToNumber.end() == itNumber)
                {
                    ERROR_LOG(L"Number not recognized.");
                    continue;
                }
                stRule.m_vecNumbers.emplace_back((*itNumber).second);
            }

            // Case 
            CEString sCase;
            m_pDb->GetData(11, sCase);
            if (!sCase.bIsEmpty())
            {
                auto itCase = m_mapStringToCase.find(sCase);
                if (m_mapStringToCase.end() == itCase)
                {
                    ERROR_LOG(L"Case not recognized.");
                    continue;
                }
                stRule.m_vecCases.emplace_back((*itCase).second);
            }

            // Strength
            CEString sStrength;
            m_pDb->GetData(12, sStrength);
            if (!sStrength.bIsEmpty())
            {
                auto itStrength = m_mapStringToRuleStrength.find(sStrength);
                if (m_mapStringToRuleStrength.end() == itStrength)
                {
                    ERROR_LOG(L"Strength not recognized.");
                    continue;
                }
                stRule.m_eStrength = (*itStrength).second;
            }

            // IsVariant
//            m_pDb->GetData(12, stRule.m_bIsVariant);

            // Targets
            CEString sTargets;
            m_pDb->GetData(13, sTargets);
            if (!sTargets.bIsEmpty())
            {
                eRet = eParseTargets(sTargets, stRule.m_vecTargets);
            }

            // Transforms
            CEString sTransforms;
            m_pDb->GetData(14, sTransforms);
            if (!sTransforms.bIsEmpty())
            {
                eRet = eParseTransforms(sTransforms, stRule.m_vecTransforms);
            }

            m_pDb->GetData(15, stRule.m_sComment);

            for (int iAt = 0; iAt < (int)sKeys.uiLength(); ++iAt)
            {
                m_mapCharToRules[sKeys[iAt]].push_back(stRule);
            }
        }

        m_pDb->Finalize();
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    return H_NO_ERROR;

}       //  eLoadTranscriptionRules()

ET_ReturnCode CTranscriber::eTranscribeTactGroup(shared_ptr<StTactGroup> pstTactGroup)
{
    pstTactGroup->sSource.SetVowels(CEString::g_szRusVowels);
    m_vecTranscription.clear();

    int iAt = 0;
    while (iAt < (int)pstTactGroup->sSource.uiLength())
    {
        if (CEString::bIsVowel(pstTactGroup->sSource[iAt]))
        {
            eHandleVowel(pstTactGroup, iAt);
        }

        if (CEString::bIsConsonant(pstTactGroup->sSource[iAt]))
        {
            eHandleConsonant(pstTactGroup, iAt);
        }
        ++iAt;
    }

//    stTactGroup.sTranscription.Erase();
    for (auto& eSound : m_vecTranscription)
    {
        auto it = m_mapSoundToTranscription.find(eSound);
        if (it != m_mapSoundToTranscription.end())
        {
            pstTactGroup->sTranscription += it->second;
        }
        else
        {
            pstTactGroup->sTranscription += L'?';
        }
    }

    auto eRet = eAddStressMark(pstTactGroup);

    return eRet;
}

ET_ReturnCode CTranscriber::eHandleVowel(shared_ptr<StTactGroup> pstTg, int& iPos)
{
    if (iPos < 0 || iPos >= (int)pstTg->sSource.uiLength())
    {
        CEString sMsg(L"Illegal character position: ");
        sMsg += CEString::sToString(iPos);
        ERROR_LOG(sMsg);
        return H_ERROR_INVALID_ARG;
    }

    wchar_t chrVowel = pstTg->sSource.chrGetAt(iPos);
    wchar_t chrNext = pstTg->sSource.chrGetAt(iPos+1);
    if (!CEString::bIsVowel(chrVowel))
    { 
        CEString sMsg(L"Character at position ");
        sMsg += CEString::sToString(iPos);
        sMsg += L" is not a vowel";
        ERROR_LOG(sMsg);
        return H_ERROR_INVALID_ARG;
    }

    auto itRules = m_mapCharToRules.find(chrVowel);
    if (m_mapCharToRules.end() == itRules)
    {
        CEString sMsg(L"No rules for vowel ");
        sMsg += chrVowel;
        return H_ERROR_UNEXPECTED;
    }

    auto bFound = false;

    auto eStressStatus = ET_VowelStressRelation::VOWEL_STRESS_RELATION_UNDEFINED;
    eGetStressStatus(pstTg, iPos, eStressStatus);
    for (auto& stRule : itRules->second)
    { 
        if (!stRule.m_sFollowedBy.bIsEmpty())
        {
            if (chrNext != stRule.m_sFollowedBy)
            {
                continue;
            }
        }

        if (!stRule.m_vecStressContexts.empty() && stRule.m_vecStressContexts.end() ==
            find(stRule.m_vecStressContexts.begin(), stRule.m_vecStressContexts.end(), eStressStatus))
        {
            continue;
        }

        for (auto& leftContext : stRule.m_vecLeftContexts)
        {
            auto eRet = eContextMatch(pstTg, leftContext, ET_ContextDirection::LEFT_CONTEXT, iPos);
            if (H_TRUE == eRet)
            {
                bFound = true;
                break;
            }
            bFound = false;
        }

        if (stRule.m_vecLeftContexts.size() > 0 && !bFound)
        {
            continue;
        }

        for (auto& leftBoundary : stRule.m_vecLeftBoundaries)
        {
            auto eRet = eBoundaryMatch(pstTg, leftBoundary, ET_ContextDirection::LEFT_CONTEXT, iPos);
            if (H_TRUE == eRet)
            {
                bFound = true;
                break;
            }
            bFound = false;
        }

        if (stRule.m_vecLeftBoundaries.size() > 0 && !bFound)
        {
            continue;
        }

        for (auto& rightContext : stRule.m_vecRightContexts)
        {
            auto eRet = eContextMatch(pstTg, rightContext, ET_ContextDirection::RIGHT_CONTEXT, iPos);
            if (H_TRUE == eRet)
            {
                bFound = true;
                break;
            }
            bFound = false;
        }

        if (stRule.m_vecRightContexts.size() > 0 && !bFound)
        {
            continue;
        }

        for (auto& rightBoundary : stRule.m_vecRightBoundaries)
        {
            auto eRet = eBoundaryMatch(pstTg, rightBoundary, ET_ContextDirection::RIGHT_CONTEXT, iPos);
            if (H_TRUE == eRet)
            {
                bFound = true;
                break;
            }
            bFound = false;
        }

        if (stRule.m_vecRightBoundaries.size() > 0 && !bFound)
        {
            continue;
        }
        
        for (auto& mContext : stRule.m_vecMorphemicContexts)
        {
            auto eRet = eMorphemeMatch(pstTg, mContext, iPos);
            if (H_TRUE == eRet)
            {
                bFound = true;
                break;
            }
            bFound = false;
        }

        if (stRule.m_vecMorphemicContexts.size() > 0 && !bFound)
        {
            continue;
        }

        if (!stRule.m_vecSubparadigms.empty())
        {
            auto eRet = eSubparadigmMatch(pstTg, stRule.m_vecSubparadigms);
            if (H_TRUE == eRet)
            {
                bFound = true;
                break;
            }
            bFound = false;
        }
        if (stRule.m_vecSubparadigms.size() > 0 && !bFound)
        {
            continue;
        }

        for (auto eSound : stRule.m_vecTargets)
        {
            m_vecTranscription.push_back(eSound);
        }

        if (!stRule.m_sFollowedBy.bIsEmpty())
        {
            ++iPos;
        }
    }       //  for (auto&& stRule : itRules->second)

    return H_NO_ERROR;

}       //  eHandleVowel()

ET_ReturnCode CTranscriber::eHandleConsonant(shared_ptr<StTactGroup> pstTg, int& iPos)
{
    if (iPos < 0 || iPos >= (int)pstTg->sSource.uiLength())
    {
        CEString sMsg(L"Illegal character position: ");
        sMsg += CEString::sToString(iPos);
        ERROR_LOG(sMsg);
        return H_ERROR_INVALID_ARG;
    }

    wchar_t chrConsonant = pstTg->sSource.chrGetAt(iPos);
    if (!CEString::bIsConsonant(chrConsonant))
    {
        CEString sMsg(L"Character at position ");
        sMsg += CEString::sToString(iPos);
        sMsg += L" is not a consonant";
        ERROR_LOG(sMsg);
        return H_ERROR_INVALID_ARG;
    }

    auto itRules = m_mapCharToRules.find(chrConsonant);
    if (m_mapCharToRules.end() == itRules)
    {
        CEString sMsg(L"No rules for consonant ");
        sMsg += chrConsonant;
        ERROR_LOG(sMsg);
        return H_ERROR_UNEXPECTED;
    }

    auto bFound = true;        // we won't break out of loop event if a
    auto bTranscribed = false;  // matching rule was found; subsequent matches    
                                // will be repored as errors

    for (auto& stRule : itRules->second)
    {
        for (auto& leftContext : stRule.m_vecLeftContexts)
        {
            auto eRet = eContextMatch(pstTg, leftContext, ET_ContextDirection::LEFT_CONTEXT, iPos);
            if (H_TRUE == eRet)
            {
                bFound = true;
                break;
            }
            bFound = false;
        }

        if (stRule.m_vecLeftContexts.size() > 0 && !bFound)
        {
            continue;
        }

        for (auto& leftBoundary : stRule.m_vecLeftBoundaries)
        {
            auto eRet = eBoundaryMatch(pstTg, leftBoundary, ET_ContextDirection::LEFT_CONTEXT, iPos);
            if (H_TRUE == eRet)
            {
                bFound = true;
                break;
            }
            bFound = false;
        }

        if (stRule.m_vecLeftBoundaries.size() > 0 && !bFound)
        {
            continue;
        }

        for (auto& rightContext : stRule.m_vecRightContexts)
        {
            auto eRet = eContextMatch(pstTg, rightContext, ET_ContextDirection::RIGHT_CONTEXT, iPos);
            if (H_TRUE == eRet)
            {
                bFound = true;
                break;
            }
            bFound = false;
        }

        if (stRule.m_vecRightContexts.size() > 0 && !bFound)
        {
            continue;
        }

        for (auto& rightBoundary : stRule.m_vecRightBoundaries)
        {
            auto eRet = eBoundaryMatch(pstTg, rightBoundary, ET_ContextDirection::RIGHT_CONTEXT, iPos);
            if (H_TRUE == eRet)
            {
                bFound = true;
                break;
            }
            bFound = false;
        }

        if (stRule.m_vecRightBoundaries.size() > 0 && !bFound)
        {
            continue;
        }

        for (auto& mContext : stRule.m_vecMorphemicContexts)
        {
            auto eRet = eMorphemeMatch(pstTg, mContext, iPos);
            if (H_TRUE == eRet)
            {
                bFound = true;
                break;
            }
            bFound = false;
        }

        if (stRule.m_vecMorphemicContexts.size() > 0 && !bFound)
        {
            continue;
        }

        if (!stRule.m_vecSubparadigms.empty())
        {
            auto eRet = eSubparadigmMatch(pstTg, stRule.m_vecSubparadigms);
            if (H_TRUE == eRet)
            {
                bFound = true;
                break;
            }
        }

        if (stRule.m_vecSubparadigms.size() > 0 && !bFound)
        {
            continue;
        }

        auto stConsonant = m_mapCharToConsonant[chrConsonant];
        if (bFound)
        {
            if (bTranscribed)
            {
                CEString sMsg(L"Redundant rule match: rule ");
                sMsg += CEString::sToString(stRule.m_llRuleId);
                ERROR_LOG(sMsg);
            }
            else
            {
                for (auto eTransform : stRule.m_vecTransforms)
                {
                    eApplyTransform(stConsonant, eTransform);
                    m_vecTranscription.push_back(m_mapConsonantToSound[stConsonant]);
                    bTranscribed = true;
                }

                if (!stRule.m_sFollowedBy.bIsEmpty())
                {
                    ++iPos;
                }
            }
        }
    }       //  for (auto&& stRule : itRules->second)

    if (!bTranscribed)
    {
        m_vecTranscription.push_back(m_mapConsonantToSound[m_mapCharToConsonant[chrConsonant]]);
    }
    
    return H_NO_ERROR;

}       //  eHandleConsonant()

ET_ReturnCode CTranscriber::eGetStressStatus(shared_ptr<StTactGroup> pstTg, int iPos, ET_VowelStressRelation& eStressStatus)
{
    eStressStatus = ET_VowelStressRelation::VOWEL_STRESS_RELATION_UNDEFINED;

    if (iPos < 0 || iPos >= (int)pstTg->sSource.uiLength())
    {
        CEString sMsg(L"Invalid character at position: ");
        sMsg += CEString::sToString(iPos);
        sMsg += L".";
        ERROR_LOG(sMsg);
        return H_ERROR_INVALID_ARG;
    }

    wchar_t chrVowel = pstTg->sSource.chrGetAt(iPos);
    if (!CEString::bIsVowel(chrVowel))
    {
        CEString sMsg(L"Character at position ");
        sMsg += CEString::sToString(iPos);
        sMsg += L" is not a vowel";
        ERROR_LOG(sMsg);
        return H_ERROR_INVALID_ARG;
    }

    int iSyllPos = -1;
    try
    {
        iSyllPos = static_cast<int>(pstTg->sSource.uiGetSyllableFromVowelPos(iPos));
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    if (iSyllPos == pstTg->iStressedSyllable)
    {
        eStressStatus = ET_VowelStressRelation::STRESSED;
        return H_NO_ERROR;
    }

    if (pstTg->iStressedSyllable - iSyllPos == 1)
    {
        eStressStatus = ET_VowelStressRelation::FIRST_PRETONIC;
        return H_NO_ERROR;
    }

    if (pstTg->iStressedSyllable - iSyllPos > 1)
    {
        eStressStatus = ET_VowelStressRelation::OTHER_PRETONIC;
        return H_NO_ERROR;
    }

    if (pstTg->iStressedSyllable - iSyllPos < 1)
    {
        eStressStatus = ET_VowelStressRelation::POSTTONIC;
        return H_NO_ERROR;
    }

    CEString sMsg(L"Unable to determine stress status.");
    ERROR_LOG(sMsg);
 
    return H_ERROR_UNEXPECTED;

}       //  eGetStressStatus()

ET_ReturnCode CTranscriber::eContextMatch(shared_ptr<StTactGroup> pTg, PhonemicContextAtom context, ET_ContextDirection eType, int iPos)
{
    auto eRet = H_NO_ERROR;

    struct StMatchTypes
    {
        shared_ptr<StTactGroup> m_pStTg;
        ET_ContextDirection m_eType;
        int m_iPos;
        ET_ReturnCode m_eRet;

        StMatchTypes(shared_ptr<StTactGroup> pStTg, ET_ContextDirection eType, int iPos, ET_ReturnCode eRet) 
            : m_pStTg(pStTg), m_eType(eType), m_iPos(iPos), m_eRet(eRet) {};

        // ET_PhonemicContext match
        bool operator()(ET_PhonemicContext& eContext)
        {
            wchar_t chrAdjacent = L'\0';
            if (ET_ContextDirection::LEFT_CONTEXT == m_eType && m_iPos > 0)
            {
                chrAdjacent = m_pStTg->sSource[m_iPos-1];
            }
            else if (ET_ContextDirection::RIGHT_CONTEXT == m_eType && m_iPos < (int)m_pStTg->sSource.uiLength()-1)
            {
                chrAdjacent = m_pStTg->sSource[m_iPos+1];
            }

            auto bMatch = false;

            switch (eContext)
            {
            case ET_PhonemicContext::VOWEL:
                if (CEString::bIn(chrAdjacent, m_Vowels))
                {
                    bMatch = true;
                }
                break;
            case ET_PhonemicContext::CONSONANT:
                if (CEString::bIn(chrAdjacent, m_Consonants))
                {
                    bMatch = true;
                }
                break;
            case ET_PhonemicContext::HARD_CONSONANT:
                if (CEString::bIn(chrAdjacent, m_HardConsonants))
                {
                    bMatch = true;
                }
                break;
            case ET_PhonemicContext::HARD_PAIRED_CONSONANT:
                if (CEString::bIn(chrAdjacent, m_PairedHardSoftConsonants))
                {
                    bMatch = true;
                }
                break;
            case ET_PhonemicContext::SOFT_CONSONANT:
                if (CEString::bIn(chrAdjacent, m_SoftConsonants))
                {
                    bMatch = true;
                }
                break;
            case ET_PhonemicContext::SOFT_CONSONANT_NO_CH_SHCH:
                if (CEString::bIn(chrAdjacent, m_PairedHardSoftConsonants))
                {
                    bMatch = true;
                }
                break;
            case ET_PhonemicContext::VOICELESS:
                if (CEString::bIn(chrAdjacent, m_VoicelessConsonants))
                {
                    bMatch = true;
                }
                break;
            default:
                m_eRet = H_ERROR_UNEXPECTED;
                CEString sMsg(L"Left context not recognized: ");
                sMsg += to_wstring(eContext).c_str();
                ERROR_LOG(sMsg);

            }       //  switch

            return bMatch; 
        }
        
        bool operator()([[maybe_unused]]ET_Sound& eContext)
        {
            [[maybe_unused]]wchar_t chrAdjacent = L'\0';
            if (ET_ContextDirection::LEFT_CONTEXT == m_eType && m_iPos > 0)
            {
                chrAdjacent = m_pStTg->sSource[m_iPos - 1];
            }
            else if (ET_ContextDirection::RIGHT_CONTEXT == m_eType && m_iPos < (int)m_pStTg->sSource.uiLength() - 1)
            {
                chrAdjacent = m_pStTg->sSource[m_iPos + 1];
            }

            auto bMatch = false;

            // Currently not supported

            return bMatch;
        }

        // string match
        bool operator()(CEString& sContext) 
        {            
            //wchar_t chrAdjacent = L'\0';
            if (ET_ContextDirection::LEFT_CONTEXT == m_eType)
            {
                if (m_iPos > 0 && CEString::bIn(m_pStTg->sSource[m_iPos-1], sContext))
                {
                    return true;
                }
            }
            else if (ET_ContextDirection::RIGHT_CONTEXT == m_eType)
            {
                if (m_iPos < (int)m_pStTg->sSource.uiLength()-1 && CEString::bIn(m_pStTg->sSource[m_iPos+1], sContext))
                {
                    return true;
                }
            }
            return false;
        }
    };

    auto match = visit(StMatchTypes(pTg, eType, iPos, eRet), context);

    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    return match ? H_TRUE : H_FALSE;

}       //  eContextMatch()

ET_ReturnCode CTranscriber::eMorphemeMatch(shared_ptr<StTactGroup> pstTg, MorphemicContextAtom context, int iPos)
{
    auto eRet = H_NO_ERROR;

    struct StMatchTypes
    {
        bool bError = false;
        shared_ptr<StTactGroup> m_pstTg = nullptr;
        shared_ptr<StWordParse> m_pstParse;
        int m_iPos = -1;
        int m_iStemLength = -1;
        ET_ReturnCode m_eRet;
        StMatchTypes(shared_ptr<StTactGroup> pstTg, int iPos, ET_ReturnCode eRet) : m_pstTg(pstTg), m_iPos(iPos), m_eRet(eRet) 
        {
            auto iWord = pstTg->iWordNumFromTextPos(iPos);
            if (iWord >= (int)pstTg->m_vecParses.size())
            {
                bError = true;
                CEString sMsg(L"Illegal word position: ");
                sMsg += CEString::sToString(iWord);
                sMsg += L", for text position ";
                sMsg += CEString::sToString(iPos);
                ERROR_LOG(sMsg);
                return;
            }
            m_pstParse = *m_pstTg->m_vecParses[iWord].begin();  // TODO other parses in the invariant set may be structurally different,
                                                                // should we check them all?
//            auto iStemLength = (int)(m_pstParse->WordForm.sWordForm().uiLength() -
//                                     m_pstParse->WordForm.sEnding().uiLength());          // TODO unused
        };

        // Enum match
        bool operator()(ET_MorphemicContext& eContext)
        {
            auto bMatch = false;
            switch (eContext)
            {
            case ET_MorphemicContext::ENDING:
                if (m_iPos >= m_iStemLength)
                {
                    return true;    // ending: return true
                }
                return false;

            case ET_MorphemicContext::NOT_ENDING:         
                if (m_iPos >= m_iStemLength)
                {
                    return false;    // ending: return false
                }
                return true;

            case ET_MorphemicContext::ROOT:     // we'll use stem as "root"
                if (m_iPos < m_iStemLength)
                {
                    return true;
                }
                return false;

            case ET_MorphemicContext::ROOT_AUSLAUT:     // we'll use stem as "root"
                if (m_iPos == m_iStemLength-1)
                {
                    return true;
                }
                return false;

            default:
                m_eRet = H_ERROR_UNEXPECTED;
                CEString sMsg(L"Morphemic context not recognized: ");
                sMsg += to_wstring(eContext).c_str();
                ERROR_LOG(sMsg);

            }       //  switch

            return bMatch;

        }       //  bool operator()

        // string match
        bool operator()(CEString& sContext)
        {
            if (CEString::bIn(m_pstTg->sSource[m_iPos], sContext))
            {
                return true;
            }
            return false;
        }
    };

    auto match = visit(StMatchTypes(pstTg, iPos, eRet), context);

    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    return match ? H_TRUE : H_FALSE;

}       //  eMorphemeMatch()

ET_ReturnCode CTranscriber::eBoundaryMatch(shared_ptr<StTactGroup> pstTg, ET_Boundary eBoundary, ET_ContextDirection eDirection, int iPos)
{
    auto iWordNum = pstTg->iWordNumFromTextPos(iPos);

    switch (eBoundary)
    {
        case ET_Boundary::BOUNDARY_WORD:
            if (ET_ContextDirection::LEFT_CONTEXT == eDirection)
            {
                if (0 == iPos)
                {
                    return H_TRUE;
                }
                else
                {
                    return H_FALSE;
                }
            }

            if (ET_ContextDirection::RIGHT_CONTEXT == eDirection)
            {
                auto uiLength = pstTg->sSource.uiLength();
                if ((int)pstTg->sSource.uiLength()-2 == iPos && L'ь' == pstTg->sSource[uiLength-1])
                {
                    if (CEString::bIsConsonant(pstTg->sSource[uiLength-2]))
                    {
                        return H_TRUE;
                    }
                }

                if (iPos == (int)pstTg->sSource.uiLength()-1)
                {
                    return H_TRUE;
                }
                else
                {
                    return H_FALSE;
                }
            }
        break;
        
        case ET_Boundary::BOUNDARY_NOT_PROCLITIC:
//            if (stTg.iNumOfWords < 2)
//            {
//                return H_TRUE;
//            }

            if (iPos > 0)
            {
                return H_FALSE;
            }

            if (LEFT_CONTEXT == eDirection)
            {
                try
                {
                    if (iWordNum > 0)
                    {
                        auto pstParse = *(pstTg->m_vecParses.at(iWordNum-1).begin());
                        if (ET_WordStressType::WORD_STRESS_TYPE_PROCLITIC == pstParse->eStressType)
                            // TODO how about "CLITC"??
                        {
                            return H_FALSE;
                        }
                        else
                        {
                            return H_TRUE;
                        }
                    }
                    else
                    {
                        return H_TRUE;
                    }
                } 
                catch (std::exception& ex)
                {
                    CEString sMsg(L"Exception: ");
                    sMsg += CEString::sToString(ex.what());
                    ERROR_LOG(sMsg);
                }
            }
            else if (RIGHT_CONTEXT == eDirection)
            {
                ERROR_LOG(L"Opearion not allowed for right context.");
            }
            else
            {
                ERROR_LOG(L"Illegal context.");
            }

        break;

        case ET_Boundary::BOUNDARY_SYNTAGM:
            if (0 == iPos)      // How to distinguish from word boundary?
            {
                return H_TRUE;
            }
            break;

        default:
            CEString sMsg(L"Left boundary not recognized: ");
            sMsg += to_wstring(eBoundary).c_str();
            ERROR_LOG(sMsg);
            return H_ERROR_UNEXPECTED;
    
    }       //  switch (eBoundary)

    return H_FALSE;

}       //  eBoundaryMatch()

ET_ReturnCode CTranscriber::eSubparadigmMatch(shared_ptr<StTactGroup> pstTg, const vector<ET_Subparadigm>& vecSp)
{
    /*
    auto m_pstParse = *m_pstTg->m_vecParses[iWord].begin();&&&&
    for (auto& stWordParse : pstTg.vecWords)
    {
        auto itRet = find(vecSp.begin(), vecSp.end(), stWordParse.WordForm.eSubparadigm());
        return itRet != vecSp.end() ? H_TRUE : H_FALSE;                         // TODO need to return array of values one for each parse
    }
    */
    return H_FALSE;
}

ET_ReturnCode CTranscriber::eGenderMatch(shared_ptr<StTactGroup> stTg, const vector<ET_Gender>& vecGenders)
{
    /*
    for (auto& stWordParse : pstTg.vecWords)
    {
        auto itRet = find(vecGenders.begin(), vecGenders.end(), stWordParse.WordForm.eGender());
        return itRet != vecGenders.end() ? H_TRUE : H_FALSE;                         // TODO need to return array of values one for each parse
    }
    */
    return H_FALSE;
}

ET_ReturnCode CTranscriber::eNumberMatch(shared_ptr<StTactGroup> stTg, const vector<ET_Number>& vecNumbers)
{
    /*
    for (auto& stWordParse : pstTg.vecWords)
    {
        auto itRet = find(vecNumbers.begin(), vecNumbers.end(), stWordParse.WordForm.eNumber());
        return itRet != vecNumbers.end() ? H_TRUE : H_FALSE;                         // TODO need to return array of values one for each parse
    }
    */
    return H_FALSE;
}

ET_ReturnCode CTranscriber::eCaseMatch(shared_ptr<StTactGroup> stTg, const vector<ET_Case>& vecCases)
{
    /*
    for (auto& stWordParse : pstTg.vecWords)
    {
        auto itRet = find(vecCases.begin(), vecCases.end(), stWordParse.WordForm.eCase());
        return itRet != vecCases.end() ? H_TRUE : H_FALSE;                         // TODO need to return array of values one for each parse
    }
    */
    return H_FALSE;
}

ET_ReturnCode CTranscriber::eApplyTransform(StConsonant& stConsonant, ET_Transform eTransform)
{
    switch (eTransform)
    {
    case ET_Transform::SOFTEN:
        stConsonant.m_ePalatalization = PALATALIZATION_SOFT;
        break;
    case ET_Transform::VOICE:
        stConsonant.m_eVoicedness = VOICEDNESS_VOICED;
        break;
    case ET_Transform::DEVOICE:
        stConsonant.m_eVoicedness = VOICEDNESS_VOICELESS;
        break;
    case ET_Transform::SELF:
        break;
    default:
        {
            CEString sMsg(L"Invalid transform value: ");
            sMsg += CEString::sToString(eTransform);
            ERROR_LOG(sMsg);
        }
    }

    return H_NO_ERROR;
}

ET_ReturnCode CTranscriber::eAddStressMark(shared_ptr<StTactGroup> pstTactGroup)
{
    if (pstTactGroup->sTranscription.uiLength() <= 0)
    {
        ERROR_LOG(L"No transcription.");
        return H_ERROR_INVALID_ARG;
    }

    int iSyll = 0;
    int iStressPos = 0;
    for (auto& chr : pstTactGroup->sTranscription)
    {
        if (CEString::bIn(chr, L"аоеиыуʌεъь"))
        {
            if (iSyll == pstTactGroup->iStressedSyllable)
            {

                break;
            }
            ++iSyll;
        }
        ++iStressPos;
    }

    if (iSyll >= pstTactGroup->iNumOfSyllables)
    {
        ERROR_LOG(L"Unable to find stressed vowel.");
        return H_ERROR_UNEXPECTED;
    }

    pstTactGroup->sTranscription.sInsert(iStressPos+1, CEString::g_chrCombiningAcuteAccent);

    return H_NO_ERROR;
}
