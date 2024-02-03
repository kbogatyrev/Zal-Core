#ifndef SINGLETON_H_INCLUDED
#define SINGLETON_H_INCLUDED

#include <memory>

#include "Enums.h"
#include "Dictionary.h"
#include "Lexeme.h"

namespace Hlib
{
    class Singleton
    {
    public:
        ET_ReturnCode eGetDictionary(shared_ptr<CDictionary>& spDictionary);

        int64_t iAddLexeme(CLexeme* spLexeme);
        ET_ReturnCode eGetLexeme(int64_t iHandle, CLexeme*& pLexeme);
        ET_ReturnCode RemoveLexeme(int64_t iHandle);

        int64_t iAddInflection(CInflection* pInflection);
        ET_ReturnCode eGetInflection(int64_t iHandle, CInflection*& pInflection);
        ET_ReturnCode eRemoveInflection(int64_t iHandle);

        int64_t iAddWordForm(CWordForm* pWordForm);
        ET_ReturnCode eGetWordForm(int64_t iHandle, CWordForm*& pWordForm);
        ET_ReturnCode eRemoveWordForm(int64_t iHandle);
        ET_ReturnCode eStoreCreatedWordForm(shared_ptr<CWordForm>);  // To avoid shared_ptr being released
        int iNCreatedForms();
        ET_ReturnCode eGetFirstCreatedWordForm(shared_ptr<CWordForm>&);
        ET_ReturnCode eGetNextCreatedWordForm(shared_ptr<CWordForm>&);
        void ClearCreatedForms();

        int64_t iAddLexemeEnumerator(shared_ptr<CLexemeEnumerator> spLexemeEnumerator);
        ET_ReturnCode eGetLexemeEnumerator(int64_t iHandle, shared_ptr<CLexemeEnumerator>& spLexemeEnumerator);
        ET_ReturnCode eRemoveLexemeEnumerator(int64_t iHandle);

        int64_t iAddInflectionEnumerator(shared_ptr<CInflectionEnumerator> spInflectionEnumerator);
        ET_ReturnCode eGetInflectionEnumerator(int64_t iHandle, shared_ptr<CInflectionEnumerator>& spInflectionEnumerator);
        ET_ReturnCode eRemoveInflectionEnumerator(int64_t iHandle);

        void SetParser(shared_ptr<CParser> spParser);
        ET_ReturnCode eGetParser(shared_ptr<CParser>& spParser);

        ET_ReturnCode eGetVerifier(shared_ptr<CVerifier>& spVerifier);

        static Singleton* pGetInstance();

    private:
        Singleton();

        shared_ptr<CDictionary> m_spDictionary;
        shared_ptr<CParser> m_spParser;
        shared_ptr<CVerifier> m_spVerifier;
        map<int64_t, CLexeme*> m_mapLexemes;
        map<int64_t, CInflection*> m_mapInflections;
        map<int64_t, CWordForm*> m_mapWordForms;
        vector<shared_ptr<CWordForm>> m_vecCreatedForms;
        vector<shared_ptr<CWordForm>>::iterator m_itCurrentCreatedForm;
        map <int64_t, shared_ptr<CLexemeEnumerator>> m_mapLexemeEnumerators;
        map <int64_t, shared_ptr<CInflectionEnumerator>> m_mapInflectionEnumerators;
    };
}

#endif    //  SINGLETON_H_INCLUDED
