set(CMAKE_INCLUDE_CURRENT_DIR ON)
#set(CMAKE_CXX_STANDARD 11)

#find_package(OTF)
find_package(OTF2 REQUIRED)
#find_package(ZLIB REQUIRED)
#find_package(JSON REQUIRED)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DMG_ENABLE_THREADS -DMG_ENABLE_HTTP_WEBSOCKET=0 -pthread")

# Includes, Definitions, Flags
include_directories(${OTF2_INCLUDE_DIRS}
#                    ${ZLIB_INCLUDE_DIRS}
#		    ${JSON_INCLUDE_DIRS}
                   )

SET(ADDED_SOURCES "")
SET(ADDED_HEADERS "")

#if (OTF_FOUND)
#    include_directories(${OTF_INCLUDE_DIRS})
#    add_definitions(-DOTF1LIB)
#    list(APPEND ADDED_HEADERS otfimporter.h)
#    list(APPEND ADDED_SOURCES otfimporter.cpp)
#endif()

# Sources and UI Files
set(Traveler_SOURCES
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
    guidrecord.cpp
    function.cpp
    importfunctor.cpp
    main.cpp
    message.cpp
    metrics.cpp
    multievent.cpp
    multirecord.cpp
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

set(Traveler_HEADERS
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
    guidrecord.h
    function.h
    importfunctor.h
    message.h
    metrics.h
    multievent.h
    multirecord.h
    otf2importer.h
    otfcollective.h
    otfconverter.h
    p2pevent.h
    primaryentitygroup.h
    ravelutils.h
    rawtrace.h
    trace.h
    external/mongoose.h
    nlohmann/json.hpp
    ${ADDED_HEADERS}
)

set(Traveler_STATIC_FILES
    index.html
    traveler.css
    jslib/d3.tip.v0.6.3.js
    jslib/jquery-1.11.0.min.js
    jslib/d3.v5.min.js
    jslib/d3-tip.js
    jslib/split.js
    jslib/horizontal.png
    jslib/vertical.png
    jslib/goldenlayout.min.js
    jslib/goldenlayout-base.css
    jslib/goldenlayout-light-theme.css
)

# Build Target
add_executable(Traveler MACOSX_BUNDLE ${Traveler_SOURCES} ${Traveler_STATIC_FILES})
set_source_Files_properties(${Traveler_STATIC_FILES} PROPERTIES
    MACOSX_PACKAGE_LOCATION MacOS)

target_link_libraries(Traveler
		      m
	              pthread
                      ${OTF2_LIBRARIES}
                      #${ZLIB_LIBRARIES}
                     )

#if (OTF_FOUND)
#    target_link_libraries(Traveler
#                          ${OTF_LIBRARIES}
#                         )
#endif()

install(TARGETS Traveler DESTINATION bin)
