#ifndef LEXEMEPROPERTIES_H_INCLUDED
#define LEXEMEPROPERTIES_H_INCLUDED

#include <map>

#include "Enums.h"
#include "EString.h"

namespace Hlib
{
    struct StLexemeProperties
    {
        //
        // Stem data as acquired from the DB
        //
        StLexemeProperties()
        {
            sSourceForm.SetVowels(CEString::g_szRusVowels);
            sGraphicStem.SetVowels(CEString::g_szRusVowels);
        }

        long long llDescriptorId { -1 };
        long long llHeadwordId { -1 };
        long long llSecondPartId { -1 };
        CEString sSourceForm;
        CEString sHeadwordComment;
        CEString sHeadwordVariant;
        CEString sHeadwordVariantComment;
        vector<int> vecSourceStressPos, vecSecondaryStressPos;
        vector<int> vecSourceVariantStressPos, vecSecondaryVariantStressPos;
        bool bIsUnstressed { false };
        vector<int> vecHomonyms;
        CEString sMainSymbol;
        CEString sInflectionType;
        CEString sMainSymbolPluralOf;
        CEString sAltMainSymbol;
        ET_Aspect eAspect{ ASPECT_UNDEFINED };
        bool bTransitive{ false };
        ET_Reflexivity eReflexive{ REFL_UNDEFINED };
        bool bIsPluralOf{ false };
        CEString sPluralOf;
        bool bIsVariant{ false };
        bool bSpryazhSm{ false };
        int iSection{ -1 };
        CEString sUsage;
        CEString sSeeRef;
        CEString sBackRef;
        CEString sComment;
        CEString sAltMainSymbolComment;
        CEString sAltInflectionComment;
        CEString sVerbStemAlternation;
        bool bPartPastPassZhd{ false };
        bool bNoComparative{ false };
        bool bAssumedForms{ false };
        bool bYoAlternation{ false };
        bool bOAlternation{ false };

        long long llSourceEntryId { -1 };
        CEString sGraphicStem;
        ET_PartOfSpeech ePartOfSpeech { POS_UNDEFINED };
        bool bIsImpersonal { false };
        bool bIsIterative { false };
        bool bHasAspectPair { false };
        int iAspectPairType { 0 };
        int iAltAspectPairType { 0 };       // NB: -1 is a valid aspect pair type, 0 means 'undefined'
        CEString sAspectPairData;
        CEString sAltAspectPairData;
        CEString sAltAspectPairComment;
        bool bHasDifficultForms { false };
        bool bHasMissingForms { false };
        bool bHasIrregularForms { false };
        CEString sIrregularFormsLeadComment;
        bool bHasIrregularVariants { false };    // true if any of irreg. forms alternates with regular
        CEString sRestrictedContexts;
        CEString sContexts;
        CEString sCognate;
        CEString sTrailingComment;
        ET_Subparadigm eSubparadigm { SUBPARADIGM_UNDEFINED };    // currently only used with numerals

        bool bNoLongForms { false };
        bool bSecondPart { false };

        //
        // Verb stems & infinitive:
        //
        CEString sInfStem;
        CEString s1SgStem;
        CEString s3SgStem;

        CEString sInfinitive;

        // Spryazh. sm. 
        CEString sSpryazhSmRefSource;
        CEString sSpryazhSmPrefix;
        CEString sSpryazhSmRefHomonyms;
        CEString sSpryazhSmComment;
        int iSpryazhSmRefPrefixLength{ -1 };
        bool bSpryazhSmNoAspectPair{ false };

        // Sklon. sm.
        bool bSklonSm{ false };
        long long llSklonSmInflectionId{ -1 };
        CEString sSklonSmRemove;
        CEString sSklonSmAdd;
        vector<int> vecSklonSmStressPos;
        vector<int> vecSklonSmSecondaryStressPos;
    };

}       //  namespace Hlib

#endif  //  LEXEMEPROPERTIES_H_INCLUDED
