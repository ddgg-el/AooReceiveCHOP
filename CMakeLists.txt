cmake_minimum_required(VERSION 3.10)
project(AooReceiveCHOP)

set(OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/plugin)

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

	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_BUILD_TYPE "Debug")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -F${TD_FRAMEWORKS_PATH}")
endif()


include_directories(
	${CMAKE_CURRENT_SOURCE_DIR}/derivative
	${CMAKE_CURRENT_SOURCE_DIR}/include
	${CMAKE_CURRENT_SOURCE_DIR}/libs/aoo/
	${CMAKE_CURRENT_SOURCE_DIR}/libs/aoo/include
)

set(AOO_USE_OPUS OFF CACHE BOOL "use Opus codec" FORCE)
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

function(BuiltCustomOp OPNAME SRC)

	cmake_parse_arguments(ARG "" "" "LINK_LIBRARIES" ${ARGN})

	add_library(${OPNAME} MODULE)
	target_sources(${OPNAME} PRIVATE ${SRC} ${DEP_SRC})
	target_sources(${OPNAME} PUBLIC ${HEADER_FILES})

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
	

	set(OPTEST ${OPNAME}_TEST)
    add_custom_target(${OPTEST})

    set_target_properties(${OPTEST} PROPERTIES 
		VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/plugin
		VS_DEBUGGER_ENVIRONMENT "TOUCH_TEXT_CONSOLE=1"
	)
    

endfunction(BuiltCustomOp)

###

message(STATUS "Python include dirs: ${Python_INCLUDE_DIRS}")
message(STATUS "Python library: ${Python_LIBRARY}")

BuiltCustomOp(${PROJECT_NAME} "src/AooReceive_CHOP.cpp" LINK_LIBRARIES "Aoo::aoo"  ${Python_LIBRARY})

target_include_directories(${PROJECT_NAME} PRIVATE 
    ${Python_INCLUDE_DIRS}
)