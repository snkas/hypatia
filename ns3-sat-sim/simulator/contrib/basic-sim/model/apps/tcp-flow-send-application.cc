/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 Georgia Institute of Technology
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
 * Adapted from BulkSendApplication by:
 * Author: George F. Riley <riley@ece.gatech.edu>
 */

#include "ns3/log.h"
#include "ns3/address.h"
#include "ns3/node.h"
#include "ns3/nstime.h"
#include "ns3/socket.h"
#include "ns3/string.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/trace-source-accessor.h"
#include "ns3/tcp-socket-factory.h"
#include "ns3/tcp-socket-base.h"
#include "ns3/tcp-tx-buffer.h"
#include "ns3/exp-util.h"
#include "tcp-flow-send-application.h"
#include <fstream>

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("TcpFlowSendApplication");

NS_OBJECT_ENSURE_REGISTERED (TcpFlowSendApplication);

TypeId
TcpFlowSendApplication::GetTypeId(void) {
    static TypeId tid = TypeId("ns3::TcpFlowSendApplication")
            .SetParent<Application>()
            .SetGroupName("Applications")
            .AddConstructor<TcpFlowSendApplication>()
            .AddAttribute("SendSize", "The amount of data to send each time.",
                          UintegerValue(100000),
                          MakeUintegerAccessor(&TcpFlowSendApplication::m_sendSize),
                          MakeUintegerChecker<uint32_t>(1))
            .AddAttribute("Remote", "The address of the destination",
                          AddressValue(),
                          MakeAddressAccessor(&TcpFlowSendApplication::m_peer),
                          MakeAddressChecker())
            .AddAttribute("MaxBytes",
                          "The total number of bytes to send. "
                          "Once these bytes are sent, "
                          "no data  is sent again. The value zero means "
                          "that there is no limit.",
                          UintegerValue(0),
                          MakeUintegerAccessor(&TcpFlowSendApplication::m_maxBytes),
                          MakeUintegerChecker<uint64_t>())
            .AddAttribute("TcpFlowId",
                          "TCP flow identifier",
                          UintegerValue(0),
                          MakeUintegerAccessor(&TcpFlowSendApplication::m_tcpFlowId),
                          MakeUintegerChecker<uint64_t>())
            .AddAttribute("Protocol", "The type of protocol to use.",
                          TypeIdValue(TcpSocketFactory::GetTypeId()),
                          MakeTypeIdAccessor(&TcpFlowSendApplication::m_tid),
                          MakeTypeIdChecker())
            .AddAttribute("EnableTcpFlowLoggingToFile",
                          "True iff you want to track some aspects (progress, CWND, RTT) of the TCP flow over time.",
                          BooleanValue(true),
                          MakeBooleanAccessor(&TcpFlowSendApplication::m_enableDetailedLogging),
                          MakeBooleanChecker())
            .AddAttribute ("BaseLogsDir",
                           "Base logging directory (logging will be placed here, i.e. logs_dir/tcp_flow_[flow id]_{progress, cwnd, rtt}.csv",
                           StringValue (""),
                           MakeStringAccessor (&TcpFlowSendApplication::m_baseLogsDir),
                           MakeStringChecker ())
            .AddAttribute ("AdditionalParameters",
                           "Additional parameter string; this might be parsed in another version of this application to "
                           "do slightly different behavior (e.g., set priority on TCP socket etc.)",
                           StringValue (""),
                           MakeStringAccessor (&TcpFlowSendApplication::m_additionalParameters),
                           MakeStringChecker ())
            .AddTraceSource("Tx", "A new packet is created and is sent",
                            MakeTraceSourceAccessor(&TcpFlowSendApplication::m_txTrace),
                            "ns3::Packet::TracedCallback");
    return tid;
}


TcpFlowSendApplication::TcpFlowSendApplication()
        : m_socket(0),
          m_connected(false),
          m_totBytes(0),
          m_completionTimeNs(-1),
          m_connFailed(false),
          m_closedNormally(false),
          m_closedByError(false),
          m_ackedBytes(0),
          m_isCompleted(false) {
    NS_LOG_FUNCTION(this);
}

TcpFlowSendApplication::~TcpFlowSendApplication() {
    NS_LOG_FUNCTION(this);
}

void
TcpFlowSendApplication::DoDispose(void) {
    NS_LOG_FUNCTION(this);

    m_socket = 0;
    // chain up
    Application::DoDispose();
}

void TcpFlowSendApplication::StartApplication(void) { // Called at time specified by Start
    NS_LOG_FUNCTION(this);

    // Create the socket if not already
    if (!m_socket) {
        m_socket = Socket::CreateSocket(GetNode(), m_tid);

        // Must be TCP basically
        if (m_socket->GetSocketType() != Socket::NS3_SOCK_STREAM &&
            m_socket->GetSocketType() != Socket::NS3_SOCK_SEQPACKET) {
            NS_FATAL_ERROR("Using TcpFlowSendApplication with an incompatible socket type. "
                           "TcpFlowSendApplication requires SOCK_STREAM or SOCK_SEQPACKET. "
                           "In other words, use TCP instead of UDP.");
        }

        // Bind socket
        if (Inet6SocketAddress::IsMatchingType(m_peer)) {
            if (m_socket->Bind6() == -1) {
                NS_FATAL_ERROR("Failed to bind socket");
            }
        } else if (InetSocketAddress::IsMatchingType(m_peer)) {
            if (m_socket->Bind() == -1) {
                NS_FATAL_ERROR("Failed to bind socket");
            }
        }

        // Connect, no receiver
        m_socket->Connect(m_peer);
        m_socket->ShutdownRecv();

        // Callbacks
        m_socket->SetConnectCallback(
                MakeCallback(&TcpFlowSendApplication::ConnectionSucceeded, this),
                MakeCallback(&TcpFlowSendApplication::ConnectionFailed, this)
        );
        m_socket->SetSendCallback(MakeCallback(&TcpFlowSendApplication::DataSend, this));
        m_socket->SetCloseCallbacks(
                MakeCallback(&TcpFlowSendApplication::SocketClosedNormal, this),
                MakeCallback(&TcpFlowSendApplication::SocketClosedError, this)
        );
        if (m_enableDetailedLogging) {
            std::ofstream ofs;

            ofs.open(m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_progress.csv", m_tcpFlowId));
            ofs << m_tcpFlowId << "," << Simulator::Now ().GetNanoSeconds () << "," << GetAckedBytes() << std::endl;
            ofs.close();

            ofs.open(m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_cwnd.csv", m_tcpFlowId));
            // Congestion window is only set upon SYN reception, so retrieving it early will just yield 0
            // As such there "is" basically no congestion window till then, so we are not going to write 0
            ofs.close();
            m_socket->TraceConnectWithoutContext ("CongestionWindow", MakeCallback (&TcpFlowSendApplication::CwndChange, this));

            ofs.open(m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_rtt.csv", m_tcpFlowId));
            // At the socket creation, there is no RTT measurement, so retrieving it early will just yield 0
            // As such there "is" basically no RTT measurement till then, so we are not going to write 0
            ofs.close();
            m_socket->TraceConnectWithoutContext ("RTT", MakeCallback (&TcpFlowSendApplication::RttChange, this));
        }
    }
    if (m_connected) {
        SendData();
    }
}

void TcpFlowSendApplication::StopApplication(void) { // Called at time specified by Stop
    NS_LOG_FUNCTION(this);
    if (m_socket) {
        m_socket->Close();
        m_connected = false;
    } else {
        NS_LOG_WARN("TcpFlowSendApplication found null socket to close in StopApplication");
    }
}

void TcpFlowSendApplication::SendData(void) {
    NS_LOG_FUNCTION(this);
    while (m_maxBytes == 0 || m_totBytes < m_maxBytes) { // Time to send more

        // uint64_t to allow the comparison later.
        // the result is in a uint32_t range anyway, because
        // m_sendSize is uint32_t.
        uint64_t toSend = m_sendSize;
        // Make sure we don't send too many
        if (m_maxBytes > 0) {
            toSend = std::min(toSend, m_maxBytes - m_totBytes);
        }

        NS_LOG_LOGIC("sending packet at " << Simulator::Now());
        Ptr <Packet> packet = Create<Packet>(toSend);
        int actual = m_socket->Send(packet);
        if (actual > 0) {
            m_totBytes += actual;
            m_txTrace(packet);
        }
        // We exit this loop when actual < toSend as the send side
        // buffer is full. The "DataSent" callback will pop when
        // some buffer space has freed up.
        if ((unsigned) actual != toSend) {
            break;
        }
    }
    // Check if time to close (all sent)
    if (m_totBytes == m_maxBytes && m_connected) {
        m_socket->Close(); // Close will only happen after send buffer is finished
        m_connected = false;
    }
}

void TcpFlowSendApplication::ConnectionSucceeded(Ptr <Socket> socket) {
    NS_LOG_FUNCTION(this << socket);
    NS_LOG_LOGIC("TcpFlowSendApplication Connection succeeded");
    m_connected = true;
    SendData();
}

void TcpFlowSendApplication::ConnectionFailed(Ptr <Socket> socket) {
    NS_LOG_FUNCTION(this << socket);
    NS_LOG_LOGIC("TcpFlowSendApplication, Connection Failed");
    m_completionTimeNs = Simulator::Now().GetNanoSeconds();
    m_connFailed = true;
    m_closedByError = false;
    m_closedNormally = false;
    m_ackedBytes = 0;
    m_isCompleted = false;
    m_socket = 0;
}

void TcpFlowSendApplication::DataSend(Ptr <Socket>, uint32_t) {
    NS_LOG_FUNCTION(this);
    if (m_connected) { // Only send new data if the connection has completed
        SendData();
    }

    // Log the progress as DataSend() is called anytime space in the transmission buffer frees up
    if (m_enableDetailedLogging) {
        InsertProgressLog(
                Simulator::Now ().GetNanoSeconds (),
                GetAckedBytes()
        );
    }

}

void TcpFlowSendApplication::SocketClosedNormal(Ptr <Socket> socket) {
    m_completionTimeNs = Simulator::Now().GetNanoSeconds();
    m_connFailed = false;
    m_closedByError = false;
    m_closedNormally = true;
    if (m_socket->GetObject<TcpSocketBase>()->GetTxBuffer()->Size() != 0) {
        throw std::runtime_error("Socket closed normally but send buffer is not empty");
    }
    m_ackedBytes = m_totBytes - m_socket->GetObject<TcpSocketBase>()->GetTxBuffer()->Size();
    m_isCompleted = m_ackedBytes == m_maxBytes;
    m_socket = 0;
}

void TcpFlowSendApplication::SocketClosedError(Ptr <Socket> socket) {
    m_completionTimeNs = Simulator::Now().GetNanoSeconds();
    m_connFailed = false;
    m_closedByError = true;
    m_closedNormally = false;
    m_ackedBytes = m_totBytes - m_socket->GetObject<TcpSocketBase>()->GetTxBuffer()->Size();
    m_isCompleted = false;
    m_socket = 0;
}

int64_t TcpFlowSendApplication::GetAckedBytes() {
    if (m_connFailed || m_closedNormally || m_closedByError) {
        return m_ackedBytes;
    } else {
        return m_totBytes - m_socket->GetObject<TcpSocketBase>()->GetTxBuffer()->Size();
    }
}

int64_t TcpFlowSendApplication::GetCompletionTimeNs() {
    return m_completionTimeNs;
}

bool TcpFlowSendApplication::IsCompleted() {
    return m_isCompleted;
}

bool TcpFlowSendApplication::IsConnFailed() {
    return m_connFailed;
}

bool TcpFlowSendApplication::IsClosedNormally() {
    return m_closedNormally;
}

bool TcpFlowSendApplication::IsClosedByError() {
    return m_closedByError;
}

void
TcpFlowSendApplication::CwndChange(uint32_t oldCwnd, uint32_t newCwnd)
{
    InsertCwndLog(Simulator::Now ().GetNanoSeconds (), newCwnd);
}

void
TcpFlowSendApplication::RttChange (Time oldRtt, Time newRtt)
{
    InsertRttLog(Simulator::Now ().GetNanoSeconds (), newRtt.GetNanoSeconds());
}

void
TcpFlowSendApplication::InsertCwndLog(int64_t timestamp, uint32_t cwnd_byte)
{
    std::ofstream ofs;
    ofs.open (m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_cwnd.csv", m_tcpFlowId), std::ofstream::out | std::ofstream::app);
    ofs << m_tcpFlowId << "," << timestamp << "," << cwnd_byte << std::endl;
    m_current_cwnd_byte = cwnd_byte;
    ofs.close();
}

void
TcpFlowSendApplication::InsertRttLog (int64_t timestamp, int64_t rtt_ns)
{
    std::ofstream ofs;
    ofs.open (m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_rtt.csv", m_tcpFlowId), std::ofstream::out | std::ofstream::app);
    ofs << m_tcpFlowId << "," << timestamp << "," << rtt_ns << std::endl;
    m_current_rtt_ns = rtt_ns;
    ofs.close();
}

void
TcpFlowSendApplication::InsertProgressLog (int64_t timestamp, int64_t progress_byte) {
    std::ofstream ofs;
    ofs.open (m_baseLogsDir + "/" + format_string("tcp_flow_%" PRIu64 "_progress.csv", m_tcpFlowId), std::ofstream::out | std::ofstream::app);
    ofs << m_tcpFlowId << "," << timestamp << "," << progress_byte << std::endl;
    ofs.close();
}

void
TcpFlowSendApplication::FinalizeDetailedLogs() {
    if (m_enableDetailedLogging) {
        int64_t timestamp;
        if (m_connFailed || m_closedByError || m_closedNormally) {
            timestamp = m_completionTimeNs;
        } else {
            timestamp = Simulator::Now ().GetNanoSeconds ();
        }
        InsertCwndLog(timestamp, m_current_cwnd_byte);
        InsertRttLog(timestamp, m_current_rtt_ns);
        InsertProgressLog(timestamp, GetAckedBytes());
    }
}

} // Namespace ns3
