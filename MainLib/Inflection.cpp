#include "SqliteWrapper.h"
#include "WordForm.h"
#include "FormBuilderNouns.h"
#include "FormBuilderAdjLong.h"
#include "FormBuilderAdjShort.h"
#include "FormBuilderAdjComparative.h"
#include "FormBuilderPronounAdj.h"
#include "FormBuilderVerbPers.h"
#include "FormBuilderVerbPast.h"
#include "FormBuilderVerbImpv.h"
#include "FormBuilderVerbNonFinite.h"
#include "FormBuilderAspectPair.h"
#include "Inflection.h"
#include "sha1.hpp"     // ThirdParty/sha1

using namespace Hlib;

CInflection::CInflection(CLexeme* spLexeme)
{
    if (nullptr == spLexeme)
    {
        throw CException(H_EXCEPTION, L"Lexeme pointer is NULL.");
    }

    m_pLexeme = spLexeme;
}

CInflection::CInflection(const CInflection& source) : enable_shared_from_this(source)
{
    auto it = source.m_mmWordForms.begin();
    for (; it != source.m_mmWordForms.end(); ++it)
    {
        auto spWfCopy = make_shared<CWordForm>(*(*it).second);
        spWfCopy->m_spInflection = this;
        pair<CEString, shared_ptr<CWordForm>> pairHW(spWfCopy->sGramHash(), spWfCopy);
        m_mmWordForms.insert(pairHW);
    }
}

CInflection::~CInflection()
{
    m_mmWordForms.clear();
//    multimap<CEString, shared_ptr<CWordForm>>::iterator itWf = m_mmWordForms.begin();
//    for (; itWf != m_mmWordForms.end(); ++itWf)
//    {
//        auto itRange = m_mmWordForms.equal_range(itWf->first);
//        for (auto& itForm = itRange.first; itForm != itRange.second; ++itForm)
//        {
//            itForm->second = nullptr;
//        }
//    }
}

bool CInflection::bHasCommonDeviation(int iCd)
{
    //    bool bRet = false;
    map<int, bool>::iterator itCd = m_stProperties.mapCommonDeviations.find(iCd);
    return (itCd != m_stProperties.mapCommonDeviations.end());
}

bool CInflection::bDeviationOptional(int iCd)
{
    //    bool bRet = false;
    map<int, bool>::iterator itCd = m_stProperties.mapCommonDeviations.find(iCd);
    if (itCd != m_stProperties.mapCommonDeviations.end())
    {
        return (*itCd).second;
    }
    else
    {
        assert(0);
        CEString sMsg(L"Common deviation expected.");
        ERROR_LOG(sMsg);
        throw CException(H_EXCEPTION, sMsg);
    }
}

ET_ReturnCode CInflection::eAddCommonDeviation(int iValue, bool bIsOptional)
{
    auto stLexemeProperties = m_pLexeme->stGetProperties();

    if (iValue < 1)
    {
        ERROR_LOG(L"Negative value for common deviation");
        return H_ERROR_UNEXPECTED;
    }
    if (POS_NOUN == stLexemeProperties.ePartOfSpeech || POS_ADJ == stLexemeProperties.ePartOfSpeech)
    {
        if (iValue > 3)
        {
            ERROR_LOG(L"Unexpected value for common deviation; must be 1, 2, or 3");
            return H_ERROR_UNEXPECTED;
        }
    }
    else if (POS_VERB == stLexemeProperties.ePartOfSpeech)
    {
        if (iValue > 9)
        {
            ERROR_LOG(L"Unexpected value for common deviation; must be 1, 2, or 3");
            return H_ERROR_UNEXPECTED;
        }
    }

    m_stProperties.mapCommonDeviations[iValue] = bIsOptional;

    return H_NO_ERROR;
}

bool CInflection::bFindCommonDeviation(int iNum, bool& bIsOptional)
{
    bIsOptional = false;

    map<int, bool>::iterator itCommonDev = m_stProperties.mapCommonDeviations.find(iNum);
    if (m_stProperties.mapCommonDeviations.end() == itCommonDev)
    {
        return false;
    }

    bIsOptional = (*itCommonDev).second;

    return true;
}

ET_ReturnCode CInflection::eMakeGraphicStem(const CEString& sSource, CEString& sGraphicStem)
{
    if (sSource.uiLength() < 1)
    {
        ERROR_LOG(L"No source form.");
        return H_ERROR_UNEXPECTED;
    }

    auto sInflectionType = m_pLexeme->stGetProperties().sInflectionType;

    if (L"м" == sInflectionType || L"мо" == sInflectionType || L"ж" == sInflectionType
        || L"жо" == sInflectionType || L"мо-жо" == sInflectionType || L"с" == sInflectionType
        || L"со" == sInflectionType || L"мс-п" == sInflectionType)
    {
        if (0 == m_stProperties.llInflectionId)
        {
            sGraphicStem = sSource;
        }

        if (sSource.bEndsWithOneOf(L"аеёиоуыэюяйь"))
        {
            if (sSource.uiLength() < 2)
            {
                ERROR_LOG(L"Source form too short.");
                return H_ERROR_UNEXPECTED;
            }
            sGraphicStem = sSource;
            sGraphicStem.sRemoveCharsFromEnd(1);
        }
        else
        {
            sGraphicStem = sSource;
        }

        return H_NO_ERROR;
    }

    if (L"мс" == m_pLexeme->stGetProperties().sMainSymbol)
    {
        sGraphicStem = sSource;
        return H_NO_ERROR;
    }

    if (L"мн." == sInflectionType || L"мн. неод." == sInflectionType || L"мн. одуш." == sInflectionType
        || L"мн. от" == sInflectionType)
    {
        if (sSource.uiLength() < 2)
        {
            ERROR_LOG(L"Source form too short.");
            return H_ERROR_UNEXPECTED;
        }
        sGraphicStem = sSource;
        sGraphicStem.sRemoveCharsFromEnd(1);
        return H_NO_ERROR;
    }

    if (L"п" == m_pLexeme->stGetProperties().sInflectionType)
    {
        if (m_pLexeme->stGetProperties().bNoLongForms)
        {
            sGraphicStem = sSource;
        }

        if (sSource.bEndsWith(L"ся"))
        {
            if (sSource.uiLength() < 5)
            {
                ERROR_LOG(L"Source form too short.");
                return H_ERROR_UNEXPECTED;
            }
            sGraphicStem = sSource;
            sGraphicStem.sRemoveCharsFromEnd(4);
        }
        else
        {
            if (sSource.uiLength() < 3)
            {
                ERROR_LOG(L"Source form too short.");
                return H_ERROR_UNEXPECTED;
            }
            sGraphicStem = sSource;
            sGraphicStem.sRemoveCharsFromEnd(2);
        }
        return H_NO_ERROR;
    }

    if (L"св" == sInflectionType || L"нсв" == sInflectionType || L"св-нсв" == sInflectionType)
    {
        if (sSource.bEndsWith(L"ти") || sSource.bEndsWith(L"ть")
            || sSource.bEndsWith(L"чь"))
        {
            if (sSource.uiLength() < 3)
            {
                ERROR_LOG(L"Source form too short.");
                return H_ERROR_UNEXPECTED;
            }
            sGraphicStem = sSource;
            sGraphicStem.sRemoveCharsFromEnd(2);
        }
        else if (sSource.bEndsWith(L"тись") || sSource.bEndsWith(L"ться")
            || sSource.bEndsWith(L"чься"))
        {
            if (sSource.uiLength() < 5)
            {
                ERROR_LOG(L"Source form too short.");
                return H_ERROR_UNEXPECTED;
            }
            sGraphicStem = sSource;
            sGraphicStem.sRemoveCharsFromEnd(4);
            m_pLexeme->stGetPropertiesForWriteAccess().eReflexive = REFL_YES;
        }
        else
        {
            sGraphicStem = sSource;
            // Assume verbal wordforms with no infinitive form, like "поезжай"
        }
        return H_NO_ERROR;
    }

    m_pLexeme->stGetPropertiesForWriteAccess().sGraphicStem = sSource;

    return H_NO_ERROR;

}   //  eMakeGraphicStem(const CEString& sSource, CEString& sGraphicStem)

ET_ReturnCode CInflection::eGetAspectPair(CEString& sAspectPair, int& iStressPos)
{
    if (!m_pLexeme->bHasAspectPair())
    {
        return H_ERROR_UNEXPECTED;
    }

    sAspectPair = m_sAspectPair;
    iStressPos = m_iAspectPairStressPos;

    return H_NO_ERROR;
}

ET_ReturnCode CInflection::eGetAltAspectPair(CEString& sAltAspectPair, int& iStressPos)
{
    if (!m_pLexeme->bHasAltAspectPair())
    {
        return H_ERROR_UNEXPECTED;
    }

    sAltAspectPair = m_sAltAspectPair;
    iStressPos = m_iAltAspectPairStressPos;

    return H_NO_ERROR;
}


ET_ReturnCode CInflection::eMakeGraphicStem()
{
    ET_ReturnCode eRet = H_NO_ERROR;
    //    if (m_stProperties.bIsSecondPart)
    //    {
    //        if (m_stProperties.llInflectionId >= 0)
    //        {
    //            ERROR_LOG(L"No second graphic stem.");
    //            return H_ERROR_UNEXPECTED;
    //            eRet = eMakeGraphicStem(m_stProperties.sSourceForm, m_stProperties.sGraphicStem2);
    //        }
    //    }
    //    else
    {

        eRet = eMakeGraphicStem(const_cast<const CEString&>(m_pLexeme->stGetProperties().sSourceForm), 
                                const_cast<CEString&>(m_pLexeme->stGetProperties().sGraphicStem));
    }
    return eRet;

}       //  eMakeGraphicStem()

void CInflection::AddWordForm(shared_ptr<CWordForm> spWordForm)
{
    if (nullptr == spWordForm)
    {
        throw CException(H_ERROR_POINTER, L"Invalid wordform: NULL instance");
    }

    if (nullptr == spWordForm->m_spInflection)
    {
        throw CException(H_ERROR_POINTER, L"Invalid wordform: inflection instance is NULL");
    }

    m_pLexeme->AssignSecondaryStress(spWordForm);

    pair<CEString, shared_ptr<CWordForm>> pairHW(spWordForm->sGramHash(), spWordForm);
    m_mmWordForms.insert(pairHW);

}  //  AddWordForm (...)

void CInflection::AddModifiedForm(shared_ptr<CWordForm> spWordForm)
{
    if (spWordForm->bIrregular())
    {
        StIrregularForm stIf(spWordForm, spWordForm->bIsVariant());
        m_mmapIrregularForms.insert(pair<CEString, StIrregularForm>(spWordForm->sGramHash(), stIf));
    }
    else
    {
//        m_mmWordForms.insert(pair<CEString, CWordForm*>(pItfWordForm->sGramHash(), pCWf));
        AddWordForm(spWordForm);
    }
}

bool CInflection::bIsHypotheticalForm(CEString& sGramHash)
{
    if (m_vecHypotheticalForms.end() != find(m_vecHypotheticalForms.begin(), m_vecHypotheticalForms.end(), sGramHash))
    {
        return true;
    }
    else
    {
        return false;
    }
}

void CInflection::SetHypotheticalForm(CEString& sGramHash)
{
    m_vecHypotheticalForms.push_back(sGramHash);
}

uint64_t CInflection::uiTotalWordForms()
{
    return m_mmWordForms.size();
}

ET_ReturnCode CInflection::eFormExists(const CEString& sFH)
{
    //    if (m_mmWordForms.end() != m_mmWordForms.find(sFH))
    //    {
    //        return H_TRUE;
    //    }

    auto& sFormHash = const_cast<CEString&> (sFH);
    sFormHash.SetBreakChars(L"_");

    if (m_pLexeme->bHasMissingForms())
    {
        if (m_vecMissingForms.end() != find(m_vecMissingForms.begin(), m_vecMissingForms.end(), sFH))
        {
            return H_FALSE;
        }
        if (m_vecMissingForms.end() != find(m_vecMissingForms.begin(), m_vecMissingForms.end(), L"SingleForm"))
        {
            if (sFormHash.uiNFields() >= 3 && sFormHash.sGetField(1) == L"Sg" && sFormHash.sGetField(2) == L"N")
            {
                return H_TRUE;
            }
            return H_FALSE;
        }
    }

    if (L"Noun_Sg_Part" == sFH)
    {
        if (bSecondGenitive())
        {
            return H_TRUE;
        }
        else
        {
            return H_FALSE;
        }
    }

    if (L"Noun_Sg_P2" == sFH)
    {
        if (bSecondPrepositional())
        {
            return H_TRUE;
        }
        else
        {
            return H_FALSE;
        }
    }


    if (L"Noun_Sg_P2_Prepositions" == sFH)
    {
        if (m_stProperties.sP2Preposition.bIsEmpty())
        {
            return H_FALSE;
        }
        else
        {
            return H_TRUE;
        }
    }

    if (L"мн." == m_pLexeme->sMainSymbol() || m_pLexeme->bIsPluralOf())
    {
        if (sFH.bStartsWith(L"Noun_Sg_"))
        {
            return H_FALSE;
        }
    }

    if (L"AdjComp" == sFH && m_pLexeme->bNoComparative())
    {
        return H_FALSE;
    }

    if (sFH.bStartsWith(L"AdjL_") && m_pLexeme->bNoLongForms())
    {
        return H_FALSE;
    }

    if (m_pLexeme->bImpersonal() && L"Pres_Sg_3" != sFH && L"Past_N" != sFH && L"Inf" != sFH)
    {
        return H_FALSE;
    }

    if (m_pLexeme->bIterative())
    {
        if (sFH.bStartsWith(L"Pres") || sFH.bStartsWith(L"PPres") || sFH.bStartsWith(L"VAdv_Pres") ||
            sFH.bStartsWith(L"Impv"))
        {
            return H_FALSE;
        }
    }

    if (POS_VERB == m_pLexeme->ePartOfSpeech() && L"AdjComp" == sFH)
    {
        return H_FALSE;
    }

    //    auto sFormHash = const_cast<CEString&> (sFH);
    //    sFormHash.SetBreakChars(L"_");

    if (L"AdjS" == sFormHash.sGetField(0) && m_pLexeme->stGetProperties().bNoLongForms)
    {
        if (L"мс-п" == m_pLexeme->sMainSymbol())
        {
            if (m_pLexeme->stGetProperties().sComment.bStartsWith(L"формы — как кф ")
                || m_pLexeme->stGetProperties().sComment.bStartsWith(L"формы см. "))

                return H_TRUE;
        }
    }

    if (L"AdjS" == sFormHash.sGetField(0) || L"AdjComp" == sFormHash.sGetField(0))
    {
        if (L"мс-п" == m_pLexeme->sMainSymbol() || L"мс" == m_pLexeme->sInflectionType() ||
            L"числ.-п" == m_pLexeme->sInflectionType())
        {
            return H_FALSE;
        }

        if (m_pLexeme->sSourceForm().bEndsWith(L"ийся"))
        {
            return H_FALSE;
        }

        //        if (POS_VERB == m_stProperties.ePartOfSpeech) // detail view for participles
        //        {
        //            if (m_mmWordForms.find())
        //            return H_FALSE;
        //        }
    }

    if (L"Inf" == sFormHash && 17 == m_pLexeme->iSection() && m_pLexeme->stGetProperties().sMainSymbol == L"нсв")
    {
        return H_FALSE;
    }

    if (m_stProperties.bShortFormsIncomplete && L"AdjS_M" == sFormHash)
    {
        return H_FALSE;
    }

    if (m_stProperties.bNoPassivePastParticiple && sFormHash.uiNFields() > 0)
    {
        if (sFormHash.sGetField(0).bStartsWith(L"PPastP"))
        {
            return H_FALSE;
        }
    }

    if ((sFormHash.bStartsWith(L"PPresA") || L"VAdv_Pres" == sFormHash) && 
        L"св" == m_pLexeme->stGetProperties().sMainSymbol)
    {
        return H_FALSE;
    }

    if (L"VAdv_Pres" == sFormHash && 17 == m_pLexeme->iSection()) // $17 -- special case
    {
        return H_TRUE;
    }

    if (sFormHash.bStartsWith(L"VAdv_Pres"))
    {
        int aNoAdvTypes[] = { 3, 8, 9, 11, 14, 15 };
        auto size = aNoAdvTypes + sizeof(aNoAdvTypes) / sizeof(int);
        if (find(aNoAdvTypes, size, m_stProperties.iType) != size)
        {
            return H_FALSE;
        }
    }

    if (sFormHash.bStartsWith(L"PPresP") && !m_pLexeme->bHasPresPassParticiple())
    {
        return H_FALSE;
    }

    if (sFormHash.bStartsWith(L"PPastP") && (!m_pLexeme->bTransitive() || REFL_YES == m_pLexeme->eIsReflexive()))
    {
        return H_FALSE;
    }

    if (sFormHash.bStartsWith(L"PPastP") && L"нсв" == m_pLexeme->sMainSymbol() && m_pLexeme->bHasAspectPair())
    {
        return H_FALSE;
    }

    if (!m_pLexeme->stGetProperties().bHasMissingForms)
    {
        return H_TRUE;
    }

    if (sFH.uiLength() < 1)
    {
        ERROR_LOG(L"Empty form hash.");
        return H_ERROR_UNEXPECTED;
    }

    bool bNegate = false;

    for (auto& sMissing : m_vecMissingForms)
    {
        if (L'!' == sMissing[0])
        {
            bNegate = true;
            sMissing = sMissing.sSubstr(1);
        }

        sMissing.SetBreakChars(L"_");
        bool bMatch = true;
        if (sMissing.uiNFields() != sFormHash.uiNFields())
        {
            bMatch = false;
        }
        else
        {
            for (int iAt = 0; iAt < (int)sFormHash.uiNFields(); ++iAt)
            {
                auto&& sFormHashFragment = (sFormHash.uiNFields() == 0) ? sFormHash : sFormHash.sGetField(iAt);
                auto&& sMissingHashFragment = (sMissing.uiNFields() == 0) ? sMissing : sMissing.sGetField(iAt);

                if (sMissingHashFragment == L"*")
                {
                    continue;
                }

                if (sMissingHashFragment != sFormHashFragment)
                {
                    bMatch = false;
                    break;
                }
            }
        }

        if (bMatch)
        {
            return bNegate ? H_TRUE : H_FALSE;
        }
    }

    return bNegate ? H_FALSE : H_TRUE;

}   //  eFormExists (...)

ET_ReturnCode CInflection::eSetFormExists(const CEString& sGramHash, bool bExists)
{
    if (L"AdjComp" == sGramHash)
    {
        m_pLexeme->stGetPropertiesForWriteAccess().bNoComparative = bExists;
        return H_NO_ERROR;
    }

    if (bExists)
    {
        auto itFound = find(m_vecMissingForms.begin(), m_vecMissingForms.end(), sGramHash);
        if (itFound == m_vecMissingForms.end())
        {
            CEString sMsg(L"Hash " + sGramHash + L" not found.");
            ERROR_LOG(sMsg);
            return H_ERROR_UNEXPECTED;
        }

        m_vecMissingForms.erase(itFound);
    }
    else
    {
        m_vecMissingForms.push_back(sGramHash);
    }

    return H_NO_ERROR;
}

ET_ReturnCode CInflection::eLoadMissingForms()
{
    if (!m_pLexeme->stGetProperties().bHasMissingForms)
    {
        return H_FALSE;
    }

    m_vecMissingForms.clear();

    CEString sQuery(L"SELECT gram_hash FROM missing_forms WHERE inflection_id = ");
//    sQuery += CEString::sToString(m_pLexeme->stGetProperties().llDescriptorId);
    sQuery += CEString::sToString(m_stProperties.llInflectionId);
    sQuery += L";";

    shared_ptr<CSqlite> spDb;

    try
    {
//        spDb = m_pDictionary->pGetDbHandle();
        spDb = m_pLexeme->spGetDb();
        spDb->PrepareForSelect(sQuery);
        while (spDb->bGetRow())
        {
            CEString sLabel;
            spDb->GetData(0, sLabel);
            m_vecMissingForms.push_back(sLabel);
            if (L"AdjL_*_*_*" == sLabel)
            {
                m_pLexeme->stGetPropertiesForWriteAccess().bNoLongForms = true;
            }
        }
        spDb->Finalize();
    }
    catch (...)
    {
        CEString sMsg;
        CEString sError;
        try
        {
            spDb->GetLastError(sError);
            sMsg += CEString(L", error %d: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(spDb->iGetLastError());
        ERROR_LOG(sMsg);
    }

    return H_NO_ERROR;

}   //  eLoadMissingForms()

ET_ReturnCode CInflection::eLoadIrregularForms()
{
    ET_ReturnCode rc = H_NO_ERROR;

    if (!(m_pLexeme->stGetProperties().bHasIrregularForms || m_pLexeme->stGetProperties().ePartOfSpeech == POS_NUM))
    {
        return H_FALSE;
    }

    m_pLexeme->stGetPropertiesForWriteAccess().bHasIrregularVariants = false;

    // NB: irregular forms are a property of the lexeme, i.e. descriptor
    CEString sQuery
        (L"SELECT id, gram_hash, wordform, is_alternative, lead_comment, trailing_comment, is_edited FROM irregular_forms WHERE inflection_id = ");
    sQuery += CEString::sToString(m_stProperties.llInflectionId);
    sQuery += L";";

    shared_ptr<CSqlite> spDb;

    m_mmapIrregularForms.clear();

    try
    {
        spDb = m_pLexeme->spGetDb();
        uint64_t uiQueryHandle = spDb->uiPrepareForSelect(sQuery);
        while (spDb->bGetRow(uiQueryHandle))
        {
            //StIrregularForm stForm;
            int iId = -1;
            CEString sHash;
            CEString sForm;
            bool bIsVariant = false;
            CEString sLeadComment;
            CEString sTrailingComment;
            bool bIsEdited;

            spDb->GetData(0, iId, uiQueryHandle);
            spDb->GetData(1, sHash, uiQueryHandle);
            spDb->GetData(2, sForm, uiQueryHandle);
            spDb->GetData(3, bIsVariant, uiQueryHandle);
            spDb->GetData(4, sLeadComment, uiQueryHandle);
            spDb->GetData(5, sTrailingComment, uiQueryHandle);
            spDb->GetData(6, bIsEdited, uiQueryHandle);

            if (bIsVariant)
            {
                m_pLexeme->SetHasIrregularVariants(true);
            }

            auto spWf = make_shared<CWordForm>(sHash, this);
            spWf->m_spInflection = this;
            spWf->m_bIrregular = true;
            spWf->m_llDbKey = iId;
            spWf->m_sWordForm = sForm;
            spWf->m_llInflectionId = this->llInflectionId();
            spWf->m_bIsVariant = bIsVariant;
            spWf->m_sLeadComment = sLeadComment;
            spWf->m_sTrailingComment = sTrailingComment;

            CEString sStressQuery(L"SELECT position, is_primary FROM irregular_stress WHERE form_id = ");
            sStressQuery += CEString::sToString(iId);
            sStressQuery += L";";

            sForm.SetVowels(CEString::g_szRusVowels);

            uint64_t uiStressHandle = spDb->uiPrepareForSelect(sStressQuery);
            while (spDb->bGetRow(uiStressHandle))
            {
                int iPos = -1;
                bool bPrimary = false;
                spDb->GetData(0, iPos, uiStressHandle);
                spDb->GetData(1, bPrimary, uiStressHandle);
                int iStressedSyll = sForm.uiGetSyllableFromVowelPos(iPos);
                spWf->m_mapStress[iStressedSyll] = bPrimary ? STRESS_PRIMARY : STRESS_SECONDARY;
            }
            spDb->Finalize(uiStressHandle);

            StIrregularForm stIf(spWf, bIsVariant);
            pair<CEString, StIrregularForm> pairHashToWordForm(sHash, stIf);
            m_mmapIrregularForms.insert(pairHashToWordForm);

        }   //  while (pDb->b_GetRow())

        spDb->Finalize(uiQueryHandle);
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        rc = H_EXCEPTION;
    }
    catch (...)
    {
        CEString sMsg;
        CEString sError;
        try
        {
            spDb->GetLastError(sError);
            sMsg += CEString(L", error: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(spDb->iGetLastError());
        ERROR_LOG(sMsg);
        rc = H_EXCEPTION;
    }

    if (m_pLexeme->stGetProperties().bSpryazhSm)
    {
        auto& sPrefix = m_pLexeme->stGetProperties().sSpryazhSmPrefix;
        auto& iPrefixLength = m_pLexeme->stGetProperties().iSpryazhSmRefPrefixLength;

        for (auto& pairIf : m_mmapIrregularForms)
        {
            auto&& sRefForm = pairIf.second.spWordForm->sWordForm();
            sRefForm.SetVowels(CEString::g_szRusVowels);
            auto sNewForm = sPrefix + sRefForm.sSubstr(iPrefixLength);
            sNewForm.SetVowels(CEString::g_szRusVowels);
            int iDiff = (int)sNewForm.uiNSyllables() - (int)sRefForm.uiNSyllables();
            pairIf.second.spWordForm->SetWordForm(sNewForm);
            map<int, ET_StressType> mapModified;
            for (auto& [iPos, eType] : pairIf.second.spWordForm->m_mapStress)
            {
                mapModified.emplace(iPos + iDiff, eType);
            }
            pairIf.second.spWordForm->m_mapStress = mapModified;
        }
    }

    return rc;

}   //  eLoadIrregularForms()

ET_ReturnCode CInflection::eLoadDifficultForms()
{
    if (!m_pLexeme->stGetProperties().bHasDifficultForms)
    {
        return H_FALSE;
    }

    m_vecDifficultForms.clear();

    CEString sQuery(L"SELECT gram_hash FROM difficult_forms WHERE inflection_id = ");
    sQuery += CEString::sToString(m_stProperties.llInflectionId);
    sQuery += L";";

    shared_ptr<CSqlite> spDb;

    try
    {
        spDb = m_pLexeme->spGetDb();
        spDb->PrepareForSelect(sQuery);
        while (spDb->bGetRow())
        {
            CEString sLabel;
            spDb->GetData(0, sLabel);
            m_vecDifficultForms.push_back(sLabel);
        }
        spDb->Finalize();
    }
    catch (...)
    {
        CEString sMsg;
        CEString sError;
        try
        {
            spDb->GetLastError(sError);
            sMsg += CEString(L", error %d: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(spDb->iGetLastError());
        ERROR_LOG(sMsg);
    }

    return H_NO_ERROR;

}       //  eLoadDifficultForms()

ET_ReturnCode CInflection::eGenerateParadigm()
{
    ET_ReturnCode rc = H_NO_ERROR;

    if (m_bFormsGenerated)
    {
        return rc;
    }

    try
    {
        if (m_pLexeme->stGetProperties().bHasIrregularForms || 
            POS_NUM == m_pLexeme->stGetProperties().ePartOfSpeech)
        {
            rc = eLoadIrregularForms();
            if (rc != H_NO_ERROR && rc != H_FALSE && rc != H_NO_MORE)
            {
//                return rc;
            }
        }

        if (m_pLexeme->stGetProperties().bHasMissingForms)
        {
            rc = eLoadMissingForms();
            if (rc != H_NO_ERROR && rc != H_FALSE && rc != H_NO_MORE)
            {
                return rc;
            }
        }

        if (m_pLexeme->stGetProperties().bHasDifficultForms)
        {
            rc = eLoadDifficultForms();
            if (rc != H_NO_ERROR && rc != H_FALSE && rc != H_NO_MORE)
            {
                return rc;
            }
        }

        m_mmWordForms.clear();

        auto& stLexemeProperties = m_pLexeme->stGetPropertiesForWriteAccess();

        if (L"мо" == stLexemeProperties.sInflectionType || L"м" == stLexemeProperties.sInflectionType || L"жо" == stLexemeProperties.sInflectionType
            || L"ж" == stLexemeProperties.sInflectionType || L"со" == stLexemeProperties.sInflectionType || L"с" == stLexemeProperties.sInflectionType
            || L"мо-жо" == stLexemeProperties.sInflectionType || L"мн." == stLexemeProperties.sInflectionType || L"мн. неод." == stLexemeProperties.sInflectionType
            || L"мн. одуш." == stLexemeProperties.sInflectionType || L"мн. от" == stLexemeProperties.sInflectionType)
        {
            CFormBuilderNouns bn(m_pLexeme, this);
            rc = bn.eBuild();
            auto spLexeme2 = m_pLexeme->spGetSecondPart();
            if (spLexeme2)
            {
                CInflectionEnumerator ie2(spLexeme2);
                shared_ptr<CInflection> spInflection2;
                rc = ie2.eGetFirstInflection(spInflection2);
                if (rc != H_NO_ERROR || nullptr == spInflection2)
                {
                    CEString sMsg(L"Unable to read inflection data for ");
                    sMsg += spLexeme2->sSourceForm();
                    ERROR_LOG(sMsg);
                    return H_ERROR_UNEXPECTED;
                }

                rc = spInflection2->eGenerateParadigm();
                rc = eAlignInflectedParts();
            }
        }
            
        if (L"п" == stLexemeProperties.sInflectionType)
        {
            if (!stLexemeProperties.bNoLongForms)
            {
                CFormBuilderLongAdj lfb(m_pLexeme, this, stLexemeProperties.sGraphicStem, m_stProperties.eAccentType1, SUBPARADIGM_LONG_ADJ);
                rc = lfb.eBuild();
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }
                auto spLexeme2 = m_pLexeme->spGetSecondPart();
                if (spLexeme2)
                {
                    CInflectionEnumerator ie2(spLexeme2);
                    shared_ptr<CInflection> spInflection2;
                    rc = ie2.eGetFirstInflection(spInflection2);
                    if (rc != H_NO_ERROR || nullptr == spInflection2)
                    {
                        CEString sMsg(L"Unable to read inflection data for ");
                        sMsg += spLexeme2->sSourceForm();
                        ERROR_LOG(sMsg);
                        return H_ERROR_UNEXPECTED;
                    }

                    rc = spInflection2->eGenerateParadigm();
//                    rc = eAlignInflectedParts();
// TODO: check and warn if there's more than one second part infl-n
                }
            }

            if (L"п" == stLexemeProperties.sMainSymbol)
            {
                CFormBuilderShortAdj sfb(m_pLexeme, this);
                rc = sfb.eBuild();
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }

                auto spLexeme2 = m_pLexeme->spGetSecondPart();
                if (spLexeme2)
                {
                    CInflectionEnumerator ie(spLexeme2);
                    shared_ptr<CInflection> spInflection2;
                    rc = ie.eGetFirstInflection(spInflection2);
                    if (rc != H_NO_ERROR || nullptr == spInflection2)
                    {
                        CEString sMsg(L"Unable to read inflection data for ");
                        sMsg += spLexeme2->sSourceForm();
                        ERROR_LOG(sMsg);
                        return H_ERROR_UNEXPECTED;
                    }

//                    rc = spInflection2->eGenerateParadigm();
                }
  
                if (!stLexemeProperties.bNoComparative && !stLexemeProperties.sSourceForm.bEndsWith(L"ийся"))
                {
                    CFormBuilderComparative cfb(m_pLexeme, this);
                    rc = cfb.eBuild();
                }
            }

            if (stLexemeProperties.bHasIrregularForms && stLexemeProperties.bNoLongForms)
            {
                // Isolated case: каков & таков, мс-п with short forms only
                if (stLexemeProperties.sComment.bStartsWith(L"формы — как кф ") 
                    || stLexemeProperties.sComment.bStartsWith(L"формы см. "))
                {
                    CFormBuilderShortAdj sfb(m_pLexeme, this);
                    rc = sfb.eBuild();
                    if (rc != H_NO_ERROR)
                    {
                        return rc;
                    }
                }
            }

            if (m_pLexeme->spGetSecondPart())
            {
                rc = eAlignInflectedParts();
            }
        }

        if (L"мс-п" == stLexemeProperties.sInflectionType || L"числ.-п" == stLexemeProperties.sInflectionType ||
           (L"мс" == stLexemeProperties.sInflectionType && (L"п" == stLexemeProperties.sMainSymbol ||
            L"числ.-п" == stLexemeProperties.sMainSymbol || POS_NOUN == stLexemeProperties.ePartOfSpeech)))
        {
            CFormBuilderPronounAdj pab (m_pLexeme, this, stLexemeProperties.sGraphicStem, m_stProperties.eAccentType1);
            rc = pab.eBuild();
        }
            
        if (stLexemeProperties.ePartOfSpeech == POS_VERB)
        {
            stLexemeProperties.eAspect = ASPECT_UNDEFINED;
            if (L"нсв" == stLexemeProperties.sInflectionType)
            {
                stLexemeProperties.eAspect = ASPECT_IMPERFECTIVE;
            }
            else if (L"св" == stLexemeProperties.sInflectionType)
            {
                stLexemeProperties.eAspect = ASPECT_PERFECTIVE;
            }

            CFormBuilderPersonal personal(m_pLexeme, this);
            rc = personal.eBuild();
            //                if (rc != H_NO_ERROR)
            //                {
            //                    return rc;
            //                }

            CFormBuilderPast past (m_pLexeme, this);
            rc = past.eBuild();
//                if (rc != H_NO_ERROR)
//                {
//                    return rc;
//                }

            CFormBuilderImperative imperative (m_pLexeme, this);
            rc = imperative.eBuild();
//                if (rc != H_NO_ERROR)
//                {
//                    return rc;
//                }

            CFormBuilderNonFinite nonFinite (m_pLexeme, this);
            rc = nonFinite.eBuild();

            if (m_pLexeme->bHasAspectPair())
            {
                CFormBuilderAspectPair aspectPair(m_pLexeme, this);
                rc = aspectPair.eBuild();
                if (H_NO_ERROR == rc)
                {
                    rc = aspectPair.eGetAspectPair(m_sAspectPair, m_iAspectPairStressPos);
                    if (H_NO_ERROR == rc && m_pLexeme->bHasAltAspectPair())
                    {
                        rc = aspectPair.eGetAltAspectPair(m_sAltAspectPair, m_iAltAspectPairStressPos);
                    }
                }
            }
        }
            
        if (POS_PRONOUN == stLexemeProperties.ePartOfSpeech || POS_NUM == stLexemeProperties.ePartOfSpeech 
            || POS_PRONOUN_PREDIC == stLexemeProperties.ePartOfSpeech)
        {
            shared_ptr<CWordForm> spWordForm;
            bool bOptional = false;
            rc = eGetFirstIrregularForm(spWordForm, bOptional);
            while (H_NO_ERROR == rc)
            {
                stLexemeProperties.eSubparadigm = spWordForm->eSubparadigm();
                AddWordForm(spWordForm);
                rc = eGetNextIrregularForm(spWordForm, bOptional);
            }
        }

        if (1 == stLexemeProperties.iSection || 2 == stLexemeProperties.iSection)
        {
            stLexemeProperties.ePartOfSpeech = POS_NULL;
        }

        if (POS_ADV == stLexemeProperties.ePartOfSpeech
            || POS_COMPAR == stLexemeProperties.ePartOfSpeech || POS_PREDIC == stLexemeProperties.ePartOfSpeech
            || POS_PREPOSITION == stLexemeProperties.ePartOfSpeech || POS_CONJUNCTION == stLexemeProperties.ePartOfSpeech
            || POS_PARTICLE == stLexemeProperties.ePartOfSpeech || POS_INTERJ == stLexemeProperties.ePartOfSpeech
            || POS_PARENTH == stLexemeProperties.ePartOfSpeech || POS_NULL == stLexemeProperties.ePartOfSpeech)
        {
            shared_ptr<CWordForm> spWordForm = make_shared<CWordForm>();
            spWordForm->m_spInflection = this;
            spWordForm->m_ePos = stLexemeProperties.ePartOfSpeech;
            spWordForm->m_sStem = stLexemeProperties.sGraphicStem;
            spWordForm->m_sWordForm = stLexemeProperties.sGraphicStem;
            spWordForm->m_llLexemeId = m_pLexeme->llLexemeId();
            for (vector<int>::iterator itStress = stLexemeProperties.vecSourceStressPos.begin();
                 itStress != stLexemeProperties.vecSourceStressPos.end(); ++itStress)
            {
                spWordForm->m_mapStress[*itStress] = STRESS_PRIMARY;
            }

            pair<CEString, shared_ptr<CWordForm>> pairHashToWf(spWordForm->sGramHash(), spWordForm);
            m_mmWordForms.insert(pairHashToWf);
        }
    }
    catch (CException& ex)
    {
        CEString sMsg(ex.szGetDescription());
        sMsg += L" lexeme = ";
        sMsg += m_pLexeme->sSourceForm();
        return H_EXCEPTION;
    }
    catch (...)
    {
        ERROR_LOG(L"Unknown exception.");
        return H_EXCEPTION;
    }

    for (auto& sHash : m_vecHypotheticalForms)
    {
        auto iRemoved = m_mmWordForms.erase(sHash);
        if (iRemoved < 1)
        {
            CEString sMsg(L"Unable to remove hypothetical form with gram. hash ");
            sMsg += sHash;
            ERROR_LOG(sMsg);
        }
    }

    m_bFormsGenerated = true;

    return H_NO_ERROR;

}   // eGenerateParadigm()

bool CInflection::bNoRegularForms(CEString sGramHash)
{
    shared_ptr<CWordForm> spWf;         // currently not used
    bool bIsVariant = false;
    ET_ReturnCode rc = eGetFirstIrregularForm(sGramHash, spWf, bIsVariant);
    if (H_FALSE == rc)
    {
        return false;
    }
    if (rc != H_NO_ERROR && rc != H_FALSE)
    {
        assert(0);
        CEString sMsg(L"hGetFirstIrregularForm() failed, error ");
        sMsg += CEString::sToString(rc);
        ERROR_LOG(sMsg);
        throw CException(H_EXCEPTION, sMsg);
    }

    if (bIsVariant)    // irregular form is optional i.e. we still need to produce regular one  
    {
        return false;
    }

    while (H_NO_ERROR == rc)
    {
        bIsVariant = false;
        rc = eGetNextIrregularForm(spWf, bIsVariant);
        if (rc != H_NO_ERROR && rc != H_NO_MORE)
        {
            assert(0);
            CEString sMsg(L"hGetNextIrregularForm() failed, error ");
            sMsg += CEString::sToString(rc);
            ERROR_LOG(sMsg);
            throw CException(H_EXCEPTION, sMsg);
        }
        if (bIsVariant)
        {
            return false;
        }
    }

    return true;

}   //  bNoRegularForms()

bool CInflection::bHasIrregularForm(CEString sGramHash)
{
    ET_ReturnCode rc = H_NO_ERROR;

    shared_ptr<CWordForm> spWf;
    bool bIsOptional = false;       // ignored
    rc = eGetFirstIrregularForm(sGramHash, spWf, bIsOptional);
    if ((rc != H_NO_ERROR && rc != H_FALSE) || (H_NO_ERROR == rc && nullptr == spWf))
    {
        assert(0);
        CEString sMsg(L"Irregular form not found in database.");
        ERROR_LOG(sMsg);
        throw CException(rc, sMsg);
    }

    return (H_NO_ERROR == rc) ? true : false;
}

ET_ReturnCode CInflection::eGetIrregularForms(CEString sHash, map<CWordForm*, bool>& mapResult)
{
    ET_ReturnCode rc = H_NO_ERROR;

    shared_ptr<CWordForm> spWf;
    bool bIsOptional = false;
    rc = eGetFirstIrregularForm(sHash, spWf, bIsOptional);
    if (rc != H_NO_ERROR && rc != H_FALSE && rc != H_NO_MORE)
    {
        assert(0);
        ERROR_LOG(L"Irregular form not found in database.");
        return H_ERROR_UNEXPECTED;
    }

    // No irregular form for this hash?
    if (H_FALSE == rc)
    {
        return H_NO_ERROR;  // expected
    }

    // Fill in data for the first irregular form
    mapResult[spWf.get()] = bIsOptional;

    // Continue for other irregular forms for this hash 
    // (which, in all likelihood do not exist)
    do
    {
        rc = eGetNextIrregularForm(spWf, bIsOptional);
        if (rc != H_NO_ERROR && rc != H_NO_MORE)
        {
            assert(0);
            ERROR_LOG(L"Error accessing irregular m Sg Past form.");
            return H_ERROR_UNEXPECTED;
        }

        if (H_NO_ERROR == rc)
        {
            map<CWordForm*, bool>::iterator it = mapResult.find(spWf.get());
            if (it != mapResult.end())
            {
                assert(0);
                ERROR_LOG(L"Duplicate irregular form.");
                return H_ERROR_UNEXPECTED;
            }

            mapResult[spWf.get()] = bIsOptional;
        }

    } while (H_NO_ERROR == rc);

    return H_NO_ERROR;

}   //  eGetIrregularForms()

ET_ReturnCode CInflection::eGetWordForm(unsigned int uiAt, shared_ptr<CWordForm>& spWordForm)
{
    if (uiAt >= m_mmWordForms.size())
    {
        return H_ERROR_INVALID_ARG;
    }

    auto it = m_mmWordForms.begin();
    if (m_mmWordForms.end() == it)
    {
        return H_ERROR_UNEXPECTED;
    }

    for (unsigned int uiWf = 0; uiWf < uiAt; ++uiWf)
    {
        ++it;
        if (m_mmWordForms.end() == it)
        {
            return H_ERROR_UNEXPECTED;
        }
    }

    spWordForm = (*it).second;

    return H_NO_ERROR;

}       //  eGetWordForm()

ET_ReturnCode CInflection::eGetFirstWordForm(shared_ptr<CWordForm>& spWf)
{
    m_itCurrentWordForm = m_mmWordForms.begin();
    if (m_mmWordForms.end() == m_itCurrentWordForm)
    {
//        pWf = NULL;
        return H_FALSE;
    }
    spWf = m_itCurrentWordForm->second;

    return H_NO_ERROR;
}

ET_ReturnCode CInflection::eGetNextWordForm(shared_ptr<CWordForm>& spWf)
{
    if (m_mmWordForms.end() == m_itCurrentWordForm || m_mmWordForms.end() == ++m_itCurrentWordForm)
    {
        spWf = NULL;
        return H_NO_MORE;
    }

    spWf = m_itCurrentWordForm->second;

    return H_NO_ERROR;
}

/*
ET_ReturnCode CInflection::eGetFirstWordForm(CWordForm*& pWf)
{
    m_itCurrentWordForm = m_mmWordForms.begin();
    if (m_mmWordForms.end() == m_itCurrentWordForm)
    {
        pWf = NULL;
        return H_FALSE;
    }
    pWf = m_itCurrentWordForm->second.get();

    return H_NO_ERROR;
}

ET_ReturnCode CInflection::eGetNextWordForm(CWordForm*& pWf)
{
    if (m_mmWordForms.end() != m_itCurrentWordForm)
    {
        ++m_itCurrentWordForm;
    }
    if (m_mmWordForms.end() == m_itCurrentWordForm)
    {
        pWf = NULL;
        return H_NO_MORE;
    }

    pWf = m_itCurrentWordForm->second.get();

    return H_NO_ERROR;
}
*/

ET_ReturnCode CInflection::eGetFirstIrregularForm(CEString sHash, shared_ptr<CWordForm>& spWordForm, bool& bIsOptional)
{
    m_pairItIfRange = m_mmapIrregularForms.equal_range(sHash);
    if (m_pairItIfRange.first == m_pairItIfRange.second)
    {
        return H_FALSE;
    }
    else
    {
        m_itCurrentIrregularForm = m_pairItIfRange.first;
        if ((*m_itCurrentIrregularForm).first != sHash)
        {
            return H_FALSE;
        }
        else
        {
            spWordForm = (*m_itCurrentIrregularForm).second.spWordForm;
            bIsOptional = (*m_itCurrentIrregularForm).second.bIsOptional;
            return H_NO_ERROR;
        }
    }
}

ET_ReturnCode CInflection::eGetFirstIrregularForm(CEString sHash, CWordForm*& pWordForm, bool& bIsOptional)
{
    m_pairItIfRange = m_mmapIrregularForms.equal_range(sHash);
    if (m_pairItIfRange.first == m_pairItIfRange.second)
    {
        return H_FALSE;
    }
    else
    {
        m_itCurrentIrregularForm = m_pairItIfRange.first;
        if ((*m_itCurrentIrregularForm).first != sHash)
        {
            return H_FALSE;
        }
        else
        {
            pWordForm = (*m_itCurrentIrregularForm).second.spWordForm.get();
            bIsOptional = (*m_itCurrentIrregularForm).second.bIsOptional;
            return H_NO_ERROR;
        }
    }
}

ET_ReturnCode CInflection::eGetFirstIrregularForm(shared_ptr<CWordForm>& spWordForm, bool& bIsOptional)
{
    m_pairItIfRange.first = m_mmapIrregularForms.begin();
    m_pairItIfRange.second = m_mmapIrregularForms.end();
    if (m_pairItIfRange.first == m_pairItIfRange.second)
    {
        return H_FALSE;
    }
    else
    {
        m_itCurrentIrregularForm = m_pairItIfRange.first;
        spWordForm = (*m_itCurrentIrregularForm).second.spWordForm;
        bIsOptional = (*m_itCurrentIrregularForm).second.bIsOptional;
        return H_NO_ERROR;
    }
}

ET_ReturnCode CInflection::eGetNextIrregularForm(shared_ptr<CWordForm>& spWordForm, bool& bIsOptional)
{
    if (m_pairItIfRange.first == m_pairItIfRange.second)
    {
        return H_ERROR_UNEXPECTED;
    }

    //    if (iHash != (*pairItIfRange.first).first)
    //    {
    //        return H_ERROR_UNEXPECTED;
    //    }

    if (m_itCurrentIrregularForm == m_mmapIrregularForms.end())
        //        || (*m_itCurrentIrregularForm).first != iHash))
    {
        return H_ERROR_UNEXPECTED;
    }

    ++m_itCurrentIrregularForm;

    if (m_itCurrentIrregularForm == m_mmapIrregularForms.end() || m_itCurrentIrregularForm == m_pairItIfRange.second)
        //        || (*m_itCurrentIrregularForm).first != iHash))
    {
        return H_NO_MORE;
    }

    //    if ((*m_itCurrentIrregularForm).first != (*m_itCurrentIrregularForm).second.pWordForm->sGramHash())
    //    {
    //        ERROR_LOG(L"Irregular form: gramm hash does not match.");
    //        return H_ERROR_UNEXPECTED;
    //    }

    spWordForm = (*m_itCurrentIrregularForm).second.spWordForm;
    bIsOptional = (*m_itCurrentIrregularForm).second.bIsOptional;

    return H_NO_ERROR;

}   //  eGetNextIrregularForm (...)

ET_ReturnCode CInflection::eGetNextIrregularForm(CWordForm*& pWordForm, bool& bIsOptional)
{
    if (m_pairItIfRange.first == m_pairItIfRange.second)
    {
        return H_ERROR_UNEXPECTED;
    }

    //    if (iHash != (*pairItIfRange.first).first)
    //    {
    //        return H_ERROR_UNEXPECTED;
    //    }

    if (m_itCurrentIrregularForm == m_mmapIrregularForms.end())
        //        || (*m_itCurrentIrregularForm).first != iHash))
    {
        return H_ERROR_UNEXPECTED;
    }

    ++m_itCurrentIrregularForm;

    if (m_itCurrentIrregularForm == m_mmapIrregularForms.end() || m_itCurrentIrregularForm == m_pairItIfRange.second)
        //        || (*m_itCurrentIrregularForm).first != iHash))
    {
        return H_NO_MORE;
    }

    //    if ((*m_itCurrentIrregularForm).first != (*m_itCurrentIrregularForm).second.pWordForm->sGramHash())
    //    {
    //        ERROR_LOG(L"Irregular form: gramm hash does not match.");
    //        return H_ERROR_UNEXPECTED;
    //    }

    pWordForm = (*m_itCurrentIrregularForm).second.spWordForm.get();
    bIsOptional = (*m_itCurrentIrregularForm).second.bIsOptional;

    return H_NO_ERROR;

}   //  eGetNextIrregularForm (...)

ET_ReturnCode CInflection::eIsFormDifficult(const CEString& sFH)
{
    auto& sFormHash = const_cast<CEString&> (sFH);
    sFormHash.SetBreakChars(L"_");

    if ((m_stProperties.bShortFormsRestricted && sFormHash.uiNFields() > 0) ||      // pometa "x"
        7 == m_pLexeme->stGetProperties().iSection)
    {
        if (sFormHash.sGetField(0).bStartsWith(L"AdjS"))
        {
            return H_TRUE;
        }
        if (sFormHash.sGetField(0).bStartsWith(L"PPastP"))
        {
            return H_TRUE;
        }
    }

    if (sFormHash.bStartsWith(L"AdjS") && L"AdjS_M" != sFormHash)
    {
        if (m_stProperties.bShortFormsIncomplete)     // boxed "x"
        {
            return H_TRUE;
        }
    }

    if (sFormHash.bStartsWith(L"PPastP"))
    {
        if (m_stProperties.bPastParticipleRestricted)
        {
            return H_TRUE;
        }
    }

    if (!m_pLexeme->stGetProperties().bHasDifficultForms)
    {
        return H_FALSE;
    }

    if (sFH.uiLength() < 1)
    {
        ERROR_LOG(L"Empty form hash.");
        return H_ERROR_UNEXPECTED;
    }

    bool bNegate = false;

    sFormHash.SetBreakChars(L"_");

    bool bMatch = false;
    for (auto sDifficult : m_vecDifficultForms)
    {
        if (L'!' == sDifficult[0])
        {
            bNegate = true;
            sDifficult = sDifficult.sSubstr(1);
        }

        sDifficult.SetBreakChars(L"_");
        if (sDifficult.uiNFields() != sFormHash.uiNFields())
        {
//            return bNegate ? H_TRUE : H_FALSE;
            continue;
        }

        bMatch = true;
        for (int iAt = 0; iAt < (int)sFormHash.uiNFields(); ++iAt)
        {
            auto&& sFormHashFragment = (sFormHash.uiNFields() == 0) ? sFormHash : sFormHash.sGetField(iAt);
            auto&& sDifficultHashFragment = (sDifficult.uiNFields() == 0) ? sDifficult : sDifficult.sGetField(iAt);

            if (sDifficultHashFragment == L"*")
            {
                continue;
            }

            if (sDifficultHashFragment != sFormHashFragment)
            {
                bMatch = false;
                break;
            }
        }

        if (bMatch)
        {
            break;
        }
    }

    if (bMatch)
    {
        return bNegate ? H_FALSE : H_TRUE;
    }
    else
    {
        return bNegate ? H_TRUE : H_FALSE;
    }

}   //  eIsFormDifficult()

bool CInflection::bIsFormDifficult(const CEString& sGramHash)           // same logic, different return type
{
    return eIsFormDifficult(sGramHash) == H_TRUE;
}


ET_ReturnCode CInflection::eSetFormDifficult(const CEString& sGramHash, bool bIsDifficult)
{
    if (bIsDifficult)
    {
        m_vecDifficultForms.push_back(sGramHash);
    }
    else
    {
        auto itFound = find(m_vecDifficultForms.begin(), m_vecDifficultForms.end(), sGramHash);
        if (itFound == m_vecDifficultForms.end())
        {
            CEString sMsg(L"Hash " + sGramHash + L" not found.");
            ERROR_LOG(sMsg);
            return H_ERROR_UNEXPECTED;
        }
        m_vecDifficultForms.erase(itFound);
    }

    return H_NO_ERROR;
}

ET_ReturnCode CInflection::eDifficultFormsHashes(vector<CEString>& vecHashes)
{
    vecHashes = m_vecDifficultForms;
    return H_NO_ERROR;
}

ET_ReturnCode CInflection::eIsFormAssumed(const CEString& sFH)
{
    auto& sFormHash = const_cast<CEString&> (sFH);
    sFormHash.SetBreakChars(L"_");

    if (m_pLexeme->stGetProperties().bAssumedForms)
    {
        if (L"Noun" == sFormHash.sGetField(0) && L"Pl" == sFormHash.sGetField(1))
        {
            return H_TRUE;
        }

        if (L"AdjS_M" == sFormHash)
        {
            return H_TRUE;
        }
    }

    return H_FALSE;

}   //  eIsFormAssumed()

ET_ReturnCode CInflection::eSetHasAssumedForms(bool bIsAssumed)
{
    m_pLexeme->stGetPropertiesForWriteAccess().bAssumedForms = bIsAssumed;
    return H_NO_ERROR;
}

//
// GDRL p. 11: a dash in a word with two main stress marks means a compound form
// with two main stresses; otherwise 
//
bool CInflection::bIsMultistressedCompound()
{
    if (m_pLexeme->stGetProperties().vecSourceStressPos.size() < 2)
    {
        return false;
    }

    unsigned int uiDashPos = m_pLexeme->stGetProperties().sGraphicStem.uiFind(L"-");
    if (ecNotFound == uiDashPos || uiDashPos < 1)
    {
        return false;
    }

    // find any two that are separated by dash
    try {
        auto itLeft = m_pLexeme->stGetProperties().vecSourceStressPos.begin();
        unsigned int uiLeftVowelPos = m_pLexeme->stGetProperties().sGraphicStem.uiGetVowelPos(*itLeft);
        if (uiLeftVowelPos > uiDashPos)
        {
            return false;
        }

        auto itRight = itLeft + 1;
        for (; itRight != m_pLexeme->stGetProperties().vecSourceStressPos.end(); ++itRight)
        {
            if (m_pLexeme->stGetProperties().sGraphicStem.uiGetVowelPos(*itRight) > uiDashPos)
            {
                return true;
            }
        }
    }
    catch (CException& ex)
    {
        CEString sMsg(L"uiGetVowelPos() error: ");
        sMsg += ex.szGetDescription();
        ERROR_LOG(sMsg);
    }

    return false;

}   //  bMultiStress (...)

ET_ReturnCode CInflection::eCreateWordForm(shared_ptr<CWordForm>& spWf)
{
    spWf = make_shared<CWordForm>(this);
    spWf->m_spInflection = this;
    spWf->m_llLexemeId = m_pLexeme->stGetProperties().llDescriptorId;

    return H_NO_ERROR;
}

ET_ReturnCode CInflection::eWordFormFromHash(CEString sHash, int iAt, CWordForm*& spWf)
{
    pair<multimap<CEString, shared_ptr<CWordForm>>::iterator, multimap<CEString, shared_ptr<CWordForm>>::iterator> pairRange;
    pairRange = m_mmWordForms.equal_range(sHash);
    if (pairRange.first == pairRange.second)
    {
        return H_FALSE;     // form does not exist; this is common, cf. locative, non-existent participia, etc.
    }

    if (distance(pairRange.first, pairRange.second) - 1 < iAt)
    {
        assert(0);
        ERROR_LOG(L"Bad form number.");
        return H_ERROR_UNEXPECTED;
    }

    auto itWf(pairRange.first);
    advance(itWf, iAt);
    if ((*itWf).first != sHash)
    {
        assert(0);
        ERROR_LOG(L"Error extracting map element.");
        return H_ERROR_INVALID_ARG;
    }

    spWf = itWf->second.get();

    return H_NO_ERROR;

}   //  WordFormFromHash (...)

ET_ReturnCode CInflection::eRemoveWordForm(CEString sHash, int iAt)
{
    pair<multimap<CEString, shared_ptr<CWordForm>>::iterator, multimap<CEString, shared_ptr<CWordForm>>::iterator> pairRange;
    pairRange = m_mmWordForms.equal_range(sHash);
    if (pairRange.first == pairRange.second)
    {
        assert(0);
        ERROR_LOG(L"Unable to locate word form in collection.");
        return H_ERROR_UNEXPECTED;
    }

    if (distance(pairRange.first, pairRange.second) - 1 < iAt)
    {
        assert(0);
        ERROR_LOG(L"Bad form number.");
        return H_ERROR_UNEXPECTED;
    }

    auto itWf(pairRange.first);
    advance(itWf, iAt);
    if ((*itWf).first != sHash)
    {
        assert(0);
        ERROR_LOG(L"Error extracting map element.");
        return H_ERROR_INVALID_ARG;
    }

    m_mmWordForms.erase(itWf);

    return H_NO_ERROR;

}   //  eRemoveWordForm (...)

ET_ReturnCode CInflection::eRemoveIrregularForms(CEString sHash)
{
    auto pairRange = m_mmapIrregularForms.equal_range(sHash);
    if (pairRange.first == pairRange.second)
    {
        //        assert(0);  NB this may be legit, e.g., if there were no irreg. forms before editing
        ERROR_LOG(L"Unable to locate word form in collection.");
        return H_ERROR_UNEXPECTED;
    }

    m_mmapIrregularForms.erase(pairRange.first, pairRange.second);

    return H_NO_ERROR;
}

int CInflection::iFormCount(CEString sHash)
{
    return (int)m_mmWordForms.count(sHash);
}

ET_ReturnCode CInflection::eSaveIrregularForms()
{
    ET_ReturnCode rc = H_NO_ERROR;

    if (!m_pLexeme->stGetProperties().bHasIrregularForms)
    {
        return H_FALSE;
    }

    auto spDb = m_pLexeme->spGetDb();
    if (nullptr == spDb)
    {
        return H_ERROR_POINTER;
    }

    try
    {
        spDb->BeginTransaction();

        sqlite3_stmt* pStmt = nullptr;
        spDb->uiPrepareForInsert(L"irregular_forms", 7, pStmt, false);
        auto llInsertHandle = (long long)pStmt;

        for (auto& pairIf : m_mmapIrregularForms)
        {
            spDb->Bind(1, (int64_t)m_stProperties.llInflectionId, llInsertHandle);
            spDb->Bind(2, pairIf.second.spWordForm->sGramHash(), llInsertHandle);
            spDb->Bind(3, pairIf.second.spWordForm->sWordForm(), llInsertHandle);
            spDb->Bind(4, pairIf.second.bIsOptional, llInsertHandle);
            spDb->Bind(5, pairIf.second.spWordForm->sLeadComment(), llInsertHandle);      //  lead comment
            spDb->Bind(6, pairIf.second.spWordForm->sTrailingComment(), llInsertHandle);  //  trailing comment
            spDb->Bind(7, true, llInsertHandle);     //  is_edited

            spDb->uiPrepareForInsert(L"irregular_stress", 4, pStmt, false);
            spDb->InsertRow();
            spDb->Finalize();

            long long llId = spDb->llGetLastKey();
            for (auto& posToType : pairIf.second.spWordForm->m_mapStress)
            {
                spDb->Bind(1, (int64_t)llId, llInsertHandle);
                int iVowelPos = pairIf.second.spWordForm->sWordForm().uiGetVowelPos(posToType.first);
                spDb->Bind(2, iVowelPos, llInsertHandle);            // position
                spDb->Bind(3, posToType.second, llInsertHandle);     // type
                spDb->Bind(4, true, llInsertHandle);                 // is_edited
                spDb->InsertRow();
                spDb->Finalize();
            }
        }

        spDb->CommitTransaction();
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        rc = H_ERROR_DB;
    }
    catch (...)
    {
        CEString sMsg;
        CEString sError;
        try
        {
            spDb->GetLastError(sError);
            sMsg += CEString(L", error: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(spDb->iGetLastError());
        ERROR_LOG(sMsg);
        rc = H_EXCEPTION;
    }

    return rc;

}   //  eSaveIrregularForms()

ET_ReturnCode CInflection::eDeleteIrregularForm(const CEString& sFormHash)
{
    // Delete irregular form and variants
    auto itRange = m_mmapIrregularForms.equal_range(sFormHash);
    if (itRange.first == itRange.second)
    {
        return H_FALSE;
    }

    shared_ptr<CSqlite> spDbHandle;
    for (auto& itCurrentForm = itRange.first; itCurrentForm != itRange.second; ++itCurrentForm)
    {
        try
        {
            CEString sSelectIrregularForms(L"SELECT id FROM irregular_forms WHERE inflection_id=\"");
            sSelectIrregularForms += CEString::sToString(m_stProperties.llInflectionId);
            sSelectIrregularForms += L"\" AND gram_hash = \"";
            sSelectIrregularForms += sFormHash;
            sSelectIrregularForms += L"\"";

            spDbHandle = m_pLexeme->spGetDb();
            spDbHandle->PrepareForSelect(sSelectIrregularForms);
            int64_t iDbKey = 0;
            while (spDbHandle->bGetRow())
            {
                spDbHandle->GetData(0, iDbKey);
                CEString sDeleteForms(L"DELETE FROM irregular_forms WHERE id = \"");
                sDeleteForms += CEString::sToString(iDbKey);
                sDeleteForms += L"\"";
                spDbHandle->Exec(sDeleteForms);

                CEString sDeleteStress(L"DELETE FROM irregular_stress WHERE form_id = \"");
                sDeleteStress += CEString::sToString(iDbKey);
                sDeleteStress += L"\"";
                spDbHandle->Exec(sDeleteStress);
            }
        }
        catch (CException& ex)
        {
            ERROR_LOG(ex.szGetDescription());
            return H_EXCEPTION;
        }
        catch (...)
        {
            CEString sMsg;
            CEString sError;
            try
            {
                spDbHandle->GetLastError(sError);
                sMsg += CEString(L", error: ");
                sMsg += sError;
            }
            catch (...)
            {
                sMsg = L"Apparent DB error ";
            }

            sMsg += CEString::sToString(spDbHandle->iGetLastError());
            ERROR_LOG(sMsg);
            return H_EXCEPTION;
        }
    }

    m_mmapIrregularForms.erase(sFormHash);

    if (m_mmapIrregularForms.empty())
    {
        m_pLexeme->stGetPropertiesForWriteAccess().bHasIrregularForms = false;
        try
        {
            vector<CEString> vecColumns = { L"has_irregular_forms" };
            spDbHandle->PrepareForUpdate(L"descriptor", vecColumns, m_pLexeme->stGetProperties().llDescriptorId);
            spDbHandle->Bind(1, false);

            spDbHandle->UpdateRow();
            spDbHandle->Finalize();
        }
        catch (CException& exc)
        {
            CEString sMsg(exc.szGetDescription());
            CEString sError;
            try
            {
                spDbHandle->GetLastError(sError);
                sMsg += CEString(L", error %d: ");
                sMsg += sError;
            }
            catch (...)
            {
                sMsg = L"Apparent DB error ";
            }

            sMsg += CEString::sToString(spDbHandle->iGetLastError());
            ERROR_LOG(sMsg);

            return H_ERROR_DB;
        }
    }

    return H_NO_ERROR;

}       //  eDeleteIrregularForm()

ET_ReturnCode CInflection::eSaveIrregularForm(const CEString& sFormHash, shared_ptr<CWordForm>& spWordForm)
{
    CEString sSelectIrregularForms(L"SELECT id FROM irregular_forms WHERE inflection_id=\"");
    sSelectIrregularForms += CEString::sToString(m_stProperties.llInflectionId);
    sSelectIrregularForms += L"\" AND gram_hash = \"";
    sSelectIrregularForms += sFormHash;
    sSelectIrregularForms += L"\"";

    auto spDbHandle = m_pLexeme->spGetDb();
    CEString sLeadComment;
    CEString sTrailingComment;

    try
    {
        bool bIgnoreOnConflict = true;
        spDbHandle->PrepareForInsert(L"irregular_forms", 7, bIgnoreOnConflict);
        spDbHandle->Bind(1, (int64_t)m_stProperties.llInflectionId);
        spDbHandle->Bind(2, sFormHash);
        spDbHandle->Bind(3, spWordForm->sWordForm());
        spDbHandle->Bind(4, spWordForm->bIsVariant());
        spDbHandle->Bind(5, spWordForm->sLeadComment());            // TODO
        spDbHandle->Bind(6, spWordForm->sTrailingComment());        // TODO
        spDbHandle->Bind(7, true);                                 // is_edited

        spDbHandle->InsertRow();
        spDbHandle->Finalize();

        long long llFormKey = spDbHandle->llGetLastKey();

        int iStressPos = -1;
        ET_StressType eStressType = ET_StressType::STRESS_TYPE_UNDEFINED;
        ET_ReturnCode eRet = spWordForm->eGetFirstStressPos(iStressPos, eStressType);
        while (H_NO_ERROR == eRet)
        {
            spDbHandle->PrepareForInsert(L"irregular_stress", 4, bIgnoreOnConflict);
            spDbHandle->Bind(1, (int64_t)llFormKey);
            spDbHandle->Bind(2, iStressPos);
            bool bIsPrimary = (ET_StressType::STRESS_PRIMARY == eStressType) ? true : false;
            spDbHandle->Bind(3, bIsPrimary);
            spDbHandle->Bind(4, true);       // is_edited

            spDbHandle->InsertRow();
            spDbHandle->Finalize();

            eRet = spWordForm->eGetNextStressPos(iStressPos, eStressType);
        }

        if (eRet != ET_ReturnCode::H_NO_MORE)
        {
            CEString sMsg(L"Error ");
            sMsg += CEString::sToString(eRet);
            sMsg += L" while reading stress data.";
            ERROR_LOG(sMsg);
        }
    }
    catch (CException& exc)
    {
        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            spDbHandle->GetLastError(sError);
            sMsg += CEString(L", error %d: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(spDbHandle->iGetLastError());
        ERROR_LOG(sMsg);

        return H_ERROR_DB;
    }

    try
    {
        vector<CEString> vecColumns = { L"has_irregular_forms", L"is_edited" };
        spDbHandle->PrepareForUpdate(L"descriptor", vecColumns, m_pLexeme->stGetProperties().llDescriptorId);
        spDbHandle->Bind(1, true);
        spDbHandle->Bind(2, true);

        spDbHandle->UpdateRow();
        spDbHandle->Finalize();
    }
    catch (CException& exc)
    {
        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            spDbHandle->GetLastError(sError);
            sMsg += CEString(L", error %d: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(spDbHandle->iGetLastError());
        ERROR_LOG(sMsg);

        return H_ERROR_DB;
    }

    return H_NO_ERROR;

}       //  eSaveIrregularForm()

ET_ReturnCode CInflection::eSaveIrregularForms(const CEString& sGramHash)
{
    ET_ReturnCode rc = H_NO_ERROR;

    auto spDb = m_pLexeme->spGetDb();
    if (nullptr == spDb)
    {
        return H_ERROR_POINTER;
    }

    try
    {
        //        pDb->BeginTransaction();

        CEString sStressDelQuery(L"DELETE FROM irregular_stress WHERE form_id IN (SELECT id FROM irregular_forms WHERE inflection_id = ");
        sStressDelQuery += CEString::sToString(m_stProperties.llInflectionId);
        sStressDelQuery += L" AND gram_hash = '";
        sStressDelQuery += sGramHash + L"')";
        spDb->Delete(sStressDelQuery);

        CEString sDelQuery(L"DELETE FROM irregular_forms WHERE inflection_id = ");
        sDelQuery += CEString::sToString(m_stProperties.llInflectionId);
        sDelQuery += L" AND gram_hash = '";
        sDelQuery += sGramHash + L"'";
        spDb->Delete(sDelQuery);

        sqlite3_stmt* pStmt = nullptr;
        spDb->uiPrepareForInsert(L"irregular_forms", 7, pStmt, false);
        auto llFormInsertHandle = (long long)pStmt;

        auto pairFormsForHash = m_mmapIrregularForms.equal_range(sGramHash);
        for (auto& it = pairFormsForHash.first; it != pairFormsForHash.second; ++it)
        {
            spDb->Bind(1, (int64_t)m_stProperties.llInflectionId, llFormInsertHandle);
            spDb->Bind(2, sGramHash, llFormInsertHandle);
            spDb->Bind(3, it->second.spWordForm->m_sWordForm, llFormInsertHandle);
            spDb->Bind(4, it->second.bIsOptional, llFormInsertHandle);
            spDb->Bind(5, it->second.spWordForm->m_sLeadComment, llFormInsertHandle);
            spDb->Bind(6, it->second.spWordForm->m_sTrailingComment, llFormInsertHandle);
            spDb->Bind(7, true, llFormInsertHandle);     //  is_edited

            spDb->InsertRow(llFormInsertHandle);
            long long llFormId = spDb->llGetLastKey(llFormInsertHandle);

            pStmt = nullptr;
            spDb->uiPrepareForInsert(L"irregular_stress", 4, pStmt, false);
            auto llStressInsertHandle = (long long)pStmt;

            for (auto pairPosToType : it->second.spWordForm->m_mapStress)
            {
                spDb->Bind(1, (int64_t)llFormId, llStressInsertHandle);
                int iVowelPos = it->second.spWordForm->sWordForm().uiGetVowelPos(pairPosToType.first);
                spDb->Bind(2, iVowelPos, llStressInsertHandle);                // char position
                spDb->Bind(3, pairPosToType.second, llStressInsertHandle);     // type
                spDb->Bind(4, true, llStressInsertHandle);                     // is_edited
                spDb->InsertRow(llStressInsertHandle);
            }
            spDb->Finalize(llStressInsertHandle);
        }
        spDb->Finalize(llFormInsertHandle);

        m_pLexeme->stGetPropertiesForWriteAccess().bHasIrregularForms = true;
        vector<CEString> vecColumns = { L"has_irregular_forms", L"is_edited" };
        spDb->PrepareForUpdate(L"descriptor", vecColumns, m_pLexeme->stGetProperties().llDescriptorId);
        spDb->Bind(1, true);
        spDb->Bind(2, true);

        spDb->UpdateRow();
        spDb->Finalize();

        //      pDb->CommitTransaction();

    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        rc = H_ERROR_DB;
    }
    catch (...)
    {
        CEString sMsg;
        CEString sError;
        try
        {
            spDb->GetLastError(sError);
            sMsg += CEString(L", error: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(spDb->iGetLastError());
        ERROR_LOG(sMsg);
        rc = H_EXCEPTION;
    }

    return rc;

}   //  eSaveIrregularForms()

ET_ReturnCode CInflection::eSaveStemsToDb()
{
    if (m_mmWordForms.empty())
    {
        //        throw CException(H_ERROR_UNEXPECTED, L"No wordforms.");
    }

    ET_ReturnCode eRet = H_NO_ERROR;

    CEString sCurrentStem;
    auto itWf = m_mmWordForms.begin();
    for (; itWf != m_mmWordForms.end(); ++itWf)
    {
        auto spWf = (*itWf).second;
        if (nullptr == spWf)
        {
            eRet = H_ERROR_POINTER;
            ERROR_LOG(L"No wordform.");
            continue;
        }

        if (spWf->m_bIrregular)
        {
            continue;
        }

        if (spWf->m_sStem != sCurrentStem)
        {
            bool bSaved = (*itWf).second->bSaveStemToDb();
            if (!bSaved)
            {
                eRet = H_ERROR_GENERAL;
                ERROR_LOG(L"Unable to save a stem in the DB.");
                continue;
            }
            sCurrentStem = (*itWf).second->m_sStem;
        }
    }

    return eRet;

}   // eSaveStemsToDb()

ET_ReturnCode CInflection::eAssignStemIds()
{
    ET_ReturnCode eRet = H_NO_ERROR;

    auto itWf = m_mmWordForms.begin();
    for (; itWf != m_mmWordForms.end(); ++itWf)
    {
        //        if (0 == (*itWf).second->m_llStemId)    // stem string not found, look up in DB
        //{
            CEString sQuery(L"SELECT id FROM stems WHERE stem_string=\"");
            sQuery += (*itWf).second->m_sStem;
            sQuery += L'"';

            try
            {
                auto spDbHandle = m_pLexeme->spGetDb();
                spDbHandle->PrepareForSelect(sQuery);
                if (spDbHandle->bGetRow())
                {
                    int64_t iStemId = 0;
                    spDbHandle->GetData(0, iStemId);
                    (*itWf).second->m_llStemId = iStemId;
                }
                else
                {
                    CEString sMsg(L"Unable to find stem id for \"");
                    sMsg += (*itWf).second->m_sStem;
                    sMsg += L"\"; lexeme = ";
                    ERROR_LOG(sMsg += m_pLexeme->stGetProperties().sSourceForm);
                }
                spDbHandle->Finalize();
            }
            catch (CException& ex)
            {
                ERROR_LOG(ex.szGetDescription());
                eRet = H_ERROR_DB;
            }
            catch (...)
            {
                ERROR_LOG(L"Unknown exception.");
            }

        //}
    }       // for (...)

    return eRet;

}       //  eAssignStemIds()

ET_ReturnCode CInflection::eSaveWordFormsToDb()
{
    if (m_mmWordForms.empty())
    {
        return H_FALSE;
    }

    ET_ReturnCode eRet = H_NO_ERROR;

    auto itWf = m_mmWordForms.begin();
    for (; itWf != m_mmWordForms.end(); ++itWf)
    {
        if (0 == (*itWf).second->m_llStemId)
        {
            CEString sMsg(L"Unable to find stem id for \"");
            sMsg += (*itWf).second->m_sStem;
            sMsg += L'"';
            ERROR_LOG(sMsg);

            continue;
        }

        bool bSaved = (*itWf).second->bSaveToDb();
        if (!bSaved)
        {
            eRet = H_ERROR_GENERAL;
        }

    }       //  for (...)

    return eRet;

}   // eSaveWordFormsToDb()

// Intended for spryazh. sm. forms only
ET_ReturnCode CInflection::eSaveIrregularFormsToDb()
{
    if (m_mmWordForms.empty())
    {
        return H_FALSE;
    }

    ET_ReturnCode eRet = H_NO_ERROR;

    auto itWf = m_mmWordForms.begin();
    for (; itWf != m_mmWordForms.end(); ++itWf)
    {
        if ((*itWf).second->m_bIrregular)
        {
            (*itWf).second->bSaveIrregularForm();
        }
    }       //  for (...)

    return eRet;
}

CEString CInflection::sHash()
{
    auto stLexemeProperties = m_pLexeme->stGetProperties();
    CEString sSource(stLexemeProperties.sSourceForm);
    vector<int>::iterator itHomonym = stLexemeProperties.vecHomonyms.begin();
    for (; itHomonym != stLexemeProperties.vecHomonyms.end(); ++itHomonym)
    {
        sSource += CEString::sToString(*itHomonym);
    }

    auto itStress = stLexemeProperties.vecSourceStressPos.begin();
    for (; itStress != stLexemeProperties.vecSourceStressPos.end(); ++itStress)
    {
        auto iPos = *itStress;
        sSource += 32 + iPos;
    }

    itStress = stLexemeProperties.vecSecondaryStressPos.begin();
    for (; itStress != stLexemeProperties.vecSecondaryStressPos.end(); ++itStress)
    {
        auto iPos = *itStress;
        sSource += 32 + iPos;
    }

    sSource += stLexemeProperties.sMainSymbol;
    sSource += 32 + m_stProperties.iType;
    sSource += 32 + m_stProperties.eAccentType1;
    sSource += 32 + m_stProperties.eAccentType2;

    //    auto charIsSecondPart = m_stProperties.bIsSecondPart ? '1' : '0';
    //    if ('1' == charIsSecondPart)
    //    {
    //        sSource += charIsSecondPart;
    //    }

    sSource += stLexemeProperties.sHeadwordComment;
    sSource += stLexemeProperties.sComment;
    sSource += stLexemeProperties.sTrailingComment;

    //    CMD5 md5;
    //    return md5.sHash(sSource);

    SHA1 hash;
    hash.update(sSource.stl_sToUtf8());
    const string utf8Hash = hash.final();

    return CEString::sFromUtf8(utf8Hash);

}   //  sHash()

CEString CInflection::sParadigmHash()
{
    CEString sSource(m_pLexeme->stGetProperties().sSourceForm);
    auto itHomonym = m_pLexeme->stGetProperties().vecHomonyms.begin();
    for (; itHomonym != m_pLexeme->stGetProperties().vecHomonyms.end(); ++itHomonym)
    {
        sSource += CEString::sToString(*itHomonym);
    }

    auto itStress = m_pLexeme->stGetProperties().vecSourceStressPos.begin();
    for (; itStress != m_pLexeme->stGetProperties().vecSourceStressPos.end(); ++itStress)
    {
        auto iPos = *itStress;
        sSource += 32 + iPos;
    }

    itStress = m_pLexeme->stGetProperties().vecSecondaryStressPos.begin();
    for (; itStress != m_pLexeme->stGetProperties().vecSecondaryStressPos.end(); ++itStress)
    {
        auto iPos = *itStress;
        sSource += 32 + iPos;
    }

    sSource += CEString::sToString(m_pLexeme->llHeadwordId());

    sSource += m_pLexeme->stGetProperties().sMainSymbol;
    sSource += m_pLexeme->stGetProperties().sInflectionType;
    sSource += m_pLexeme->stGetProperties().sAltMainSymbol;
    sSource += 32 + m_stProperties.iType;
    sSource += 32 + m_stProperties.eAccentType1;
    sSource += 32 + m_stProperties.eAccentType2;

    //    auto charIsSecondPart = m_stProperties.bIsSecondPart ? '1' : '0';
    //    if ('1' == charIsSecondPart)
    //    {
    //        sSource += charIsSecondPart;
    //    }

    sSource += m_pLexeme->stGetProperties().sHeadwordComment;
    sSource += m_pLexeme->stGetProperties().sComment;
    sSource += m_pLexeme->stGetProperties().sTrailingComment;

    sSource += m_stProperties.bFleetingVowel ? L'1' : L'0';
    sSource += m_pLexeme->stGetProperties().bNoComparative ? L'1' : L'0';
    sSource += m_pLexeme->stGetProperties().bNoLongForms ? L'1' : L'0';
    sSource += m_pLexeme->stGetProperties().bYoAlternation ? L'1' : L'0';
    sSource += m_pLexeme->stGetProperties().bOAlternation ? L'1' : L'0';
    sSource += 32 + m_pLexeme->stGetProperties().ePartOfSpeech;
    sSource += m_pLexeme->stGetProperties().bPartPastPassZhd ? L'1' : L'0';
    sSource += 32 + m_pLexeme->stGetProperties().iSection;
    sSource += 32 + m_stProperties.iStemAugment;
    for (auto& pairCd : m_stProperties.mapCommonDeviations)
    {
        sSource += 32 + pairCd.first;
        sSource += pairCd.second ? '1' : '0';
    }

    sSource += m_pLexeme->stGetProperties().sSpryazhSmRefSource;
    sSource += m_pLexeme->stGetProperties().sSpryazhSmPrefix;
    sSource += m_pLexeme->stGetProperties().sSpryazhSmRefHomonyms;
    sSource += 33 + m_pLexeme->stGetProperties().iSpryazhSmRefPrefixLength; // NB can be -1
    sSource += (m_pLexeme->stGetProperties().bSpryazhSmNoAspectPair) ? L'1' : L'0';

    SHA1 hash;
    hash.update(sSource.stl_sToUtf8());
    const string utf8Hash = hash.final();

    return CEString::sFromUtf8(utf8Hash);

}   //  sParadigmHash()

ET_ReturnCode CInflection::eSaveTestData()
{
    if (m_mmWordForms.empty())
    {
        return H_FALSE;
    }

    shared_ptr<CSqlite> spDbHandle;
    spDbHandle = m_pLexeme->spGetDb();

    spDbHandle->BeginTransaction();

    try
    {
        CEString sSelectQuery = L"SELECT td.id FROM test_data AS td WHERE td.lexeme_id = \'";
        sSelectQuery += sHash();
        sSelectQuery += L"\'; ";

        int64_t iTestDataId = -1;
        spDbHandle->PrepareForSelect(sSelectQuery);
        vector <long long> vecTestDataIds;
        while (spDbHandle->bGetRow())
        {
            spDbHandle->GetData(0, iTestDataId);
            vecTestDataIds.push_back(iTestDataId);
        }

        CEString sDeleteQuery = L"DELETE FROM test_data_stress WHERE test_data_id = ";
        for (auto llId : vecTestDataIds)
        {
            spDbHandle->Delete(sDeleteQuery + CEString::sToString(llId) + L"; ");
        }

        sDeleteQuery = L"DELETE FROM test_data WHERE lexeme_id = \'";
        sDeleteQuery += sHash();
        sDeleteQuery += L"\' ";
        spDbHandle->Delete(sDeleteQuery);

        auto itWf = m_mmWordForms.begin();
        for (; itWf != m_mmWordForms.end(); ++itWf)
        {
            ET_ReturnCode eRet = (*itWf).second->eSaveTestData();
            if (Hlib::CErrorCode::bError(eRet))
            {
                spDbHandle->RollbackTransaction();
                return eRet;
                //            eRet = H_ERROR_GENERAL;
            }
        }       //  for (...)
    }
    catch (CException& exc)
    {
        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            spDbHandle->GetLastError(sError);
            sMsg += CEString(L", error %d: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(spDbHandle->iGetLastError());
        ERROR_LOG(sMsg);
    }

    try
    {
        spDbHandle = m_pLexeme->spGetDb();

        CEString sDeleteQuery(L"DELETE FROM lexeme_hash_to_descriptor WHERE descriptor_id = ");
        sDeleteQuery += CEString::sToString(m_pLexeme->stGetProperties().llDescriptorId);
        spDbHandle->Exec(sDeleteQuery);

        spDbHandle->PrepareForInsert(L"lexeme_hash_to_descriptor", 3);
        spDbHandle->Bind(1, sHash());
        spDbHandle->Bind(2, (int64_t)m_pLexeme->stGetProperties().llDescriptorId);
        spDbHandle->Bind(3, (int64_t)m_stProperties.llInflectionId);
        spDbHandle->InsertRow();
        spDbHandle->Finalize();
    }
    catch (CException& exc)
    {
        CEString sMsg(exc.szGetDescription());
        CEString sError;
        try
        {
            spDbHandle->GetLastError(sError);
            sMsg += CEString(L", error %d: ");
            sMsg += sError;
        }
        catch (...)
        {
            sMsg = L"Apparent DB error ";
        }

        sMsg += CEString::sToString(spDbHandle->iGetLastError());
        ERROR_LOG(sMsg);
    }

    if (m_pLexeme->bHasAspectPair())
    {
        CEString sAspectPairHash(L"AspectPair");

        spDbHandle->PrepareForInsert(L"test_data", 3);
        spDbHandle->Bind(1, sHash());    // lexeme hash
        spDbHandle->Bind(2, sAspectPairHash);
        spDbHandle->Bind(3, m_sAspectPair);
        spDbHandle->InsertRow();
        spDbHandle->Finalize();
        auto llId = spDbHandle->llGetLastKey();

        spDbHandle->PrepareForInsert(L"test_data_stress", 3);
        spDbHandle->Bind(1, llId);
        spDbHandle->Bind(2, m_iAspectPairStressPos);
        ET_StressType eType = STRESS_PRIMARY;
        spDbHandle->Bind(3, eType);
        spDbHandle->InsertRow();
        spDbHandle->Finalize();

        if (m_pLexeme->bHasAltAspectPair())
        {
            CEString sAltAspectPairHash(L"AltAspectPair");

            spDbHandle->PrepareForInsert(L"test_data", 3);
            spDbHandle->Bind(1, sHash());    // lexeme hash
            spDbHandle->Bind(2, sAltAspectPairHash);
            spDbHandle->Bind(3, m_sAltAspectPair);
            spDbHandle->InsertRow();
            spDbHandle->Finalize();
            llId = spDbHandle->llGetLastKey();

            spDbHandle->PrepareForInsert(L"test_data_stress", 3);
            spDbHandle->Bind(1, llId);
            spDbHandle->Bind(2, m_iAltAspectPairStressPos);
            ET_StressType eType = STRESS_PRIMARY;
            spDbHandle->Bind(3, eType);
            spDbHandle->InsertRow();
            spDbHandle->Finalize();
        }
    }

    spDbHandle->CommitTransaction();

    return H_NO_ERROR;

}   // eSaveTestData()

ET_ReturnCode CInflection::eAlignInflectedParts()
{
    auto spLexeme2 = m_pLexeme->spGetSecondPart();
    if (!spLexeme2)
    {
        ERROR_LOG(L"No second part.");
        return H_ERROR_POINTER;
    }

    ET_ReturnCode rc = H_NO_ERROR;

    CInflectionEnumerator ie(spLexeme2);
    shared_ptr<CInflection> spInflection2;
    rc = ie.eGetFirstInflection(spInflection2);
    if (rc != H_NO_ERROR || nullptr == spInflection2)
    {
        CEString sMsg(L"Unable to read inflection data for ");
        sMsg += m_pLexeme->sSourceForm();
        ERROR_LOG(sMsg);
        return H_ERROR_UNEXPECTED;
    }

//    rc = spInflection2->eGenerateParadigm();

// TODO: warning if multiple inflections for the second part

    for (auto keyValPairLeft = m_mmWordForms.begin();
        keyValPairLeft != m_mmWordForms.end();
        keyValPairLeft = m_mmWordForms.upper_bound((*keyValPairLeft).first))  // next gramm hash
    {
        auto sGramHashLeft = (*keyValPairLeft).first;
        sGramHashLeft.SetBreakChars(L"_");
        CEString sGramHashRight;
        bool bMatch = false;

        for (auto& kvpRight : spInflection2->m_mmWordForms)
        {
            sGramHashRight = kvpRight.first;
            sGramHashRight.SetBreakChars(L"_");
            auto nFieldsLeft = sGramHashLeft.uiGetNumOfFields();
            auto nFieldsRight = sGramHashRight.uiGetNumOfFields();
            if (L"AdjL" == sGramHashLeft.sGetField(0))
            {
                if (nFieldsLeft == 4 && nFieldsRight == 4)
                {
                    if (sGramHashLeft.sGetField(nFieldsLeft - 1) == sGramHashRight.sGetField(nFieldsRight - 1) &&
                        sGramHashLeft.sGetField(nFieldsLeft - 2) == sGramHashRight.sGetField(nFieldsRight - 2) &&
                        sGramHashLeft.sGetField(nFieldsLeft - 3) == sGramHashRight.sGetField(nFieldsRight - 3))
                    {
                        bMatch = true;
                    }
                }
                else if (nFieldsLeft == 3 && nFieldsRight == 3)
                {
                    if (sGramHashLeft.sGetField(nFieldsLeft - 1) == sGramHashRight.sGetField(nFieldsRight - 1) &&
                        sGramHashLeft.sGetField(nFieldsLeft - 2) == sGramHashRight.sGetField(nFieldsRight - 2))
                    {
                        bMatch = true;
                    }
                }
            }
            else if (L"AdjS" == sGramHashLeft.sGetField(0))
            {
                if (nFieldsLeft != 2 || nFieldsRight != 2)
                {
                    continue;
                }
                if (sGramHashLeft.sGetField(nFieldsLeft - 1) == sGramHashRight.sGetField(nFieldsRight - 1))
                {
                    bMatch = true;
                }
            }
            else if (L"AdjComp" == sGramHashLeft && L"AdjComp" == sGramHashRight)
            {
                bMatch = true;
            }
            else if (L"Noun" == sGramHashLeft.sGetField(0))
            {
                if (nFieldsLeft != 3 || nFieldsRight < 3)
                {
                    continue;
                }
                if (sGramHashLeft.sGetField(nFieldsLeft - 1) == sGramHashRight.sGetField(nFieldsRight - 1) &&
                    sGramHashLeft.sGetField(nFieldsLeft - 2) == sGramHashRight.sGetField(nFieldsRight - 2))
                {
                    bMatch = true;
                }
            }
            else
            {
                ERROR_LOG(L"Unsupported combination of gram hashes: " + sGramHashLeft + L", " + sGramHashRight);
                continue;
            }

            if (bMatch)
            {
                break;
            }
        }       //  for (auto& kvpRight : m_spSecondPart->m_mmWordForms)

//        ET_ReturnCode eRet = H_NO_ERROR;
        auto pairLeftValueRange = m_mmWordForms.equal_range(sGramHashLeft);   // get left and right word forms for the same gram hash
        auto pairRightValueRange = spInflection2->m_mmWordForms.equal_range(sGramHashRight);
        int iCountLeft = 0;
        int iCountRight = 0;
        for (auto& itValueLeft = pairLeftValueRange.first; itValueLeft != pairLeftValueRange.second; ++itValueLeft, ++iCountLeft)
        {
//            if ((*itValueLeft).second->bIrregular())
//            {
//                continue;       // irregular forms don't need alignment, they are stored in the left wf
//            }

            auto& itValueRight = pairRightValueRange.first;
            if (0 == (*itValueRight).second->m_spInflection->iType())
            {
                auto spWfLeft = (*itValueLeft).second;
                auto spWfRight = (*itValueRight).second;
                rc = eConcatenateInflectedParts(*spWfLeft, *spWfRight);
            }
            else
            {
                auto spWfLeft = (*itValueLeft).second;
                auto spWfRight = (*itValueRight).second;
                rc = eConcatenateInflectedParts(*spWfLeft, *spWfRight);
                if (++iCountRight < (int)spInflection2->m_mmWordForms.count(sGramHashRight))
                {
                    ++itValueRight;
                }
            }
        }
    }
    return rc;

}       //  eAlignInflectedParts()

ET_ReturnCode CInflection::eConcatenateInflectedParts(CWordForm& wfLeft, CWordForm& wfRight)
{
    //    int iStressPos = -1;
    //    ET_StressType eStressType = ET_StressType::STRESS_TYPE_UNDEFINED;
    for (auto& pairPosType : wfRight.m_mapStress)
    {
        auto sLeft = wfLeft.sWordForm();
        sLeft.SetVowels(CEString::g_szRusVowels);
        auto sRight = wfRight.sWordForm();
        sRight.SetVowels(CEString::g_szRusVowels);
        auto iPos = pairPosType.first + sLeft.uiNSyllables();
        wfLeft.m_mapStress[iPos] = pairPosType.second;
    }
    wfLeft.SetWordForm(wfLeft.sWordForm() + L"-" + wfRight.sWordForm());

    return H_NO_ERROR;
}
