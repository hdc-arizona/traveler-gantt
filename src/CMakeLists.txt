set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Dependencies over Qt5
find_package(OTF)
find_package(OTF2 REQUIRED)
find_package(ZLIB REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

set(CMAKE_AUTOMOC ON)

# Includes, Definitions, Flags
include_directories(${OTF2_INCLUDE_DIRS}
                    ${ZLIB_INCLUDE_DIRS}
                   )

SET(ADDED_SOURCES "")
SET(ADDED_HEADERS "")

if (OTF_FOUND)
    include_directories(${OTF_INCLUDE_DIRS})
    add_definitions(-DOTF1LIB)
    list(APPEND ADDED_HEADERS otfimporter.h)
    list(APPEND ADDED_SOURCES otfimporter.cpp)
endif()

# Sources and UI Files
set(Ravel_SOURCES
    main.cpp
    trace.cpp
    event.cpp
    message.cpp
    mainwindow.cpp
    commrecord.cpp
    eventrecord.cpp
    rawtrace.cpp
    otfconverter.cpp
    function.cpp
    importfunctor.cpp
    collectiverecord.cpp
    otfcollective.cpp
    commevent.cpp
    p2pevent.cpp
    collectiveevent.cpp
    counter.cpp
    counterrecord.cpp
    otf2importer.cpp
    entity.cpp
    entitygroup.cpp
    primaryentitygroup.cpp
    metrics.cpp
    ${ADDED_SOURCES}
)

set(Ravel_HEADERS
    trace.h
    event.h
    message.h
    mainwindow.h
    commrecord.h
    eventrecord.h
    rawtrace.h
    otfconverter.h
    function.h
    ravelutils.h
    importfunctor.h
    collectiverecord.h
    otfcollective.h
    commevent.h
    p2pevent.h
    collectiveevent.h
    counter.h
    counterrecord.h
    otf2importer.h
    entity.h
    clusterentity.h
    entitygroup.h
    primaryentitygroup.h
    metrics.h
    ${ADDED_HEADERS}
)

# Build Target
add_executable(Ravel MACOSX_BUNDLE ${Ravel_SOURCES} ${Ravel_UIC})

qt5_use_modules(Ravel)

target_link_libraries(Ravel
                      ${OTF2_LIBRARIES}
                      ${ZLIB_LIBRARIES}
                     )

if (OTF_FOUND)
    target_link_libraries(Ravel
                          ${OTF_LIBRARIES}
                         )
endif()

install(TARGETS Ravel DESTINATION bin)
