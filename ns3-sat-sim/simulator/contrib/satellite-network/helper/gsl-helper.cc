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
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 * (based on point-to-point helper)
 * Author: Jens Eirik Saethre  June 2019
 *         Andre Aguas         March 2020
 *         Simon               2020
 * 
 */


#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/gsl-net-device.h"
#include "ns3/gsl-channel.h"
#include "ns3/queue.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/config.h"
#include "ns3/packet.h"
#include "ns3/names.h"
#include "ns3/string.h"
#include "ns3/mpi-interface.h"
#include "ns3/mpi-receiver.h"

#include "ns3/trace-helper.h"
#include "ns3/gsl-helper.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("GSLHelper");

GSLHelper::GSLHelper ()
{
  m_queueFactory.SetTypeId ("ns3::DropTailQueue<Packet>");
  m_deviceFactory.SetTypeId ("ns3::GSLNetDevice");
  m_channelFactory.SetTypeId ("ns3::GSLChannel");
}

void 
GSLHelper::SetQueue (std::string type,
                     std::string n1, const AttributeValue &v1,
                     std::string n2, const AttributeValue &v2,
                     std::string n3, const AttributeValue &v3,
                     std::string n4, const AttributeValue &v4)
{
  QueueBase::AppendItemTypeIfNotPresent (type, "Packet");

  m_queueFactory.SetTypeId (type);
  m_queueFactory.Set (n1, v1);
  m_queueFactory.Set (n2, v2);
  m_queueFactory.Set (n3, v3);
  m_queueFactory.Set (n4, v4);
}

void 
GSLHelper::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  m_deviceFactory.Set (n1, v1);
}

void 
GSLHelper::SetChannelAttribute (std::string n1, const AttributeValue &v1)
{
  m_channelFactory.Set (n1, v1);
}

NetDeviceContainer 
GSLHelper::Install (NodeContainer satellites, NodeContainer ground_stations, std::vector<std::tuple<int32_t, double>>& node_gsl_if_info)
{

    // Primary channel
    Ptr<GSLChannel> channel = m_channelFactory.Create<GSLChannel> ();

    // All network devices we added
    NetDeviceContainer allNetDevices;

    // Satellite network devices
    for (size_t sid = 0; sid < satellites.GetN(); sid++)  {
        size_t num_ifs = std::get<0>(node_gsl_if_info[sid]);
        for (size_t j = 0; j < num_ifs; j++) {
            allNetDevices.Add(Install(satellites.Get(sid), channel));
        }
    }

    // Ground station network devices
    size_t satellites_offset = satellites.GetN();
    for (size_t gid = 0; gid < ground_stations.GetN(); gid++)  {

        // Node
        Ptr<Node> gs_node = ground_stations.Get(gid);

        // Add interfaces
        size_t num_ifs = std::get<0>(node_gsl_if_info[satellites_offset + gid]);
        for (size_t j = 0; j < num_ifs; j++) {
            allNetDevices.Add(Install(gs_node, channel));
        }

    }

    // The lower bound for the GSL channel must be set to facilitate distributed simulation.
    // However, this is challenging, as delays vary over time based on the movement.
    // As such, for now this delay = lookahead time is set to 0.
    // (see also the Delay attribute in gsl-channel.cc)
    channel->SetAttribute("Delay", TimeValue(Seconds(0)));

    return allNetDevices;
}

Ptr<GSLNetDevice>
GSLHelper::Install (Ptr<Node> node, Ptr<GSLChannel> channel) {

    // Create device
    Ptr<GSLNetDevice> dev = m_deviceFactory.Create<GSLNetDevice>();

    // Set unique MAC address
    dev->SetAddress (Mac48Address::Allocate ());

    // Add device to the node
    node->AddDevice (dev);

    // Set device queue
    Ptr<Queue<Packet> > queue = m_queueFactory.Create<Queue<Packet>>();
    dev->SetQueue (queue);

    // Aggregate NetDeviceQueueInterface objects to connect
    // the device queue to the interface (used by traffic control layer)
    Ptr<NetDeviceQueueInterface> ndqi = CreateObject<NetDeviceQueueInterface>();
    ndqi->GetTxQueue (0)->ConnectQueueTraces (queue);
    dev->AggregateObject (ndqi);

    // Aggregate MPI receivers // TODO: Why?
    Ptr<MpiReceiver> mpiRec = CreateObject<MpiReceiver> ();
    mpiRec->SetReceiveCallback (MakeCallback (&GSLNetDevice::Receive, dev));
    dev->AggregateObject(mpiRec);

    // Attach to channel
    dev->Attach (channel);

    return dev;
}


} // namespace ns3
