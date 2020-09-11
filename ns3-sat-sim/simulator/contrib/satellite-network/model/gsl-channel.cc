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
    .AddAttribute ("Delay", "Inital propagation delay through the channel",
                   TimeValue (Seconds (0)),
                   MakeTimeAccessor (&GSLChannel::m_initialDelay),
                   MakeTimeChecker ())
    .AddAttribute ("PropagationSpeed", "Propagation speed through the channel",
                   DoubleValue (299792458.0),
                   MakeDoubleAccessor (&GSLChannel::m_propagationSpeed),
                   MakeDoubleChecker<double> ())
    .AddAttribute ("Range", "Maximum distance satellite and ground station can communicate",
                   DoubleValue (1089686),  // from SpaceX recent FCC filing (in meters): sqrt(550000^2 + 940700^2)
                   MakeDoubleAccessor (&GSLChannel::m_range),
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
    Ptr<GSLNetDevice> dst = it->second.m_net_device;
    bool sameSystem = (src->GetNode()->GetSystemId() == dst->GetNode()->GetSystemId());
    return TransmitTo(p, src, it->second.m_net_device, txTime, sameSystem);
  }

  return false;
}

bool
GSLChannel::TransmitTo(Ptr<const Packet> p, Ptr<GSLNetDevice> srcNetDevice, Ptr<GSLNetDevice> destNetDevice, Time txTime, bool isSameSystem) {
  Ptr<MobilityModel> senderMobility = srcNetDevice->GetNode()->GetObject<MobilityModel>();

  Ptr<Node> receiverNode = destNetDevice->GetNode();
  Ptr<MobilityModel> receiverMobility = receiverNode->GetObject<MobilityModel>();

  // if the satellite is not in range do not send the packet
  /* TODO: For now, we assume the forwarding state is able to prevent this value getting exceeded too much.
  double distance = senderMobility->GetDistanceFrom(receiverMobility);
  if (distance > m_range) {
    // do not send the packet
    return false;
  }
   */

  NS_LOG_LOGIC ("transmitting to: " << destNetDevice->GetNode()->GetId());
  Time delay = this->GetDelay(senderMobility, receiverMobility); 

  if (isSameSystem) {
    Simulator::ScheduleWithContext (receiverNode->GetId(),
                                    txTime + delay, &GSLNetDevice::Receive,
                                    destNetDevice, p->Copy ());
  }
  else {
#ifdef NS3_MPI
    Time rxTime = Simulator::Now () + txTime + delay;
    MpiInterface::SendPacket (p->Copy (), rxTime, destNetDevice->GetNode()->GetId (), destNetDevice->GetIfIndex());
#else
    NS_FATAL_ERROR ("Can't use distributed simulator without MPI compiled in");
#endif
  }

  return true;
}

void
GSLChannel::Attach (Ptr<GSLNetDevice> device)
{
    NS_LOG_FUNCTION (this << device);
    NS_ASSERT (device != 0);

    GSLLink gslLink = GSLLink();
    gslLink.m_net_device = device;
    gslLink.m_state = IDLE;

    Mac48Address address48 = Mac48Address::ConvertFrom (device->GetAddress());
    m_link[address48] = gslLink;
    m_net_devices.push_back(device);
}

std::size_t
GSLChannel::GetNDevices (void) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_net_devices.size();
}

Ptr<GSLNetDevice>
GSLChannel::GetGSLDevice (std::size_t i) const
{
  NS_LOG_FUNCTION_NOARGS ();
  return m_net_devices[i];
}

Ptr<NetDevice>
GSLChannel::GetDevice (std::size_t i) const
{
  NS_LOG_FUNCTION_NOARGS ();
  if (i == 0) {
    return GetGSLDevice(0);
  }
  // TODO: Why this?
  uint32_t node_0_system_id = GetGSLDevice(0)->GetNode()->GetSystemId();
  for (uint32_t i = 0; i < GetNDevices(); i++) {
    if (GetGSLDevice(i)->GetNode()->GetSystemId() != node_0_system_id) {
      return GetGSLDevice(i);
    }
  }
  return GetGSLDevice (i);
}

Time
GSLChannel::GetDelay (Ptr<MobilityModel> a, Ptr<MobilityModel> b) const
{
  double distance = a->GetDistanceFrom (b);
  double seconds = distance / m_propagationSpeed;
  return Seconds (seconds);
}

bool
GSLChannel::IsInitialized (void) const
{
  for (auto it = m_link.begin(); it != m_link.end(); it++) {
    NS_ASSERT (it->second.m_state != INITIALIZING);
  }

  return true;
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

} // namespace ns3
