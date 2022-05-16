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
        StLexemeProperties() :
            llDescriptorId(-1),
            llHeadwordId(-1),
            llSecondPartId(0),
            bIsUnstressed(false),
            bSpryazhSm(false),
            llSourceEntryId(-1),
            bIsVariant(false),
            bIsPluralOf(false),
            bTransitive(false),
            eReflexive(REFL_UNDEFINED),
            ePartOfSpeech(POS_UNDEFINED),
            iSection(-1),
            bPartPastPassZhd(false),
            bNoComparative(false),
            bAssumedForms(false),
            bYoAlternation(false),
            bOAlternation(false),
            bSecondGenitive(false),
            bSecondPrepositional(false),
            bIsImpersonal(false),
            bIsIterative(false),
            bHasAspectPair(false),
            iAspectPairType(0),
            iAltAspectPairType(0),
            bHasDifficultForms(false),
            bHasMissingForms(false),
            bHasIrregularForms(false),
            bHasIrregularVariants(false),
            bPrimaryInflectionGroup(false),
            iType(-1),
            eAccentType1(AT_UNDEFINED),
            eAccentType2(AT_UNDEFINED),
            bShortFormsRestricted(false),
            bPastParticipleRestricted(false),
            bNoLongForms(false),
            bShortFormsIncomplete(false),
            bNoPassivePastParticiple(false),
            bFleetingVowel(false),
            iStemAugment(-1),
            bSecondPart(false),
            iSpryazhSmRefPrefixLength(-1),
            bSpryazhSmNoAspectPair(false)
        {
                sSourceForm.SetVowels(CEString::g_szRusVowels);
                sGraphicStem.SetVowels(CEString::g_szRusVowels);
        }

        long long llDescriptorId;
        long long llHeadwordId;
        long long llSecondPartId;
        CEString sSourceForm;
        CEString sHeadwordComment;
        CEString sHeadwordVariant;
        CEString sHeadwordVariantComment;
        vector<int> vecSourceStressPos, vecSecondaryStressPos;
        vector<int> vecSourceVariantStressPos, vecSecondaryVariantStressPos;
        bool bIsUnstressed;
        vector<int> vecHomonyms;
        CEString sPluralOf;
        CEString sUsage;
        CEString sSeeRef;
        CEString sBackRef;
        bool bSpryazhSm;
        long long llSourceEntryId;
        CEString sGraphicStem;
        bool bIsVariant;
        CEString sMainSymbol;
        bool bIsPluralOf;
        bool bTransitive;
        ET_Reflexivity eReflexive;
        CEString sMainSymbolPluralOf;
        CEString sAltMainSymbol;
        ET_Aspect eAspect;
        CEString sInflectionType;
        ET_PartOfSpeech ePartOfSpeech;
        CEString sComment;
        CEString sAltMainSymbolComment;
        CEString sAltInflectionComment;
        CEString sVerbStemAlternation;
        int iSection;
        bool bPartPastPassZhd;
        bool bNoComparative;
        bool bAssumedForms;
        bool bYoAlternation;
        bool bOAlternation;
        bool bSecondGenitive;
        bool bSecondPrepositional;
        bool bSecondPrepositionalOptional;
        CEString sP2Preposition;
        bool bIsImpersonal;
        bool bIsIterative;
        bool bHasAspectPair;
        int iAspectPairType;
        int iAltAspectPairType;
        CEString sAspectPairData;
        CEString sAltAspectPairData;
        CEString sAltAspectPairComment;
        bool bHasDifficultForms;
        bool bHasMissingForms;
        bool bHasIrregularForms;
        CEString sIrregularFormsLeadComment;
        bool bHasIrregularVariants;    // true if any of irreg. forms alternates with regular
        CEString sRestrictedContexts;
        CEString sContexts;
        CEString sCognate;
        CEString sTrailingComment;
        ET_Subparadigm eSubparadigm;    // currently only used with numerals
        bool bHasPresPassParticiple;

        // From inflection table:
        long long llInflectionId;
        bool bPrimaryInflectionGroup;
        int iType;
        ET_AccentType eAccentType1;
        ET_AccentType eAccentType2;
        bool bShortFormsRestricted;
        bool bPastParticipleRestricted;
        bool bNoLongForms;
        bool bShortFormsIncomplete;
        bool bNoPassivePastParticiple;
        bool bFleetingVowel;
        int iStemAugment;
        bool bSecondPart;
        int iSpryazhSmRefPrefixLength;
        bool bSpryazhSmNoAspectPair;

        //
        // Verb stems & infinitive:
        //
        CEString sInfStem;
        CEString s1SgStem;
        CEString s3SgStem;

        CEString sInfinitive;

        map<int, bool> mapCommonDeviations;

        // Spryazh sm. 
        CEString sSpryazhSmRefSource;
        CEString sSpryazhSmPrefix;
    };

}       //  namespace Hlib

#endif  //  LEXEMEPROPERTIES_H_INCLUDED
