// MainLib.cpp : Implementation of DLL Exports.

#ifndef MAINLIB_H_INCLUDED
#define MAINLIB_H_INCLUDED

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

        int64_t iAddLexeme(shared_ptr<CLexeme> spLexeme);
        ET_ReturnCode eGetLexeme(int64_t iHandle, shared_ptr<CLexeme>& spLexeme);
        ET_ReturnCode RemoveLexeme(int64_t iHandle);

        int64_t iAddInflection(shared_ptr<CInflection> spInflection);
        ET_ReturnCode eGetInflection(int64_t iHandle, shared_ptr<CInflection>& spInflection);
        ET_ReturnCode RemoveInflection(int64_t iHandle);

        int64_t iAddWordForm(shared_ptr<CWordForm> spWordForm);
        ET_ReturnCode eGetWordForm(int64_t iHandle, shared_ptr<CWordForm>& spWordForm);
        ET_ReturnCode RemoveWordForm(int64_t iHandle);

        int64_t iAddLexemeEnumerator(shared_ptr<CLexemeEnumerator> spLexemeEnumerator);
        ET_ReturnCode eGetLexemeEnumerator(int64_t iHandle, shared_ptr<CLexemeEnumerator>& spLexemeEnumerator);
        ET_ReturnCode RemoveLexemeEnumerator(int64_t iHandle);

        int64_t iAddInflectionEnumerator(shared_ptr<CInflectionEnumerator> spInflectionEnumerator);
        ET_ReturnCode eGetInflectionEnumerator(int64_t iHandle, shared_ptr<CInflectionEnumerator>& spInflectionEnumerator);
        ET_ReturnCode RemoveInflectionEnumerator(int64_t iHandle);

        void Singleton::SetParser(shared_ptr<CParser> spParser);
        ET_ReturnCode eGetParser(shared_ptr<CParser>& spParser);

        ET_ReturnCode eGetVerifier(shared_ptr<CVerifier>& spVerifier);

        static Singleton* pGetInstance();

    private:
        Singleton();

        shared_ptr<CDictionary> m_spDictionary;
        shared_ptr<CParser> m_spParser;
        shared_ptr<CVerifier> m_spVerifier;
        map<int64_t, shared_ptr<CLexeme>> m_mapLexemes;
        map<int64_t, shared_ptr<CInflection>> m_mapInflections;
        map<int64_t, shared_ptr<CWordForm>> m_mapWordForms;
        map <int64_t, shared_ptr<CLexemeEnumerator>> m_mapLexemeEnumerators;
        map <int64_t, shared_ptr<CInflectionEnumerator>> m_mapInflectionEnumerators;
    };
}

//extern "C"
//{
    static Hlib::Singleton * pSingleton;
//}

#endif    //  MAINLIB_H_INCLUDED
