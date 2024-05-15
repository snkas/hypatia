/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2008 INRIA
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Simon
 * Adapted from PointToPointHelper by:
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/point-to-point-channel.h"
#include "ns3/queue.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/config.h"
#include "ns3/packet.h"
#include "ns3/names.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/pointer.h"

#ifdef NS3_MPI
#include "ns3/mpi-interface.h"
#include "ns3/mpi-receiver.h"
#include "ns3/point-to-point-remote-channel.h"
#endif

#include "ns3/trace-helper.h"
#include "point-to-point-ab-helper.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("PointToPointAbHelper");

    PointToPointAbHelper::PointToPointAbHelper() {
        m_queueFactoryA.SetTypeId("ns3::DropTailQueue<Packet>");
        m_deviceFactoryA.SetTypeId("ns3::PointToPointNetDevice");
        m_queueFactoryB.SetTypeId("ns3::DropTailQueue<Packet>");
        m_deviceFactoryB.SetTypeId("ns3::PointToPointNetDevice");
        m_channelFactory.SetTypeId("ns3::PointToPointChannel");
#ifdef NS3_MPI
        m_remoteChannelFactory.SetTypeId ("ns3::PointToPointRemoteChannel");
#endif
    }

    void
    PointToPointAbHelper::SetQueueFactoryA(ObjectFactory queueFactoryA) {
        m_queueFactoryA = queueFactoryA;
    }

    void
    PointToPointAbHelper::SetQueueFactoryB(ObjectFactory queueFactoryB) {
        m_queueFactoryB = queueFactoryB;
    }

    void
    PointToPointAbHelper::SetDeviceAttributeA(std::string n1, const AttributeValue &v1) {
        m_deviceFactoryA.Set(n1, v1);
    }

    void
    PointToPointAbHelper::SetDeviceAttributeB(std::string n1, const AttributeValue &v1) {
        m_deviceFactoryB.Set(n1, v1);
    }

    void
    PointToPointAbHelper::SetChannelAttribute(std::string n1, const AttributeValue &v1) {
        m_channelFactory.Set(n1, v1);
#ifdef NS3_MPI
        m_remoteChannelFactory.Set (n1, v1);
#endif
    }

    NetDeviceContainer
    PointToPointAbHelper::Install(Ptr <Node> a, Ptr <Node> b) {
        NetDeviceContainer container;

        // Network device A
        Ptr <PointToPointNetDevice> devA = m_deviceFactoryA.Create<PointToPointNetDevice>();
        devA->SetAddress(Mac48Address::Allocate());
        a->AddDevice(devA);
        Ptr <Queue<Packet>> queueA = m_queueFactoryA.Create<Queue<Packet>> ();
        devA->SetQueue(queueA);

        // Network device B
        Ptr <PointToPointNetDevice> devB = m_deviceFactoryB.Create<PointToPointNetDevice>();
        devB->SetAddress(Mac48Address::Allocate());
        b->AddDevice(devB);
        Ptr <Queue<Packet>> queueB = m_queueFactoryB.Create<Queue<Packet>> ();
        devB->SetQueue(queueB);

        // Aggregate NetDeviceQueueInterface objects
        Ptr <NetDeviceQueueInterface> ndqiA = CreateObject<NetDeviceQueueInterface>();
        ndqiA->GetTxQueue(0)->ConnectQueueTraces(queueA);
        devA->AggregateObject(ndqiA);
        Ptr <NetDeviceQueueInterface> ndqiB = CreateObject<NetDeviceQueueInterface>();
        ndqiB->GetTxQueue(0)->ConnectQueueTraces(queueB);
        devB->AggregateObject(ndqiB);

        Ptr <PointToPointChannel> channel = 0;

        // If MPI is enabled, we need to see if both nodes have the same system id
        // (rank), and the rank is the same as this instance.  If both are true,
        // use a normal p2p channel, otherwise use a remote channel
#ifdef NS3_MPI
        bool useNormalChannel = true;
        if (MpiInterface::IsEnabled ())
          {
            uint32_t n1SystemId = a->GetSystemId ();
            uint32_t n2SystemId = b->GetSystemId ();
            uint32_t currSystemId = MpiInterface::GetSystemId ();
            if (n1SystemId != currSystemId || n2SystemId != currSystemId)
              {
                useNormalChannel = false;
              }
          }
        if (useNormalChannel)
          {
            channel = m_channelFactory.Create<PointToPointChannel> ();
          }
        else
          {
            channel = m_remoteChannelFactory.Create<PointToPointRemoteChannel> ();
            Ptr<MpiReceiver> mpiRecA = CreateObject<MpiReceiver> ();
            Ptr<MpiReceiver> mpiRecB = CreateObject<MpiReceiver> ();
            mpiRecA->SetReceiveCallback (MakeCallback (&PointToPointNetDevice::Receive, devA));
            mpiRecB->SetReceiveCallback (MakeCallback (&PointToPointNetDevice::Receive, devB));
            devA->AggregateObject (mpiRecA);
            devB->AggregateObject (mpiRecB);
          }
#else
        channel = m_channelFactory.Create<PointToPointChannel>();
#endif

        devA->Attach(channel);
        devB->Attach(channel);
        container.Add(devA);
        container.Add(devB);

        return container;
    }

} // namespace ns3
