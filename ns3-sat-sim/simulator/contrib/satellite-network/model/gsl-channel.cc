/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007 University of Washington
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
 * (Based on point-to-point channel)
 * Author: Andre Aguas  March 2020
 *         Simon        2020
 */


#include "gsl-channel.h"
#include "ns3/core-module.h"
#include "ns3/abort.h"
#include "ns3/mpi-interface.h"
#include "ns3/gsl-net-device.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("GSLChannel");

NS_OBJECT_ENSURE_REGISTERED (GSLChannel);

TypeId 
GSLChannel::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::GSLChannel")
    .SetParent<Channel> ()
    .SetGroupName ("GSL")
    .AddConstructor<GSLChannel> ()
    .AddAttribute ("Delay",
                   "The lower-bound propagation delay through the channel (it is accessed by the distributed simulator to determine lookahead time)",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&GSLChannel::m_lowerBoundDelay),
                   MakeTimeChecker ())
    .AddAttribute ("PropagationSpeed",
                   "Propagation speed through the channel in m/s (default is the speed of light)",
                   DoubleValue (299792458.0), // Default is speed of light
                   MakeDoubleAccessor (&GSLChannel::m_propagationSpeedMetersPerSecond),
                   MakeDoubleChecker<double> ())
  ;
  return tid;
}

GSLChannel::GSLChannel()
  :
    Channel ()
{
  NS_LOG_FUNCTION_NOARGS ();
}

bool
GSLChannel::TransmitStart (
  Ptr<const Packet> p,
  Ptr<GSLNetDevice> src,
  Address dst_address,
  Time txTime)
{
  NS_LOG_FUNCTION (this << p << src);
  NS_LOG_LOGIC ("UID is " << p->GetUid () << ")");

  Mac48Address address48 = Mac48Address::ConvertFrom (dst_address);
  MacToNetDeviceI it = m_link.find (address48);
  if (it != m_link.end ()) {
    Ptr<GSLNetDevice> dst = it->second;
    bool sameSystem = (src->GetNode()->GetSystemId() == dst->GetNode()->GetSystemId());
    return TransmitTo(p, src, it->second, txTime, sameSystem);
  }

  NS_ABORT_MSG("MAC address could not be mapped to a network device.");
  return false;
}

bool
GSLChannel::TransmitTo(Ptr<const Packet> p, Ptr<GSLNetDevice> srcNetDevice, Ptr<GSLNetDevice> destNetDevice, Time txTime, bool isSameSystem) {

  // Mobility models for source and destination
  Ptr<MobilityModel> senderMobility = srcNetDevice->GetNode()->GetObject<MobilityModel>();
  Ptr<Node> receiverNode = destNetDevice->GetNode();
  Ptr<MobilityModel> receiverMobility = receiverNode->GetObject<MobilityModel>();

  // Calculate delay
  Time delay = this->GetDelay(senderMobility, receiverMobility);
  NS_LOG_DEBUG(
          "Sending packet " << p << " from node " << srcNetDevice->GetNode()->GetId()
          << " to " << destNetDevice->GetNode()->GetId() << " with delay " << delay
  );

  // Distributed mode is not enabled
  NS_ABORT_MSG_UNLESS(isSameSystem, "MPI distributed mode is currently not supported by the GSL channel.");

  // Schedule arrival of packet at destination network device
  Simulator::ScheduleWithContext(
          receiverNode->GetId(),
          txTime + delay,
          &GSLNetDevice::Receive,
          destNetDevice,
          p->Copy ()
  );

  // Re-enabled below code if distributed is again enabled:
  //  if (isSameSystem) {
  //
  //  } else {
  //#ifdef NS3_MPI
  //    Time rxTime = Simulator::Now () + txTime + delay;
  //    MpiInterface::SendPacket (p->Copy (), rxTime, destNetDevice->GetNode()->GetId (), destNetDevice->GetIfIndex());
  //#else
  //    NS_FATAL_ERROR ("Can't use distributed simulator without MPI compiled in");
  //#endif
  //  }

  return true;
}

void
GSLChannel::Attach (Ptr<GSLNetDevice> device)
{
    NS_LOG_FUNCTION (this << device);
    NS_ABORT_MSG_IF (device == 0, "Cannot add zero pointer network device.");

    Mac48Address address48 = Mac48Address::ConvertFrom (device->GetAddress());
    m_link[address48] = device;
    m_net_devices.push_back(device);
}

Time
GSLChannel::GetDelay (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  double distance_m = a->GetDistanceFrom (b);
  double seconds = distance_m / m_propagationSpeedMetersPerSecond;
  return Seconds (seconds);
}

size_t Mac48AddressHash::operator() (Mac48Address const &x) const
{
    uint8_t address[6]; //!< address value
    x.CopyTo(address);

    uint32_t host = 0;
    uint8_t byte = 0;
    for (size_t i = 0; i < 6; i++) {
        byte = address[i];
        host <<= 8;
        host |= byte;
    }

    return host;
}

std::size_t
GSLChannel::GetNDevices (void) const
{
    NS_LOG_FUNCTION_NOARGS ();
    return m_net_devices.size();
}

Ptr<NetDevice>
GSLChannel::GetDevice (std::size_t i) const
{
    NS_LOG_FUNCTION (this << i);
    return m_net_devices.at(i);
}

} // namespace ns3
