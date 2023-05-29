cmake_minimum_required (VERSION 3.16)

project (Zal-Core VERSION 5.0.0)

#
#	Main library
#

if (MSVC)
	if (CMAKE_BUILD_TYPE MATCHES "Debug")
		message (STATUS "Windows debug mode")
		add_compile_definitions(
			WIN32
			_WINDOWS
			_UNICODE
			UNICODE
			_DEBUG
			_CONSOLE
		)
		add_compile_options(
			/source-charset:utf-8
			/wd4996		# unsafe function -- ignored
			/JMC /ifcOutput "out/build/x64-Debug/" /GS /W3 /Zc:wchar_t 
			/Zi /Gm- /Od #/Fd "out/build/x64-Debug/ZalMainLib.pdb" 
			/Zc:inline /fp:precise #/D "_DEBUG" 
			/errorReport:prompt /WX- /Zc:forScope /RTC1 /Gd /MDd /std:c++17 /FC 
#			/Fa "out/build/x64-Debug" /EHsc /nologo /Fo "out/build/x64-Debug" /Fp "out/build/x64-Debug/ZalMainLib.pch" 
			/diagnostics:column 
		)
	else()
		message (STATUS "Windows release mode")
		add_compile_definitions(
			WIN32
			_WINDOWS
			_UNICODE
			UNICODE
			_CONSOLE
			NDEBUG
		)
		add_compile_options(
			/source-charset:utf-8
			/wd4996		# unsafe function -- ignored
			/ifcOutput "out/build/x64-Release/" /GS /GL /W3 /Gy /Zc:wchar_t 
			/Zi /Gm- /O2 #/Fd"C:\dev\Zal-Windows\lib\x64\Release\ZalMainLib.pdb" 
			/Zc:inline /fp:precise 
			/errorReport:prompt /WX- /Zc:forScope /Gd /Oi /MD 
			/std:c++17 /FC 
			#/Fa"x64\Release\" 
			/EHsc /nologo 
			#/Fo"x64\Release\" /Fp"x64\Release\ZalMainLib.pch" 
			/diagnostics:column 
		)
	endif()
else()
	set(CMAKE_CXX_STANDARD 17)
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

#if (MSVC)
#	set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
#endif()

add_library (ZalMainLib)

target_include_directories (ZalMainLib PUBLIC
	HLib
	ThirdParty/Sqlite
	ThirdParty/sha1
)

target_sources (ZalMainLib PRIVATE
	MainLib/Analytics.h
	MainLib/Analytics.cpp
	MainLib/Dictionary.h
	MainLib/Dictionary.cpp
#	MainLib/dllmain.cpp
	MainLib/Endings.h
	MainLib/Endings.cpp
	MainLib/FormBuilderAdjComparative.h
	MainLib/FormBuilderAdjComparative.cpp
	MainLib/FormBuilderAdjLong.h
	MainLib/FormBuilderAdjLong.cpp
	MainLib/FormBuilderAdjShort.h
	MainLib/FormBuilderAdjShort.cpp
	MainLib/FormBuilderAspectPair.h
	MainLib/FormBuilderAspectPair.cpp
	MainLib/FormBuilderBase.h
	MainLib/FormBuilderBase.cpp
	MainLib/FormBuilderBaseConj.h
	MainLib/FormBuilderBaseConj.cpp
	MainLib/FormBuilderBaseDecl.h
	MainLib/FormBuilderBaseDecl.cpp
	MainLib/FormBuilderNouns.h
	MainLib/FormBuilderNouns.cpp
	MainLib/FormBuilderPronounAdj.h
	MainLib/FormBuilderPronounAdj.cpp
	MainLib/FormBuilderVerbImpv.h
	MainLib/FormBuilderVerbImpv.cpp
	MainLib/FormBuilderVerbNonFinite.h
	MainLib/FormBuilderVerbNonFinite.cpp
	MainLib/FormBuilderVerbPast.h
	MainLib/FormBuilderVerbPast.cpp
	MainLib/FormBuilderVerbPers.h
	MainLib/FormBuilderVerbPers.cpp
	MainLib/Inflection.h
	MainLib/Inflection.cpp
	MainLib/Lexeme.h
	MainLib/Lexeme.cpp
	MainLib/Parser.h
	MainLib/Parser.cpp
	MainLib/ParsingTree.h
	MainLib/ParsingTree.cpp
	MainLib/Singleton.h
	MainLib/Singleton.cpp
	MainLib/Transcriber.h
	MainLib/Transcriber.cpp
	MainLib/Verifier.h
	MainLib/Verifier.cpp
	MainLib/WordForm.h
	MainLib/WordForm.cpp
	ThirdParty/Sqlite/sqlite3.c
)

if (CMAKE_BUILD_TYPE MATCHES "Debug")
	add_custom_command(TARGET ZalMainLib POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ZalMainLib> ../../../../Zal-Windows/TestApp/bin/debug
	)
else()
	add_custom_command(TARGET ZalMainLib POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ZalMainLib> ../../../../Zal-Windows/TestApp/bin/release
	)
endif()

# ---------------------------------------------------------------

#
#	Python wrapper
#

add_library (ZalPythonItf) 

target_include_directories (ZalPythonItf PUBLIC
	HLib
	ThirdParty/Sqlite
	ThirdParty/sha1
	MainLib
)

target_sources (ZalPythonItf PRIVATE
#	Zal-CPP/MainLibCTypes/dllmain.cpp
	MainLibCTypes/ZalPythonItf.cpp
)

target_link_directories (ZalPythonItf PRIVATE
	../../Zal-Core/out/build/x64-debug
)

target_link_libraries (ZalPythonItf 
	ZalMainLib)


# ---------------------------------------------------------------
#
#	Tests
#
# ---------------------------------------------------------------

add_executable (ZalStringTest)

target_sources (ZalStringTest PRIVATE
	HLib/StringTest.cpp
)

add_executable (ZalMainTest)

#-----------------------------------------------------------------

target_sources (ZalMainTest PRIVATE
	MainLib/Test.cpp
)

target_include_directories (ZalMainTest PUBLIC
	HLib
	ThirdParty/Sqlite
	ThirdParty/sha1
)

if (MSVC)
	target_link_libraries(ZalMainTest ZalMainLib)
else()
	target_link_libraries(ZalMainTest ZalMainLib pthread dl)
endif()
