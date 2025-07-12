cmake_minimum_required(VERSION 3.10)
project(AooReceiveCHOP)

set(OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/plugin)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE "Debug")

if (APPLE)
	set(TD_PATH "/Applications/TouchDesigner.app/Contents/")
	set(TD_FRAMEWORKS_PATH "${TD_PATH}/Frameworks")
	set(TD_PYTHON_ROOT "${TD_FRAMEWORKS_PATH}/Python.framework")
	set(Python_INCLUDE_DIRS "${TD_PYTHON_ROOT}/Versions/3.11/include/python3.11")
	set(Python_LIBRARY "-framework Python")

	execute_process(COMMAND whoami OUTPUT_VARIABLE CURRENT_USER OUTPUT_STRIP_TRAILING_WHITESPACE)
    set(USER_PLUGIN_DIR "/Users/${CURRENT_USER}/Library/Application Support/Derivative/TouchDesigner099/Plugins")

    if(EXISTS ${USER_PLUGIN_DIR})
        message(STATUS "Found TouchDesigner plugin directory: ${USER_PLUGIN_DIR}")
        set(OUTPUT_DIRECTORY ${USER_PLUGIN_DIR})
    else()
        message(WARNING "TouchDesigner plugin directory not found. Using default output: ${OUTPUT_DIRECTORY}")
    endif()
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -F${TD_FRAMEWORKS_PATH}")
endif()

if(WIN32)
	message(STATUS "Configuring for Windows...\n")
	set(TD_PATH "C:/Program Files/Derivative/TouchDesigner")
	set(TD_PYTHON_ROOT "${TD_PATH}/bin/python.exe")

	
	set(Python_INCLUDE_DIRS 
		"${TD_PATH}/Samples/CPlusPlus/3rdParty/Python/Include"
		"${TD_PATH}/Samples/CPlusPlus/3rdParty/Python/Include/PC"
	)
	set(Python_LIBRARY "${TD_PATH}/Samples/CPlusPlus/3rdParty/Python/lib/x64/python311.lib")

	set(USER_PLUGIN_DIR "C:/Users/$ENV{USERNAME}/Documents/Derivative/Plugins")

	if(EXISTS ${USER_PLUGIN_DIR})
		message(STATUS "Found TouchDesigner plugin directory: ${USER_PLUGIN_DIR}")
		set(OUTPUT_DIRECTORY ${USER_PLUGIN_DIR})
	else()
		message(WARNING "TouchDesigner plugin directory not found. Using default output: ${OUTPUT_DIRECTORY}")
	endif()

	add_compile_definitions(WIN32_LEAN_AND_MEAN)

	# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
    # set(CMAKE_SHARED_LIBRARY_PREFIX "")  # No "lib" prefix for DLLs
    # set(CMAKE_SHARED_LIBRARY_SUFFIX ".dll")

endif()

message(STATUS "Python include dirs: ${Python_INCLUDE_DIRS}")
message(STATUS "Python library: ${Python_LIBRARY}\n")


include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/derivative
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/libs/aoo/
	${CMAKE_CURRENT_SOURCE_DIR}/libs/aoo/include
	${Python_INCLUDE_DIRS}
)

set(AOO_USE_OPUS OFF CACHE BOOL "use Opus codec" FORCE)
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	set(AOO_LOG_LEVEL "Debug")
endif()

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/aoo EXCLUDE_FROM_ALL)


file(GLOB DEP_SRC
	${CMAKE_CURRENT_SOURCE_DIR}/derivative/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

file(GLOB_RECURSE HEADER_FILES 
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/derivative/*.h
)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OUTPUT_DIRECTORY})

###

message(STATUS "Plugin Output Directory: ${OUTPUT_DIRECTORY}")

function(BuiltCustomOp OPNAME SRC)

	cmake_parse_arguments(ARG "" "" "LINK_LIBRARIES" ${ARGN})

	add_library(${OPNAME} MODULE)
	target_sources(${OPNAME} PRIVATE ${SRC} ${DEP_SRC})
	target_sources(${OPNAME} PRIVATE ${HEADER_FILES})

	set_target_properties(${OPNAME} PROPERTIES 
		LINKER_LANGUAGE CXX
		PREFIX ""
		BUNDLE TRUE
		BUNDLE_EXTENSION "plugin"
	)
	
	
	foreach(lib IN LISTS ARG_LINK_LIBRARIES)
		message(STATUS "Linking ${OPNAME} with library: ${lib}")
		target_link_libraries(${OPNAME} PRIVATE ${lib})
	endforeach()

	if(WIN32)
		target_link_libraries(${OPNAME} PRIVATE ws2_32)
	endif()
	
	set(OPTEST ${OPNAME}_TEST)
    add_custom_target(${OPTEST})

    set_target_properties(${OPTEST} PROPERTIES 
		VS_DEBUGGER_WORKING_DIRECTORY ${OUTPUT_DIRECTORY}
		VS_DEBUGGER_ENVIRONMENT "TOUCH_TEXT_CONSOLE=1"
	)

    
endfunction(BuiltCustomOp)

###

BuiltCustomOp(${PROJECT_NAME} "src/AooReceive_CHOP.cpp" LINK_LIBRARIES Aoo::aoo ${Python_LIBRARY})