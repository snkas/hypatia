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
#include "ns3/ipv6-address.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/inet6-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "udp-rtt-client.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("UdpRttClientApplication");

NS_OBJECT_ENSURE_REGISTERED (UdpRttClient);

TypeId
UdpRttClient::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::UdpRttClient")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<UdpRttClient>()
            .AddAttribute("Interval",
                          "The time to wait between packets",
                          TimeValue(Seconds(1.0)),
                          MakeTimeAccessor(&UdpRttClient::m_interval),
                          MakeTimeChecker())
            .AddAttribute("RemoteAddress",
                          "The destination Address of the outbound packets",
                          AddressValue(),
                          MakeAddressAccessor(&UdpRttClient::m_peerAddress),
                          MakeAddressChecker())
            .AddAttribute("RemotePort",
                          "The destination port of the outbound packets",
                          UintegerValue(0),
                          MakeUintegerAccessor(&UdpRttClient::m_peerPort),
                          MakeUintegerChecker<uint16_t>())
            .AddAttribute("FromNodeId",
                          "From node identifier",
                          UintegerValue(0),
                          MakeUintegerAccessor(&UdpRttClient::m_fromNodeId),
                          MakeUintegerChecker<uint64_t>())
            .AddAttribute("ToNodeId",
                          "To node identifier",
                          UintegerValue(0),
                          MakeUintegerAccessor(&UdpRttClient::m_toNodeId),
                          MakeUintegerChecker<uint64_t>());
    return tid;
}

UdpRttClient::UdpRttClient() {
    NS_LOG_FUNCTION(this);
    m_socket = 0;
    m_sent = 0;
    m_sendEvent = EventId();
}

UdpRttClient::~UdpRttClient() {
    NS_LOG_FUNCTION(this);
    m_socket = 0;
}

void
UdpRttClient::DoDispose(void) {
    NS_LOG_FUNCTION(this);
    Application::DoDispose();
}

void
UdpRttClient::StartApplication(void) {
    NS_LOG_FUNCTION(this);
    if (!m_socket) {
        TypeId tid = TypeId::LookupByName("ns3::UdpSocketFactory");
        m_socket = Socket::CreateSocket(GetNode(), tid);
        if (Ipv4Address::IsMatchingType(m_peerAddress) == true) {
            if (m_socket->Bind() == -1) {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            m_socket->Connect(InetSocketAddress(Ipv4Address::ConvertFrom(m_peerAddress), m_peerPort));
        } else if (InetSocketAddress::IsMatchingType(m_peerAddress) == true) {
            if (m_socket->Bind() == -1) {
                NS_FATAL_ERROR("Failed to bind socket");
            }
            m_socket->Connect(m_peerAddress);
        } else {
            NS_ASSERT_MSG(false, "Incompatible address type: " << m_peerAddress);
        }
    }
    m_socket->SetRecvCallback(MakeCallback(&UdpRttClient::HandleRead, this));
    m_socket->SetAllowBroadcast(true);
    ScheduleTransmit(Seconds(0.));
}

void
UdpRttClient::StopApplication() {
    NS_LOG_FUNCTION(this);
    if (m_socket) {
        m_socket->Close();
        m_socket->SetRecvCallback(MakeNullCallback < void, Ptr < Socket > > ());
        m_socket = 0;
    }
    Simulator::Cancel(m_sendEvent);
}

void
UdpRttClient::ScheduleTransmit(Time dt) {
    NS_LOG_FUNCTION(this << dt);
    m_sendEvent = Simulator::Schedule(dt, &UdpRttClient::Send, this);
}

void
UdpRttClient::Send(void) {
    NS_LOG_FUNCTION(this);
    NS_ASSERT(m_sendEvent.IsExpired());

    // Packet with timestamp
    Ptr<Packet> p = Create<Packet>();
    SeqTsHeader seqTs; // Creates one with the current timestamp
    seqTs.SetSeq(m_sent);
    p->AddHeader(seqTs);

    // Timestamps
    m_sendRequestTimestamps.push_back(Simulator::Now().GetNanoSeconds());
    m_replyTimestamps.push_back(-1);
    m_receiveReplyTimestamps.push_back(-1);
    m_sent++;

    // Send out
    m_socket->Send(p);

    // Schedule next transmit
    ScheduleTransmit(m_interval);

}

void
UdpRttClient::HandleRead(Ptr <Socket> socket) {
    NS_LOG_FUNCTION(this << socket);
    Ptr <Packet> packet;
    Address from;
    Address localAddress;
    while ((packet = socket->RecvFrom(from))) {

        // Receiving header
        SeqTsHeader incomingSeqTs;
        packet->RemoveHeader (incomingSeqTs);
        uint32_t seqNo = incomingSeqTs.GetSeq();

        // Update the local timestamps
        m_replyTimestamps[seqNo] = incomingSeqTs.GetTs().GetNanoSeconds();
        m_receiveReplyTimestamps[seqNo] = Simulator::Now().GetNanoSeconds();


    }
}

uint32_t UdpRttClient::GetFromNodeId() {
    return m_fromNodeId;
}

uint32_t UdpRttClient::GetToNodeId() {
    return m_toNodeId;
}

uint32_t UdpRttClient::GetSent() {
    return m_sent;
}

std::vector<int64_t> UdpRttClient::GetSendRequestTimestamps() {
    return m_sendRequestTimestamps;
}

std::vector<int64_t> UdpRttClient::GetReplyTimestamps() {
    return m_replyTimestamps;
}

std::vector<int64_t> UdpRttClient::GetReceiveReplyTimestamps() {
    return m_receiveReplyTimestamps;
}

} // Namespace ns3
