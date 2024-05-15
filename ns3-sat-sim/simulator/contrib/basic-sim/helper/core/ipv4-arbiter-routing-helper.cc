/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 University of Washington
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

#include "ipv4-arbiter-routing-helper.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE ("Ipv4ArbiterRoutingHelper");

Ipv4ArbiterRoutingHelper::Ipv4ArbiterRoutingHelper() {
    // Left empty intentionally
}

Ipv4ArbiterRoutingHelper::Ipv4ArbiterRoutingHelper (const Ipv4ArbiterRoutingHelper &o) {
    // Left empty intentionally
}

Ipv4ArbiterRoutingHelper* Ipv4ArbiterRoutingHelper::Copy (void) const {
  return new Ipv4ArbiterRoutingHelper (*this);
}

Ptr<Ipv4RoutingProtocol> Ipv4ArbiterRoutingHelper::Create (Ptr<Node> node) const {
  return CreateObject<Ipv4ArbiterRouting> ();
}

} // namespace ns3
