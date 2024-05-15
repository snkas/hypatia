/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright 2007 University of Washington
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
 */

#ifndef UDP_RTT_CLIENT_H
#define UDP_RTT_CLIENT_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/traced-callback.h"
#include "ns3/seq-ts-header.h"

namespace ns3 {

class Socket;
class Packet;

class UdpRttClient : public Application 
{
public:
  static TypeId GetTypeId (void);
  UdpRttClient ();
  virtual ~UdpRttClient ();

  uint32_t GetFromNodeId();
  uint32_t GetToNodeId();
  uint32_t GetSent();
  std::vector<int64_t> GetSendRequestTimestamps();
  std::vector<int64_t> GetReplyTimestamps();
  std::vector<int64_t> GetReceiveReplyTimestamps();

protected:
  virtual void DoDispose (void);

private:

  virtual void StartApplication (void);
  virtual void StopApplication (void);
  void ScheduleTransmit (Time dt);
  void Send (void);
  void HandleRead (Ptr<Socket> socket);

  Time m_interval; //!< Packet inter-send time
  Ptr<Socket> m_socket; //!< Socket
  Address m_peerAddress; //!< Remote peer address
  uint16_t m_peerPort; //!< Remote peer port
  EventId m_sendEvent; //!< Event to send the next packet

  uint32_t m_fromNodeId;
  uint32_t m_toNodeId;
  uint32_t m_sent; //!< Counter for sent packets
  std::vector<int64_t> m_sendRequestTimestamps;
  std::vector<int64_t> m_replyTimestamps;
  std::vector<int64_t> m_receiveReplyTimestamps;

};

} // namespace ns3

#endif /* UDP_RTT_CLIENT_H */
