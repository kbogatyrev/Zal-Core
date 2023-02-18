// MainLib.cpp : Implementation of DLL Exports.

#include <memory>

#include "Enums.h"
#include "Dictionary.h"

using namespace Hlib;

/*
    ET_ReturnCode GetDictionaryItf(IDictionary*& d)
    {
        d = new CDictionary;
        if (NULL == d)
        {
            return H_ERROR_POINTER;
        }

        return H_NO_ERROR;
    }

    ET_ReturnCode GetDictionary (IDictionary *& d)
    {
        d = new CDictionary;
        if (NULL == d)
        {
            return H_ERROR_POINTER;
        }

        return H_NO_ERROR;
    }
}
*/

class Singleton
{
    shared_ptr<CDictionary> m_spDictionary;

public:

    ET_ReturnCode GetDictionaryRaw(CDictionary *& pDict) 
    {
        if (nullptr == m_spDictionary)
        {
            m_spDictionary = make_shared<CDictionary>();
        }

        if (nullptr == m_spDictionary)
        {
            ERROR_LOG(L"Unable to retrieve the dictionary.");
            return H_ERROR_POINTER;
        }

        pDict = m_spDictionary.get();

        return H_NO_ERROR;
    }

    ET_ReturnCode GetDictionarySmart(shared_ptr<CDictionary>& spDict)
    {
        if (nullptr == m_spDictionary)
        {
            m_spDictionary = make_shared<CDictionary>();
        }

        if (nullptr == m_spDictionary)
        {
            ERROR_LOG(L"Unable to retrieve the dictionary.");
            return H_ERROR_POINTER;
        }

        spDict = m_spDictionary;

        return H_NO_ERROR;
    }

};

extern "C"
{
    static Singleton singleton;
}
