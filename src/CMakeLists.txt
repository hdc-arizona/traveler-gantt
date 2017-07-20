set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(OTF)
find_package(OTF2 REQUIRED)
find_package(ZLIB REQUIRED)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMG_ENABLE_THREADS -DMG_ENABLE_HTTP_WEBSOCKET=0 -pthread")

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
    collectiveevent.cpp
    collectiverecord.cpp
    commevent.cpp
    commrecord.cpp
    counter.cpp
    counterrecord.cpp
    entity.cpp
    entitygroup.cpp
    event.cpp
    eventrecord.cpp
    function.cpp
    importfunctor.cpp
    main.cpp
    message.cpp
    metrics.cpp
    otf2importer.cpp
    otfcollective.cpp
    otfconverter.cpp
    p2pevent.cpp
    primaryentitygroup.cpp
    rawtrace.cpp
    trace.cpp
    external/mongoose.cpp
    ${ADDED_SOURCES}
)

set(Ravel_HEADERS
    collectiveevent.h
    collectiverecord.h
    commevent.h
    commrecord.h
    counter.h
    counterrecord.h
    entity.h
    entitygroup.h
    event.h
    eventrecord.h
    function.h
    importfunctor.h
    message.h
    metrics.h
    otf2importer.h
    otfcollective.h
    otfconverter.h
    p2pevent.h
    primaryentitygroup.h
    ravelutils.h
    rawtrace.h
    trace.h
    external/mongoose.h
    ${ADDED_HEADERS}
)

# Build Target
add_executable(Ravel MACOSX_BUNDLE ${Ravel_SOURCES})

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
