#ifndef INFLECTIONPROPERTIES_H_INCLUDED
#define INFLECTIONPROPERTIES_H_INCLUDED

#include <map>

#include "Enums.h"
//#include "EString.h"

namespace Hlib
{
    struct StInflectionProperties
    {
        // From inflection table:
        // From inflection table:
        int64_t llInflectionId { -1 };
        bool bPrimaryInflectionGroup { false };
        int iType { -1 };
        ET_AccentType eAccentType1 { AT_UNDEFINED };
        ET_AccentType eAccentType2 { AT_UNDEFINED };
        bool bShortFormsRestricted { false };
        bool bPastParticipleRestricted { false };
//        bool bNoLongForms { false };
        bool bShortFormsIncomplete { false };
        bool bNoPassivePastParticiple { false };
        bool bFleetingVowel { false };
        int iStemAugment { -1 };
        bool bSecondGenitive{ false };
        CEString sComment;
        bool bSecondPrepositional{ false };
        bool bSecondPrepositionalOptional{ false };
        CEString sP2Preposition;

        int64_t llSklonSmId { -1 };
        CEString sAddition;

        map<int, bool> mapCommonDeviations;     // Number in circle --> is optional
    };
}       //  namespace Hlib

#endif  //  INFLECTIONPROPERTIES_H_INCLUDED
