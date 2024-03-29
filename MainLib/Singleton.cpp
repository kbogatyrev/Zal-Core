#include <memory>

#include "Enums.h"
#include "Dictionary.h"
#include "Singleton.h"

using namespace Hlib;

Hlib::Singleton* pSingleton;

Singleton* Singleton::pGetInstance()
{
    if (!pSingleton)
    {
        pSingleton = new Singleton();
    }
    return pSingleton;
}

Singleton::Singleton()
{
    m_itCurrentCreatedForm = m_vecCreatedForms.end();
}

ET_ReturnCode Singleton::eGetDictionary(shared_ptr<CDictionary>& spDictionary)
{
    if (nullptr == m_spDictionary)
    {
        m_spDictionary = make_shared<CDictionary>();
        m_spDictionary->eInit();
    }

    if (nullptr == m_spDictionary)
    {
        ERROR_LOG(L"Unable to retrieve the dictionary.");
        return H_ERROR_UNEXPECTED;
    }

    spDictionary = m_spDictionary;

    return H_NO_ERROR;
}

int64_t Singleton::iAddLexeme(CLexeme* pLexeme)
{
    if (nullptr == pLexeme)
    {
        ERROR_LOG(L"Lexeme pointer is NULL.");
        return -1;
    }
    auto iHandle = (int64_t)pLexeme;
    m_mapLexemes[iHandle] = pLexeme;

    return iHandle;
}

ET_ReturnCode Singleton::eGetLexeme(int64_t iHandle, CLexeme*& pLexeme)
{
    auto itLexeme = m_mapLexemes.find(iHandle);
    if (m_mapLexemes.end() == itLexeme)
    {
        ERROR_LOG(L"Unable to retrieve lexeme pointer.");
        return H_ERROR_UNEXPECTED;
    }
    pLexeme = itLexeme->second;

    return H_NO_ERROR;
}

ET_ReturnCode Singleton::RemoveLexeme(int64_t iHandle)
{
    if (iHandle < 0)
    {
        ERROR_LOG(L"Invalid lexeme handle.");
        return H_ERROR_UNEXPECTED;
    }

    auto itLexeme = m_mapLexemes.find(iHandle);
    if (m_mapLexemes.end() == itLexeme)
    {
        ERROR_LOG(L"Unable to remove lexeme.");
        return H_ERROR_UNEXPECTED;
    }

    m_mapLexemes.erase(itLexeme);

    return H_NO_ERROR;
}

int64_t Singleton::iAddInflection(CInflection* pInflection)
{
    if (nullptr == pInflection)
    {
        ERROR_LOG(L"Inflection pointer is NULL.");
        return -1;
    }
    auto iHandle = (int64_t)pInflection;
    m_mapInflections[iHandle] = pInflection;

    return iHandle;
}

ET_ReturnCode Singleton::eGetInflection(int64_t iHandle, CInflection*& spInflection)
{
    auto itInfection = m_mapInflections.find(iHandle);
    if (m_mapInflections.end() == itInfection)
    {
        ERROR_LOG(L"Unable to retrieve inflection pointer.");
        return H_ERROR_UNEXPECTED;
    }
    spInflection = itInfection->second;

    return H_NO_ERROR;
}

ET_ReturnCode Singleton::eRemoveInflection(int64_t iHandle)
{
    if (iHandle < 0)
    {
        ERROR_LOG(L"Invalid inflection handle.");
        return H_ERROR_UNEXPECTED;
    }

    auto itInflection = m_mapInflections.find(iHandle);
    if (m_mapInflections.end() == itInflection)
    {
        ERROR_LOG(L"Unable to remove inflection.");
        return H_ERROR_UNEXPECTED;
    }

    m_mapInflections.erase(itInflection);

    return H_NO_ERROR;
}

int64_t Singleton::iAddWordForm(CWordForm* pWordForm)
{
    if (nullptr == pWordForm)
    {
        ERROR_LOG(L"Word form pointer is NULL.");
        return -1;
    }
    auto iHandle = (int64_t)pWordForm;
    m_mapWordForms[iHandle] = pWordForm;

    return iHandle;
}

ET_ReturnCode Singleton::eGetWordForm(int64_t iHandle, CWordForm*& pWordForm)
{
    auto itWordForm = m_mapWordForms.find(iHandle);
    if (m_mapWordForms.end() == itWordForm)
    {
        ERROR_LOG(L"Unable to retrieve word form pointer.");
        return H_ERROR_UNEXPECTED;
    }
    pWordForm = itWordForm->second;

    return H_NO_ERROR;
}

ET_ReturnCode Singleton::eRemoveWordForm(int64_t iHandle)
{
    if (iHandle < 0)
    {
        ERROR_LOG(L"Invalid word form handle.");
        return H_ERROR_UNEXPECTED;
    }

    auto itWordForm = m_mapWordForms.find(iHandle);
    if (m_mapWordForms.end() == itWordForm)
    {
        ERROR_LOG(L"Unable to remove word form.");
        return H_ERROR_UNEXPECTED;
    }

    m_mapWordForms.erase(itWordForm);

    return H_NO_ERROR;
}

ET_ReturnCode Singleton::eStoreCreatedWordForm(shared_ptr<CWordForm> spWordForm)
{
    m_vecCreatedForms.push_back(spWordForm);
    return H_NO_ERROR;
}

int Singleton::iNCreatedForms()
{
    return (int)m_vecCreatedForms.size();
}

ET_ReturnCode Singleton::eGetFirstCreatedWordForm(shared_ptr<CWordForm>& spWordForm)
{
    m_itCurrentCreatedForm = m_vecCreatedForms.begin();
    if (m_vecCreatedForms.end() == m_itCurrentCreatedForm)
    {
        spWordForm = nullptr;
        return H_FALSE;
    }
    spWordForm = *m_itCurrentCreatedForm;
    return H_NO_ERROR;
}

ET_ReturnCode Singleton::eGetNextCreatedWordForm(shared_ptr<CWordForm>& spWordForm)
{
    if (m_vecCreatedForms.end() == ++m_itCurrentCreatedForm)
    {
        spWordForm = nullptr;
        return H_FALSE;
    }
    spWordForm = *m_itCurrentCreatedForm;
    return H_NO_ERROR;
}

void Singleton::ClearCreatedForms()
{
    m_vecCreatedForms.clear();
}

int64_t Singleton::iAddLexemeEnumerator(shared_ptr<CLexemeEnumerator> spLexemeEnumerator)
{
    if (nullptr == spLexemeEnumerator)
    {
        ERROR_LOG(L"Lexeme enumerator pointer is NULL.");
        return -1;
    }
    auto iHandle = (int64_t)spLexemeEnumerator.get();
    m_mapLexemeEnumerators[iHandle] = spLexemeEnumerator;

    return iHandle;
}

ET_ReturnCode Singleton::eGetLexemeEnumerator(int64_t iHandle, shared_ptr<CLexemeEnumerator>& spLexemeEnumerator)
{
    auto itLexemeEnumerator = m_mapLexemeEnumerators.find(iHandle);
    if (m_mapLexemeEnumerators.end() == itLexemeEnumerator)
    {
        ERROR_LOG(L"Unable to retrieve lexeme enumerator pointer.");
        return H_ERROR_UNEXPECTED;
    }
    spLexemeEnumerator = itLexemeEnumerator->second;

    return H_NO_ERROR;
}

ET_ReturnCode Singleton::eRemoveLexemeEnumerator(int64_t iHandle)
{
    if (iHandle < 0)
    {
        ERROR_LOG(L"Invalid lexeme enumerator handle.");
        return H_ERROR_UNEXPECTED;
    }

    auto itLexemeEnumerator = m_mapLexemeEnumerators.find(iHandle);
    if (m_mapLexemeEnumerators.end() == itLexemeEnumerator)
    {
        ERROR_LOG(L"Unable to remove lexeme enumerator.");
        return H_ERROR_UNEXPECTED;
    }

    m_mapLexemeEnumerators.erase(itLexemeEnumerator);

    return H_NO_ERROR;
}

int64_t Singleton::iAddInflectionEnumerator(shared_ptr<CInflectionEnumerator> spInflectionEnumerator)
{
    if (nullptr == spInflectionEnumerator)
    {
        ERROR_LOG(L"Inflection enumerator pointer is NULL.");
        return -1;
    }
    auto iHandle = (int64_t)spInflectionEnumerator.get();
    m_mapInflectionEnumerators[iHandle] = spInflectionEnumerator;

    return iHandle;
}

ET_ReturnCode Singleton::eGetInflectionEnumerator(int64_t iHandle, shared_ptr<CInflectionEnumerator>& spInflectionEnumerator)
{
    auto itInflectionEnumerator = m_mapInflectionEnumerators.find(iHandle);
    if (m_mapInflectionEnumerators.end() == itInflectionEnumerator)
    {
        ERROR_LOG(L"Unable to retrieve inflection enumerator pointer.");
        return H_ERROR_UNEXPECTED;
    }
    spInflectionEnumerator = itInflectionEnumerator->second;

    return H_NO_ERROR;
}

ET_ReturnCode Singleton::eRemoveInflectionEnumerator(int64_t iHandle)
{
    if (iHandle < 0)
    {
        ERROR_LOG(L"Invalid inflection enumerator handle.");
        return H_ERROR_UNEXPECTED;
    }

    auto itInflectionEnumerator = m_mapInflectionEnumerators.find(iHandle);
    if (m_mapInflectionEnumerators.end() == itInflectionEnumerator)
    {
        ERROR_LOG(L"Unable to remove inflection enumerator.");
        return H_ERROR_UNEXPECTED;
    }

    m_mapInflectionEnumerators.erase(itInflectionEnumerator);

    return H_NO_ERROR;
}

ET_ReturnCode Singleton::eGetParser(shared_ptr<CParser>& spParser)
{
    if (nullptr == m_spParser)
    {
        if (!m_spDictionary)
        {
            ERROR_LOG(L"The dictionary is not initialized.");
            return H_ERROR_UNEXPECTED;
        }
    }

    auto rc = m_spDictionary->eGetParser(m_spParser);
    if (rc != H_NO_ERROR || !m_spParser)
    {
        ERROR_LOG(L"Unable to retrieve the parser.");
        return H_ERROR_UNEXPECTED;
    }

    spParser = m_spParser;

    return H_NO_ERROR;
}

void Singleton::SetParser(shared_ptr<CParser> spParser)
{
    m_spParser = spParser;
}

ET_ReturnCode Singleton::eGetVerifier(shared_ptr<CVerifier>& spVerifier)
{
    if (nullptr == m_spVerifier)
    {
        if (!m_spDictionary)
        {
            ERROR_LOG(L"The dictionary is not initialized.");
            return H_ERROR_UNEXPECTED;
        }
    }

    auto rc = m_spDictionary->eGetVerifier(m_spVerifier);
    if (rc != H_NO_ERROR || !m_spVerifier)
    {
        ERROR_LOG(L"Unable to retrieve the verifier.");
        return H_ERROR_UNEXPECTED;
    }

    spVerifier = m_spVerifier;

    return H_NO_ERROR;
}
