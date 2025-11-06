include_directories(${MPI_INCLUDE_PATH})

build_lib(
    LIBNAME satellite
    SOURCE_FILES
        model/iers-data.cc
        model/julian-date.cc
        model/satellite.cc
        model/satellite-position-helper.cc
        model/satellite-position-mobility-model.cc
        model/sgp4ext.cpp
        model/sgp4io.cpp
        model/sgp4unit.cpp
        model/vector-extensions.cc
    HEADER_FILES
        model/iers-data.h
        model/julian-date.h
        model/satellite.h
        model/satellite-position-helper.h
        model/satellite-position-mobility-model.h
        model/sgp4ext.h
        model/sgp4io.h
        model/sgp4unit.h
        model/vector-extensions.h
)