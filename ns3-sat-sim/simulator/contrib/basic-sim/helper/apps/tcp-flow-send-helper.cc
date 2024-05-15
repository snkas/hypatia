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
 * Author: Simon
 * Adapted from BulkSendHelper by:
 * Author: Geoge Riley <riley@ece.gatech.edu>
 * Adapted from OnOffHelper by:
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#include "tcp-flow-send-helper.h"
#include "ns3/inet-socket-address.h"
#include "ns3/packet-socket-address.h"
#include "ns3/string.h"
#include "ns3/boolean.h"
#include "ns3/names.h"

namespace ns3 {

TcpFlowSendHelper::TcpFlowSendHelper (std::string protocol, Address address, uint64_t maxBytes, int64_t flowId, bool enableFlowLoggingToFile, std::string baseLogsDir, std::string additionalParameters)
{
  m_factory.SetTypeId ("ns3::TcpFlowSendApplication");
  m_factory.Set ("Protocol", StringValue (protocol));
  m_factory.Set ("Remote", AddressValue (address));
  m_factory.Set ("MaxBytes", UintegerValue (maxBytes));
  m_factory.Set ("TcpFlowId", UintegerValue (flowId));
  m_factory.Set ("EnableTcpFlowLoggingToFile", BooleanValue (enableFlowLoggingToFile));
  m_factory.Set ("BaseLogsDir", StringValue (baseLogsDir));
  m_factory.Set ("AdditionalParameters", StringValue (additionalParameters));
}

ApplicationContainer
TcpFlowSendHelper::Install (Ptr<Node> node) const
{
  return ApplicationContainer (InstallPriv (node));
}

Ptr<Application>
TcpFlowSendHelper::InstallPriv (Ptr<Node> node) const
{
  Ptr<Application> app = m_factory.Create<Application> ();
  node->AddApplication (app);

  return app;
}

} // namespace ns3
