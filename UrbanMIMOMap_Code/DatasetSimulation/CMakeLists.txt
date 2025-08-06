# Define library. Only source files here!
set(PROJECT_NAME PropagationRunMSMIMO)
set(PROJECT_DESC "Example of computing an outdoor propagation plus MS superposition using the Altair WinProp CPP API")

cmake_minimum_required(VERSION 3.9)

project("${PROJECT_NAME}" 
	DESCRIPTION ${PROJECT_DESC}
	LANGUAGES CXX C)

add_library( Engine SHARED IMPORTED )
if(NOT WIN32)
set_target_properties( Engine PROPERTIES
	IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../../bin/libEngine.so" 
	)
else(NOT WIN32)
set_target_properties( Engine PROPERTIES
	IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../../export/winprop/64/Engine.dll" 
	IMPORTED_IMPLIB "${PROJECT_SOURCE_DIR}/../../export/winprop/64/Engine.lib" 
	)
endif(NOT WIN32)

add_library( EngineMS SHARED IMPORTED )
if(NOT WIN32)
set_target_properties( EngineMS PROPERTIES
	IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../../bin/libEngineMS.so" 
	)
else(NOT WIN32)
set_target_properties( EngineMS PROPERTIES
	IMPORTED_LOCATION "${PROJECT_SOURCE_DIR}/../../export/winprop/64/EngineMS.dll" 
	IMPORTED_IMPLIB "${PROJECT_SOURCE_DIR}/../../export/winprop/64/EngineMS.lib" 
	)
endif(NOT WIN32)

add_executable(${PROJECT_NAME}
	"${PROJECT_SOURCE_DIR}/outdoor_propagation_runms_mimo.cpp"
	"${PROJECT_SOURCE_DIR}/outdoor_propagation_runms_mimo.h")

# Definition of Macros
target_compile_definitions(${PROJECT_NAME} PRIVATE
	_CRT_SECURE_NO_WARNINGS
	
	_WINPROP_API_
	API_DATA_FOLDER=\"${PROJECT_SOURCE_DIR}/../../data/\"
	PROJ_FOLDER=\"${PROJECT_SOURCE_DIR}/\"
	
	# library imports
	_DLL_ENGINE_IMPORT
	_DLL_ENGINE_MS_IMPORT
)

if(NOT WIN32)
target_compile_definitions(${PROJECT_NAME} PRIVATE 
	__LINUX
	_NO_MFC
)
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
	"${PROJECT_SOURCE_DIR}/../../source"
	"${PROJECT_SOURCE_DIR}/../../source/Public"
)

# Depend on a library that we defined in the top-level file
target_link_libraries(${PROJECT_NAME} PRIVATE
	Engine
	EngineMS)

set_target_properties(${PROJECT_NAME} 
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../bin
	RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/../../bin
	RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/../../bin
	OUTPUT_NAME ${PROJECT_NAME}
	OUTPUT_NAME_DEBUG "${PROJECT_NAME}_debug"
)