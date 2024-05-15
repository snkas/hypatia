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

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/address-utils.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"

#include "udp-rtt-server.h"

namespace ns3 {

    NS_LOG_COMPONENT_DEFINE ("UdpRttServerApplication");

    NS_OBJECT_ENSURE_REGISTERED (UdpRttServer);

    TypeId
    UdpRttServer::GetTypeId(void) {
        static TypeId tid = TypeId("ns3::UdpRttServer")
                .SetParent<Application>()
                .SetGroupName("Applications")
                .AddConstructor<UdpRttServer>()
                .AddAttribute("Port", "Port on which we listen for incoming packets.",
                              UintegerValue(9),
                              MakeUintegerAccessor(&UdpRttServer::m_port),
                              MakeUintegerChecker<uint16_t>());
        return tid;
    }

    UdpRttServer::UdpRttServer() {
        NS_LOG_FUNCTION(this);
    }

    UdpRttServer::~UdpRttServer() {
        NS_LOG_FUNCTION(this);
        m_socket = 0;
    }

    void
    UdpRttServer::DoDispose(void) {
        NS_LOG_FUNCTION(this);
        Application::DoDispose();
    }

    void
    UdpRttServer::StartApplication(void) {
        NS_LOG_FUNCTION(this);

        if (!m_socket) {
            TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
            m_socket = Socket::CreateSocket(GetNode(), tid);
            InetSocketAddress local = InetSocketAddress(Ipv4Address::GetAny(), m_port);
            if (m_socket->Bind(local) == -1) {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            if (addressUtils::IsMulticast(m_local)) {
                throw std::runtime_error("Multi-cast is not supported.");
            }
        }

        m_socket->SetRecvCallback(MakeCallback(&UdpRttServer::HandleRead, this));
    }

    void
    UdpRttServer::StopApplication() {
        NS_LOG_FUNCTION(this);
        if (m_socket) {
            m_socket->Close();
            m_socket->SetRecvCallback(MakeNullCallback < void, Ptr < Socket > > ());
        }
    }

    void
    UdpRttServer::HandleRead(Ptr <Socket> socket) {
        NS_LOG_FUNCTION(this << socket);
        Ptr <Packet> packet;
        Address from;
        while ((packet = socket->RecvFrom(from))) {

            // What we receive
            SeqTsHeader incomingSeqTs;
            packet->RemoveHeader (incomingSeqTs);

            // Add header
            SeqTsHeader outgoingSeqTs; // Creates one with the current timestamp
            outgoingSeqTs.SetSeq(incomingSeqTs.GetSeq());
            packet->AddHeader(outgoingSeqTs);

            // Remove any tags
            packet->RemoveAllPacketTags();
            packet->RemoveAllByteTags();

            // Send back with the new timestamp on it
            socket->SendTo(packet, 0, from);

        }
    }

} // Namespace ns3
