#ifndef I_WORDFORM_H_INCLUDED
#define I_WORDFORM_H_INCLUDED

#include "Enums.h"
#include "LexemeProperties.h"
#include "Estring.h"

namespace Hlib
{
    struct ILexeme;

    struct IWordForm
    {
        virtual ~IWordForm() {};
        virtual ILexeme * pLexeme() = 0;
        virtual void SetLexeme(ILexeme * pLexeme) = 0;
        virtual long long llDbId() = 0;
        virtual CEString sWordForm() = 0;
        virtual void SetWordForm(const CEString&) = 0;
        virtual CEString sStem() = 0;
        virtual void SetStem(const CEString&) = 0;
        virtual __int64 llLexemeId() = 0;
        virtual ET_PartOfSpeech ePos() = 0;
        virtual void SetPos(ET_PartOfSpeech) = 0;
        virtual ET_Case eCase() = 0;
        virtual void SetCase(ET_Case) = 0;
        virtual ET_Number eNumber() = 0;
        virtual void SetNumber(ET_Number) = 0;
        virtual ET_Subparadigm eSubparadigm() = 0;
        virtual void SetSubparadigm(ET_Subparadigm) = 0;
        virtual ET_Gender eGender() = 0;
        virtual void SetGender(ET_Gender) = 0;
        virtual ET_Person ePerson() = 0;
        virtual void SetPerson(ET_Person) = 0;
        virtual ET_Animacy eAnimacy() = 0;
        virtual void SetAnimacy(ET_Animacy) = 0;
        virtual ET_Reflexivity eReflexive() = 0;
        virtual void SetReflexive(ET_Reflexivity) = 0;
        virtual ET_Aspect eAspect() = 0;
        virtual void SetAspect(ET_Aspect) = 0;
        virtual ET_Status eStatus() = 0;
        virtual void SetStatus(ET_Status) = 0;
        virtual bool bIrregular() = 0;      // came from the DB as opposed to being generated by the app
        virtual void SetIrregular(bool) = 0;
        virtual CEString sLeadComment() = 0;
        virtual void SetLeadComment(const CEString&) = 0;
        virtual CEString sTrailingComment() = 0;
        virtual bool bIsEdited() = 0;
        virtual void SetIsEdited(bool) = 0;
        virtual bool bIsVariant() = 0;
        virtual void SetIsVariant(bool) = 0;
        virtual void SetTrailingComment(const CEString&) = 0;
        virtual ET_ReturnCode eGetFirstStressPos(int& iPos, ET_StressType& eType) = 0;
        virtual ET_ReturnCode eGetNextStressPos(int& iPos, ET_StressType& eType) = 0;
        virtual ET_ReturnCode eSetStressPositions(map<int, ET_StressType> mapStress) = 0;
        virtual ET_ReturnCode eSaveIrregularForm() = 0;
//        virtual ET_ReturnCode eSaveIrregularStress(map<int, ET_StressType>&) = 0;

        virtual CEString sGramHash() = 0;
        virtual ET_ReturnCode eInitFromHash(const CEString& sHash) = 0;
//        virtual ET_ReturnCode SaveTestData() = 0;

        virtual bool bSaveToDb() = 0;

        virtual ET_ReturnCode eClone(IWordForm *&) = 0;

    };

}

#endif  //  I_WORDFORM_H_INCLUDED
