# Source files
set(source_files
    model/point-to-point-laser-net-device.cc
    model/point-to-point-laser-channel.cc
    model/point-to-point-laser-remote-channel.cc
    model/gsl-net-device.cc
    model/gsl-channel.cc
    model/ground-station.cc
    helper/gsl-helper.cc
    helper/point-to-point-laser-helper.cc
    model/topology-satellite-network.cc
    model/arbiter-satnet.cc
    model/arbiter-single-forward.cc
    helper/arbiter-single-forward-helper.cc
    helper/gsl-if-bandwidth-helper.cc
)

# Header files
set(header_files
    model/point-to-point-laser-net-device.h
    model/point-to-point-laser-channel.h
    model/point-to-point-laser-remote-channel.h
    model/gsl-net-device.h
    model/gsl-channel.h
    model/ground-station.h
    helper/gsl-helper.h
    helper/point-to-point-laser-helper.h
    model/topology-satellite-network.h
    model/arbiter-satnet.h
    model/arbiter-single-forward.h
    helper/arbiter-single-forward-helper.h
    helper/gsl-if-bandwidth-helper.h
)

# Tests
set(TEST_FILES
    test/satellite-network-test-suite.cc
)

# Add the executable
build_lib(
  LIBNAME satellite-network
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
  LIBRARIES_TO_LINK
    ${libcore}
    ${libinternet}
    ${libapplications}
    ${libpoint-to-point}
    ${libtraffic-control}
    ${libmpi}
    ${libsatellite}
    ${libmobility}
    ${libinternet-apps}
    ${libbasic-sim}
)