# -*- Mode: python; py-indent-offset: 4; indent-tabs-mode: nil; coding: utf-8; -*-

def build(bld):

    # Register 'basic-sim' module with dependencies
    module = bld.create_ns3_module('basic-sim', ['core', 'internet', 'applications', 'point-to-point', 'traffic-control'])

    # Source files
    module.source = [
        'model/core/basic-simulation.cc',
        'model/core/exp-util.cc',
        'model/core/log-update-helper.cc',
        'model/core/topology-ptop.cc',
        'model/core/topology-ptop-queue-selector-default.cc',
        'model/core/topology-ptop-tc-qdisc-selector-default.cc',
        'model/core/arbiter.cc',
        'model/core/arbiter-ptop.cc',
        'model/core/arbiter-ecmp.cc',
        'model/core/ipv4-arbiter-routing.cc',
        'model/core/ptop-link-utilization-tracker.cc',
        'model/core/ptop-link-queue-tracker.cc',

        'helper/core/arbiter-ecmp-helper.cc',
        'helper/core/ipv4-arbiter-routing-helper.cc',
        'helper/core/ptop-link-utilization-tracker-helper.cc',
        'helper/core/ptop-link-queue-tracker-helper.cc',
        'helper/core/tcp-optimizer.cc',
        'helper/core/point-to-point-ab-helper.cc',

        'model/apps/tcp-flow-send-application.cc',
        'model/apps/tcp-flow-sink.cc',
        'model/apps/udp-rtt-client.cc',
        'model/apps/udp-burst-application.cc',
        'model/apps/udp-rtt-server.cc',
        'model/apps/udp-burst-info.cc',
        'model/apps/id-seq-header.cc',

        'helper/apps/tcp-flow-send-helper.cc',
        'helper/apps/tcp-flow-sink-helper.cc',
        'helper/apps/tcp-flow-schedule-reader.cc',
        'helper/apps/tcp-flow-scheduler.cc',
        'helper/apps/udp-rtt-helper.cc',
        'helper/apps/udp-burst-helper.cc',
        'helper/apps/udp-burst-schedule-reader.cc',
        'helper/apps/udp-burst-scheduler.cc',
        'helper/apps/pingmesh-scheduler.cc',
        ]

    # Header files
    headers = bld(features='ns3header')
    headers.module = 'basic-sim'
    headers.source = [
        'model/core/basic-simulation.h',
        'model/core/exp-util.h',
        'model/core/log-update-helper.h',
        'model/core/topology.h',
        'model/core/topology-ptop.h',
        'model/core/topology-ptop-queue-selector-default.h',
        'model/core/topology-ptop-tc-qdisc-selector-default.h',
        'model/core/arbiter.h',
        'model/core/arbiter-ptop.h',
        'model/core/arbiter-ecmp.h',
        'model/core/ipv4-arbiter-routing.h',
        'model/core/ptop-link-utilization-tracker.h',
        'model/core/ptop-link-queue-tracker.h',

        'helper/core/arbiter-ecmp-helper.h',
        'helper/core/ipv4-arbiter-routing-helper.h',
        'helper/core/ptop-link-utilization-tracker-helper.h',
        'helper/core/ptop-link-queue-tracker-helper.h',
        'helper/core/tcp-optimizer.h',
        'helper/core/point-to-point-ab-helper.h',

        'model/apps/tcp-flow-send-application.h',
        'model/apps/tcp-flow-sink.h',
        'model/apps/udp-rtt-client.h',
        'model/apps/udp-burst-application.h',
        'model/apps/udp-rtt-server.h',
        'model/apps/udp-burst-info.h',
        'model/apps/id-seq-header.h',

        'helper/apps/tcp-flow-send-helper.h',
        'helper/apps/tcp-flow-sink-helper.h',
        'helper/apps/tcp-flow-schedule-reader.h',
        'helper/apps/tcp-flow-scheduler.h',
        'helper/apps/udp-rtt-helper.h',
        'helper/apps/udp-burst-helper.h',
        'helper/apps/udp-burst-schedule-reader.h',
        'helper/apps/udp-burst-scheduler.h',
        'helper/apps/pingmesh-scheduler.h',

        ]

    # Tests
    module_test = bld.create_ns3_module_test_library('basic-sim')
    module_test.source = [
        'test/basic-sim-test-suite.cc',
        ]

    # Main
    bld.recurse('main')

    # Examples
    if bld.env.ENABLE_EXAMPLES:
        bld.recurse('examples')

    # For now, no Python bindings are generated
    # bld.ns3_python_bindings()
