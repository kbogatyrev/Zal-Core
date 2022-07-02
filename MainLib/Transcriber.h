#ifndef C_TRANSCRIBER_H_INCLUDED
#define C_TRANSCRIBER_H_INCLUDED

#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <memory>
#include <variant>

#include "Enums.h"
#include "EString.h"
#include "SqliteWrapper.h"

namespace Hlib
{
    struct StTactGroup;

    static CEString m_LowerCaseCyrillic(L"абвгдеёжзийклмнопрстуфхцчшщъыьэюя");
    static CEString m_Vowels(CEString::g_szRusVowels);
    static CEString m_Consonants(CEString::g_szRusConsonants);
    static CEString m_VoicedConsonants(CEString::g_szRusVoicedConsonants);
    static CEString m_VoicelessConsonants(CEString::g_szRusVoicelessConsonants);
    static CEString m_PairedVoiced(L"бвдзжг");
    static CEString m_PairedVoicelessConsonants(L"пфтсшк");
    static CEString m_PairedHardSoftConsonants(L"бпвфдтзслнмркгх");
    static CEString m_HardHushers(L"жцш");
    static CEString m_SoftHushers(L"чщ");
    static CEString m_Hushers(CEString::g_szRusHushers);
    static CEString m_HardConsonants = m_PairedHardSoftConsonants + m_HardHushers;
    static CEString m_SoftConsonants = m_PairedHardSoftConsonants + m_SoftHushers;

    // Input: a Cyrillic character, a sequence of characters, or two 
    // character sequences separated by "+"
//    using PairInput = pair<CEString, CEString>;     // 1st element must exist, 2nd element is optional
    using PhonemicContextAtom = variant<ET_PhonemicContext, ET_Sound, CEString>;
    using MorphemicContextAtom = variant<ET_MorphemicContext, CEString>;

    struct StRule
    {
        int64_t m_llRuleId;
        CEString m_sFollowedBy;       // after "+", e.g., a+o
        vector<ET_VowelStressRelation> m_vecStressContexts;
        vector<PhonemicContextAtom> m_vecLeftContexts;
        vector<PhonemicContextAtom> m_vecRightContexts;
        vector<ET_Boundary> m_vecLeftBoundaries;
        vector<ET_Boundary> m_vecRightBoundaries;
        vector<MorphemicContextAtom> m_vecMorphemicContexts;
        vector<ET_Subparadigm> m_vecSubparadigms;
        vector<ET_Gender> m_vecGenders;
        vector<ET_Number> m_vecNumbers;
        vector<ET_Case> m_vecCases;  
        ET_RuleStrength m_eStrength;
        bool m_bIsVariant;
        CEString m_sComment;
        vector<ET_Sound> m_vecTargets;
        vector<ET_Transform> m_vecTransforms;
    };

    class CTranscriber
    {
        struct StConsonant;
        
//    public:
//        enum class EC_CONTEXT_TYPE { LEFT, RIGHT };

    public:
        CTranscriber(shared_ptr<CSqlite>);
        ET_ReturnCode eLoadTranscriptionRules();
        ET_ReturnCode eTranscribe();
        ET_ReturnCode eTranscribeTactGroup(StTactGroup&);

    private:
        ET_ReturnCode eSplitSource(CEString& sSource, vector<CEString>& vecOutput);
        ET_ReturnCode eParseContexts(CEString& sSource, vector<PhonemicContextAtom>& vecTarget);
        ET_ReturnCode eParseBoundaries(CEString& sSource, vector<ET_Boundary>& vecTarget);
        ET_ReturnCode eParseTargets(CEString& sSource, vector<ET_Sound>& vecTargets);
        ET_ReturnCode eParseTransforms(CEString& sSource, vector<ET_Transform>& vecTransform);
        bool bIsProclitic(CWordForm& wf);           // stub
        bool bIsEnclitic(CWordForm& wf);            // stub
        ET_ReturnCode eHandleVowel(StTactGroup&, int& iPos);
        ET_ReturnCode eHandleConsonant(StTactGroup&, int& iPos);
        ET_ReturnCode eGetStressStatus(StTactGroup&, int iPos, ET_VowelStressRelation&);
        ET_ReturnCode eContextMatch(StTactGroup& stTg, PhonemicContextAtom, ET_ContextDirection, int iPos);
        ET_ReturnCode eMorphemeMatch(StTactGroup& stTg, MorphemicContextAtom, int iPos);
        ET_ReturnCode eBoundaryMatch(StTactGroup& stTg, ET_Boundary, ET_ContextDirection eRightOrLeft, int iPos);
        ET_ReturnCode eSubparadigmMatch(StTactGroup& stTg, const vector<ET_Subparadigm>& vecSp);
        ET_ReturnCode eGenderMatch(StTactGroup& stTg, const vector<ET_Gender>& vecGenders);
        ET_ReturnCode eNumberMatch(StTactGroup& stTg, const vector<ET_Number>& vecNumbers);
        ET_ReturnCode eCaseMatch(StTactGroup& stTg, const vector<ET_Case>& vecCases);
        ET_ReturnCode eApplyTransform(StConsonant&, ET_Transform);
        ET_ReturnCode eAddStressMark(StTactGroup&);

    private:
        unordered_map<wchar_t, vector<StRule>> m_mapCharToRules;
        unordered_map<ET_VowelStressRelation, vector<StRule>> m_mapLeftContextToRules;
        shared_ptr<CSqlite> m_pDb;

    public :
        vector<ET_Sound> m_vecTranscription;

    private:
        struct StConsonant
        {
            ET_Place m_ePlace;
            ET_Manner m_eManner;
            ET_Voicedness m_eVoicedness;
            ET_Palatalization m_ePalatalization;
            ET_Nasality m_eNasality;
            
            StConsonant() {}
            StConsonant(ET_Place ePlace, ET_Manner eManner, ET_Voicedness eVoicedness, ET_Palatalization ePalatalization, ET_Nasality eNasality) :
                m_ePlace(ePlace), m_eManner(eManner), m_eVoicedness(eVoicedness), m_ePalatalization(ePalatalization), m_eNasality(eNasality)
            {}

            // Needed to be used as map key
            bool operator == (const StConsonant& stRhs) const
            {
                if (m_ePlace == stRhs.m_ePlace && m_eManner == stRhs.m_eManner && m_eVoicedness == stRhs.m_eVoicedness &&
                    m_ePalatalization == stRhs.m_ePalatalization && m_eNasality == stRhs.m_eNasality)
                {
                    return true;
                }
                return false;
            }

            bool operator != (const StConsonant& stRhs) const
            {
                if (m_ePlace == stRhs.m_ePlace && m_eManner == stRhs.m_eManner && m_eVoicedness == stRhs.m_eVoicedness &&
                    m_ePalatalization == stRhs.m_ePalatalization && m_eNasality == stRhs.m_eNasality)
                {
                    return false;
                }
                return true;
            }

            bool operator < (const StConsonant& stRhs) const
            {
                if (m_ePlace != stRhs.m_ePlace)
                {
                    return m_ePlace < stRhs.m_ePlace;
                }
                else if (m_eManner != stRhs.m_eManner)
                {
                    return m_eManner < stRhs.m_eManner;
                }
                else if (m_eVoicedness != stRhs.m_eVoicedness)
                {
                    return m_eVoicedness < stRhs.m_eVoicedness;
                }
                else if (m_ePalatalization != stRhs.m_ePalatalization)
                {
                    return m_ePalatalization < stRhs.m_ePalatalization;
                }
                else if (m_eNasality != stRhs.m_eNasality)
                {
                    return m_eNasality < stRhs.m_eNasality;
                }
                return false;
            }
        };

        struct StVowel
        {
            ET_VowelStressRelation m_eStress;
            ET_Frontness m_eFrontness;
            ET_Height m_eHeight;

            StVowel() {}
            StVowel(ET_VowelStressRelation eStress, ET_Frontness eFrontness, ET_Height eHeight) :
                m_eStress(eStress), m_eFrontness(eFrontness), m_eHeight(eHeight)
            {}
            
            // Needed to be used as map key
            bool operator == (const StVowel& stRhs) const
            {
                if (m_eStress == stRhs.m_eStress && m_eHeight == stRhs.m_eHeight && m_eFrontness == stRhs.m_eFrontness)
                {
                    return true;
                }
                return false;
            }

            bool operator != (const StVowel& stRhs) const
            {
                if (m_eStress == stRhs.m_eStress && m_eHeight == stRhs.m_eHeight && m_eFrontness == stRhs.m_eFrontness)
                {
                    return true;
                }
                return false;
            }

            bool operator < (const StVowel& stRhs) const
            {
                if (m_eStress != stRhs.m_eStress)
                {
                    return m_eStress < stRhs.m_eStress;
                }
                else if (m_eHeight != stRhs.m_eHeight)
                {
                    return m_eHeight < stRhs.m_eHeight;
                }
                else if (m_eFrontness != stRhs.m_eFrontness)
                {
                    return m_eFrontness < stRhs.m_eFrontness;
                }
                return false;
            }
        };

        struct StAcoustic
        {
            ET_AcousticFeatureValue m_eVocalicNonVocalic;
            ET_AcousticFeatureValue m_eConsonantalNonconsonantal;
            ET_AcousticFeatureValue m_eCompactDiffuse;
            ET_AcousticFeatureValue m_eGraveAcute;
            ET_AcousticFeatureValue m_eFlatPlain;
            ET_AcousticFeatureValue m_eNasalOral;
            ET_AcousticFeatureValue m_eTenseLax;
            ET_AcousticFeatureValue m_eContinuantInterrupted;
            ET_AcousticFeatureValue m_eStridentMellow;

            StAcoustic(ET_AcousticFeatureValue eVocalic, ET_AcousticFeatureValue eCons, ET_AcousticFeatureValue eCompDiff,
                ET_AcousticFeatureValue eGraveAcute, ET_AcousticFeatureValue eFlatPlain, ET_AcousticFeatureValue eNasalOral,
                ET_AcousticFeatureValue eTenseLax, ET_AcousticFeatureValue eContInt, ET_AcousticFeatureValue eStriMel) :
                m_eVocalicNonVocalic(eVocalic),
                m_eConsonantalNonconsonantal(eCons),
                m_eCompactDiffuse(eCompDiff),
                m_eGraveAcute(eGraveAcute),
                m_eFlatPlain(eFlatPlain),
                m_eNasalOral(eNasalOral),
                m_eTenseLax(eTenseLax),
                m_eContinuantInterrupted(eContInt),
                m_eStridentMellow(eStriMel)
            {}
        };

    private:
        map<CEString, ET_VowelStressRelation> m_mapStringToStressRelation =
        {
            { L"STRESSED", ET_VowelStressRelation::STRESSED },
            { L"PRETONIC", ET_VowelStressRelation::PRETONIC },
            { L"FIRST_PRETONIC", ET_VowelStressRelation::FIRST_PRETONIC },
            { L"OTHER_PRETONIC", ET_VowelStressRelation::OTHER_PRETONIC },
            { L"POSTTONIC", ET_VowelStressRelation::POSTTONIC },
            { L"UNSTRESSED", ET_VowelStressRelation::UNSTRESSED }
        };

        map<CEString, ET_PhonemicContext> m_mapStringToContext =
        {
            { L"VOWEL", ET_PhonemicContext::VOWEL },
            { L"CONSONANT", ET_PhonemicContext::CONSONANT },
            { L"HARD_CONSONANT", ET_PhonemicContext::HARD_CONSONANT },
            { L"HARD_PAIRED_CONSONANT", ET_PhonemicContext::HARD_PAIRED_CONSONANT },
            { L"SOFT_CONSONANT", ET_PhonemicContext::SOFT_CONSONANT},
            { L"SOFT_CONSONANT_NO_CH_SHCH", ET_PhonemicContext::SOFT_CONSONANT_NO_CH_SHCH },
            { L"VOICELESS", ET_PhonemicContext::VOICELESS }
        };

        map<CEString, ET_Boundary> m_mapStringToBoundary =
        {
            { L"BOUNDARY_WORD", ET_Boundary::BOUNDARY_WORD },
            { L"BOUNDARY_NOT_PROCLITIC", ET_Boundary::BOUNDARY_NOT_PROCLITIC },
            { L"BOUNDARY_SYNTAGM", ET_Boundary::BOUNDARY_SYNTAGM }
        };

        map<CEString, ET_Sound> m_mapStringToSound =
        {
            { L"STRESSED_A", ET_Sound::STRESSED_A },
            { L"STRESSED_O", ET_Sound::STRESSED_O },
            { L"STRESSED_E", ET_Sound::STRESSED_E },
            { L"I", ET_Sound::I },
            { L"Y", ET_Sound::Y },
            { L"U", ET_Sound::U },
            { L"PRETONIC_A", ET_Sound::PRETONIC_A },
            { L"PRETONIC_LONG_A", ET_Sound::PRETONIC_LONG_A},
            { L"PRETONIC_E", ET_Sound::PRETONIC_E },   // word-initial or after vowel: экран, эпоха, поэтесса ($36, $45)
            { L"PRETONIC_HARD_IE", ET_Sound::PRETONIC_HARD_IE },
            { L"PRETONIC_SOFT_IE", ET_Sound::PRETONIC_SOFT_IE },
            { L"SCHWA", ET_Sound::SCHWA },
            { L"UNSTRESSED_FRONT_VOWEL", ET_Sound::UNSTRESSED_FRONT_VOWEL },
            { L"UNSTRESSED_E", ET_Sound::UNSTRESSED_E },
            { L"UNSTRESSED_O", ET_Sound::UNSTRESSED_O },
            { L"P_HARD", ET_Sound::P_HARD },
            { L"P_SOFT", ET_Sound::P_SOFT },
            { L"B_HARD", ET_Sound::B_HARD },
            { L"B_SOFT", ET_Sound::B_SOFT },
            { L"M_HARD", ET_Sound::M_HARD },
            { L"M_SOFT", ET_Sound::M_SOFT },
            { L"F_HARD", ET_Sound::F_HARD },
            { L"F_SOFT", ET_Sound::F_SOFT },
            { L"V_HARD", ET_Sound::V_HARD },
            { L"V_SOFT", ET_Sound::V_SOFT },
            { L"N_HARD", ET_Sound::N_HARD },
            { L"N_SOFT", ET_Sound::N_SOFT },
            { L"T_HARD", ET_Sound::T_HARD },
            { L"T_SOFT", ET_Sound::T_SOFT },
            { L"D_HARD", ET_Sound::D_HARD },
            { L"D_SOFT", ET_Sound::D_SOFT },
            { L"TS_HARD", ET_Sound::TS_HARD },
            { L"TS_SOFT", ET_Sound::TS_SOFT },         // Цюрих, хуацяо
            { L"S_HARD", ET_Sound::S_HARD },
            { L"S_SOFT", ET_Sound::S_SOFT },
            { L"L_HARD", ET_Sound::L_HARD },
            { L"L_SOFT", ET_Sound::L_SOFT },
            { L"Z_HARD", ET_Sound::Z_HARD },
            { L"Z_SOFT", ET_Sound::Z_SOFT },
            { L"R_HARD", ET_Sound::R_HARD },
            { L"R_SOFT", ET_Sound::R_SOFT },
            { L"SH_HARD", ET_Sound::SH_HARD },
            { L"SH_SOFT", ET_Sound::SH_SOFT },
            { L"ZH_HARD", ET_Sound::ZH_HARD },
            { L"ZH_SOFT", ET_Sound::ZH_SOFT },
            { L"JOD", ET_Sound::JOD },
            { L"K_HARD", ET_Sound::K_HARD },
            { L"K_SOFT", ET_Sound::K_SOFT },
            { L"X_HARD", ET_Sound::X_HARD },
            { L"X_SOFT", ET_Sound::X_SOFT },
            { L"G_FRICATIVE_HARD", ET_Sound::G_FRICATIVE_HARD },
            { L"G_FRICATIVE_SOFT", ET_Sound::G_FRICATIVE_SOFT }
        };

        unordered_map<ET_Sound, CEString> m_mapSoundToTranscription = 
        {
            { ET_Sound::STRESSED_A, CEString(L"а")},
            { ET_Sound::STRESSED_O, CEString(L"о") },
            { ET_Sound::STRESSED_E, CEString(L"е") },
            { ET_Sound::I, CEString(L"и") },
            { ET_Sound::Y, CEString(L"ы") },
            { ET_Sound::U, CEString(L"у") },
            { ET_Sound::PRETONIC_A, CEString(L"ʌ") },
            { ET_Sound::PRETONIC_LONG_A, CEString(L"͞ʌ") },
            { ET_Sound::PRETONIC_E, CEString(L"ε") },   // word-initial or after vowel: экран, эпоха, поэтесса ($36, $45)
            { ET_Sound::PRETONIC_HARD_IE, CEString(L"ыᵉ") },
            { ET_Sound::PRETONIC_SOFT_IE, CEString(L"иᵉ") },
            { ET_Sound::SCHWA, CEString(L"ъ") },
            { ET_Sound::UNSTRESSED_FRONT_VOWEL, CEString(L"ь") },
            { ET_Sound::UNSTRESSED_E, CEString(L"ыᵉ") },
            { ET_Sound::UNSTRESSED_O, CEString(L"о") },
            { ET_Sound::P_HARD, CEString(L"п") },
            { ET_Sound::P_SOFT, CEString(L"пʼ") },
            { ET_Sound::B_HARD, CEString(L"б") },
            { ET_Sound::B_SOFT, CEString(L"бʼ") },
            { ET_Sound::M_HARD, CEString(L"м") },
            { ET_Sound::M_SOFT, CEString(L"мʼ") },
            { ET_Sound::F_HARD, CEString(L"ф") },
            { ET_Sound::F_SOFT, CEString(L"фʼ") },
            { ET_Sound::V_HARD, CEString(L"в") },
            { ET_Sound::V_SOFT, CEString(L"вʼ") },
            { ET_Sound::N_HARD, CEString(L"н") },
            { ET_Sound::N_SOFT, CEString(L"нʼ") },
            { ET_Sound::T_HARD, CEString(L"т") },
            { ET_Sound::T_SOFT, CEString(L"тʼ") },
            { ET_Sound::D_HARD, CEString(L"д") },
            { ET_Sound::D_SOFT, CEString(L"дʼ") },
            { ET_Sound::TS_HARD, CEString(L"ц") },
            { ET_Sound::TS_SOFT, CEString(L"цʼ") },         // Цюрих, хуацяо
            { ET_Sound::CH, CEString(L"ч") },
            { ET_Sound::S_HARD, CEString(L"с") },
            { ET_Sound::S_SOFT, CEString(L"сʼ") },
            { ET_Sound::L_HARD, CEString(L"л") },
            { ET_Sound::L_SOFT, CEString(L"лʼ") },
            { ET_Sound::Z_HARD, CEString(L"з") },
            { ET_Sound::Z_SOFT, CEString(L"зʼ") },
            { ET_Sound::R_HARD, CEString(L"р") },
            { ET_Sound::R_SOFT, CEString(L"рʼ") },
            { ET_Sound::SH_HARD, CEString(L"ш") },
            { ET_Sound::SH_SOFT, CEString(L"шʼ") },
            { ET_Sound::ZH_HARD, CEString(L"ж") },
            { ET_Sound::ZH_SOFT, CEString(L"жʼ") },
            { ET_Sound::JOD, CEString(L"j") },
            { ET_Sound::K_HARD, CEString(L"к") },
            { ET_Sound::K_SOFT, CEString(L"кʼ") },
            { ET_Sound::G_HARD, CEString(L"г") },
            { ET_Sound::G_SOFT, CEString(L"гʼ") },
            { ET_Sound::X_HARD, CEString(L"х") },
            { ET_Sound::X_SOFT, CEString(L"хʼ") },
            { ET_Sound::G_FRICATIVE_HARD, CEString(L"γ") },
            { ET_Sound::G_FRICATIVE_SOFT, CEString(L"γʼ") }
        };

        map<CEString, ET_Transform> m_mapStringToTransform =
        {
            { L"SOFTEN", ET_Transform::SOFTEN },
            { L"VOICE", ET_Transform::VOICE },
            { L"DEVOICE", ET_Transform::DEVOICE },
            { L"GEMINATE", ET_Transform::GEMINATE },
            { L"SELF", ET_Transform::SELF }
        };

        map<CEString, ET_MorphemicContext> m_mapStringToMorphemicContext =
        {
            { L"ENDING", ET_MorphemicContext::ENDING },
            { L"NOT_ENDING", ET_MorphemicContext::NOT_ENDING },
            { L"ROOT", ET_MorphemicContext::ROOT },
            { L"ROOT_AUSLAUT", ET_MorphemicContext::ROOT_AUSLAUT }
        };

        map<CEString, ET_SyllabicPosition> m_mapStringToSyllabicPosition =
        {
            { L"ULTIMA", ET_SyllabicPosition::SYLLABIC_POSITION_ULTIMA },
            { L"PAENULT", ET_SyllabicPosition::SYLLABIC_POSITION_PENULT },
            { L"ANTEPAENULT", ET_SyllabicPosition::SYLLABIC_POSITION_ANTEPENULT }
        };

        map<CEString, ET_Subparadigm> m_mapStringToSubparadigm =
        {
            { L"SUBPARADIGM_NOUN", ET_Subparadigm::SUBPARADIGM_NOUN },
            { L"SUBPARADIGM_LONG_ADJ", ET_Subparadigm::SUBPARADIGM_LONG_ADJ },
            { L"SUBPARADIGM_SHORT_ADJ", ET_Subparadigm::SUBPARADIGM_SHORT_ADJ },
            { L"SUBPARADIGM_COMPARATIVE", ET_Subparadigm::SUBPARADIGM_COMPARATIVE },
            { L"SUBPARADIGM_PRONOUN", ET_Subparadigm::SUBPARADIGM_PRONOUN },
            { L"SUBPARADIGM_PRONOUN_ADJ", ET_Subparadigm::SUBPARADIGM_PRONOUN_ADJ },
            { L"SUBPARADIGM_INFINITIVE", ET_Subparadigm::SUBPARADIGM_INFINITIVE },
            { L"SUBPARADIGM_PRESENT_TENSE", ET_Subparadigm::SUBPARADIGM_PRESENT_TENSE },
            { L"SUBPARADIGM_PAST_TENSE", ET_Subparadigm::SUBPARADIGM_PAST_TENSE },
            { L"SUBPARADIGM_IMPERATIVE", ET_Subparadigm::SUBPARADIGM_IMPERATIVE },
            { L"SUBPARADIGM_ADVERBIAL_PRESENT", ET_Subparadigm::SUBPARADIGM_ADVERBIAL_PRESENT },
            { L"SUBPARADIGM_ADVERBIAL_PAST", ET_Subparadigm::SUBPARADIGM_ADVERBIAL_PAST },
            { L"SUBPARADIGM_PART_PRES_ACT", ET_Subparadigm::SUBPARADIGM_PART_PRES_ACT },
            { L"SUBPARADIGM_PART_PRES_PASS_LONG", ET_Subparadigm::SUBPARADIGM_PART_PRES_PASS_LONG },
            { L"SUBPARADIGM_PART_PRES_PASS_SHORT", ET_Subparadigm::SUBPARADIGM_PART_PRES_PASS_SHORT },
            { L"SUBPARADIGM_PART_PAST_ACT", ET_Subparadigm::SUBPARADIGM_PART_PAST_ACT },
            { L"SUBPARADIGM_PART_PAST_PASS_LONG", ET_Subparadigm::SUBPARADIGM_PART_PAST_PASS_LONG },
            { L"SUBPARADIGM_PART_PAST_PASS_SHORT", ET_Subparadigm::SUBPARADIGM_PART_PAST_PASS_SHORT },
            { L"SUBPARADIGM_NUM", ET_Subparadigm::SUBPARADIGM_NUM },
            { L"SUBPARADIGM_NUM_ADJ", ET_Subparadigm::SUBPARADIGM_NUM_ADJ },
            { L"SUBPARADIGM_ASPECT_PAIR", ET_Subparadigm::SUBPARADIGM_ASPECT_PAIR },
            { L"SUBPARADIGM_PARTICLE", ET_Subparadigm::SUBPARADIGM_PARTICLE },
            { L"SUBPARADIGM_PREPOSITION", ET_Subparadigm::SUBPARADIGM_PREPOSITION },
            { L"SUBPARADIGM_ADVERB", ET_Subparadigm::SUBPARADIGM_ADVERB },
            { L"SUBPARADIGM_CONJUNCTION", ET_Subparadigm::SUBPARADIGM_CONJUNCTION },
            { L"SUBPARADIGM_INTERJECTION", ET_Subparadigm::SUBPARADIGM_INTERJECTION },
            { L"SUBPARADIGM_PARENTHESIS", ET_Subparadigm::SUBPARADIGM_PARENTHESIS },
            { L"SUBPARADIGM_PREDICATIVE", ET_Subparadigm::SUBPARADIGM_PREDICATIVE }
        };

        map<CEString, ET_Gender> m_mapStringToGender =
        {
            { L"GENDER_M", ET_Gender::GENDER_M },
            { L"GENDER_F", ET_Gender::GENDER_F },
            { L"GENDER_N", ET_Gender::GENDER_N }
        };

        map<CEString, ET_Number> m_mapStringToNumber =
        {
            { L"NUM_SG", ET_Number::NUM_SG },
            { L"NUM_PL", ET_Number::NUM_PL }
        };

        map<CEString, ET_Case> m_mapStringToCase =
        {
            { L"CASE_NOM", ET_Case::CASE_NOM },
            { L"CASE_ACC", ET_Case::CASE_ACC },
            { L"CASE_GEN", ET_Case::CASE_GEN },
            { L"CASE_DAT", ET_Case::CASE_DAT },
            { L"CASE_INST", ET_Case::CASE_INST },
            { L"CASE_PREP", ET_Case::CASE_PREP },
            { L"CASE_LOC", ET_Case::CASE_LOC },
            { L"CASE_NUM", ET_Case::CASE_NUM }
        };

        map<CEString, ET_RuleStrength> m_mapStringToRuleStrength =
        {
            { L"DEFAULT", ET_RuleStrength::RULE_STRENGTH_DEFAULT },
            { L"VARIATION", ET_RuleStrength::RULE_STRENGTH_VARIATION },
            { L"RECOMMENDED", ET_RuleStrength::RULE_STRENGTH_RECOMMENDED },
            { L"OPTIONAL", ET_RuleStrength::RULE_STRENGTH_OPTIONAL }
        };

        unordered_map<ET_Sound, StVowel> m_mapSoundToVowel =
        {
            { STRESSED_A, StVowel(ET_VowelStressRelation::STRESSED, ET_Frontness::FRONTNESS_CENTRAL, ET_Height::HEIGHT_LOW) },
            { STRESSED_O, StVowel(ET_VowelStressRelation::STRESSED, ET_Frontness::FRONTNESS_BACK, ET_Height::HEIGHT_MID) },
            { STRESSED_E, StVowel(ET_VowelStressRelation::STRESSED, ET_Frontness::FRONTNESS_FRONT, ET_Height::HEIGHT_MID) },
            { I, StVowel(ET_VowelStressRelation::VOWEL_STRESS_RELATION_UNDEFINED, ET_Frontness::FRONTNESS_FRONT, ET_Height::HEIGHT_HIGH) },
            { Y, StVowel(ET_VowelStressRelation::VOWEL_STRESS_RELATION_UNDEFINED, ET_Frontness::FRONTNESS_CENTRAL, ET_Height::HEIGHT_HIGH) },
            { U, StVowel(ET_VowelStressRelation::VOWEL_STRESS_RELATION_UNDEFINED, ET_Frontness::FRONTNESS_BACK, ET_Height::HEIGHT_HIGH) },
            { PRETONIC_A, StVowel(ET_VowelStressRelation::PRETONIC, ET_Frontness::FRONTNESS_CENTRAL, ET_Height::HEIGHT_LOW) },
            { PRETONIC_LONG_A, StVowel(ET_VowelStressRelation::PRETONIC, ET_Frontness::FRONTNESS_CENTRAL, ET_Height::HEIGHT_LOW) },
            { PRETONIC_HARD_IE, StVowel(ET_VowelStressRelation::PRETONIC, ET_Frontness::FRONTNESS_CENTRAL, ET_Height::HEIGHT_HIGH) },
            { PRETONIC_SOFT_IE, StVowel(ET_VowelStressRelation::PRETONIC, ET_Frontness::FRONTNESS_FRONT, ET_Height::HEIGHT_HIGH) },
            { SCHWA, StVowel(ET_VowelStressRelation::POSTTONIC, ET_Frontness::FRONTNESS_CENTRAL, ET_Height::HEIGHT_MID) },
            { UNSTRESSED_FRONT_VOWEL, StVowel(ET_VowelStressRelation::POSTTONIC, ET_Frontness::FRONTNESS_FRONT, ET_Height::HEIGHT_HIGH) },
            { UNSTRESSED_E, StVowel(ET_VowelStressRelation::POSTTONIC, ET_Frontness::FRONTNESS_FRONT, ET_Height::HEIGHT_MID) },
            { UNSTRESSED_O, StVowel(ET_VowelStressRelation::POSTTONIC, ET_Frontness::FRONTNESS_BACK, ET_Height::HEIGHT_MID) }
        };

        unordered_map<ET_Sound, StConsonant> m_mapSoundToConsonant =
        {
            { P_HARD, StConsonant(ET_Place::PLACE_BILABIAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { P_SOFT, StConsonant(ET_Place::PLACE_BILABIAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) },
            { B_HARD, StConsonant(ET_Place::PLACE_BILABIAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { B_SOFT, StConsonant(ET_Place::PLACE_BILABIAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { M_HARD, StConsonant(ET_Place::PLACE_BILABIAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NASAL) },
            { M_SOFT, StConsonant(ET_Place::PLACE_BILABIAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NASAL) },
            { F_HARD, StConsonant(ET_Place::PLACE_LABIODENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { F_SOFT, StConsonant(ET_Place::PLACE_LABIODENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) },
            { V_HARD, StConsonant(ET_Place::PLACE_LABIODENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { V_SOFT, StConsonant(ET_Place::PLACE_LABIODENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) },
            { N_HARD, StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NASAL) },
            { N_SOFT, StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NASAL) },
            { T_HARD, StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { T_SOFT, StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) },
            { D_HARD, StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { D_SOFT, StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) },
            { TS_HARD, StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { TS_SOFT, StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) },
            { S_HARD, StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { S_SOFT, StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) },
            { Z_HARD, StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { Z_SOFT, StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) },
            { L_HARD, StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_APPROXIMANT, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { L_SOFT, StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_APPROXIMANT, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) },
            { R_HARD, StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_TRILL, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },                // post-alveolar?
            { R_SOFT, StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_TRILL, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) },
            { SH_HARD, StConsonant(ET_Place::PLACE_ALVEOLAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { SH_SOFT, StConsonant(ET_Place::PLACE_ALVEOLAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) },
            { ZH_HARD, StConsonant(ET_Place::PLACE_ALVEOLAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },         // post-alveolar?
            { ZH_SOFT, StConsonant(ET_Place::PLACE_ALVEOLAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) },      // palatal?
            { JOD, StConsonant(ET_Place::PLACE_PALATAL, ET_Manner::MANNER_APPROXIMANT, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) },
            { K_HARD, StConsonant(ET_Place::PLACE_VELAR, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },         // post-alveolar?
            { K_SOFT, StConsonant(ET_Place::PLACE_VELAR, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) },      // palatal?
            { G_HARD, StConsonant(ET_Place::PLACE_VELAR, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },         // post-alveolar?
            { G_SOFT, StConsonant(ET_Place::PLACE_VELAR, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) },      // palatal?
            { X_HARD, StConsonant(ET_Place::PLACE_VELAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },         // post-alveolar?
            { X_SOFT, StConsonant(ET_Place::PLACE_VELAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) },      // palatal?
            { G_FRICATIVE_HARD, StConsonant(ET_Place::PLACE_VELAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },         // post-alveolar?
            { G_FRICATIVE_SOFT, StConsonant(ET_Place::PLACE_VELAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) }       // palatal?
        };

        map<StVowel, ET_Sound> m_mapVowelToSound =
        {
            { StVowel(ET_VowelStressRelation::STRESSED, ET_Frontness::FRONTNESS_CENTRAL, ET_Height::HEIGHT_LOW), STRESSED_A },
            { StVowel(ET_VowelStressRelation::STRESSED, ET_Frontness::FRONTNESS_BACK, ET_Height::HEIGHT_MID), STRESSED_O },
            { StVowel(ET_VowelStressRelation::STRESSED, ET_Frontness::FRONTNESS_FRONT, ET_Height::HEIGHT_MID), STRESSED_E },
            { StVowel(ET_VowelStressRelation::VOWEL_STRESS_RELATION_UNDEFINED, ET_Frontness::FRONTNESS_FRONT, ET_Height::HEIGHT_HIGH), I },
            { StVowel(ET_VowelStressRelation::VOWEL_STRESS_RELATION_UNDEFINED, ET_Frontness::FRONTNESS_CENTRAL, ET_Height::HEIGHT_HIGH), Y },
            { StVowel(ET_VowelStressRelation::VOWEL_STRESS_RELATION_UNDEFINED, ET_Frontness::FRONTNESS_BACK, ET_Height::HEIGHT_HIGH), U },
            { StVowel(ET_VowelStressRelation::PRETONIC, ET_Frontness::FRONTNESS_CENTRAL, ET_Height::HEIGHT_LOW), PRETONIC_A },
            { StVowel(ET_VowelStressRelation::PRETONIC, ET_Frontness::FRONTNESS_CENTRAL, ET_Height::HEIGHT_LOW), PRETONIC_LONG_A },
            { StVowel(ET_VowelStressRelation::PRETONIC, ET_Frontness::FRONTNESS_CENTRAL, ET_Height::HEIGHT_HIGH), PRETONIC_HARD_IE },
            { StVowel(ET_VowelStressRelation::PRETONIC, ET_Frontness::FRONTNESS_FRONT, ET_Height::HEIGHT_HIGH), PRETONIC_SOFT_IE },
            { StVowel(ET_VowelStressRelation::POSTTONIC, ET_Frontness::FRONTNESS_CENTRAL, ET_Height::HEIGHT_MID), SCHWA },
            { StVowel(ET_VowelStressRelation::POSTTONIC, ET_Frontness::FRONTNESS_FRONT, ET_Height::HEIGHT_HIGH), UNSTRESSED_FRONT_VOWEL },
            { StVowel(ET_VowelStressRelation::POSTTONIC, ET_Frontness::FRONTNESS_FRONT, ET_Height::HEIGHT_MID), UNSTRESSED_E },
            { StVowel(ET_VowelStressRelation::POSTTONIC, ET_Frontness::FRONTNESS_BACK, ET_Height::HEIGHT_MID), UNSTRESSED_O }
        };

        map<StConsonant, ET_Sound> m_mapConsonantToSound =
        {
            { StConsonant(ET_Place::PLACE_BILABIAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL), P_HARD },
            { StConsonant(ET_Place::PLACE_BILABIAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), P_SOFT },
            { StConsonant(ET_Place::PLACE_BILABIAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL), B_HARD },
            { StConsonant(ET_Place::PLACE_BILABIAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), B_SOFT },
            { StConsonant(ET_Place::PLACE_BILABIAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NASAL), M_HARD },
            { StConsonant(ET_Place::PLACE_BILABIAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NASAL), M_SOFT },
            { StConsonant(ET_Place::PLACE_LABIODENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL), F_HARD },
            { StConsonant(ET_Place::PLACE_LABIODENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), F_SOFT },
            { StConsonant(ET_Place::PLACE_LABIODENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL), V_HARD },
            { StConsonant(ET_Place::PLACE_LABIODENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), V_SOFT },
            { StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NASAL), N_HARD },
            { StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NASAL), N_SOFT },
            { StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL), T_HARD },
            { StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), T_SOFT },
            { StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL), D_HARD },
            { StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), D_SOFT },
            { StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL), TS_HARD },
            { StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), TS_SOFT },
            { StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL), S_HARD },
            { StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), S_SOFT },
            { StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL), Z_HARD },
            { StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), Z_SOFT },
            { StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_APPROXIMANT, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL), L_HARD },
            { StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_APPROXIMANT, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), L_SOFT },
            { StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_TRILL, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL), R_HARD },                // post-alveolar?
            { StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_TRILL, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), R_SOFT },
            { StConsonant(ET_Place::PLACE_ALVEOLAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL), SH_HARD },
            { StConsonant(ET_Place::PLACE_ALVEOLAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), SH_SOFT },
            { StConsonant(ET_Place::PLACE_ALVEOLAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL), ZH_HARD },         // post-alveolar?
            { StConsonant(ET_Place::PLACE_ALVEOLAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), ZH_SOFT },      // palatal?
            { StConsonant(ET_Place::PLACE_PALATAL, ET_Manner::MANNER_APPROXIMANT, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), JOD },
            { StConsonant(ET_Place::PLACE_VELAR, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL), K_HARD },         // post-alveolar?
            { StConsonant(ET_Place::PLACE_VELAR, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), K_SOFT },      // palatal?
            { StConsonant(ET_Place::PLACE_VELAR, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL), G_HARD },         // post-alveolar?
            { StConsonant(ET_Place::PLACE_VELAR, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), G_SOFT },      // palatal?
            { StConsonant(ET_Place::PLACE_VELAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL), X_HARD },         // post-alveolar?
            { StConsonant(ET_Place::PLACE_VELAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), X_SOFT },      // palatal?
            { StConsonant(ET_Place::PLACE_VELAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL), G_FRICATIVE_HARD },         // post-alveolar?
            { StConsonant(ET_Place::PLACE_VELAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), G_FRICATIVE_SOFT },      // palatal?
            { StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_AFFRICATE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL), TS_HARD },
            { StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_AFFRICATE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), TS_SOFT },
            { StConsonant(ET_Place::PLACE_ALVEOLAR, ET_Manner::MANNER_AFFRICATE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL), CH}
        };

        unordered_map<wchar_t, StConsonant> m_mapCharToConsonant =
        {
            { L'б', StConsonant(ET_Place::PLACE_BILABIAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { L'в', StConsonant(ET_Place::PLACE_LABIODENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { L'г', StConsonant(ET_Place::PLACE_VELAR, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },         // post-alveolar?
            { L'д', StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { L'ж', StConsonant(ET_Place::PLACE_ALVEOLAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },         // post-alveolar?
            { L'з', StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { L'й', StConsonant(ET_Place::PLACE_PALATAL, ET_Manner::MANNER_APPROXIMANT, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) },
            { L'к', StConsonant(ET_Place::PLACE_VELAR, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },         // post-alveolar?
            { L'л', StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_APPROXIMANT, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { L'м', StConsonant(ET_Place::PLACE_BILABIAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NASAL) },
            { L'н', StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NASAL) },
            { L'п', StConsonant(ET_Place::PLACE_BILABIAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { L'р', StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_TRILL, ET_Voicedness::VOICEDNESS_VOICED, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },                // post-alveolar?
            { L'с', StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { L'т', StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_STOP, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { L'ф', StConsonant(ET_Place::PLACE_LABIODENTAL, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { L'х', StConsonant(ET_Place::PLACE_VELAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },         // post-alveolar?
            { L'ц', StConsonant(ET_Place::PLACE_DENTAL, ET_Manner::MANNER_AFFRICATE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { L'ч', StConsonant(ET_Place::PLACE_ALVEOLAR, ET_Manner::MANNER_AFFRICATE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) },
            { L'ш', StConsonant(ET_Place::PLACE_ALVEOLAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_HARD, ET_Nasality::NASALITY_NON_NASAL) },
            { L'щ', StConsonant(ET_Place::PLACE_ALVEOLAR, ET_Manner::MANNER_FRICATIVE, ET_Voicedness::VOICEDNESS_VOICELESS, ET_Palatalization::PALATALIZATION_SOFT, ET_Nasality::NASALITY_NON_NASAL) }
        };
    };      //  class CTranscriber

}   // namespace Hlib

#endif      //  C_TRANSCRIBER_H_INCLUDED
