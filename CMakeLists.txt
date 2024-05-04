cmake_minimum_required (VERSION 3.16)

project (Zal-Core VERSION 5.0.0)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
  tomplusplus
  GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
  GIT_TAG        30172438cee64926dc41fdd9c11fb3ba5b2ba9de	# v3.4.0
  SOURCE_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/TomPlusPlus"
)
FetchContent_MakeAvailable(tomplusplus)

FetchContent_Declare(
  sha1
  GIT_REPOSITORY https://github.com/vog/sha1.git
  GIT_TAG        3f8a4aa									 # 1.4
  SOURCE_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/Sha1"
)
FetchContent_MakeAvailable(sha1)

#FetchContent_Declare(
#  spdlog
#  GIT_REPOSITORY https://github.com/gabime/spdlog.git
#  GIT_TAG        7e635fc									 # 1.12.0
#  SOURCE_DIR "${PROJECT_SOURCE_DIR}/ThirdParty/spdlog"
#)
#FetchContent_MakeAvailable(spdlog)

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
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
#	set(CMAKE_CXX_STANDARD 17)
#    add_compile_options(-Wall -Wextra -pedantic -Werror)
    add_compile_options(-Wall -Wextra -pedantic)
endif()

#if (MSVC)
#	set (CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
#endif()

add_library (ZalMainLib)

target_include_directories (ZalMainLib PUBLIC
	HLib
	ThirdParty/Sqlite
	ThirdParty/Sha1
	ThirdParty/TomPlusPlus
)

target_sources (ZalMainLib PRIVATE
	MainLib/Analytics.h
	MainLib/Analytics.cpp
	MainLib/Dictionary.h
	MainLib/Dictionary.cpp
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
	ThirdParty/Sqlite/sqlite3.c)

	if(MSVC)
		if (CMAKE_BUILD_TYPE MATCHES "Debug")
			add_custom_command(TARGET ZalMainLib POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ZalMainLib> ../../../../Zal-Windows/TestApp/bin/debug
		)
		else()
			add_custom_command(TARGET ZalMainLib POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:ZalMainLib> ../../../../Zal-Windows/TestApp/bin/release
		)
		endif()
	endif()

# ---------------------------------------------------------------

#
#	Python wrapper
#

add_library (ZalPythonItf SHARED) 

target_include_directories (ZalPythonItf PUBLIC
	HLib
	ThirdParty/Sqlite
	ThirdParty/sha1
	MainLib
)

target_sources (ZalPythonItf PRIVATE
#	Zal-CPP/MainLibCTypes/dllmain.cpp
	MainLibCTypes/ZalPythonItf.cpp
 "RunAnalytics/RunAnalytics.h")
 
target_link_directories (ZalPythonItf PRIVATE
if (CMAKE_BUILD_TYPE MATCHES "Debug")
	../../Zal-Core/out/build/x64-debug
else()
	../../Zal-Core/out/build/x64-release
endif()
)

target_link_libraries (ZalPythonItf 
	ZalMainLib)

#-----------------------------------------------------------------

#
#    Executables
#

add_executable (ZalGenerateAllForms)

target_sources (ZalGenerateAllForms PRIVATE
	GenerateAllForms/GenerateAllForms.cpp
)

target_include_directories (ZalGenerateAllForms PUBLIC
	HLib
	MainLib
	ThirdParty/Sqlite
	ThirdParty/sha1
)

if (MSVC)
	target_link_libraries(ZalGenerateAllForms ZalMainLib)
else()
	target_link_libraries(ZalGenerateAllForms ZalMainLib pthread dl)
endif()

#-----------------------------------------------------------------

add_executable (ZalRunAnalytics)

target_sources (ZalRunAnalytics PRIVATE
	RunAnalytics/RunAnalytics.cpp
	RunAnalytics/RunAnalytics.h)

target_include_directories (ZalRunAnalytics PUBLIC
	HLib
	MainLib
	ThirdParty/Sqlite
	ThirdParty/sha1
)

if (MSVC)
	target_link_libraries(ZalRunAnalytics ZalMainLib)
else()
	target_link_libraries(ZalRunAnalytics ZalMainLib pthread dl)
endif()

#-------------------------------------------------------------------

add_executable (ZalRunMetrics)

target_sources (ZalRunMetrics PRIVATE
	RunMetrics/RunMetrics.cpp
	RunMetrics/RunMetrics.h)

target_include_directories (ZalRunMetrics PUBLIC
	HLib
	MainLib
	ThirdParty/Sqlite
	ThirdParty/sha1
)

if (MSVC)
	target_link_libraries(ZalRunMetrics ZalMainLib)
else()
	target_link_libraries(ZalRunMetrics ZalMainLib pthread dl)
endif()

#-------------------------------------------------------------------

add_executable (ZalRunPhonotactics)

target_sources (ZalRunPhonotactics PRIVATE
	RunPhonotactics/RunPhonotactics.cpp
	RunPhonotactics/RunPhonotactics.h)

target_include_directories (ZalRunPhonotactics PUBLIC
	HLib
	MainLib
	ThirdParty/Sqlite
	ThirdParty/sha1
)

if (MSVC)
	target_link_libraries(ZalRunPhonotactics ZalMainLib)
else()
	target_link_libraries(ZalRunPhonotactics ZalMainLib pthread dl)
endif()

# ---------------------------------------------------------------
#
#	Tests
#
# ---------------------------------------------------------------

add_executable (ZalStringTest)

target_sources (ZalStringTest PRIVATE
	HLib/StringTest.cpp)
# "RunAnalytics/RunAnalytics.h")

# ---------------------------------------------------------------

add_executable (ZalLeakTest)

target_include_directories (ZalLeakTest PUBLIC
	HLib
	ThirdParty/Sqlite
	ThirdParty/sha1
)
target_sources (ZalLeakTest PRIVATE
	"MainLib/LeakTest.cpp")
# "RunAnalytics/RunAnalytics.h")

if (MSVC)
	target_link_libraries(ZalLeakTest ZalMainLib)
else()
	target_link_libraries(ZalLeakTest ZalMainLib pthread dl)
endif()

