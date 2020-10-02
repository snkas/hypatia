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
 * Author: Andre Aguas         March 2020
 *         Simon               2020
 */


#include "ns3/abort.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/point-to-point-laser-net-device.h"
#include "ns3/point-to-point-laser-channel.h"
#include "ns3/point-to-point-laser-remote-channel.h"
#include "ns3/queue.h"
#include "ns3/net-device-queue-interface.h"
#include "ns3/config.h"
#include "ns3/packet.h"
#include "ns3/names.h"
#include "ns3/string.h"
#include "ns3/mpi-interface.h"
#include "ns3/mpi-receiver.h"

#include "ns3/trace-helper.h"
#include "point-to-point-laser-helper.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PointToPointLaserHelper");

PointToPointLaserHelper::PointToPointLaserHelper ()
{
  m_queueFactory.SetTypeId ("ns3::DropTailQueue<Packet>");
  m_deviceFactory.SetTypeId ("ns3::PointToPointLaserNetDevice");
  m_channelFactory.SetTypeId ("ns3::PointToPointLaserChannel");
  m_remoteChannelFactory.SetTypeId ("ns3::PointToPointLaserRemoteChannel");
}

void 
PointToPointLaserHelper::SetQueue (std::string type,
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
PointToPointLaserHelper::SetDeviceAttribute (std::string n1, const AttributeValue &v1)
{
  m_deviceFactory.Set (n1, v1);
}

void 
PointToPointLaserHelper::SetChannelAttribute (std::string n1, const AttributeValue &v1)
{
  m_channelFactory.Set (n1, v1);
  m_remoteChannelFactory.Set (n1, v1);
}

NetDeviceContainer 
PointToPointLaserHelper::Install (NodeContainer c)
{
  NS_ASSERT (c.GetN () == 2);
  return Install (c.Get (0), c.Get (1));
}

NetDeviceContainer 
PointToPointLaserHelper::Install (Ptr<Node> a, Ptr<Node> b)
{
  // set the initial delay of the channel as the delay estimation for the lookahead of the
  // distributed scheduler
  Ptr<MobilityModel> aMobility = a->GetObject<MobilityModel>();
  Ptr<MobilityModel> bMobility = b->GetObject<MobilityModel>();
  double propagation_speed(299792458.0);
  double distance = aMobility->GetDistanceFrom (bMobility);
  double delay = distance / propagation_speed;
  SetChannelAttribute("Delay", StringValue(std::to_string(delay) + "s"));

  NetDeviceContainer container;

  Ptr<PointToPointLaserNetDevice> devA = m_deviceFactory.Create<PointToPointLaserNetDevice> ();
  devA->SetAddress (Mac48Address::Allocate ());
  devA->SetDestinationNode(b);
  a->AddDevice (devA);
  Ptr<Queue<Packet> > queueA = m_queueFactory.Create<Queue<Packet> > ();
  devA->SetQueue (queueA);
  Ptr<PointToPointLaserNetDevice> devB = m_deviceFactory.Create<PointToPointLaserNetDevice> ();
  devB->SetAddress (Mac48Address::Allocate ());
  devB->SetDestinationNode(a);
  b->AddDevice (devB);
  Ptr<Queue<Packet> > queueB = m_queueFactory.Create<Queue<Packet> > ();
  devB->SetQueue (queueB);

  // Aggregate NetDeviceQueueInterface objects
  Ptr<NetDeviceQueueInterface> ndqiA = CreateObject<NetDeviceQueueInterface> ();
  ndqiA->GetTxQueue (0)->ConnectQueueTraces (queueA);
  devA->AggregateObject (ndqiA);
  Ptr<NetDeviceQueueInterface> ndqiB = CreateObject<NetDeviceQueueInterface> ();
  ndqiB->GetTxQueue (0)->ConnectQueueTraces (queueB);
  devB->AggregateObject (ndqiB);

  // Distributed mode
  NS_ABORT_MSG_IF(MpiInterface::IsEnabled(), "Distributed mode is not currently supported for point-to-point lasers.");

  // Distributed mode is not currently supported, enable the below if it is:
//  // If MPI is enabled, we need to see if both nodes have the same system id
//  // (rank), and the rank is the same as this instance.  If both are true,
//  //use a normal p2p channel, otherwise use a remote channel
//  bool useNormalChannel = true;
//  Ptr<PointToPointLaserChannel> channel = 0;
//
//  if (MpiInterface::IsEnabled ()) {
//      uint32_t n1SystemId = a->GetSystemId ();
//      uint32_t n2SystemId = b->GetSystemId ();
//      uint32_t currSystemId = MpiInterface::GetSystemId ();
//      if (n1SystemId != currSystemId || n2SystemId != currSystemId) {
//          useNormalChannel = false;
//      }
//  }
//  if (useNormalChannel) {
//    channel = m_channelFactory.Create<PointToPointLaserChannel> ();
//  }
//  else {
//    channel = m_remoteChannelFactory.Create<PointToPointLaserRemoteChannel>();
//    Ptr<MpiReceiver> mpiRecA = CreateObject<MpiReceiver> ();
//    Ptr<MpiReceiver> mpiRecB = CreateObject<MpiReceiver> ();
//    mpiRecA->SetReceiveCallback (MakeCallback (&PointToPointLaserNetDevice::Receive, devA));
//    mpiRecB->SetReceiveCallback (MakeCallback (&PointToPointLaserNetDevice::Receive, devB));
//    devA->AggregateObject (mpiRecA);
//    devB->AggregateObject (mpiRecB);
//  }

  // Create and attach channel
  Ptr<PointToPointLaserChannel> channel = m_channelFactory.Create<PointToPointLaserChannel> ();
  devA->Attach (channel);
  devB->Attach (channel);
  container.Add (devA);
  container.Add (devB);

  return container;
}

} // namespace ns3
