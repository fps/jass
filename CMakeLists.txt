cmake_minimum_required (VERSION 2.6)
#set(CMAKE_BUILD_TYPE RelWithDebInfo)
set(CMAKE_BUILD_TYPE Release)
project (jass)

set(PROJECT_VERSION "0.9.5")
set(ARCHIVE_NAME ${CMAKE_PROJECT_NAME}-${PROJECT_VERSION})
add_custom_target(dist
    COMMAND git archive --prefix=${ARCHIVE_NAME}/ HEAD
        | bzip2 > ${CMAKE_BINARY_DIR}/${ARCHIVE_NAME}.tar.bz2
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_custom_command(
	COMMAND xsdcxx cxx-tree --generate-serialization --generate-ostream --output-dir ${PROJECT_BINARY_DIR}/ ${PROJECT_SOURCE_DIR}/jass.xsd
	OUTPUT ${PROJECT_BINARY_DIR}/jass.hxx ${PROJECT_BINARY_DIR}/jass.cxx
	DEPENDS ${PROJECT_SOURCE_DIR}/jass.xsd
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/
)

if(NO_JACK_SESSION) 
	add_definitions(-DNO_JACK_SESSION)
endif()

add_definitions(-O3 )

find_package(PkgConfig)

find_package(Boost COMPONENTS program_options)

find_package(Qt4)
#qt4_automoc("main_window.cc qfunctor.cc")
qt4_wrap_cpp(moc_srcs keyboard_channel_widget.h mute_widget.h sample_range_widget.h main_window.h velocity_widget.h velocity_range_widget.h dial_widget.h keyboard_widget.h  adsr_widget.h qfunctor.h engine.h generator_widget.h waveform_widget.h)
include(${QT_USE_FILE})
add_executable(jass voice.cc adsr_widget.cc keyboard_widget.cc generator_widget.cc waveform_widget.cc disposable.cc  engine.cc  heap.cc  main.cc  main_window.cc  ${PROJECT_BINARY_DIR}/jass.cxx ${moc_srcs})

pkg_check_modules(JASS samplerate sndfile jack)
find_library(XERCES_C xerces-c)
target_link_libraries(jass ${XERCES_C} ${QT_LIBRARIES} samplerate sndfile jack ${Boost_PROGRAM_OPTIONS_LIBRARY})
include_directories(${JASS_INCLUDE_DIRS})
include_directories(${PROJECT_BINARY_DIR})

install(TARGETS jass RUNTIME DESTINATION bin)

