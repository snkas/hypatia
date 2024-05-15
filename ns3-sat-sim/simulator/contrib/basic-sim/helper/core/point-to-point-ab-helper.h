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
 * Adapted from PointToPointHelper by:
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#ifndef POINT_TO_POINT_AB_HELPER_H
#define POINT_TO_POINT_AB_HELPER_H

#include <string>

#include "ns3/object-factory.h"
#include "ns3/net-device-container.h"
#include "ns3/node-container.h"

#include "ns3/trace-helper.h"

namespace ns3 {

    class NetDevice;
    class Node;
    class PointToPointAbHelper {
    public:
        PointToPointAbHelper();

        void SetQueueFactoryA(ObjectFactory queueFactoryA);

        void SetQueueFactoryB(ObjectFactory queueFactoryB);

        void SetDeviceAttributeA(std::string name, const AttributeValue &value);

        void SetDeviceAttributeB(std::string name, const AttributeValue &value);

        void SetChannelAttribute(std::string name, const AttributeValue &value);

        NetDeviceContainer Install(Ptr <Node> a, Ptr <Node> b);

    private:
        ObjectFactory m_queueFactoryA;         //!< Queue Factory A
        ObjectFactory m_deviceFactoryA;        //!< Device Factory A
        ObjectFactory m_queueFactoryB;         //!< Queue Factory B
        ObjectFactory m_deviceFactoryB;        //!< Device Factory B
        ObjectFactory m_channelFactory;        //!< Channel Factory
#ifdef NS3_MPI
        ObjectFactory m_remoteChannelFactory; //!< Remote Channel Factory
#endif
    };

} // namespace ns3

#endif /* POINT_TO_POINT_AB_HELPER_H */
