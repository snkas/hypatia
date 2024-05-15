set(base_path ${CMAKE_CURRENT_SOURCE_DIR}/model)
set(data_path ${base_path}/data)
set(src ${data_path}/constants-gen.cc)
set(header ${base_path}/iers-data.h)
set(source ${base_path}/iers-data.cc)

add_executable(cgen ${src})
set_target_properties(cgen PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${data_path})

add_custom_command(OUTPUT ${header} ${source}
                                            COMMAND ${data_path}/cgen header ./
                                            COMMAND ${data_path}/cgen source ./
                                            WORKING_DIRECTORY ${base_path}
                                            DEPENDS cgen
)

set(source_files
    model/iers-data.cc
    model/julian-date.cc
    model/satellite.cc
    model/satellite-position-helper.cc
    model/satellite-position-mobility-model.cc
    model/sgp4ext.cpp
    model/sgp4io.cpp
    model/sgp4unit.cpp
    model/vector-extensions.cc
)

set(header_files
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

build_lib(
  LIBNAME satellite
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
  LIBRARIES_TO_LINK
    ${libcore}
    ${libmobility}
)
