/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2020 ETH Zurich
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
 * Author: Simon <isniska@gmail.com>
 */

#ifndef IPV4_ARBITER_ROUTING_H
#define IPV4_ARBITER_ROUTING_H

#include <list>
#include <utility>
#include <stdint.h>
#include "ns3/ipv4-address.h"
#include "ns3/ipv4-header.h"
#include "ns3/socket.h"
#include "ns3/ptr.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-routing-protocol.h"
#include "ns3/arbiter.h"
#include "ns3/point-to-point-net-device.h"
#include "ns3/point-to-point-channel.h"

namespace ns3 {

class Packet;
class NetDevice;
class Ipv4Interface;
class Ipv4Address;
class Ipv4Header;
class Node;

class Ipv4ArbiterRouting : public Ipv4RoutingProtocol
{
public:
  static TypeId GetTypeId (void);

  Ipv4ArbiterRouting ();
  virtual ~Ipv4ArbiterRouting ();

  /**
   * Used by the transport-layer to output.
   *
   * @param p           Pointer to packet
   * @param header      IPv4 header of the packet
   * @param oif         Output interface
   * @param sockerr     Socket error (out)
   *
   * @return IPv4 route entry
   */
  virtual Ptr<Ipv4Route> RouteOutput(
          Ptr<Packet> p,
          const Ipv4Header &header,
          Ptr<NetDevice> oif,
          Socket::SocketErrno &sockerr
  );

  /**
   * A packet arrived at an interface, where to deliver next.
   *
   * @param p           Packet
   * @param header      IPv4 header of the packet
   * @param idev        Input interface device
   * @param ucb         Uni-cast forward call-back
   * @param mcb         Multi-cast forward call-back
   * @param lcb         Local delivery call-back (it was destined for here)
   * @param ecb         Error call-back
   *
   * @return True if found an interface to forward to
   */
  virtual bool RouteInput(
          Ptr<const Packet> p,
          const Ipv4Header &header,
          Ptr<const NetDevice> idev,
          const UnicastForwardCallback &ucb,
          const MulticastForwardCallback &mcb,
          const LocalDeliverCallback &lcb,
          const ErrorCallback &ecb
          );

  virtual void NotifyInterfaceUp (uint32_t interface);
  virtual void NotifyInterfaceDown (uint32_t interface);
  virtual void NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address);
  virtual void PrintRoutingTable (Ptr<OutputStreamWrapper> stream, Time::Unit unit = Time::S) const;
  virtual void SetIpv4 (Ptr<Ipv4> ipv4);
  void SetArbiter (Ptr<Arbiter> arbiter);
  Ptr<Arbiter> GetArbiter ();

private:
    Ptr<Ipv4> m_ipv4;
    Ptr<Ipv4Route> LookupArbiter (const Ipv4Address& dest, const Ipv4Header &header, Ptr<const Packet> p, Ptr<NetDevice> oif = 0);
    Ptr<Arbiter> m_arbiter = 0;
    Ipv4Address m_nodeSingleIpAddress;
    Ipv4Mask loopbackMask = Ipv4Mask("255.0.0.0");
    Ipv4Address loopbackIp = Ipv4Address("127.0.0.1");
    uint32_t m_nodeId;

};

} // Namespace ns3

#endif /* IPV4_ARBITER_ROUTING_H */
