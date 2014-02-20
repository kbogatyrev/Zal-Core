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
        virtual ILexeme * pLexeme() = 0;
        virtual CEString sWordForm() = 0;
        virtual CEString sLemma() = 0;
        virtual __int64 llLexemeId() = 0;
        //    &&&&map<int, ET_StressType> m_mapStress = 0; // <stressed syll, primary/secondary>
        virtual ET_PartOfSpeech ePos() = 0;
        virtual ET_Case eCase() = 0;
        virtual ET_Number eNumber() = 0;
        virtual ET_Subparadigm eSubparadigm() = 0;
        virtual ET_Gender eGender() = 0;
        virtual ET_Person ePerson() = 0;
        virtual ET_Animacy eAnimacy() = 0;
        virtual ET_Reflexive eReflexive() = 0;
        virtual ET_Aspect eAspect() = 0;
        virtual ET_Status eStatus() = 0;
        virtual bool bIrregular() = 0;      // came from the DB as opposed to being generated by the app

        virtual int iGramHash() = 0;
        virtual ET_ReturnCode eInitFromHash(int iHash) = 0;
//        virtual ET_ReturnCode SaveTestData() = 0;

    };

}

#endif  //  I_WORDFORM_H_INCLUDED
