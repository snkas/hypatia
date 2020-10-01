/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2007, 2008 University of Washington
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
 * (Based on point-to-point network device)
 * Author: Andre Aguas    March 2020
 *         Simon          2020
 * 
 */


#include "ns3/log.h"
#include "ns3/queue.h"
#include "ns3/simulator.h"
#include "ns3/mac48-address.h"
#include "ns3/llc-snap-header.h"
#include "ns3/error-model.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/uinteger.h"
#include "ns3/pointer.h"
#include "ns3/ppp-header.h"
#include "point-to-point-laser-net-device.h"
#include "point-to-point-laser-channel.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("PointToPointLaserNetDevice");

NS_OBJECT_ENSURE_REGISTERED (PointToPointLaserNetDevice);

TypeId 
PointToPointLaserNetDevice::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::PointToPointLaserNetDevice")
    .SetParent<NetDevice> ()
    .SetGroupName ("PointToPoint")
    .AddConstructor<PointToPointLaserNetDevice> ()
    .AddAttribute ("Mtu", "The MAC-level Maximum Transmission Unit",
                   UintegerValue (DEFAULT_MTU),
                   MakeUintegerAccessor (&PointToPointLaserNetDevice::SetMtu,
                                         &PointToPointLaserNetDevice::GetMtu),
                   MakeUintegerChecker<uint16_t> ())
    .AddAttribute ("Address", 
                   "The MAC address of this device.",
                   Mac48AddressValue (Mac48Address ("ff:ff:ff:ff:ff:ff")),
                   MakeMac48AddressAccessor (&PointToPointLaserNetDevice::m_address),
                   MakeMac48AddressChecker ())
    .AddAttribute ("DataRate", 
                   "The default data rate for point to point links",
                   DataRateValue (DataRate ("32768b/s")),
                   MakeDataRateAccessor (&PointToPointLaserNetDevice::m_bps),
                   MakeDataRateChecker ())
    .AddAttribute ("ReceiveErrorModel", 
                   "The receiver error model used to simulate packet loss",
                   PointerValue (),
                   MakePointerAccessor (&PointToPointLaserNetDevice::m_receiveErrorModel),
                   MakePointerChecker<ErrorModel> ())
    .AddAttribute ("InterframeGap", 
                   "The time to wait between packet (frame) transmissions",
                   TimeValue (Seconds (0.0)),
                   MakeTimeAccessor (&PointToPointLaserNetDevice::m_tInterframeGap),
                   MakeTimeChecker ())

    //
    // Transmit queueing discipline for the device which includes its own set
    // of trace hooks.
    //
    .AddAttribute ("TxQueue", 
                   "A queue to use as the transmit queue in the device.",
                   PointerValue (),
                   MakePointerAccessor (&PointToPointLaserNetDevice::m_queue),
                   MakePointerChecker<Queue<Packet> > ())

    //
    // Trace sources at the "top" of the net device, where packets transition
    // to/from higher layers.
    //
    .AddTraceSource ("MacTx", 
                     "Trace source indicating a packet has arrived "
                     "for transmission by this device",
                     MakeTraceSourceAccessor (&PointToPointLaserNetDevice::m_macTxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacTxDrop", 
                     "Trace source indicating a packet has been dropped "
                     "by the device before transmission",
                     MakeTraceSourceAccessor (&PointToPointLaserNetDevice::m_macTxDropTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacPromiscRx", 
                     "A packet has been received by this device, "
                     "has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  "
                     "This is a promiscuous trace,",
                     MakeTraceSourceAccessor (&PointToPointLaserNetDevice::m_macPromiscRxTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("MacRx", 
                     "A packet has been received by this device, "
                     "has been passed up from the physical layer "
                     "and is being forwarded up the local protocol stack.  "
                     "This is a non-promiscuous trace,",
                     MakeTraceSourceAccessor (&PointToPointLaserNetDevice::m_macRxTrace),
                     "ns3::Packet::TracedCallback")
#if 0
    // Not currently implemented for this device
    .AddTraceSource ("MacRxDrop", 
                     "Trace source indicating a packet was dropped "
                     "before being forwarded up the stack",
                     MakeTraceSourceAccessor (&PointToPointLaserNetDevice::m_macRxDropTrace),
                     "ns3::Packet::TracedCallback")
#endif
    //
    // Trace sources at the "bottom" of the net device, where packets transition
    // to/from the channel.
    //
    .AddTraceSource ("PhyTxBegin", 
                     "Trace source indicating a packet has begun "
                     "transmitting over the channel",
                     MakeTraceSourceAccessor (&PointToPointLaserNetDevice::m_phyTxBeginTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyTxEnd", 
                     "Trace source indicating a packet has been "
                     "completely transmitted over the channel",
                     MakeTraceSourceAccessor (&PointToPointLaserNetDevice::m_phyTxEndTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyTxDrop", 
                     "Trace source indicating a packet has been "
                     "dropped by the device during transmission",
                     MakeTraceSourceAccessor (&PointToPointLaserNetDevice::m_phyTxDropTrace),
                     "ns3::Packet::TracedCallback")
#if 0
    // Not currently implemented for this device
    .AddTraceSource ("PhyRxBegin", 
                     "Trace source indicating a packet has begun "
                     "being received by the device",
                     MakeTraceSourceAccessor (&PointToPointLaserNetDevice::m_phyRxBeginTrace),
                     "ns3::Packet::TracedCallback")
#endif
    .AddTraceSource ("PhyRxEnd", 
                     "Trace source indicating a packet has been "
                     "completely received by the device",
                     MakeTraceSourceAccessor (&PointToPointLaserNetDevice::m_phyRxEndTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PhyRxDrop", 
                     "Trace source indicating a packet has been "
                     "dropped by the device during reception",
                     MakeTraceSourceAccessor (&PointToPointLaserNetDevice::m_phyRxDropTrace),
                     "ns3::Packet::TracedCallback")

    //
    // Trace sources designed to simulate a packet sniffer facility (tcpdump).
    // Note that there is really no difference between promiscuous and 
    // non-promiscuous traces in a point-to-point link.
    //
    .AddTraceSource ("Sniffer", 
                    "Trace source simulating a non-promiscuous packet sniffer "
                     "attached to the device",
                     MakeTraceSourceAccessor (&PointToPointLaserNetDevice::m_snifferTrace),
                     "ns3::Packet::TracedCallback")
    .AddTraceSource ("PromiscSniffer", 
                     "Trace source simulating a promiscuous packet sniffer "
                     "attached to the device",
                     MakeTraceSourceAccessor (&PointToPointLaserNetDevice::m_promiscSnifferTrace),
                     "ns3::Packet::TracedCallback")
  ;
  return tid;
}


PointToPointLaserNetDevice::PointToPointLaserNetDevice () 
  :
    m_txMachineState (READY),
    m_channel (0),
    m_linkUp (false),
    m_currentPkt (0)
{
  NS_LOG_FUNCTION (this);
}

PointToPointLaserNetDevice::~PointToPointLaserNetDevice ()
{
  NS_LOG_FUNCTION (this);
}

void
PointToPointLaserNetDevice::AddHeader (Ptr<Packet> p, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << p << protocolNumber);
  PppHeader ppp;
  ppp.SetProtocol (EtherToPpp (protocolNumber));
  p->AddHeader (ppp);
}

bool
PointToPointLaserNetDevice::ProcessHeader (Ptr<Packet> p, uint16_t& param)
{
  NS_LOG_FUNCTION (this << p << param);
  PppHeader ppp;
  p->RemoveHeader (ppp);
  param = PppToEther (ppp.GetProtocol ());
  return true;
}

void
PointToPointLaserNetDevice::DoDispose ()
{
  NS_LOG_FUNCTION (this);
  m_node = 0;
  m_channel = 0;
  m_receiveErrorModel = 0;
  m_currentPkt = 0;
  m_queue = 0;
  NetDevice::DoDispose ();
}

void
PointToPointLaserNetDevice::SetDataRate (DataRate bps)
{
  NS_LOG_FUNCTION (this);
  m_bps = bps;
}

void
PointToPointLaserNetDevice::SetInterframeGap (Time t)
{
  NS_LOG_FUNCTION (this << t.GetSeconds ());
  m_tInterframeGap = t;
}

bool
PointToPointLaserNetDevice::TransmitStart (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  NS_LOG_LOGIC ("UID is " << p->GetUid () << ")");

  //
  // This function is called to start the process of transmitting a packet.
  // We need to tell the channel that we've started wiggling the wire and
  // schedule an event that will be executed when the transmission is complete.
  //
  NS_ASSERT_MSG (m_txMachineState == READY, "Must be READY to transmit");
  m_txMachineState = BUSY;
  m_currentPkt = p;
  m_phyTxBeginTrace (m_currentPkt);
  TrackUtilization(true);

  Time txTime = m_bps.CalculateBytesTxTime (p->GetSize ());
  Time txCompleteTime = txTime + m_tInterframeGap;

  NS_LOG_LOGIC ("Schedule TransmitCompleteEvent in " << txCompleteTime.GetSeconds () << "sec");
  Simulator::Schedule (txCompleteTime, &PointToPointLaserNetDevice::TransmitComplete, this);

  bool result = m_channel->TransmitStart (p, this, m_destination_node, txTime);
  if (result == false)
    {
      m_phyTxDropTrace (p);
    }
  return result;
}

void
PointToPointLaserNetDevice::TransmitComplete (void)
{
  NS_LOG_FUNCTION (this);

  //
  // This function is called to when we're all done transmitting a packet.
  // We try and pull another packet off of the transmit queue.  If the queue
  // is empty, we are done, otherwise we need to start transmitting the
  // next packet.
  //
  NS_ASSERT_MSG (m_txMachineState == BUSY, "Must be BUSY if transmitting");
  m_txMachineState = READY;

  NS_ASSERT_MSG (m_currentPkt != 0, "PointToPointLaserNetDevice::TransmitComplete(): m_currentPkt zero");

  m_phyTxEndTrace (m_currentPkt);
  TrackUtilization(false);
  m_currentPkt = 0;

  Ptr<Packet> p = m_queue->Dequeue ();
  if (p == 0)
    {
      NS_LOG_LOGIC ("No pending packets in device queue after tx complete");
      return;
    }

  //
  // Got another packet off of the queue, so start the transmit process again.
  //
  m_snifferTrace (p);
  m_promiscSnifferTrace (p);
  TransmitStart (p);
}

bool
PointToPointLaserNetDevice::Attach (Ptr<PointToPointLaserChannel> ch)
{
  NS_LOG_FUNCTION (this << &ch);

  m_channel = ch;

  m_channel->Attach (this);

  //
  // This device is up whenever it is attached to a channel.  A better plan
  // would be to have the link come up when both devices are attached, but this
  // is not done for now.
  //
  NotifyLinkUp ();
  return true;
}

void
PointToPointLaserNetDevice::SetQueue (Ptr<Queue<Packet> > q)
{
  NS_LOG_FUNCTION (this << q);
  m_queue = q;
}

void
PointToPointLaserNetDevice::SetReceiveErrorModel (Ptr<ErrorModel> em)
{
  NS_LOG_FUNCTION (this << em);
  m_receiveErrorModel = em;
}

void
PointToPointLaserNetDevice::Receive (Ptr<Packet> packet)
{
  NS_LOG_FUNCTION (this << packet);
  uint16_t protocol = 0;

  if (m_receiveErrorModel && m_receiveErrorModel->IsCorrupt (packet) ) 
    {
      // 
      // If we have an error model and it indicates that it is time to lose a
      // corrupted packet, don't forward this packet up, let it go.
      //
      m_phyRxDropTrace (packet);
    }
  else 
    {
      // 
      // Hit the trace hooks.  All of these hooks are in the same place in this 
      // device because it is so simple, but this is not usually the case in
      // more complicated devices.
      //
      m_snifferTrace (packet);
      m_promiscSnifferTrace (packet);
      m_phyRxEndTrace (packet);

      //
      // Trace sinks will expect complete packets, not packets without some of the
      // headers.
      //
      Ptr<Packet> originalPacket = packet->Copy ();

      //
      // Strip off the point-to-point protocol header and forward this packet
      // up the protocol stack.  Since this is a simple point-to-point link,
      // there is no difference in what the promisc callback sees and what the
      // normal receive callback sees.
      //
      ProcessHeader (packet, protocol);

      if (!m_promiscCallback.IsNull ())
        {
          m_macPromiscRxTrace (originalPacket);
          m_promiscCallback (this, packet, protocol, GetRemote (), GetAddress (), NetDevice::PACKET_HOST);
        }

      m_macRxTrace (originalPacket);
      m_rxCallback (this, packet, protocol, GetRemote ());
    }
}

Ptr<Queue<Packet> >
PointToPointLaserNetDevice::GetQueue (void) const
{ 
  NS_LOG_FUNCTION (this);
  return m_queue;
}

void
PointToPointLaserNetDevice::NotifyLinkUp (void)
{
  NS_LOG_FUNCTION (this);
  m_linkUp = true;
  m_linkChangeCallbacks ();
}

void
PointToPointLaserNetDevice::SetIfIndex (const uint32_t index)
{
  NS_LOG_FUNCTION (this);
  m_ifIndex = index;
}

uint32_t
PointToPointLaserNetDevice::GetIfIndex (void) const
{
  return m_ifIndex;
}

Ptr<Channel>
PointToPointLaserNetDevice::GetChannel (void) const
{
  return m_channel;
}

//
// This is a point-to-point device, so we really don't need any kind of address
// information.  However, the base class NetDevice wants us to define the
// methods to get and set the address.  Rather than be rude and assert, we let
// clients get and set the address, but simply ignore them.

void
PointToPointLaserNetDevice::SetAddress (Address address)
{
  NS_LOG_FUNCTION (this << address);
  m_address = Mac48Address::ConvertFrom (address);
}

Address
PointToPointLaserNetDevice::GetAddress (void) const
{
  return m_address;
}

void
PointToPointLaserNetDevice::SetDestinationNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this << node);
  m_destination_node = node;
}

Ptr<Node>
PointToPointLaserNetDevice::GetDestinationNode (void) const
{
  return m_destination_node;
}

bool
PointToPointLaserNetDevice::IsLinkUp (void) const
{
  NS_LOG_FUNCTION (this);
  return m_linkUp;
}

void
PointToPointLaserNetDevice::AddLinkChangeCallback (Callback<void> callback)
{
  NS_LOG_FUNCTION (this);
  m_linkChangeCallbacks.ConnectWithoutContext (callback);
}

//
// This is a point-to-point device, so every transmission is a broadcast to
// all of the devices on the network.
//
bool
PointToPointLaserNetDevice::IsBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

//
// We don't really need any addressing information since this is a 
// point-to-point device.  The base class NetDevice wants us to return a
// broadcast address, so we make up something reasonable.
//
Address
PointToPointLaserNetDevice::GetBroadcast (void) const
{
  NS_LOG_FUNCTION (this);
  return Mac48Address ("ff:ff:ff:ff:ff:ff");
}

bool
PointToPointLaserNetDevice::IsMulticast (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

Address
PointToPointLaserNetDevice::GetMulticast (Ipv4Address multicastGroup) const
{
  NS_LOG_FUNCTION (this);
  return Mac48Address ("01:00:5e:00:00:00");
}

Address
PointToPointLaserNetDevice::GetMulticast (Ipv6Address addr) const
{
  NS_LOG_FUNCTION (this << addr);
  return Mac48Address ("33:33:00:00:00:00");
}

bool
PointToPointLaserNetDevice::IsPointToPoint (void) const
{
  NS_LOG_FUNCTION (this);
  return true;
}

bool
PointToPointLaserNetDevice::IsBridge (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

bool
PointToPointLaserNetDevice::Send (
  Ptr<Packet> packet, 
  const Address &dest, 
  uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << dest << protocolNumber);
  NS_LOG_LOGIC ("p=" << packet << ", dest=" << &dest);
  NS_LOG_LOGIC ("UID is " << packet->GetUid ());

  //
  // If IsLinkUp() is false it means there is no channel to send any packet 
  // over so we just hit the drop trace on the packet and return an error.
  //
  if (IsLinkUp () == false)
    {
      m_macTxDropTrace (packet);
      return false;
    }

  //
  // Stick a point to point protocol header on the packet in preparation for
  // shoving it out the door.
  //
  AddHeader (packet, protocolNumber);

  m_macTxTrace (packet);

  //
  // We should enqueue and dequeue the packet to hit the tracing hooks.
  //
  if (m_queue->Enqueue (packet))
    {
      //
      // If the channel is ready for transition we send the packet right now
      // 
      if (m_txMachineState == READY)
        {
          packet = m_queue->Dequeue ();
          m_snifferTrace (packet);
          m_promiscSnifferTrace (packet);
          bool ret = TransmitStart (packet);
          return ret;
        }
      return true;
    }

  // Enqueue may fail (overflow)

  m_macTxDropTrace (packet);
  return false;
}

bool
PointToPointLaserNetDevice::SendFrom (Ptr<Packet> packet, 
                                 const Address &source, 
                                 const Address &dest, 
                                 uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << packet << source << dest << protocolNumber);
  return false;
}

Ptr<Node>
PointToPointLaserNetDevice::GetNode (void) const
{
  return m_node;
}

void
PointToPointLaserNetDevice::SetNode (Ptr<Node> node)
{
  NS_LOG_FUNCTION (this);
  m_node = node;
}

bool
PointToPointLaserNetDevice::NeedsArp (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

void
PointToPointLaserNetDevice::SetReceiveCallback (NetDevice::ReceiveCallback cb)
{
  m_rxCallback = cb;
}

void
PointToPointLaserNetDevice::SetPromiscReceiveCallback (NetDevice::PromiscReceiveCallback cb)
{
  m_promiscCallback = cb;
}

bool
PointToPointLaserNetDevice::SupportsSendFrom (void) const
{
  NS_LOG_FUNCTION (this);
  return false;
}

void
PointToPointLaserNetDevice::DoMpiReceive (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  Receive (p);
}

Address 
PointToPointLaserNetDevice::GetRemote (void) const
{
  NS_LOG_FUNCTION (this);
  NS_ASSERT (m_channel->GetNDevices () == 2);
  for (std::size_t i = 0; i < m_channel->GetNDevices (); ++i)
    {
      Ptr<NetDevice> tmp = m_channel->GetDevice (i);
      if (tmp != this)
        {
          return tmp->GetAddress ();
        }
    }
  NS_ASSERT (false);
  // quiet compiler.
  return Address ();
}

bool
PointToPointLaserNetDevice::SetMtu (uint16_t mtu)
{
  NS_LOG_FUNCTION (this << mtu);
  m_mtu = mtu;
  return true;
}

uint16_t
PointToPointLaserNetDevice::GetMtu (void) const
{
  NS_LOG_FUNCTION (this);
  return m_mtu;
}

uint16_t
PointToPointLaserNetDevice::PppToEther (uint16_t proto)
{
  NS_LOG_FUNCTION_NOARGS();
  switch(proto)
    {
    case 0x0021: return 0x0800;   //IPv4
    case 0x0057: return 0x86DD;   //IPv6
    default: NS_ASSERT_MSG (false, "PPP Protocol number not defined!");
    }
  return 0;
}

uint16_t
PointToPointLaserNetDevice::EtherToPpp (uint16_t proto)
{
  NS_LOG_FUNCTION_NOARGS();
  switch(proto)
    {
    case 0x0800: return 0x0021;   //IPv4
    case 0x86DD: return 0x0057;   //IPv6
    default: NS_ASSERT_MSG (false, "PPP Protocol number not defined!");
    }
  return 0;
}

void
PointToPointLaserNetDevice::EnableUtilizationTracking(int64_t interval_ns) {
    m_utilization_tracking_enabled = true;
    m_interval_ns = interval_ns;
    m_prev_time_ns = 0;
    m_current_interval_start = 0;
    m_current_interval_end = m_interval_ns;
    m_idle_time_counter_ns = 0;
    m_busy_time_counter_ns = 0;
    m_current_state_is_on = false;
}

void
PointToPointLaserNetDevice::TrackUtilization(bool next_state_is_on) {
    if (m_utilization_tracking_enabled) {

        // Current time in nanoseconds
        int64_t now_ns = Simulator::Now().GetNanoSeconds();
        while (now_ns >= m_current_interval_end) {

            // Add everything until the end of the interval
            if (next_state_is_on) {
                m_idle_time_counter_ns += m_current_interval_end - m_prev_time_ns;
            } else {
                m_busy_time_counter_ns += m_current_interval_end - m_prev_time_ns;
            }

            // Save into the utilization array
            m_utilization.push_back(((double) m_busy_time_counter_ns) / ((double) m_interval_ns));

            // This must match up
            NS_ABORT_MSG_IF(m_idle_time_counter_ns + m_busy_time_counter_ns != m_interval_ns, "Not all time is accounted for");

            // Move to next interval
            m_idle_time_counter_ns = 0;
            m_busy_time_counter_ns = 0;
            m_prev_time_ns = m_current_interval_end;
            m_current_interval_start += m_interval_ns;
            m_current_interval_end += m_interval_ns;
        }

        // If not at the end of a new interval, just keep track of it all
        if (next_state_is_on) {
            m_idle_time_counter_ns += now_ns - m_prev_time_ns;
        } else {
            m_busy_time_counter_ns += now_ns - m_prev_time_ns;
        }

        // This has become the previous call
        m_current_state_is_on = next_state_is_on;
        m_prev_time_ns = now_ns;

    }
}



const std::vector<double>&
PointToPointLaserNetDevice::FinalizeUtilization() {
    TrackUtilization(!m_current_state_is_on);
    return m_utilization;
}


} // namespace ns3
