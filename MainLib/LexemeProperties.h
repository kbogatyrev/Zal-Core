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
        CEString sPluralOf;
        CEString sUsage;
        CEString sSeeRef;
        CEString sBackRef;
        bool bSpryazhSm { false };
        long long llSourceEntryId { -1 };
        CEString sGraphicStem;
        bool bIsVariant { false };
        CEString sMainSymbol;
        bool bIsPluralOf { false };
        bool bTransitive { false };
        ET_Reflexivity eReflexive { REFL_UNDEFINED };
        CEString sMainSymbolPluralOf;
        CEString sAltMainSymbol;
        ET_Aspect eAspect { ASPECT_UNDEFINED };
        CEString sInflectionType;
        ET_PartOfSpeech ePartOfSpeech { POS_UNDEFINED };
        CEString sComment;
        CEString sAltMainSymbolComment;
        CEString sAltInflectionComment;
        CEString sVerbStemAlternation;
        int iSection{ -1 };
        bool bPartPastPassZhd { false };
        bool bNoComparative { false };
        bool bAssumedForms { false };
        bool bYoAlternation { false };
        bool bOAlternation { false };
//        bool bSecondGenitive { false };
//        bool bSecondPrepositional { false };
//        bool bSecondPrepositionalOptional { false };
//        CEString sP2Preposition;
        bool bIsImpersonal { false };
        bool bIsIterative { false };
        bool bHasAspectPair { false };
        int iAspectPairType { -1 };
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
        bool bHasPresPassParticiple { false };

        bool bNoLongForms { false };
        bool bSecondPart { false };
        int iSpryazhSmRefPrefixLength { -1 };
        bool bSpryazhSmNoAspectPair { false };

        //
        // Verb stems & infinitive:
        //
        CEString sInfStem;
        CEString s1SgStem;
        CEString s3SgStem;

        CEString sInfinitive;

        // Spryazh sm. 
        CEString sSpryazhSmRefSource;
        CEString sSpryazhSmPrefix;
    };

}       //  namespace Hlib

#endif  //  LEXEMEPROPERTIES_H_INCLUDED
