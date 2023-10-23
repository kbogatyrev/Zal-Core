#include "EString.h"
#include "Lexeme.h"
#include "WordForm.h"
#include "FormBuilderAspectPair.h"

using namespace Hlib;

ET_ReturnCode CFormBuilderAspectPair::eBuild()
{
    StLexemeProperties stLexProperties = m_pLexeme->stGetProperties();
    if (!stLexProperties.bHasAspectPair)
    {
        return H_FALSE;
    }

    if (m_bBuilt)
    {
        return H_NO_ERROR;
    }

    ET_ReturnCode eRet = H_NO_ERROR;

    m_sSource = m_pLexeme->sSourceForm();
    if (m_sSource.bEndsWith(L"ся") || m_sSource.bEndsWith(L"сь"))
    {
        m_sReflexiveSuffix = m_sSource.sSubstr(m_sSource.uiLength()-3);
        m_sSource.sRemoveCharsFromEnd(2);
    }

    for (int iCount = 0; iCount < 2; ++iCount)
    {
        int iType = (0 == iCount) ? stLexProperties.iAspectPairType : stLexProperties.iAltAspectPairType;
        bool bIsVariant = (0 == iCount) ? false : true;
        CEString& sData = (0 == iCount) ? stLexProperties.sAspectPairData : stLexProperties.sAltAspectPairData;
        ET_ExtraData& eExtraData = (0 == iCount) ? m_eExtraData : m_eAltExtraData;

        if (!sData.bIsEmpty())
        {
            if (sData.bStartsWith(L"-") && sData.bEndsWith(L"-"))
            {
                eExtraData = ET_ExtraData::Vowel;
            }
            else if (sData.bStartsWith(L"-"))
            {
                eExtraData = ET_ExtraData::Suffix;
            }
            else
            {
                eExtraData = ET_ExtraData::WholeWord;
            }
        }

        if (ET_ExtraData::WholeWord == eExtraData)
        {
//            auto& stLexProperties = m_pLexeme->stGetProperties();
//            auto& stInflProperties = m_pInflection->stGetProperties();
            sData = bIsVariant ? stLexProperties.sAltAspectPairData : stLexProperties.sAspectPairData;
            CEString& sAspectPair = bIsVariant ? m_sAltAspectPairSource : m_sAspectPairSource;

            if (!stLexProperties.bHasAspectPair || sData.bIsEmpty())
            {
                return H_ERROR_UNEXPECTED;
            }

            sAspectPair = sData;

            if (stLexProperties.bSpryazhSm)
            {
                sAspectPair = stLexProperties.sSpryazhSmPrefix + 
                    sAspectPair.sSubstr(stLexProperties.iSpryazhSmRefPrefixLength);
            }

            eRet = eExtractStressMark(bIsVariant);
            if (H_NO_ERROR == eRet)
            {
                m_bBuilt = true;
            }

            return eRet;
        }

        switch (iType)
        {
            case 0:     // noop
            {
                if (0 == iCount)
                {
                    eRet = H_ERROR_UNEXPECTED;
                }
                else
                {
                    eRet = H_NO_ERROR;
                }
                break;
            }

            case -1:    // Type I
            {
                eRet = eSvToNsvTypeI(bIsVariant);
                if (eRet != H_NO_ERROR)
                {
                    return eRet;
                }

                eRet = eFindStressPositionI(bIsVariant);
                if (eRet != H_NO_ERROR)
                {
                    return eRet;
                }

                if (ET_ExtraData::Vowel == m_eExtraData)
                {
                    eRet = eApplyNsvVowelModification(bIsVariant);
                }

                break;
            }

            case -2:    // Type II
            {
                eRet = eSvToNsvTypeII(bIsVariant);
                break;
            }

            case -3:    // Type III
            {
                eRet = eSvToNsvTypeIII(bIsVariant);
                break;
            }

            case 1:
            case 5:
            case 6:
            {
                eRet = eNsvToSvTypes1_5_6(bIsVariant);
                break;
            }

            case 2:
            {
                eRet = eNsvToSvTypes2(bIsVariant);
                break;
            }

            case 3:
            {
                eRet = eNsvToSvTypes3(bIsVariant);
                break;
            }

            case 4:
            {
                eRet = eNsvToSvTypes4(bIsVariant);
                break;
            }

            case 7:
            {
                eRet = eNsvToSvTypes7(bIsVariant);
                break;
            }

            case 8:
            {
                eRet = eNsvToSvTypes8(bIsVariant);
                break;
            }

            case 9:
            {
                eRet = eNsvToSvTypes9(bIsVariant);
                break;
            }

            case 10:
            {
                eRet = eNsvToSvTypes10(bIsVariant);
                break;
            }

            case 14:
            {
                eRet = eNsvToSvTypes14(bIsVariant);
                break;
            }

            case 11:
            case 12:
            case 13:
            case 15:
            case 16:
            {
                eRet = eNsvToSvTypes11_12_13_15_16(bIsVariant);
                break;
            }

            default:
            {
                eRet = H_ERROR_UNEXPECTED;
            }
        }       // switch

        if (H_NO_ERROR != eRet)
        {
            return eRet;
        }

        if (iType >= 1)
        {
            if (ET_ExtraData::Vowel == eExtraData)
            {
                eRet = eApplySvVowelModification(bIsVariant);
            }
            else if (ET_ExtraData::Suffix == eExtraData)
            {
                eRet = eApplySvSuffix(bIsVariant);
            }
            else if (ET_ExtraData::WholeWord == eExtraData)
            {
                eRet = eApplySvWholeWord(bIsVariant);
            }
        }

        CEString& sAspectPair = (0 == iCount) ? m_sAspectPairSource : m_sAltAspectPairSource;

//        if (stProperties.bSpryazhSm)
//        {
//            sAspectPair = stProperties.sSpryazhSmPrefix + sAspectPair.sSubstr(stProperties.iSpryazhSmRefPrefixLength);
//        }

        if (iType >= 1)
        {
            if (sAspectPair.bStartsWith(L"вы"))
            {
                int& iStressPos = (0 == iCount) ? m_iStressPos : m_iAltStressPos;
                auto iOldStressPos = iStressPos;
                if (iOldStressPos >= (int)sAspectPair.uiLength())
                {
                    return H_ERROR_UNEXPECTED;
                }

                iStressPos = 1;

                if (iOldStressPos != iStressPos && L'ё' == sAspectPair[iOldStressPos])
                {
                    sAspectPair[iOldStressPos] = L'е';
                }
            }
        }

        if (!m_sReflexiveSuffix.bIsEmpty())
        {
            sAspectPair += L"ся";
        }
    }       // for (int iCount = 0; iCount < 2; ++iCount)

    m_bBuilt = true;

    return eRet;

}       //  eBuild()

ET_ReturnCode CFormBuilderAspectPair::eGetAspectPair(CEString& sPair, int& iStressPos)
{
    sPair = L"";
    iStressPos = -1;

    if (!m_bBuilt)
    {
        return H_ERROR_UNEXPECTED;
    }

    if (m_sAspectPairSource.bIsEmpty())
    {
        return H_FALSE;
    }

    sPair = m_sAspectPairSource;
    iStressPos = m_iStressPos;

    return H_NO_ERROR;
}

ET_ReturnCode CFormBuilderAspectPair::eGetAltAspectPair(CEString& sAltPair, int& iAltStressPos)
{
    sAltPair = L"";
    iAltStressPos = -1;

    if (!m_bBuilt)
    {
        return H_ERROR_UNEXPECTED;
    }

    if (m_sAltAspectPairSource.bIsEmpty())
    {
        return H_FALSE;
    }

    sAltPair = m_sAltAspectPairSource;
    iAltStressPos = m_iAltStressPos;

    return H_NO_ERROR;
}

ET_ReturnCode CFormBuilderAspectPair::eGet1PersonWordForm()
{
    CGramHasher hasher1Sg(POS_VERB, SUBPARADIGM_PRESENT_TENSE, CASE_UNDEFINED, NUM_SG,
        GENDER_UNDEFINED, PERSON_1, ANIM_UNDEFINED, m_pLexeme->eAspect(),
        m_pLexeme->eIsReflexive());

    m_p1stPersonWordForm = nullptr;
    ET_ReturnCode eRet = m_pInflection->eWordFormFromHash(hasher1Sg.sGramHash(), 0, m_p1stPersonWordForm);
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    if (nullptr == m_p1stPersonWordForm)
    {
        return H_ERROR_POINTER;
    }

    return H_NO_ERROR;

}       //  eGet1PersonWordForm()

ET_ReturnCode CFormBuilderAspectPair::eGetInfinitiveWordForm()
{
    CGramHasher hasherInf(POS_VERB, SUBPARADIGM_INFINITIVE, CASE_UNDEFINED, NUM_UNDEFINED,
        GENDER_UNDEFINED, PERSON_UNDEFINED, ANIM_UNDEFINED, m_pLexeme->eAspect(),
        m_pLexeme->eIsReflexive());

    ET_ReturnCode eRet = m_pInflection->eWordFormFromHash(hasherInf.sGramHash(), 0, m_pInfWordForm);

    if (eRet != H_NO_ERROR)
    {
        m_pInfWordForm = nullptr;
        return eRet;
    }

    if (nullptr == m_pInfWordForm)
    {
        return H_ERROR_POINTER;
    }

    return H_NO_ERROR;

}       // eGetInfinitiveWordForm()

ET_ReturnCode CFormBuilderAspectPair::eGetInfinitiveStressPos(int& iStressPos)
{
    ET_ReturnCode eRet = H_NO_ERROR;
    iStressPos = -1;

    if (nullptr == m_pInfWordForm)
    {
        eRet = eGetInfinitiveWordForm();
        if (eRet != H_NO_ERROR)
        {
            return eRet;
        }
    }

//    int iInfStressPos = -1;
    ET_StressType eType;
    eRet = m_pInfWordForm->eGetFirstStressPos(iStressPos, eType);
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    while (eType != ET_StressType::STRESS_PRIMARY && H_NO_ERROR == eRet)
    {
        eRet = m_pInfWordForm->eGetNextStressPos(iStressPos, eType);
    }

    return eRet;

}       //  eGetInfinitiveStressPos()

ET_ReturnCode CFormBuilderAspectPair::eGetNsvDerivationType(CWordForm* spInfinitive, int& iType)
{
    iType = 0;

    if (nullptr == spInfinitive)
    {
        return H_ERROR_POINTER;
    }

    CEString sInfinitive = spInfinitive->sWordForm();
    
    int iInfStressPos = -1;
    ET_ReturnCode eRet = eGetInfinitiveStressPos(iInfStressPos);
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    if (iInfStressPos < 0)
    {
        return H_ERROR_UNEXPECTED;
    }

    if (sInfinitive.bEndsWith(L"ся"))
    {
        sInfinitive.sRemoveCharsFromEnd(2);
    }

    if (sInfinitive.bEndsWith(L"ивать") || sInfinitive.bEndsWith(L"ывать"))
    {
        try
        {
            int iStressedSyllable = spInfinitive->sWordForm().uiGetSyllableFromVowelPos(iInfStressPos);
            if (iStressedSyllable < spInfinitive->sWordForm().uiNSyllables()-2)
            {
                iType = -1;
                return H_NO_ERROR;
            }
        }
        catch (CException& ex)
        {
            ERROR_LOG(ex.szGetDescription());
            return H_ERROR_UNEXPECTED;
        }
    }

    if ((sInfinitive.bEndsWith(L"ать") && !sInfinitive.bEndsWith(L"вать")) || sInfinitive.bEndsWith(L"ять"))
    {
        int iStressedSyllable = spInfinitive->sWordForm().uiGetSyllableFromVowelPos(iInfStressPos);
        assert(iStressedSyllable > 0);
        if (sInfinitive.uiNSyllables()-1 == iStressedSyllable) // end-stressed?
        {
            iType = -2;
            return H_NO_ERROR;
        }
    }

    if (sInfinitive.bEndsWith(L"вать"))
    {
        int iStressedSyllable = spInfinitive->sWordForm().uiGetSyllableFromVowelPos(iInfStressPos);
        assert(iStressedSyllable > 0);
        if (sInfinitive.uiNSyllables()-1 == iStressedSyllable) // end-stressed?
        {
            iType = -3;
            return H_NO_ERROR;
        }
    }

    return H_ERROR_UNEXPECTED;

}       //  eGetNsvDerivationType()

////////////////////////////////////////////////////////////////////////////////////////////////

ET_ReturnCode CFormBuilderAspectPair::eSvToNsvTypeI(bool bIsVariant)
{
    ET_ReturnCode eRet = H_NO_ERROR;

    CEString& sAspectPair = bIsVariant ? m_sAltAspectPairSource : m_sAspectPairSource;
//    ET_ExtraData eExtraData = bIsVariant ? m_eAltExtraData : m_eExtraData;    
    CEString sData;

    if (m_sSource.bEndsWith(L"ать") || m_sSource.bEndsWith(L"оть"))
    {
        CEString sStem(m_sSource);
        sStem.sRemoveCharsFromEnd(3);
        eRet = eBuildTypeIa(sStem, sAspectPair);
        return eRet;
    }
    else if (m_sSource.bEndsWith(L"нуть") && !m_sSource.bEndsWith(L"ьнуть"))
    {
        CEString sStem(m_sSource);
        sStem.sRemoveCharsFromEnd(4);
        eRet = eBuildTypeIa(sStem, sAspectPair);
        return eRet;
    }

    if (m_sSource.bEndsWith(L"ять"))
    {
        CEString sStem(m_sSource);
        sStem.sRemoveCharsFromEnd(3);
        eRet = eBuildTypeIb(sStem, sAspectPair);
        return eRet;
    }
    else if (m_sSource.bEndsWith(L"ьнуть"))
    {
        CEString sStem(m_sSource);
        sStem.sRemoveCharsFromEnd(5);
        eRet = eBuildTypeIb(sStem, sAspectPair);
        return eRet;
    }

    // All other stem types:
    eRet = eBuildTypeIc(sAspectPair);

    return eRet;

}       // eSvToNsvTypeI()

ET_ReturnCode CFormBuilderAspectPair::eBuildTypeIa(const CEString& sStem, CEString& sOutput)
{
    if (CEString::bIn(sStem[sStem.uiLength() - 1], CEString(CEString::g_szRusHushers) + CEString(L"кгх")))
    {
        sOutput = sStem + L"ивать";
    }
    else
    {
        sOutput = sStem + L"ывать";
    }

    return H_NO_ERROR;
}

ET_ReturnCode CFormBuilderAspectPair::eBuildTypeIb(const CEString& sStem, CEString& sOutput)
{
    sOutput = sStem + L"ивать";
    return H_NO_ERROR;
}

ET_ReturnCode CFormBuilderAspectPair::eBuildTypeIc(CEString& sOutput)
{
    ET_ReturnCode eRet = eGet1PersonWordForm();
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }
    
    if (nullptr == m_p1stPersonWordForm)
    {
        return H_ERROR_POINTER;
    }

    CEString sEnding(m_p1stPersonWordForm->sEnding());
    if (sEnding.bEndsWith(L"сь"))
    {
        sEnding.sRemoveCharsFromEnd(2);
    }

    if (sEnding.bEndsWith(L"у"))
    {
        if (m_p1stPersonWordForm->sStem().bEndsWithOneOf(CEString(CEString::g_szRusHushers) + CEString(L"кгх")))
        {
            sOutput = m_p1stPersonWordForm->sStem() + L"ивать";
        }
        else
        {
            sOutput = m_p1stPersonWordForm->sStem() + L"ывать";
        }
    }
    else if (sEnding.bEndsWith(L"ю"))
    {
        sOutput = m_p1stPersonWordForm->sStem() + L"ивать";
    }

    return H_NO_ERROR;

}       //  eBuildTypeIc()

ET_ReturnCode CFormBuilderAspectPair::eFindStressPositionI(bool bIsVariant)
{
    ET_ReturnCode eRet = eGetInfinitiveWordForm();
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    int iSourceStressPos = -1;
    eRet = eGetInfinitiveStressPos(iSourceStressPos);
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    try
    {
        int& iStressPos = bIsVariant ? m_iAltStressPos : m_iStressPos;
        int iStressedSyllable = m_pInfWordForm->sWordForm().uiGetSyllableFromVowelPos(iSourceStressPos);
        if (m_pInfWordForm->sWordForm().bStartsWith(L"вы") ||
            m_sSource.uiNSyllables()-1 == iStressedSyllable)
        {
            CEString& sAspectPair = bIsVariant ? m_sAltAspectPairSource : m_sAspectPairSource;
            assert(sAspectPair.bEndsWith(L"ивать") || sAspectPair.bEndsWith(L"ывать") ||
                   sAspectPair.bEndsWith(L"иваться") || sAspectPair.bEndsWith(L"ываться"));
            iStressPos = sAspectPair.uiGetVowelPos(sAspectPair.uiNSyllables() - 3);
            if (L'е' == sAspectPair[iStressPos])
            {
                if (m_pLexeme->bHasYoAlternation())
                {
                    sAspectPair[iStressPos] = L'ё';
                }
                else if (m_pLexeme->bHasOAlternation())
                {
                    sAspectPair[iStressPos] = L'о';
                }
            }
        }
        else
        {
            iStressPos = iSourceStressPos;
        }
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_ERROR_UNEXPECTED;
    }

    return H_NO_ERROR;

}       //  eFindStressPositionI()

ET_ReturnCode CFormBuilderAspectPair::eExtractStressMark(bool bIsVariant)
{
    CEString& sAspectPair = bIsVariant ? m_sAltAspectPairSource : m_sAspectPairSource;
//    ET_ExtraData eExtraData = bIsVariant ? m_eAltExtraData : m_eExtraData;
    int& iStressPos = bIsVariant ? m_iAltStressPos : m_iStressPos;

    unsigned int uiPos = sAspectPair.uiFind(L"/");
    if (uiPos != ERelation::ecNotFound)
    {
        iStressPos = uiPos;
        sAspectPair = sAspectPair.sSubstr(0, uiPos) + sAspectPair.sSubstr(uiPos+1);
        auto uiYoPos = sAspectPair.uiFind(L"ё");            // needs review
        if (uiYoPos != ERelation::ecNotFound)
        {
            if ((int)uiYoPos != iStressPos)
            {
                if (uiYoPos >= sAspectPair.uiLength())
                {
                    return H_ERROR_UNEXPECTED;
                }
                if (L'ё' == sAspectPair[uiYoPos])
                {
                    sAspectPair[uiYoPos] = L'е';
                }
            }
        }
    }
    else
    {
        auto uiYoPos = sAspectPair.uiFind(L"ё");
        if (uiYoPos != ERelation::ecNotFound)
        {
            iStressPos = (int)uiYoPos;
        }
        else
        {
            if (1 == sAspectPair.uiNSyllables())
            {
                iStressPos = sAspectPair.uiGetVowelPos(0);
            }
            else
            {
                return H_ERROR_UNEXPECTED;
            }
        }
    }

    return H_NO_ERROR;
}

ET_ReturnCode CFormBuilderAspectPair::eApplyNsvVowelModification(bool bIsVariant)
{
    StLexemeProperties stProperties = m_pLexeme->stGetProperties();
    if (!stProperties.bHasAspectPair)
    {
        return H_ERROR_UNEXPECTED;
    }

    CEString& sData = bIsVariant ? stProperties.sAltAspectPairData : stProperties.sAspectPairData;
    if (sData.bIsEmpty())
    {
        return H_ERROR_UNEXPECTED;
    }

    if (!sData.bStartsWith(L"-") || !sData.bEndsWith(L"-") || sData.uiLength() > 4 || sData.uiLength() < 3)
    {
        return H_ERROR_UNEXPECTED;
    }

    sData = sData.sSubstr(1, sData.uiLength()-2);

//    bool bStressed = false;
    if (sData.bStartsWith(L"/"))
    {
//        bStressed = true;
        sData = sData.sSubstr(1);
    }

    // assert(bStressed) ???

    if (sData.uiLength() != 1)
    {
        return H_ERROR_UNEXPECTED;
    }

    if (sData != L'ё' && sData != L'о' && sData != L'а')
    {
        return H_ERROR_UNEXPECTED;
    }

    CEString& sAspectPair = bIsVariant ? m_sAltAspectPairSource : m_sAspectPairSource;
    int& iStressPos = bIsVariant ? m_iAltStressPos : m_iStressPos;
    try
    {
        sAspectPair[iStressPos] = sData[0];
    }
    catch (...)
    {
        return H_ERROR_UNEXPECTED;
    }

    return H_NO_ERROR;

}       //  eApplyNsvVowelModification()

ET_ReturnCode CFormBuilderAspectPair::eApplySvVowelModification(bool bIsVariant)
{    
    StLexemeProperties stProperties = m_pLexeme->stGetProperties();
    if (!stProperties.bHasAspectPair)
    {
        return H_ERROR_UNEXPECTED;
    }

    CEString& sData = bIsVariant ? stProperties.sAltAspectPairData : stProperties.sAspectPairData;
    if (sData.bIsEmpty())
    {
        return H_ERROR_UNEXPECTED;
    }

    if (!sData.bStartsWith(L"-") || !sData.bEndsWith(L"-") || sData.uiLength() > 4 || sData.uiLength() < 3)
    {
        return H_ERROR_UNEXPECTED;
    }

    sData = sData.sSubstr(1, sData.uiLength()-2);

    bool bStressed = false;
    if (sData.bStartsWith(L"/"))
    {
        bStressed = true;
        sData = sData.sSubstr(1);
    }

    // assert(bStressed) ???

    if (sData.uiLength() != 1)
    {
        return H_ERROR_UNEXPECTED;
    }

    if (sData != L'ё' && sData != L'о' && sData != L'а')
    {
        return H_ERROR_UNEXPECTED;
    }

    CEString& sAspectPair = bIsVariant ? m_sAltAspectPairSource : m_sAspectPairSource;
    int iStressPos = bIsVariant ? m_iAltStressPos : m_iStressPos;

    vector<CEString> vecEndSegments = { CEString(L"ать"), CEString(L"ять"), CEString(L"еть"), CEString(L"ить"), CEString(L"нуть"), CEString(L"чь") };

    bool bFound = false;
    try
    {
        for (auto& sEnd : vecEndSegments)
        {
            if (sAspectPair.bEndsWith(sEnd))
            {
                bFound = true;

                CEString sCopy(sAspectPair);
                sCopy.sRemoveCharsFromEnd(sEnd.uiLength());
                auto uiVPos = sCopy.uiFindLastOf(L"ае");
                if (ERelation::ecNotFound == uiVPos)
                {
                    return H_ERROR_UNEXPECTED;
                }

                if (L'е' == sAspectPair[uiVPos])
                {
                    if (iStressPos != (int)uiVPos)
                    {
                        return H_ERROR_UNEXPECTED;
                    }
                }

                sAspectPair[uiVPos] = sData[0];
                if (bStressed)
                {
                    iStressPos = (int)uiVPos;
                }

                break;
            }
        }
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }
    catch (...)
    {
        ERROR_LOG(L"Exception while applying vowel modification.");
        return H_EXCEPTION;
    }

    if (!bFound)
    {
        return H_ERROR_UNEXPECTED;
    }

    return H_NO_ERROR;

}       //  eApplySvVowelModification(bool bIsVariant)

ET_ReturnCode CFormBuilderAspectPair::eApplySvSuffix(bool bIsVariant)
{
    StLexemeProperties stProperties = m_pLexeme->stGetProperties();
    if (!stProperties.bHasAspectPair)
    {
        return H_ERROR_UNEXPECTED;
    }

    CEString& sData = bIsVariant ? stProperties.sAltAspectPairData : stProperties.sAspectPairData;
    if (sData.bIsEmpty())
    {
        return H_ERROR_UNEXPECTED;
    }

    if (!(sData.bStartsWith(L"-") && !sData.bEndsWith(L"-")))
    {
        return H_ERROR_UNEXPECTED;
    }

    sData = sData.sSubstr(1, sData.uiLength()-1);
    
    if (REFL_YES == stProperties.eReflexive)
    {
        if (!sData.bEndsWith(L"ся") && !sData.bEndsWith(L"сь"))
        {
            return H_ERROR_UNEXPECTED;
        }

        sData.sRemoveCharsFromEnd(2);
    }

    CEString& sAspectPair = bIsVariant ? m_sAltAspectPairSource : m_sAspectPairSource;
//    int iStressPos = bIsVariant ? m_iAltStressPos : m_iStressPos;

    vector<CEString> vecEndSegments = { CEString(L"бить"), CEString(L"пить"), CEString(L"вить"), CEString(L"фить"), CEString(L"мить"),
                                        CEString(L"б/ить"), CEString(L"п/ить"), CEString(L"в/ить"), CEString(L"ф/ить"), CEString(L"м/ить") };

    bool bFound = false;
    for (auto& sEnd : vecEndSegments)
    {
        if (sData.bEndsWith(sEnd))
        {
            bFound = true;
            break;
        }
    }

    int iExtraCharsToTruncate = 0;
    if (bFound)
    {
        iExtraCharsToTruncate = 1;
    }
    else if (sData.bEndsWith(L"стить") || sData.bEndsWith(L"ст/ить") || sData.bEndsWith(L"остить") ||
        sData.bEndsWith(L"/остить") || sData.bEndsWith(L"ост/ить"))
    {
        iExtraCharsToTruncate = -1;
    }
    else if (m_sSource.bEndsWith(L"ждать"))
    {
        int iInfStressPos = -1;
        ET_ReturnCode eRet = eGetInfinitiveStressPos(iInfStressPos);
        if (eRet != H_NO_ERROR || iInfStressPos < 0)
        {
            return eRet;
        }

        if (iInfStressPos == (int)m_sSource.uiLength() - 3)
        {
            iExtraCharsToTruncate = 1;
        }
    }

    if (CEString::bIn (L'/', sData))
    {
        --iExtraCharsToTruncate;
    }

    int iCharsToTruncate = sData.uiLength() + iExtraCharsToTruncate;
    if (iCharsToTruncate > (int)sAspectPair.uiLength())
    {
        ERROR_LOG(L"Error truncating aspect suffix.");
        return H_ERROR_UNEXPECTED;
    }

    sAspectPair.sRemoveCharsFromEnd(iCharsToTruncate);
    sAspectPair += sData;

    auto eRet = eExtractStressMark(bIsVariant);

    return eRet;

}       //  eApplySvSuffix()

ET_ReturnCode CFormBuilderAspectPair::eApplySvWholeWord([[maybe_unused]]bool bIsVariant)
{
    // TODO why is this not implemented?
    return H_NO_ERROR;
}


ET_ReturnCode CFormBuilderAspectPair::eSvToNsvTypeII(bool bIsVariant)
{
    ET_ReturnCode eRet = H_NO_ERROR;

    CEString& sOutput = bIsVariant ? m_sAltAspectPairSource : m_sAspectPairSource;
    ET_ExtraData eExtraData = bIsVariant ? m_eAltExtraData : m_eExtraData;
    if (ET_ExtraData::WholeWord == eExtraData)
    {
        StLexemeProperties stProperties = m_pLexeme->stGetProperties();
        CEString sData = bIsVariant ? stProperties.sAltAspectPairData : stProperties.sAspectPairData;
        if (!stProperties.bHasAspectPair || sData.bIsEmpty())
        {
            return H_ERROR_UNEXPECTED;
        }

        sOutput = stProperties.sAspectPairData;

        return H_NO_ERROR;
    }

    if (m_sSource.bEndsWith(L"ать"))
    {
        sOutput = m_sSource;
    }
    else if (m_sSource.bEndsWith(L"нуть"))
    {
        sOutput = m_sSource;
        sOutput = sOutput.sRemoveCharsFromEnd(4) + L"ать";
    }
    else
    {
        eRet = eGet1PersonWordForm();
        if (eRet != H_NO_ERROR)
        {
            return eRet;
        }

        if (nullptr == m_p1stPersonWordForm)
        {
            return H_ERROR_POINTER;
        }

        CEString sEnding(m_p1stPersonWordForm->sEnding());
        if (sEnding.bEndsWith(L"сь"))
        {
            sEnding.sRemoveCharsFromEnd(2);
        }

        if (sEnding.bEndsWith(L"у"))
        {
            sOutput = m_p1stPersonWordForm->sStem() + L"ать";
        }
        else
        {
            assert(sEnding == L"ю");
            sOutput = m_p1stPersonWordForm->sStem() + L"ять";
        }
    }

    int& iStressPos = bIsVariant ? m_iAltStressPos : m_iStressPos;
    iStressPos = sOutput.uiGetVowelPos(sOutput.uiNSyllables()-1);

    return eRet;

}       //  eSvToNsvTypeII()

ET_ReturnCode CFormBuilderAspectPair::eSvToNsvTypeIII(bool bIsVariant)
{
    CEString& sOutput = bIsVariant ? m_sAltAspectPairSource : m_sAspectPairSource;
    ET_ExtraData eExtraData = bIsVariant ? m_eAltExtraData : m_eExtraData;
    if (ET_ExtraData::WholeWord == eExtraData)        
    {
        StLexemeProperties stProperties = m_pLexeme->stGetProperties();
        CEString sData = bIsVariant ? stProperties.sAltAspectPairData : stProperties.sAspectPairData;
        if (!stProperties.bHasAspectPair || sData.bIsEmpty())
        {
            return H_ERROR_UNEXPECTED;
        }

        sOutput = stProperties.sAspectPairData;

        return H_NO_ERROR;
    }

    if (m_sSource.bEndsWith(L"нуть"))
    {
        sOutput = m_sSource;
        sOutput = sOutput.sRemoveCharsFromEnd(4) + L"вать";
    }
    else
    {
        sOutput = m_sSource;
        sOutput = sOutput.sRemoveCharsFromEnd(2) + L"вать";
    }

    int& iStressPos = bIsVariant ? m_iAltStressPos : m_iStressPos;
    iStressPos = sOutput.uiGetVowelPos(sOutput.uiNSyllables()-1);

    return H_NO_ERROR;

}       //  eSvToNsvTypeIII()

ET_ReturnCode CFormBuilderAspectPair::eNsvToSvTypes1_5_6(bool bIsVariant)
{
    ET_ReturnCode eRet = H_NO_ERROR;
    CEString& sAspectPair = bIsVariant ? m_sAltAspectPairSource : m_sAspectPairSource;
    int& iStressPos = bIsVariant ? m_iAltStressPos : m_iStressPos;

    if (nullptr == m_pInfWordForm)
    {
        eRet = eGetInfinitiveWordForm();
        if (eRet != H_NO_ERROR)
        {
            return eRet;
        }
    }

    int iInfStressPos = -1;
    eRet = eGetInfinitiveStressPos(iInfStressPos);
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    if (iInfStressPos < 0)
    {
        return H_ERROR_UNEXPECTED;
    }

    int iStressedSyllable = -1;
    try
    {
        iStressedSyllable = m_pInfWordForm->sWordForm().uiGetSyllableFromVowelPos(iInfStressPos);
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_ERROR_UNEXPECTED;
    }

    CEString sInfinitive = m_pInfWordForm->sWordForm();
    if (sInfinitive.bEndsWith(L"ся"))
    {
        sInfinitive.sRemoveCharsFromEnd(2);
    }

    int iDerivationType = 0;
    eRet = eGetNsvDerivationType(m_pInfWordForm, iDerivationType);
    if (eRet != H_NO_ERROR || iDerivationType < -3 || iDerivationType > -1)
    {
        return H_ERROR_UNEXPECTED;
    }

    if (-1 == iDerivationType)      //  unstressed -yvat'/-ivat'
    {
        if (sInfinitive.bEndsWith(L"ывать"))
        {
            sAspectPair = sInfinitive;
            sAspectPair = sAspectPair.sRemoveCharsFromEnd(5) + L"ать";
        }
        else
        {
            assert(sInfinitive.bEndsWith(L"ивать"));
            CEString sStem(sInfinitive);
            sStem.sRemoveCharsFromEnd(5);
            if (CEString::bIn(sStem[sStem.uiLength() - 1], CEString(CEString::g_szRusHushers) + CEString(L"кгх")))
            {
                sAspectPair = sStem + L"ать";
            }
            else
            {
                sAspectPair = sStem + L"ять";
            }
        }

        iStressPos = sAspectPair.uiGetVowelPos(iStressedSyllable);
        return H_NO_ERROR;

    }           //  unstressed -yvat'/-ivat'

    if (-2 == iDerivationType)
    {
        int iStressedSyllable = m_pInfWordForm->sWordForm().uiGetSyllableFromVowelPos(iInfStressPos);
        assert(iStressedSyllable > 0);
        if (sInfinitive.uiNSyllables()-1 == iStressedSyllable) // end-stressed?
        {
            sAspectPair = sInfinitive;
            iStressPos = sAspectPair.uiGetVowelPos(iStressedSyllable-1);
            return H_NO_ERROR;
        }
    }

    if (sInfinitive.bEndsWith(L"вать"))
    {
        int iStressedSyllable = m_pInfWordForm->sWordForm().uiGetSyllableFromVowelPos(iInfStressPos);
        assert(iStressedSyllable > 0);
        if (m_pInfWordForm->sWordForm().uiNSyllables()-1 == iStressedSyllable) // end-stressed?
        {
            sAspectPair = sInfinitive.sRemoveCharsFromEnd(4) + L"ть";
            iStressPos = sAspectPair.uiGetVowelPos(iStressedSyllable-1);
            return H_NO_ERROR;
        }
    }

    return H_ERROR_UNEXPECTED;

}       //  eNsvToSvTypes1_5_6()

ET_ReturnCode CFormBuilderAspectPair::eNsvToSvTypes2(bool bIsVariant)
{
    ET_ReturnCode eRet = H_NO_ERROR;
    CEString& sAspectPair = bIsVariant ? m_sAltAspectPairSource : m_sAspectPairSource;
    int& iStressPos = bIsVariant ? m_iAltStressPos : m_iStressPos;

    if (nullptr == m_pInfWordForm)
    {
        eRet = eGetInfinitiveWordForm();
        if (eRet != H_NO_ERROR)
        {
            return eRet;
        }
    }

    int iInfStressPos = -1;
    eRet = eGetInfinitiveStressPos(iInfStressPos);
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    if (iInfStressPos < 0)
    {
        return H_ERROR_UNEXPECTED;
    }

    int iDerivationType = 0;
    eRet = eGetNsvDerivationType(m_pInfWordForm, iDerivationType);
    if (eRet != H_NO_ERROR || iDerivationType < -3 || iDerivationType > -1)
    {
        return H_ERROR_UNEXPECTED;
    }

    if (-1 == iDerivationType)      //  unstressed -yvat'/-ivat'
    {
        CEString sInfinitive = m_pInfWordForm->sWordForm();
        if (sInfinitive.bEndsWith(L"ся"))
        {
            sInfinitive.sRemoveCharsFromEnd(2);
        }

        if (sInfinitive.bEndsWith(L"овывать") && !sInfinitive.bEndsWith(L"цовывать"))
        {
            int iStressedSyllable = m_pInfWordForm->sWordForm().uiGetSyllableFromVowelPos(iInfStressPos);
//            assert(iStressedSyllable > 0);
            if (m_pInfWordForm->sWordForm().uiNSyllables() - 3 == iStressedSyllable) // -ovyvat'
            {
                sAspectPair = m_pInfWordForm->sWordForm().sRemoveCharsFromEnd(7);
                sAspectPair += L"овать";
                iStressPos = sAspectPair.uiGetVowelPos(sAspectPair.uiNSyllables()-1);
                return H_NO_ERROR;
            }
        }

        if (sInfinitive.bEndsWith(L"цовывать"))
        {
            int iStressedSyllable = m_pInfWordForm->sWordForm().uiGetSyllableFromVowelPos(iInfStressPos);
            assert(iStressedSyllable > 0);
            if (m_pInfWordForm->sWordForm().uiNSyllables() - 3 == iStressedSyllable)
            {
                sAspectPair = m_pInfWordForm->sWordForm().sRemoveCharsFromEnd(8);
                sAspectPair += L"цевать";
                iStressPos = sAspectPair.uiGetVowelPos(sAspectPair.uiNSyllables()-1);
                return H_NO_ERROR;
            }
        }

        if (sInfinitive.bEndsWith(L"ёвывать"))
        {
            sAspectPair = m_pInfWordForm->sWordForm().sRemoveCharsFromEnd(7);
            sAspectPair += L"евать";
            iStressPos = sAspectPair.uiGetVowelPos(sAspectPair.uiNSyllables()-1);
            return H_NO_ERROR;
        }
    }

    return H_ERROR_UNEXPECTED;

}       //  eNsvToSvTypes2()

ET_ReturnCode CFormBuilderAspectPair::eNsvToSvTypes3(bool bIsVariant)
{
    ET_ReturnCode eRet = H_NO_ERROR;
    CEString& sAspectPair = bIsVariant ? m_sAltAspectPairSource : m_sAspectPairSource;
    int& iStressPos = bIsVariant ? m_iAltStressPos : m_iStressPos;

    if (nullptr == m_pInfWordForm)
    {
        eRet = eGetInfinitiveWordForm();
        if (eRet != H_NO_ERROR)
        {
            return eRet;
        }
    }

    int iInfStressPos = -1;
    eRet = eGetInfinitiveStressPos(iInfStressPos);
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    if (iInfStressPos < 0)
    {
        return H_ERROR_UNEXPECTED;
    }

    int iDerivationType = 0;
    eRet = eGetNsvDerivationType(m_pInfWordForm, iDerivationType);
    if (eRet != H_NO_ERROR || iDerivationType < -3 || iDerivationType > -1)
    {
        return H_ERROR_UNEXPECTED;
    }

    if (-1 == iDerivationType)      //  unstressed -yvat'/-ivat'
    {
        CEString sInfinitive = m_pInfWordForm->sWordForm();
        if (sInfinitive.bEndsWith(L"ся"))
        {
            sInfinitive.sRemoveCharsFromEnd(2);
        }

        if (sInfinitive.bEndsWith(L"ивать") || sInfinitive.bEndsWith(L"ывать"))
        {
            try
            {
                int iStressedSyllable = m_pInfWordForm->sWordForm().uiGetSyllableFromVowelPos(iInfStressPos);
                if (iStressedSyllable < m_pInfWordForm->sWordForm().uiNSyllables() - 2)
                {
                    CEString sStem(sInfinitive);
                    sStem.sRemoveCharsFromEnd(5);
                    sAspectPair = sStem + L"нуть";
                    iStressPos = sAspectPair.uiGetVowelPos(iStressedSyllable);
                }

                return H_NO_ERROR;
            }
            catch (CException& ex)
            {
                ERROR_LOG(ex.szGetDescription());
                return H_ERROR_UNEXPECTED;
            }
        }           //  unstressed -yvat'/-ivat'
    }

    if (-2 == iDerivationType)
    {
        CEString sInfinitive = m_pInfWordForm->sWordForm();
        int iStressedSyllable = m_pInfWordForm->sWordForm().uiGetSyllableFromVowelPos(iInfStressPos);
        if (m_pInfWordForm->sWordForm().uiNSyllables()-1 == iStressedSyllable) // end-stressed?
        {
            CEString sStem(sInfinitive);
            sStem.sRemoveCharsFromEnd(3);
            sAspectPair = sStem + L"нуть";
            iStressedSyllable = m_pInfWordForm->sWordForm().uiNSyllables()-2;
//            assert(iStressedSyllable > 0);
            iStressPos = sAspectPair.uiGetVowelPos(iStressedSyllable);
            return H_NO_ERROR;
        }
    }

    if (-3 == iDerivationType)
    {
        int iStressedSyllable = m_pInfWordForm->sWordForm().uiGetSyllableFromVowelPos(iInfStressPos);
        assert(iStressedSyllable > 0);
        if (m_pInfWordForm->sWordForm().uiNSyllables() - 1 == iStressedSyllable) // end-stressed?
        {
            sAspectPair = m_pInfWordForm->sWordForm().sRemoveCharsFromEnd(4) + L"ть";
            iStressPos = sAspectPair.uiGetVowelPos(iStressedSyllable - 1);
            return H_NO_ERROR;
        }
    }

    return H_ERROR_UNEXPECTED;

}       //  eNsvToSvTypes3()

ET_ReturnCode CFormBuilderAspectPair::eNsvToSvTypes4(bool bIsVariant)
{
    ET_ReturnCode eRet = H_NO_ERROR;
    CEString& sAspectPair = bIsVariant ? m_sAltAspectPairSource : m_sAspectPairSource;
    int& iStressPos = bIsVariant ? m_iAltStressPos : m_iStressPos;

    if (nullptr == m_pInfWordForm)
    {
        eRet = eGetInfinitiveWordForm();
        if (eRet != H_NO_ERROR)
        {
            return eRet;
        }
    }

    int iInfStressPos = -1;
    eRet = eGetInfinitiveStressPos(iInfStressPos);
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    if (iInfStressPos < 0)
    {
        return H_ERROR_UNEXPECTED;
    }

    int iDerivationType = 0;
    eRet = eGetNsvDerivationType(m_pInfWordForm, iDerivationType);
    if (eRet != H_NO_ERROR || iDerivationType < -3 || iDerivationType > -1)
    {
        return H_ERROR_UNEXPECTED;
    }
    
    CEString sInfinitive = m_pInfWordForm->sWordForm();
    if (sInfinitive.bEndsWith(L"ся"))
    {
        sInfinitive.sRemoveCharsFromEnd(2);
    }

    int iStressedSyllable = m_pInfWordForm->sWordForm().uiGetSyllableFromVowelPos(iInfStressPos);
    assert(iStressedSyllable >= 0);

    if (-1 == iDerivationType)
    {
        CEString sStem(sInfinitive);
        sStem.sRemoveCharsFromEnd(5);
        sAspectPair = sStem + L"ить";
        iStressPos = sAspectPair.uiGetVowelPos(iStressedSyllable);
        return H_NO_ERROR;
    }           //  unstressed -yvat'/-ivat'

    if (-2 == iDerivationType)
    {
        CEString sStem(sInfinitive);
        sStem.sRemoveCharsFromEnd(3);
        sAspectPair = sStem + L"ить";
        iStressedSyllable = m_pInfWordForm->sWordForm().uiNSyllables()-2;
//        assert(iStressedSyllable > 0);
        iStressPos = sAspectPair.uiGetVowelPos(iStressedSyllable);

        return H_NO_ERROR;
    }

    return H_ERROR_UNEXPECTED;

}       //  eNsvToSvTypes4()

ET_ReturnCode CFormBuilderAspectPair::eNsvToSvTypes7(bool bIsVariant)
{
    ET_ReturnCode eRet = H_NO_ERROR;
    CEString& sAspectPair = bIsVariant ? m_sAltAspectPairSource : m_sAspectPairSource;
//    int iStressPos = bIsVariant ? m_iAltStressPos : m_iStressPos;

    if (nullptr == m_pInfWordForm)
    {
        eRet = eGetInfinitiveWordForm();
        if (eRet != H_NO_ERROR)
        {
            return eRet;
        }
    }

    int iInfStressPos = -1;
    eRet = eGetInfinitiveStressPos(iInfStressPos);
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    if (iInfStressPos < 0)
    {
        return H_ERROR_UNEXPECTED;
    }

    int iDerivationType = 0;
    eRet = eGetNsvDerivationType(m_pInfWordForm, iDerivationType);
    if (eRet != H_NO_ERROR || iDerivationType < -3 || iDerivationType > -1)
    {
        return H_ERROR_UNEXPECTED;
    }

//    int iStressedSyllable = m_pInfWordForm->sWordForm().uiGetSyllableFromVowelPos(iInfStressPos);
    CEString&& sInfinitive = m_pInfWordForm->sWordForm();
    if (sInfinitive.bEndsWith(L"ся"))
    {
        sInfinitive.sRemoveCharsFromEnd(2);
    }

    if (-1 == iDerivationType)
    {
        if (sInfinitive.bEndsWith(L"зывать"))
        {
            assert(sInfinitive.uiLength() > 6);
            sAspectPair = sInfinitive;
            sAspectPair = sAspectPair.sRemoveCharsFromEnd(6) + L"зть";
//            iStressPos = sAspectPair.uiGetVowelPos(iStressedSyllable);
            return H_NO_ERROR;
        }

        if (sInfinitive.bEndsWith(L"тывать") || sInfinitive.bEndsWith(L"дывать") || sInfinitive.bEndsWith(L"сывать") || sInfinitive.bEndsWith(L"бывать"))
        {
            assert(sInfinitive.uiLength() > 6);
            sAspectPair = sInfinitive;
            sAspectPair = sAspectPair.sRemoveCharsFromEnd(6) + L"сть";
//            iStressPos = sAspectPair.uiGetVowelPos(iStressedSyllable);
            return H_NO_ERROR;
        }
    }

    if (-2 == iDerivationType)
    {
        if (sInfinitive.bEndsWith(L"зать"))
        {
            assert(sInfinitive.uiLength() > 4);
            sAspectPair = sInfinitive;
            sAspectPair = sAspectPair.sRemoveCharsFromEnd(4) + L"зть";
//            iStressPos = sAspectPair.uiGetVowelPos(iStressedSyllable-1);
            return H_NO_ERROR;
        }

        if (sInfinitive.bEndsWith(L"тать") || sInfinitive.bEndsWith(L"дать") || sInfinitive.bEndsWith(L"сать") || sInfinitive.bEndsWith(L"бать"))
        {
            assert(sInfinitive.uiLength() > 4);
            sAspectPair = sInfinitive;
            sAspectPair = sAspectPair.sRemoveCharsFromEnd(4) + L"сть";
//            iStressPos = sAspectPair.uiGetVowelPos(iStressedSyllable-1);
            return H_NO_ERROR;
        }
    }

    return H_ERROR_UNEXPECTED;

}       //  eNsvToSvTypes7()

ET_ReturnCode CFormBuilderAspectPair::eNsvToSvTypes8(bool bIsVariant)
{
    ET_ReturnCode eRet = H_NO_ERROR;
    CEString& sAspectPair = bIsVariant ? m_sAltAspectPairSource : m_sAspectPairSource;
    int& iStressPos = bIsVariant ? m_iAltStressPos : m_iStressPos;

    if (nullptr == m_pInfWordForm)
    {
        eRet = eGetInfinitiveWordForm();
        if (eRet != H_NO_ERROR)
        {
            return eRet;
        }
    }

    int iInfStressPos = -1;
    eRet = eGetInfinitiveStressPos(iInfStressPos);
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    if (iInfStressPos < 0)
    {
        return H_ERROR_UNEXPECTED;
    }

    int iDerivationType = 0;
    eRet = eGetNsvDerivationType(m_pInfWordForm, iDerivationType);
    if (eRet != H_NO_ERROR || iDerivationType < -3 || iDerivationType > -1)
    {
        return H_ERROR_UNEXPECTED;
    }

    int iStressedSyllable = m_pInfWordForm->sWordForm().uiGetSyllableFromVowelPos(iInfStressPos);
    CEString sInfinitive = m_pInfWordForm->sWordForm();
    if (sInfinitive.bEndsWith(L"ся"))
    {
        sInfinitive.sRemoveCharsFromEnd(2);
    }

    if (-1 == iDerivationType)
    {
        if (sInfinitive.bEndsWith(L"кивать") || sInfinitive.bEndsWith(L"гивать"))
        {
            assert(sInfinitive.uiLength() > 6);
            sAspectPair = sInfinitive;
            sAspectPair = sAspectPair.sRemoveCharsFromEnd(6) + L"чь";
            iStressPos = sAspectPair.uiGetVowelPos(iStressedSyllable);
            return H_NO_ERROR;
        }
    }

    if (-2 == iDerivationType)
    {
        if (sInfinitive.bEndsWith(L"кать") || sInfinitive.bEndsWith(L"гать"))
        {
            assert(sInfinitive.uiLength() > 4);
            sAspectPair = sInfinitive;
            sAspectPair = sAspectPair.sRemoveCharsFromEnd(4) + L"чь";
            iStressPos = sAspectPair.uiGetVowelPos(iStressedSyllable-1);
            return H_NO_ERROR;
        }
    }

    return H_ERROR_UNEXPECTED;

}       //  eNsvToSvTypes8()

ET_ReturnCode CFormBuilderAspectPair::eNsvToSvTypes9(bool bIsVariant)
{
    ET_ReturnCode eRet = H_NO_ERROR;
    CEString& sAspectPair = bIsVariant ? m_sAltAspectPairSource : m_sAspectPairSource;
    int& iStressPos = bIsVariant ? m_iAltStressPos : m_iStressPos;

    if (nullptr == m_pInfWordForm)
    {
        eRet = eGetInfinitiveWordForm();
        if (eRet != H_NO_ERROR)
        {
            return eRet;
        }
    }

    int iInfStressPos = -1;
    eRet = eGetInfinitiveStressPos(iInfStressPos);
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    if (iInfStressPos < 0)
    {
        return H_ERROR_UNEXPECTED;
    }

    int iDerivationType = 0;
    eRet = eGetNsvDerivationType(m_pInfWordForm, iDerivationType);
    if (eRet != H_NO_ERROR || iDerivationType < -3 || iDerivationType > -1)
    {
        return H_ERROR_UNEXPECTED;
    }

//    int iStressedSyllable = m_pInfWordForm->sWordForm().uiGetSyllableFromVowelPos(iInfStressPos);
    CEString sInfinitive = m_pInfWordForm->sWordForm();
    if (sInfinitive.bEndsWith(L"ся"))
    {
        sInfinitive.sRemoveCharsFromEnd(2);
    }

    if (-2 == iDerivationType)
    {
        if (sInfinitive.bEndsWith(L"ирать"))
        {
            assert(sInfinitive.uiLength() > 5);
            sAspectPair = sInfinitive;
            sAspectPair = sAspectPair.sRemoveCharsFromEnd(5) + L"ереть";
            iStressPos = sAspectPair.uiGetVowelPos(sAspectPair.uiNSyllables()-1);
            return H_NO_ERROR;
        }
    }

    return H_ERROR_UNEXPECTED;

}       //  eNsvToSvTypes9()

ET_ReturnCode CFormBuilderAspectPair::eNsvToSvTypes10(bool bIsVariant)
{
    ET_ReturnCode eRet = H_NO_ERROR;
    CEString& sAspectPair = bIsVariant ? m_sAltAspectPairSource : m_sAspectPairSource;
    int& iStressPos = bIsVariant ? m_iAltStressPos : m_iStressPos;

    if (nullptr == m_pInfWordForm)
    {
        eRet = eGetInfinitiveWordForm();
        if (eRet != H_NO_ERROR)
        {
            return eRet;
        }
    }

    int iInfStressPos = -1;
    eRet = eGetInfinitiveStressPos(iInfStressPos);
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    if (iInfStressPos < 0)
    {
        return H_ERROR_UNEXPECTED;
    }

    int iDerivationType = 0;
    eRet = eGetNsvDerivationType(m_pInfWordForm, iDerivationType);
    if (eRet != H_NO_ERROR || iDerivationType < -3 || iDerivationType > -1)
    {
        return H_ERROR_UNEXPECTED;
    }

//    int iStressedSyllable = m_pInfWordForm->sWordForm().uiGetSyllableFromVowelPos(iInfStressPos);
    CEString sInfinitive = m_pInfWordForm->sWordForm();
    if (sInfinitive.bEndsWith(L"ся"))
    {
        sInfinitive.sRemoveCharsFromEnd(2);
    }

    if (-1 == iDerivationType)
    {
        if (sInfinitive.bEndsWith(L"алывать"))
        {
            assert(sInfinitive.uiLength() > 7);
            sAspectPair = sInfinitive;
            sAspectPair = sAspectPair.sRemoveCharsFromEnd(7) + L"олоть";
            iStressPos = sAspectPair.uiGetVowelPos(sAspectPair.uiNSyllables()-1);
            return H_NO_ERROR;
        }
        if (sInfinitive.bEndsWith(L"арывать"))
        {
            assert(sInfinitive.uiLength() > 7);
            sAspectPair = sInfinitive;
            sAspectPair = sAspectPair.sRemoveCharsFromEnd(7) + L"ороть";
            iStressPos = sAspectPair.uiGetVowelPos(sAspectPair.uiNSyllables()-1);
            return H_NO_ERROR;
        }
    }

    return H_ERROR_UNEXPECTED;

}       //  eNsvToSvTypes10()

ET_ReturnCode CFormBuilderAspectPair::eNsvToSvTypes14(bool bIsVariant)
{
    ET_ReturnCode eRet = H_NO_ERROR;
    CEString& sAspectPair = bIsVariant ? m_sAltAspectPairSource : m_sAspectPairSource;
    int& iStressPos = bIsVariant ? m_iAltStressPos : m_iStressPos;

    if (nullptr == m_pInfWordForm)
    {
        eRet = eGetInfinitiveWordForm();
        if (eRet != H_NO_ERROR)
        {
            return eRet;
        }
    }

    int iInfStressPos = -1;
    eRet = eGetInfinitiveStressPos(iInfStressPos);
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    if (iInfStressPos < 0)
    {
        return H_ERROR_UNEXPECTED;
    }

    int iDerivationType = 0;
    eRet = eGetNsvDerivationType(m_pInfWordForm, iDerivationType);
    if (eRet != H_NO_ERROR || iDerivationType < -3 || iDerivationType > -1)
    {
        return H_ERROR_UNEXPECTED;
    }

    int iStressedSyllable = m_pInfWordForm->sWordForm().uiGetSyllableFromVowelPos(iInfStressPos);
    CEString sInfinitive = m_pInfWordForm->sWordForm();
    if (sInfinitive.bEndsWith(L"ся"))
    {
        sInfinitive.sRemoveCharsFromEnd(2);
    }

    if (-2 == iDerivationType)
    {
        if (sInfinitive.bEndsWith(L"имать") || sInfinitive.bEndsWith(L"инать"))
        {
            if (sInfinitive.uiNSyllables()-1 == iStressedSyllable || sInfinitive.uiNSyllables()-2 == iStressedSyllable)
            {
                assert(sInfinitive.uiLength() > 5);
                CEString sStem(sInfinitive);
                sStem.sRemoveCharsFromEnd(5);
                if (CEString::bIn(sStem[sStem.uiLength() - 1], CEString(CEString::g_szRusHushers)))
                {
                    sAspectPair = sStem + L"ать";
                }
                else
                {
                    sAspectPair = sStem + L"ять";
                }
                iStressPos = sAspectPair.uiGetVowelPos(sAspectPair.uiNSyllables()-1);
                return H_NO_ERROR;
            }
        }
    }

    return H_ERROR_UNEXPECTED;

}       //  eNsvToSvTypes14()

ET_ReturnCode CFormBuilderAspectPair::eNsvToSvTypes11_12_13_15_16(bool bIsVariant)
{
    ET_ReturnCode eRet = H_NO_ERROR;
    CEString& sAspectPair = bIsVariant ? m_sAltAspectPairSource : m_sAspectPairSource;
    int& iStressPos = bIsVariant ? m_iAltStressPos : m_iStressPos;

    if (nullptr == m_pInfWordForm)
    {
        eRet = eGetInfinitiveWordForm();
        if (eRet != H_NO_ERROR)
        {
            return eRet;
        }
    }

    int iInfStressPos = -1;
    eRet = eGetInfinitiveStressPos(iInfStressPos);
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    if (iInfStressPos < 0)
    {
        return H_ERROR_UNEXPECTED;
    }

    int iDerivationType = 0;
    eRet = eGetNsvDerivationType(m_pInfWordForm, iDerivationType);
    if (eRet != H_NO_ERROR || iDerivationType < -3 || iDerivationType > -1)
    {
        return H_ERROR_UNEXPECTED;
    }

    int iStressedSyllable = m_pInfWordForm->sWordForm().uiGetSyllableFromVowelPos(iInfStressPos);
    CEString sInfinitive = m_pInfWordForm->sWordForm();
    if (sInfinitive.bEndsWith(L"ся"))
    {
        sInfinitive.sRemoveCharsFromEnd(2);
    }

    if (-3 == iDerivationType)
    {
        if (sInfinitive.bEndsWith(L"вать"))
        {
            if (sInfinitive.uiNSyllables()-1 == iStressedSyllable)
            {
                assert(sInfinitive.uiLength() > 3);
                sAspectPair = sInfinitive;
                sAspectPair = sAspectPair.sRemoveCharsFromEnd(4) + L"ть";
                iStressPos = sAspectPair.uiGetVowelPos(sAspectPair.uiNSyllables()-1);
                return H_NO_ERROR;
            }
        }
    }

    return H_ERROR_UNEXPECTED;

}       //  eNsvToSvTypes11_12_13_15_16()
