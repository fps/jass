cmake_minimum_required (VERSION 2.6)
project (Jass)



find_package(PkgConfig)

find_package(Qt4)
qt4_automoc("main_window.cc qfunctor.cc")
qt4_wrap_cpp(moc_srcs main_window.h qfunctor.h)
include(${QT_USE_FILE})
add_executable(jass disposable.cc  engine.cc  heap.cc  main.cc  main_window.cc  test_stuff.cc jass.cxx ${moc_srcs})

pkg_check_modules(JASS samplerate sndfile jack)
find_library(XERCES_C xerces-c)
target_link_libraries(jass ${XERCES_C} ${QT_LIBRARIES} samplerate sndfile jack)
include_directories(${JASS_INCLUDE_DIRS})

