#include <toml.hpp>

#include "RunMetrics.h"

using namespace Hlib;

ET_ReturnCode CMetrics::eInit(const CEString& sDbPath)
{
    try
    {
        m_spDb = make_shared<CSqlite>(sDbPath);
        if (!m_spDb)
        {
            return H_ERROR_DB;
        }
    }
    catch (CException& ex)
    {
        ERROR_LOG(ex.szGetDescription());
        return H_EXCEPTION;
    }

    auto rc = eLoadLineIds();

    return rc;

}       // eInit()

ET_ReturnCode CMetrics::eLoadLineIds()
{
    CEString sQuery(L"SELECT DISTINCT text_id, line_id FROM lines_in_text as lit \
                       INNER JOIN tact_group as tg ON tg.line_id=lit.id \
                       ORDER BY text_id, line_id;");
    try
    {
        m_spDb->PrepareForSelect(sQuery);
        while (m_spDb->bGetRow())
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

ET_ReturnCode CMetrics::eLoadSegment(int64_t iSegmentId)
{
    static CEString sQueryTemplate = L"SELECT source FROM tact_group WHERE line_id = #LINE_ID#;";
    m_sCurrentSequence.Erase();
    try
    {
        auto sQuery = sQueryTemplate.sReplace(L"#LINE_ID#", CEString::sToString(iSegmentId));
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

ET_ReturnCode CMetrics::eHandleDbException(CException&& e, CEString& sMsg)
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
        ERROR_LOG(L"Argument expected: \'ZalRunMetrics metrics.toml\'");
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
        ERROR_LOG(sMsg)
            return H_ERROR_UNEXPECTED;
    }

    CMetrics Runner;

#ifdef WIN32
    auto utf8DbPath = config["paths"]["db_path_windows"].as_string()->get();
    auto utf8TextPath = config["paths"]["text_path_windows"].as_string()->get();
#else
    auto utf8DbPath = config["paths"]["db_path_linux"].as_string()->get();
    auto utf8TextPath = config["paths"]["text_path_linux"].as_string()->get();
    Runner.eInit(Hlib::CEString::sFromUtf8(utf8DbPath));
#endif

    return 0;
}
