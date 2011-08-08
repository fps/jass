cmake_minimum_required (VERSION 2.6)
set(CMAKE_BUILD_TYPE Debug)
project (Jass)



find_package(PkgConfig)

find_package(Boost COMPONENTS program_options)

find_package(Qt4)
qt4_automoc("main_window.cc qfunctor.cc")
qt4_wrap_cpp(moc_srcs main_window.h qfunctor.h engine.h)
include(${QT_USE_FILE})
add_executable(jass disposable.cc  engine.cc  heap.cc  main.cc  main_window.cc  test_stuff.cc jass.cxx ${moc_srcs})

pkg_check_modules(JASS samplerate sndfile jack)
find_library(XERCES_C xerces-c)
target_link_libraries(jass ${XERCES_C} ${QT_LIBRARIES} samplerate sndfile jack ${Boost_PROGRAM_OPTIONS_LIBRARY})
include_directories(${JASS_INCLUDE_DIRS})

install(TARGETS jass RUNTIME DESTINATION bin)

