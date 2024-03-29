﻿#ifndef PHONETICS_H
#define PHONETICS_H

#include <vector>

#include <toml.hpp>

#include "SqliteWrapper.h"
#include "EString.h"
#include "Dictionary.h"
#include "Singleton.h"
#include "SqliteWrapper.h"
#include "Transcriber.h"
#include "RunPhonotactics.h"

using namespace Hlib;

ET_ReturnCode CPhonotactics::eInit(const CEString& sDbPath)
{
    auto pSingleton = Hlib::Singleton::pGetInstance();
    shared_ptr<CDictionary> spDictionary;
    auto rc = pSingleton->eGetDictionary(spDictionary);
    if (rc != H_NO_ERROR)
    {
        ERROR_LOG(L"Failed to set DB path.");
        return rc;
    }

    rc = spDictionary->eSetDbPath(sDbPath);
    if (rc != H_NO_ERROR)
    {
        ERROR_LOG(L"Failed to set DB path.");
        return rc;
    }

    m_sCurrentSequence.SetBreakChars(L" ");

    return H_NO_ERROR;

}       // eInit()

ET_ReturnCode CPhonotactics::eCountSoundPairs()
{
    int64_t llCurrentTextId = -1;
    for (const auto& [llTextId, llLineId] : m_mmapTextIdToLineIds)
    {
        if (llCurrentTextId != llTextId)
        {
            if (!m_mapSoundPairIdToCount.empty())
            {
                auto rc = eSaveTextData(llCurrentTextId);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }
                m_mapSoundPairIdToCount.clear();
            }
        }
        auto rc = eLoadSegment(llLineId);
        if (rc != H_NO_ERROR)
        {
            CEString sMsg(L"Failed to load segment ");
            sMsg += CEString::sToString(llLineId);
            ERROR_LOG(sMsg);
            continue;
        }

        if (m_sCurrentSequence.bIsEmpty())
        {
            ERROR_LOG(L"WARNING: Empty segment.");
            continue;
        }

        for (int iAt = 0; iAt < (int)m_sCurrentSequence.uiNFields(); ++iAt)
        {
            ET_Sound eLhs = SOUND_UNDEFINED;
            ET_Sound eRhs = SOUND_UNDEFINED;

            ET_ReturnCode rc { H_NO_ERROR };
            rc = eStringToSound(m_sCurrentSequence.sGetField(iAt), eLhs);
            if (rc != H_NO_ERROR)
            {
                ERROR_LOG(L"Failed to acquire 1st LHS sound.");
                return rc;
            }
            rc = eStringToSound(m_sCurrentSequence.sGetField(iAt), eRhs);
            if (rc != H_NO_ERROR)
            {
                ERROR_LOG(L"Failed to acquire 1st RHS sound.");
                return rc;
            }

            if (m_mapSoundPairToId.count(make_pair(eLhs, eRhs)) < 1) 
            {
                int64_t llSoundPairId{-1};
                auto rc = eRegisterSoundPair(eLhs, eRhs, llSoundPairId);
                if (rc != H_NO_ERROR)
                {
                    return rc;
                }
                m_mapSoundPairToId[make_pair(eLhs, eRhs)] = llSoundPairId;
            }

            auto llSoundPairId = m_mapSoundPairToId[make_pair(eLhs, eRhs)];
            if (m_mapSoundPairIdToCount.count(llSoundPairId) < 1) 
            {
                m_mapSoundPairIdToCount[llSoundPairId] = 1;
            }
            else
            {
                m_mapSoundPairIdToCount[llSoundPairId] = ++m_mapSoundPairIdToCount[llSoundPairId];
            }
        }
    }
    return H_NO_ERROR;

}       //  eCountSoundPairs()

//  -------------------------------------------------------------------------
ET_ReturnCode CPhonotactics::eLoadPhonemicPairs()
{
    CEString sQuery(L"SELECT id, lhs_phoneme, rhs_phoneme FROM phonemic_pairs;");
    try
    {
        int64_t llPairId{-1};
        m_spDb->PrepareForSelect(sQuery);
        if (m_spDb->bGetRow())
        {
            m_spDb->GetData(0, llPairId);
            int i1stSound{0};
            m_spDb->GetData(1, i1stSound);
            if (i1stSound < (int)ET_Sound::SOUND_UNDEFINED
                || i1stSound >(int)ET_Sound::SOUND_COUNT)
            {
                ERROR_LOG(L"Unrecognized left ET_Sound value.");
                return H_ERROR_UNEXPECTED;
            }
            int i2ndSound{0};
            m_spDb->GetData(2, i2ndSound);
            if (i1stSound < (int)ET_Sound::SOUND_UNDEFINED
                || i1stSound >(int)ET_Sound::SOUND_COUNT)
            {
                ERROR_LOG(L"Unrecognized right ET_Sound value.");
                return H_ERROR_UNEXPECTED;
            }
            m_mapSoundPairToId[make_pair((ET_Sound)i1stSound, (ET_Sound)i2ndSound)] = llPairId;
        }
        m_spDb->Finalize();
    }
    catch (CException& e)
    {
        CEString sMsg;
        eHandleDbException(move(e), sMsg);
        return H_ERROR_DB;
    }

    return H_NO_ERROR;
}

ET_ReturnCode CPhonotactics::eLoadLineIds()
{
    CEString sQuery (L"SELECT DISTINCT text_id, line_id FROM lines_in_text as lit \
                       INNER JOIN tact_group as tg ON tg.line_id=lit.id \
                       ORDER BY text_id, line_id;");
    try
    {
        m_spDb->PrepareForSelect(sQuery);
        if (m_spDb->bGetRow())
        {
            int64_t llTextId{ -1 };
            m_spDb->GetData(0, llTextId);
            int64_t llLineId{ -1 };
            m_spDb->GetData(1, llLineId);

            m_mmapTextIdToLineIds.insert(make_pair(llTextId, llLineId));
        }
        m_spDb->Finalize();
    }
    catch (CException& e)
    {
        CEString sMsg;
        eHandleDbException(move(e), sMsg);
        return H_ERROR_DB;
    }

    return H_NO_ERROR;
}

ET_ReturnCode CPhonotactics::eLoadSegment(int64_t iSegmentId)
{
    static CEString sQueryTemplate = L"SELECT phonemic_sequence FROM tact_group WHERE line_id = #LINE_ID#;";
    m_sCurrentSequence.Erase();
    try
    {
        auto sQuery = sQueryTemplate.sReplace(L"#LINE_ID", CEString::sToString(iSegmentId));
        m_spDb->PrepareForSelect(sQuery);
        while (m_spDb->bGetRow())
        {
            CEString sTg;
            m_spDb->GetData(0, sTg);
            if (!m_sCurrentSequence.bIsEmpty())
            {
                m_sCurrentSequence += L" ";
            }
            m_sCurrentSequence += sTg;
        }
        m_spDb->Finalize();
    }
    catch (CException& e)
    {
        CEString sMsg;
        eHandleDbException(move(e), sMsg);
        return H_ERROR_DB;
    }
    return H_NO_ERROR;

}       //  eLoadSegment()

ET_ReturnCode CPhonotactics::eRegisterSoundPair(ET_Sound eLhs, ET_Sound eRhs, int64_t& llId)
{
    try
    {
        sqlite3_stmt* pStmt = nullptr;
        m_spDb->uiPrepareForInsert(L"phonemic_pairs", 2, pStmt);
        auto llInsertHandle = (unsigned long long)pStmt;
        m_spDb->Bind(1, eLhs, llInsertHandle);
        m_spDb->Bind(2, eRhs, llInsertHandle);
        m_spDb->InsertRow(llInsertHandle);
        llId = m_spDb->llGetLastKey();
        m_spDb->Finalize(llInsertHandle);
    }
    catch (CException& e)
    {
        CEString sMsg;
        eHandleDbException(move(e), sMsg);
        return H_ERROR_DB;
    }
    return H_NO_ERROR;
}

ET_ReturnCode CPhonotactics::eSaveTextData(int64_t llTextId)
{
    CEString sDeleteQuery(L"DELETE FROM phonemic_counts WHERE text_id = #;");
    sDeleteQuery = sDeleteQuery.sReplace(L"#", CEString::sToString(llTextId));
    try
    {
        m_spDb->Delete(sDeleteQuery);
    }
    catch (CException& ex)
    {
        CEString sMsg;
        eHandleDbException(move(ex), sMsg);
        return H_ERROR_DB;
    }

    for (auto pairIdToCount : m_mapSoundPairIdToCount)
    {
        try
        {
            sqlite3_stmt* pStmt = nullptr;
            m_spDb->uiPrepareForInsert(L"phonemic_counts", 3, pStmt);
            auto llInsertHandle = (unsigned long long)pStmt;
            m_spDb->Bind(1, llTextId, llInsertHandle);
            m_spDb->Bind(2, pairIdToCount.first, llInsertHandle);
            m_spDb->Bind(3, pairIdToCount.second, llInsertHandle);
            m_spDb->InsertRow(llInsertHandle);
            m_spDb->Finalize(llInsertHandle);
        }
        catch (CException& e)
        {
            CEString sMsg;
            eHandleDbException(move(e), sMsg);
            return H_ERROR_DB;
        }
        return H_NO_ERROR;
    }
    return H_NO_ERROR;
}

ET_ReturnCode CPhonotactics::eStringToSound(CEString&& sDescriptor, ET_Sound& eSound)
{
    // Currently stored as a pair: "<transcription symbol(s)>|<sound code>", e.g., "и|4"
    sDescriptor.SetBreakChars(L"|");
    try
    {
        eSound = static_cast<ET_Sound>(static_cast<ET_Sound>(CEString::iToInt(sDescriptor.sGetField(1))));
    }
    catch (CException& e)
    {
        CEString sMsg{ L"Exception: " };
        sMsg += e.szGetDescription();
        return H_EXCEPTION;
    }
    return H_NO_ERROR;
}

ET_ReturnCode CPhonotactics::eHandleDbException(CException&& e, CEString& sMsg)
{
    sMsg = e.szGetDescription();
    CEString sError;
    try
    {
        m_spDb->GetLastError(sError);
        sMsg += CEString(L", error: ");
        sMsg += sError;
    }
    catch (...)
    {
        sMsg = L"Apparent DB error ";
    }
    return H_NO_ERROR;
    
}       //  eHandleDbException()

int main(int argc, char* argv[]) {

    [[maybe_unused]] ET_ReturnCode rc = H_NO_ERROR;

    if (argc < 2)
    {
        ERROR_LOG(L"Argument expected: \'ZalPhonotactics analytics.toml\'");
        return H_ERROR_INVALID_ARG;
    }

    toml::table config;

    try
    {
        string path = argv[1];
        config = toml::parse_file(path);
    }
    catch (const toml::parse_error& err)
    {
        CEString sMsg(L"Unable to parse config.");
        sMsg += Hlib::CEString::sFromUtf8(err.description().data());
        ERROR_LOG(sMsg);
        return H_ERROR_UNEXPECTED;
    }

    CPhonotactics Runner;

#ifdef WIN32
    auto utf8DbPath = config["paths"]["db_path_windows"].as_string()->get();
    rc = Runner.eInit(Hlib::CEString::sFromUtf8(utf8DbPath));
#else
    auto utf8DbPath = config["paths"]["db_path_linux"].as_string()->get();
    Runner.eInit(Hlib::CEString::sFromUtf8(utf8DbPath));
#endif

    return 0;
}

#endif  // PHONETICS_H