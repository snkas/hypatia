# Source files
set(source_files
    model/core/basic-simulation.cc
    model/core/exp-util.cc
    model/core/log-update-helper.cc
    model/core/topology-ptop.cc
    model/core/topology-ptop-queue-selector-default.cc
    model/core/topology-ptop-tc-qdisc-selector-default.cc
    model/core/arbiter.cc
    model/core/arbiter-ptop.cc
    model/core/arbiter-ecmp.cc
    model/core/ipv4-arbiter-routing.cc
    model/core/ptop-link-utilization-tracker.cc
    model/core/ptop-link-queue-tracker.cc
    helper/core/arbiter-ecmp-helper.cc
    helper/core/ipv4-arbiter-routing-helper.cc
    helper/core/ptop-link-utilization-tracker-helper.cc
    helper/core/ptop-link-queue-tracker-helper.cc
    helper/core/tcp-optimizer.cc
    helper/core/point-to-point-ab-helper.cc
    model/apps/tcp-flow-send-application.cc
    model/apps/tcp-flow-sink.cc
    model/apps/udp-rtt-client.cc
    model/apps/udp-burst-application.cc
    model/apps/udp-rtt-server.cc
    model/apps/udp-burst-info.cc
    model/apps/id-seq-header.cc
    helper/apps/tcp-flow-send-helper.cc
    helper/apps/tcp-flow-sink-helper.cc
    helper/apps/tcp-flow-schedule-reader.cc
    helper/apps/tcp-flow-scheduler.cc
    helper/apps/udp-rtt-helper.cc
    helper/apps/udp-burst-helper.cc
    helper/apps/udp-burst-schedule-reader.cc
    helper/apps/udp-burst-scheduler.cc
    helper/apps/pingmesh-scheduler.cc
    main/main_full.cc
)

# Header files
set(header_files
    model/core/basic-simulation.h
    model/core/exp-util.h
    model/core/log-update-helper.h
    model/core/topology.h
    model/core/topology-ptop.h
    model/core/topology-ptop-queue-selector-default.h
    model/core/topology-ptop-tc-qdisc-selector-default.h
    model/core/arbiter.h
    model/core/arbiter-ptop.h
    model/core/arbiter-ecmp.h
    model/core/ipv4-arbiter-routing.h
    model/core/ptop-link-utilization-tracker.h
    model/core/ptop-link-queue-tracker.h
    helper/core/arbiter-ecmp-helper.h
    helper/core/ipv4-arbiter-routing-helper.h
    helper/core/ptop-link-utilization-tracker-helper.h
    helper/core/ptop-link-queue-tracker-helper.h
    helper/core/tcp-optimizer.h
    helper/core/point-to-point-ab-helper.h
    model/apps/tcp-flow-send-application.h
    model/apps/tcp-flow-sink.h
    model/apps/udp-rtt-client.h
    model/apps/udp-burst-application.h
    model/apps/udp-rtt-server.h
    model/apps/udp-burst-info.h
    model/apps/id-seq-header.h
    helper/apps/tcp-flow-send-helper.h
    helper/apps/tcp-flow-sink-helper.h
    helper/apps/tcp-flow-schedule-reader.h
    helper/apps/tcp-flow-scheduler.h
    helper/apps/udp-rtt-helper.h
    helper/apps/udp-burst-helper.h
    helper/apps/udp-burst-schedule-reader.h
    helper/apps/udp-burst-scheduler.h
    helper/apps/pingmesh-scheduler.h
)

# Tests
set(TEST_FILES
    test/basic-sim-test-suite.cc
)


build_lib(
  LIBNAME basic-sim
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
  LIBRARIES_TO_LINK
    ${libcore}
    ${libinternet}
    ${libapplications}
    ${libpoint-to-point}
    ${libtraffic-control}
)