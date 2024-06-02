#include "Dictionary.h"
#include "FormBuilderNouns.h"
#include "FormBuilderAdjLong.h"
#include "FormBuilderAdjShort.h"
#include "FormBuilderPronounAdj.h"
#include "FormBuilderAdjComparative.h"
#include "FormBuilderVerbPers.h"
#include "FormBuilderVerbImpv.h"
#include "FormBuilderVerbPast.h"
#include "FormBuilderVerbNonFinite.h"
#include "FormBuilderAspectPair.h"
#include "Lexeme.h"

using namespace Hlib;

ET_ReturnCode CInflectionEnumerator::eReset()
{
    if (nullptr == m_spLexeme)
    {
        return H_ERROR_POINTER;
    }

    m_itCurrentInflection = m_spLexeme->m_vecInflections.begin();
    
    return H_NO_ERROR;
}

ET_ReturnCode CInflectionEnumerator::eGetFirstInflection(shared_ptr<CInflection>& spInflection)
{
    if (nullptr == m_spLexeme)
    {
        return H_ERROR_POINTER;
    }

    m_itCurrentInflection = m_spLexeme->m_vecInflections.begin();
    if (m_spLexeme->m_vecInflections.end() == m_itCurrentInflection)
    {
        return H_FALSE;
    }

    spInflection = *m_itCurrentInflection;

    return H_NO_ERROR;
}

ET_ReturnCode CInflectionEnumerator::eGetNextInflection(shared_ptr<CInflection>& spInflection)
{
    if (nullptr == m_spLexeme)
    {
        return H_ERROR_POINTER;
    }

    if (m_itCurrentInflection != m_spLexeme->m_vecInflections.end())
    {
        ++m_itCurrentInflection;
    }

    if (m_spLexeme->m_vecInflections.end() == m_itCurrentInflection)
    {
        return H_NO_MORE;
    }

    spInflection = *m_itCurrentInflection;

    return H_NO_ERROR;
}

CLexeme::CLexeme(shared_ptr<CDictionary> spD) : m_spDictionary(spD.get()), m_spSecondPart(nullptr)
{
    Init();
}

CLexeme::CLexeme(const CLexeme& source) : enable_shared_from_this(source)
{
    Init();

    m_stProperties = source.m_stProperties;
    m_spDictionary = source.m_spDictionary;
    m_itCurrentStressPos = source.m_itCurrentStressPos;
    m_spSecondPart = source.m_spSecondPart;

}

CLexeme::~CLexeme()
{}

void CLexeme::Init()
{
    m_iAspectPairStressPos = -1;
    m_iAltAspectPairStressPos = -1;

    vector<CEString> vecMainSymbol = { L"м", L"мо", L"ж", L"жо", L"с", L"со", L"мо-жо", L"мн.",
     L"мн. неод.", L"мн. одуш.", L"мн. _от_", L"п", L"мс", L"мс-п", L"числ.", L"числ.-п", 
     L"св", L"нсв", L"св-нсв", L"н", L"предл.", L"союз", L"предик.", L"вводн.", L"сравн.", 
     L"част.", L"межд." , L"предикативное мс"};

    try
    {
        for (ET_MainSymbol eMs = MS_START; eMs < MS_END; ++eMs)
        {
            m_mapMainSymbol[vecMainSymbol[eMs]] = eMs;
        }

        for (int iMs = 0; iMs < (int)MS_END; ++iMs)
        {
            CEString sMs = vecMainSymbol[iMs];
            ET_MainSymbol eMs = m_mapMainSymbol[sMs];
            switch (eMs)
            {
            case MS_M:
            {
                m_mapMainSymbolToAnimacy[sMs] = ANIM_NO;
                m_mapMainSymbolToGender[sMs] = GENDER_M;
                break;
            }
            case MS_MO:
            {
                m_mapMainSymbolToAnimacy[sMs] = ANIM_YES;
                m_mapMainSymbolToGender[sMs] = GENDER_M;
                break;
            }
            case MS_ZH:
            {
                m_mapMainSymbolToAnimacy[sMs] = ANIM_NO;
                m_mapMainSymbolToGender[sMs] = GENDER_F;
                break;
            }
            case MS_ZHO:
            case MS_MO_ZHO:
            {
                m_mapMainSymbolToAnimacy[sMs] = ANIM_YES;
                m_mapMainSymbolToGender[sMs] = GENDER_F;
                break;
            }
            case MS_S:
            {
                m_mapMainSymbolToAnimacy[sMs] = ANIM_NO;
                m_mapMainSymbolToGender[sMs] = GENDER_N;
                break;
            }
            case MS_SO:
            {
                m_mapMainSymbolToAnimacy[sMs] = ANIM_YES;
                m_mapMainSymbolToGender[sMs] = GENDER_N;
                break;
            }
            case MS_MN:
            {
                m_mapMainSymbolToAnimacy[sMs] = ANIM_UNDEFINED;
                m_mapMainSymbolToGender[sMs] = GENDER_UNDEFINED;
                break;
            }
            case MS_MN_NEOD:
            {
                m_mapMainSymbolToAnimacy[sMs] = ANIM_NO;
                m_mapMainSymbolToGender[sMs] = GENDER_UNDEFINED;
                break;
            }
            case MS_MN_ODUSH:
            {
                m_mapMainSymbolToAnimacy[sMs] = ANIM_YES;
                m_mapMainSymbolToGender[sMs] = GENDER_UNDEFINED;
                break;
            }
            case MS_MN_OT:
            {
                m_mapMainSymbolToAnimacy[sMs] = ANIM_NO;
                m_mapMainSymbolToGender[sMs] = GENDER_UNDEFINED;
                break;
            }
            case MS_SV:
            case MS_NSV:
            case MS_SV_NSV:
            case MS_P:
            case MS_MS:
            case MS_MS_P:
            case MS_N:
            case MS_CHISL:
            case MS_CHISL_P:
            case MS_CHAST:
            case MS_PREDL:
            case MS_SOJUZ:
            case MS_VVODN:
            case MS_PREDIC:
            case MS_MEZHD:
            case MS_SRAVN:
            case MS_PREDIC_MS:
            {
                m_mapMainSymbolToAnimacy[sMs] = ANIM_UNDEFINED;
                m_mapMainSymbolToGender[sMs] = GENDER_UNDEFINED;
                break;
            }
            default:
            {
                assert(0);
                ERROR_LOG(L"Unrecognized type.");
                return;
            }

            }    // switch

        }   //  for (int iMs = 0; ... )

        const wchar_t * arr1stStage[] =
        { L"б",  L"п",  L"в",  L"ф",  L"м",  L"з", L"с", L"д", L"т", L"ст", L"г", L"к", L"х", L"ск" };
        const wchar_t * arr2ndStage[] =
        { L"бл", L"пл", L"вл", L"фл", L"мл", L"ж", L"ш", L"ж", L"ч", L"щ",  L"ж", L"ч", L"ш", L"щ" };

        for (int i_sa = 0; i_sa < (int)(sizeof(arr1stStage)/sizeof(wchar_t *)); ++i_sa)
        {
            m_mapStandardAlternations[arr1stStage[i_sa]] = arr2ndStage[i_sa];
        }
/*
        const wchar_t * arrPreverbs[] = { L"в", L"над", L"об", L"от", L"под", L"пред", L"с" };
        for (int i_ap = 0; i_ap < (int)(sizeof(arrPreverbs)/sizeof(wchar_t *)); ++i_ap)
        {
            m_vecAlternatingPreverbs.push_back(arrPreverbs[i_ap]);
        }

        const wchar_t * arrPreverbsV[] = { L"вс", L"вз", L"вос", L"воз", L"ис", L"из", L"нис", L"низ", L"рас", L"раз" };
        for (int i_ap = 0; i_ap < (int)(sizeof(arrPreverbsV)/sizeof(wchar_t *)); ++i_ap)
        {
            m_vecAlternatingPreverbsWithVoicing.push_back(arrPreverbsV[i_ap]);
        }
*/
        m_stProperties.s1SgStem.SetVowels(CEString::g_szRusVowels);
        m_stProperties.s3SgStem.SetVowels(CEString::g_szRusVowels);
        m_stProperties.sInfinitive.SetVowels(CEString::g_szRusVowels);
        m_stProperties.sInfStem.SetVowels(CEString::g_szRusVowels);
    }
    catch (...)
    {
        ERROR_LOG(L"Exception during Lexeme class instantiation");
    }
}   // Init()

void CLexeme::SetDictionary(shared_ptr<CDictionary> spDict)
{
    m_spDictionary = spDict.get();
}

ET_ReturnCode CLexeme::eCreateInflectionEnumerator(shared_ptr<CInflectionEnumerator>& pIe)
{
    m_spInflectionEnumerator = make_shared<CInflectionEnumerator>(shared_from_this());
    if (!m_spInflectionEnumerator)
    {
        ERROR_LOG(L"Error retrieving CInflectionEnumerator.");
        return H_ERROR_POINTER;
    }

    pIe = m_spInflectionEnumerator;

    return H_NO_ERROR;
}

int CLexeme::nInflections()
{
    return (int)m_vecInflections.size();
}

ET_ReturnCode CLexeme::eGetInflectionInstance(int iAt, shared_ptr<CInflection>& spInflection)
{
    if (iAt < 0 || iAt >= (int)m_vecInflections.size())
    {
        ERROR_LOG(L"Lexeme index out of bounds.");
        return H_ERROR_INVALID_ARG;
    }

    spInflection = m_vecInflections[iAt];

    return H_NO_ERROR;
}

ET_ReturnCode CLexeme::eGetInflectionById(long long llInflectionId, shared_ptr<CInflection>& spInflection)
{
    for (auto spInfl : m_vecInflections)
    {
        if (spInfl->llInflectionId() == llInflectionId)
        {
            spInflection = spInfl;
            return H_NO_ERROR;
        }
    }

    return H_ERROR_UNEXPECTED;
}

ET_ReturnCode CLexeme::eCreateInflectionForEdit(shared_ptr<CInflection>& spInflection)
{
    spInflection = make_shared<CInflection>(this);
    if (nullptr == spInflection)
    {
        return H_ERROR_POINTER;
    }

    return H_NO_ERROR;
}

bool CLexeme::bFindStandardAlternation (const CEString& sKey, CEString& sValue)
{
    map<CEString, CEString>::iterator itAlt = m_mapStandardAlternations.find (sKey);
    if (m_mapStandardAlternations.end() != itAlt)
    {
        sValue = (*itAlt).second;
        return true;
    }
    else
    {
        sValue = L"";
        return false;
    }
}

ET_ReturnCode CLexeme::eGetStemStressPositions (const CEString& sStem, vector<int>& vecPosition)
{
    //
    // Find the sequence # of the stressed vowel in infinitive
    //
    CEString sCopy(sStem);

    vector<int>::iterator itInfStresPos = m_stProperties.vecSourceStressPos.begin();
    for (; itInfStresPos != m_stProperties.vecSourceStressPos.end(); ++itInfStresPos)
    {
        sCopy.SetVowels(CEString::g_szRusVowels);
        int iStemSyllables = sCopy.uiNSyllables();
        if (iStemSyllables < 1)
        {
            assert(0);
            ERROR_LOG (L"No vowels in verb stem.");
            return H_ERROR_GENERAL;
        }

        if (*itInfStresPos >= iStemSyllables)
        {
            vecPosition.push_back (iStemSyllables - 1);  // last stem syllable
        }
        else
        {
            vecPosition.push_back (*itInfStresPos);    // same syllable as in infinitive
        }

    }   //  for (; it_sourceStressPos != vec_SourceStressPos.end(); ... )

    return H_NO_ERROR;

}   //  hGetStemStressPositions (...)

void CLexeme::AssignPrimaryStress (shared_ptr<CWordForm> spWordForm)
{
    vector<int>::iterator itStressPos = m_stProperties.vecSourceStressPos.begin();
    for (; itStressPos != m_stProperties.vecSourceStressPos.end(); ++itStressPos)
    {
        spWordForm->SetStressPos(*itStressPos, STRESS_PRIMARY);
    }
}

void CLexeme::AssignSecondaryStress (shared_ptr<CWordForm> spWordForm)
{
    vector<int>::iterator itPos = m_stProperties.vecSecondaryStressPos.begin();
    for (; itPos != m_stProperties.vecSecondaryStressPos.end(); ++itPos)
    {
        int iStressedSyll = *itPos; // it _is_ the syllable number, so we don't need that conversion
//        if (iStressedSyll < 0 || iStressedSyll >= (int)m_stProperties.sGraphicStem.uiGetNumOfSyllables())
        if (iStressedSyll < 0 || iStressedSyll >= (int)spWordForm->sWordForm().uiGetNumOfSyllables())
        {
            throw CException (H_ERROR_UNEXPECTED, L"Secondary stress position out of bounds.");
        }

        spWordForm->SetStressPos(iStressedSyll, STRESS_SECONDARY);
    }
}

ET_ReturnCode CLexeme::eUpdateDescriptorInfo(CLexeme* spLexeme)
{
    return m_spDictionary->eUpdateDescriptorInfo(spLexeme);
}

void CLexeme::AddInflection(shared_ptr<CInflection> spInflection)
{
    m_vecInflections.emplace_back(spInflection);
}

CEString CLexeme::sGramHashNSgMLong()
{
    CGramHasher hasher (POS_ADJ, SUBPARADIGM_LONG_ADJ, CASE_UNDEFINED, NUM_UNDEFINED, GENDER_UNDEFINED, PERSON_UNDEFINED,                             
                        ANIM_NO, ASPECT_UNDEFINED, REFL_UNDEFINED);
    auto sHash = hasher.sGramHash();
    return sHash;
}

void CLexeme::SetSecondPart(shared_ptr<CLexeme> spRhs)
{
    m_spSecondPart = spRhs;
}

ET_ReturnCode CLexeme::eSetDb (const CEString& sDbPath)
{
    return m_spDictionary->eSetDbPath(sDbPath);
}

ET_ReturnCode CLexeme::eExtractStressSymbols()
{
    m_stProperties.vecSourceStressPos.clear();
    m_stProperties.vecSecondaryStressPos.clear();

    vector<int> vecMainStressedVowelPos, vecSecondaryStressedVowelPos;

    try
    {
        unsigned int iAt = 0;
        while (iAt < m_stProperties.sSourceForm.uiLength() - 1)
        {
            auto uiPos = m_stProperties.sSourceForm.uiFindOneOf(iAt, L"/\\");
            if (ecNotFound == uiPos)
            {
                break;
            }

            if (uiPos >= m_stProperties.sSourceForm.uiLength() - 1)
            {
                ERROR_LOG(L"Stress mark at end of source form.");
                return H_ERROR_UNEXPECTED;
            }

            if (!CEString::bIsVowel(m_stProperties.sSourceForm[uiPos + 1]))
            {
                ERROR_LOG(L"Stress mark not over vowel.");
                return H_ERROR_UNEXPECTED;
            }

            if (L'/' == m_stProperties.sSourceForm[uiPos])
            {
                m_stProperties.vecSourceStressPos.push_back(uiPos);
            }
            else if (L'\\' == m_stProperties.sSourceForm[uiPos])
            {
                m_stProperties.vecSecondaryStressPos.push_back(uiPos);
            }

            m_stProperties.sSourceForm.sErase(uiPos, 1);
        }
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }
    catch (...)
    {
        ERROR_LOG(L"Unknown exception.");
    }

    return H_NO_ERROR;
}

ET_ReturnCode CLexeme::eInitializeFromProperties()
{
    if (m_stProperties.sSourceForm.uiLength() < 1)
    {
        CEString sError(L"No source form.");
        ERROR_LOG(sError);
        m_sLastErrorMsg = sError;
        return H_ERROR_UNEXPECTED;
    }

    ET_ReturnCode eRet = eExtractStressSymbols();
    if (eRet != H_NO_ERROR)
    {
        return eRet;
    }

    if (m_stProperties.sMainSymbol.uiLength() < 1)
    {
        CEString sError(L"No main symbol.");
        ERROR_LOG(sError);
        m_sLastErrorMsg = sError;
        return H_ERROR_UNEXPECTED;
    }

    if (m_stProperties.sMainSymbol.uiLength() < 1)
    {
        CEString sError(L"No main symbol.");
        ERROR_LOG(sError);
        m_sLastErrorMsg = sError;
        return H_ERROR_UNEXPECTED;
    }

    if (0 == m_mapMainSymbol.count(m_stProperties.sMainSymbol))
    {
        CEString sError(L"Unknown main symbol.");
        ERROR_LOG(sError);
        m_sLastErrorMsg = sError;
        return H_ERROR_UNEXPECTED;
    }

    if (m_stProperties.sInflectionType.uiLength() < 1)
    {
        m_stProperties.sInflectionType = m_stProperties.sMainSymbol;
    }

    auto eMainSymbol = m_mapMainSymbol[m_stProperties.sMainSymbol];
    switch (eMainSymbol)
    {
    case MS_M:
    case MS_MO:
    case MS_ZH:
    case MS_ZHO:
    case MS_S:
    case MS_SO:
    case MS_MO_ZHO:
    case MS_MN:
    case MS_MN_NEOD:
    case MS_MN_ODUSH:
    case MS_MN_OT:
        m_stProperties.ePartOfSpeech = POS_NOUN;
        break;

    case MS_P:
        m_stProperties.ePartOfSpeech = POS_ADJ;
        break;

    case MS_MS:
        m_stProperties.ePartOfSpeech = POS_PRONOUN;
        break;

    case MS_MS_P:
        m_stProperties.ePartOfSpeech = POS_PRONOUN_ADJ;
        break;

    case MS_CHISL:
        m_stProperties.ePartOfSpeech = POS_NUM;
        break;

    case MS_CHISL_P:
        m_stProperties.ePartOfSpeech = POS_NUM_ADJ;
        break;

    case MS_SV:
    case MS_NSV:
    case MS_SV_NSV:
        m_stProperties.ePartOfSpeech = POS_VERB;
        break;

    case MS_N:
        m_stProperties.ePartOfSpeech = POS_ADV;
        break;

    case MS_PREDL:
        m_stProperties.ePartOfSpeech = POS_PREPOSITION;
        break;

    case MS_SOJUZ:
        m_stProperties.ePartOfSpeech = POS_CONJUNCTION;
        break;

    case MS_PREDIC:
        m_stProperties.ePartOfSpeech = POS_PREDIC;
        break;

    case MS_VVODN:
        m_stProperties.ePartOfSpeech = POS_PARENTH;
        break;

    case MS_SRAVN:
        m_stProperties.ePartOfSpeech = POS_COMPAR;
        break;

    case MS_CHAST:
        m_stProperties.ePartOfSpeech = POS_PARTICLE;
        break;

    case MS_MEZHD:
        m_stProperties.ePartOfSpeech = POS_INTERJ;
        break;

    case MS_END:
    case MS_UNDEFINED:
    default:
        break;
    }               //  switch (eMainSymbol)

    //eRet = eMakeGraphicStem();

    return eRet;

}       //  eInitializeFromProperties()

ET_ReturnCode CLexeme::eHandleSpryazhSmEntry()
{
    uint64_t uiQueryHandle = 0;
    CEString sQuery(L"SELECT hw.source, hw.id FROM headword AS hw INNER JOIN descriptor AS d ON hw.id = d.word_id WHERE d.id = ");
    sQuery += CEString::sToString(m_stProperties.llDescriptorId);
    sQuery += L";";

    CEString sRefSource;
    CEString sHeadWordId;
    auto spDb = m_spDictionary->spGetDb();
    uiQueryHandle = spDb->uiPrepareForSelect(sQuery);
    while (spDb->bGetRow(uiQueryHandle))
    {
        spDb->GetData(0, sRefSource, uiQueryHandle);
        spDb->GetData(1, sHeadWordId, uiQueryHandle);
    }

    spDb->Finalize(uiQueryHandle);

    CEString sHomonymsQuery(L"SELECT homonym_number FROM homonyms WHERE headword_id = ");
    sHomonymsQuery += sHeadWordId;
    sHomonymsQuery += L";";
    uiQueryHandle = spDb->uiPrepareForSelect(sHomonymsQuery);
    CEString sHomonyms;
    while (spDb->bGetRow(uiQueryHandle))
    {
        CEString sNum;
        spDb->GetData(0, sHomonyms, uiQueryHandle);
        if (!sHomonyms.bIsEmpty())
        {
            sHomonyms += L"-";
        }
    }

    sRefSource.SetVowels(CEString::g_szRusVowels);
    m_stProperties.sSpryazhSmRefSource = sRefSource;
    m_stProperties.sSpryazhSmRefHomonyms = sHomonyms;

    m_stProperties.iSpryazhSmRefPrefixLength = (int)sRefSource.uiLength();
    m_stProperties.sSpryazhSmPrefix = m_stProperties.sSourceForm;
    for (int iAt1 = sRefSource.uiLength() - 1, iAt2 = m_stProperties.sSourceForm.uiLength() - 1; 
        (iAt1 >= 0) && (iAt2 >= 0);
        --iAt1, --iAt2)
    {
        if (sRefSource[iAt1] == m_stProperties.sSourceForm[iAt2])
        {
            --m_stProperties.iSpryazhSmRefPrefixLength;
            m_stProperties.sSpryazhSmPrefix.sRemoveCharsFromEnd(1);
        }
        else
        {
            break;
        }
    }

    auto& sRefStem = m_stProperties.sGraphicStem;
    sRefStem.SetVowels(CEString::g_szRusVowels);
    auto sNewStem = m_stProperties.sSpryazhSmPrefix + 
        sRefStem.sSubstr(m_stProperties.iSpryazhSmRefPrefixLength);
    sNewStem.SetVowels(CEString::g_szRusVowels);
//    int iDiff = (int)sNewStem.uiNSyllables() - (int)sRefStem.uiNSyllables();

    m_stProperties.sGraphicStem = sNewStem;

    return H_NO_ERROR;

}   //  eHandleSpryazhSmEntry()

//ET_ReturnCode CLexeme::eClone(shared_ptr<CLexeme>& spClonedObject)
//{
//    spClonedObject = make_shared<CLexeme>(const_cast<const CLexeme&>(*this));
//    return H_NO_ERROR;
//}

ET_ReturnCode CLexeme::eCheckLexemeProperties()
{
    return eInitializeFromProperties();
}

ET_ReturnCode CLexeme::eGetErrorMsg(CEString& sErrorMsg)
{
    sErrorMsg = m_sLastErrorMsg;
    return sErrorMsg.bIsEmpty() ? H_FALSE : H_NO_ERROR;
}

shared_ptr<CSqlite> CLexeme::spGetDb()
{
    shared_ptr<CSqlite> spDb = m_spDictionary->spGetDb();
    if (!spDb)
    {
        throw CException (H_ERROR_POINTER, L"No database handle.");
    }
    return spDb;
}

ET_ReturnCode CLexeme::eGetSourceFormWithStress(CEString& sSourceForm, bool bIsVariant)
{
    CEString sRet = bIsVariant ? m_stProperties.sHeadwordVariant : m_stProperties.sSourceForm;
    vector<int> vecStressedSyllables = bIsVariant ? m_stProperties.vecSourceVariantStressPos : m_stProperties.vecSourceStressPos;
    vector<int> vecSecondaryStressedSyllables = bIsVariant ? m_stProperties.vecSecondaryVariantStressPos : m_stProperties.vecSecondaryStressPos;

    map<int, bool> mapStressedVowelPositions;

    try
    {
        for (auto iSyll : vecStressedSyllables)
        {
            int iVowelPos = m_stProperties.sSourceForm.uiGetVowelPos(iSyll);
            mapStressedVowelPositions[iVowelPos] = true;
        }

        for (auto iSyll : vecSecondaryStressedSyllables)
        {
            int iVowelPos = m_stProperties.sSourceForm.uiGetVowelPos(iSyll);
            mapStressedVowelPositions[iVowelPos] = false;
        }

        int iIncrement = 0;
        for (auto&& pairVowelPos : mapStressedVowelPositions)
        {
            if (pairVowelPos.second)
            {
                sRet.sInsert((unsigned int)(pairVowelPos.first + iIncrement), L'/');
                ++iIncrement;
            }
            else
            {
                sRet.sInsert((unsigned int)(pairVowelPos.first + iIncrement), L'\\');
                ++iIncrement;
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
        ERROR_LOG(L"Unknown exception.");
    }

    sSourceForm = sRet;

    return H_NO_ERROR;

}   //  eGetSourceFormWithStress()

ET_ReturnCode CLexeme::eGetSourceFormWithDiacritics(CEString& sSourceForm, bool bIsVariant)
{
    CEString sRet = bIsVariant ? m_stProperties.sHeadwordVariant : m_stProperties.sSourceForm;
    vector<int> vecStressedSyllables = bIsVariant ? m_stProperties.vecSourceVariantStressPos : m_stProperties.vecSourceStressPos;
    vector<int> vecSecondaryStressedSyllables = bIsVariant ? m_stProperties.vecSecondaryVariantStressPos : m_stProperties.vecSecondaryStressPos;

    map<int, bool> mapStressedVowelPositions;

    if (sRet.uiNSyllables() < 2)
    {
        sSourceForm = sRet;
        return H_NO_ERROR;
    }

    try
    {
        for (auto iSyll : vecStressedSyllables)
        {
            int iVowelPos = m_stProperties.sSourceForm.uiGetVowelPos(iSyll);
            mapStressedVowelPositions[iVowelPos] = true;
        }

        for (auto iSyll : vecSecondaryStressedSyllables)
        {
            int iVowelPos = m_stProperties.sSourceForm.uiGetVowelPos(iSyll);
            mapStressedVowelPositions[iVowelPos] = false;
        }

        int iIncrement = 0;
        for (auto[iPos, bPrimary] : mapStressedVowelPositions)
        {
            if (bPrimary)
            {
                if (sRet[iPos + iIncrement] != L'ё')
                {
                    sRet.sInsert((unsigned int)(iPos + iIncrement + 1), Hlib::CEString::g_chrCombiningAcuteAccent);
                    ++iIncrement;
                }
            }
            else
            {
                sRet.sInsert((unsigned int)(iPos + iIncrement + 1), Hlib::CEString::g_chrCombiningGraveAccent);
                ++iIncrement;
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
        ERROR_LOG(L"Unknown exception.");
    }

    sSourceForm = sRet;

    return H_NO_ERROR;

}   //  eGetSourceFormWithDiacritics()

ET_ReturnCode CLexeme::eGetFirstStemStressPos (int& iPos)
{
    sort(m_stProperties.vecSourceStressPos.begin(), m_stProperties.vecSourceStressPos.end());   // edge case but...
    m_itCurrentStressPos = m_stProperties.vecSourceStressPos.begin();
    if (m_stProperties.vecSourceStressPos.end() == m_itCurrentStressPos)
    {
        iPos = -1;
        return H_FALSE;
    }
    
    iPos = *m_itCurrentStressPos;

    return H_NO_ERROR;
}

ET_ReturnCode CLexeme::eGetNextStemStressPos (int& iPos)
{
    if (m_itCurrentStressPos != m_stProperties.vecSourceStressPos.end())
    {
        ++m_itCurrentStressPos;
    }

    if (m_stProperties.vecSourceStressPos.end() == m_itCurrentStressPos)
    {
        iPos = -1;
        return H_NO_MORE;
    }

    iPos = *m_itCurrentStressPos;
    if (m_stProperties.llSecondPartId >= 0)
    {
        if (iPos >= (int)m_stProperties.sGraphicStem.uiGetNumOfSyllables())
        {
            iPos = -1;
            return H_NO_MORE;
        }
    }


    return H_NO_ERROR;
}

ET_ReturnCode CLexeme::eGetFirstSecondaryStemStressPos(int& iPos)
{
    m_itCurrentStressPos = m_stProperties.vecSecondaryStressPos.begin();
    if (m_stProperties.vecSecondaryStressPos.end() == m_itCurrentStressPos)
    {
        iPos = -1;
        return H_FALSE;
    }

    iPos = *m_itCurrentStressPos;

    return H_NO_ERROR;
}

ET_ReturnCode CLexeme::eGetNextSecondaryStemStressPos(int& iPos)
{
    if (m_itCurrentStressPos != m_stProperties.vecSecondaryStressPos.end())
    {
        ++m_itCurrentStressPos;
    }

    if (m_stProperties.vecSecondaryStressPos.end() == m_itCurrentStressPos)
    {
        iPos = -1;
        return H_NO_MORE;
    }

    iPos = *m_itCurrentStressPos;

    return H_NO_ERROR;
}

ET_ReturnCode CLexeme::eGetAspectPair(CEString& sAspectPair, int& iStressPos)
{
    if (!bHasAspectPair())
    {
        return H_ERROR_UNEXPECTED;
    }

    // Only the first and only inflection object will have it
    if (m_vecInflections.size() < 1)
    {
        ERROR_LOG(L"No inflections.");
        return H_ERROR_UNEXPECTED;
    }

    auto& spInflection = m_vecInflections[0];
    if (!spInflection)
    {
        ERROR_LOG(L"Inflection pointer is NULL");
        return H_ERROR_POINTER;
    }

    spInflection->eGetAspectPair(sAspectPair, iStressPos);

    return H_NO_ERROR;
}

ET_ReturnCode CLexeme::eGetAltAspectPair(CEString& sAltAspectPair, int& iAltStressPos)
{
    if (!bHasAltAspectPair())
    {
        return H_ERROR_UNEXPECTED;
    }

    // Only the first and only inflection object will have it
    if (m_vecInflections.size() < 1)
    {
        ERROR_LOG(L"No inflections.");
        return H_ERROR_UNEXPECTED;
    }

    auto& spInflection = m_vecInflections[0];
    if (!spInflection)
    {
        ERROR_LOG(L"Inflection pointer is NULL");
        return H_ERROR_POINTER;
    }

    spInflection->eGetAltAspectPair(sAltAspectPair, iAltStressPos);

    return H_NO_ERROR;
}
