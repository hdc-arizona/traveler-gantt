find_path(OTF2_INCLUDE_DIRS 
    NAMES otf2/otf2.h
    PATHS $ENV{HOME}/opt/include /usr/opt/otf2/include
)

find_library(OTF2_LIBRARIES 
    NAMES libotf2.so libotf2.a otf2
    PATHS $ENV{HOME}/opt/lib/ /usr/opt/otf2/lib
)


find_package_handle_standard_args(OTF2
	FAIL_MESSAGE "Couldn't find OTF2 library."
	REQUIRED_VARS OTF2_INCLUDE_DIRS OTF2_LIBRARIES
	)

