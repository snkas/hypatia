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


#ifndef GSL_CHANNEL_H
#define GSL_CHANNEL_H

#include "ns3/channel.h"
#include "ns3/data-rate.h"
#include "ns3/mobility-model.h"
#include "ns3/sgi-hashmap.h"
#include "ns3/mac48-address.h"

namespace ns3 {

class GSLNetDevice;
class Packet;

class Mac48AddressHash : public std::unary_function<Mac48Address, size_t> {
    public:
        size_t operator() (Mac48Address const &x) const;
};

class GSLChannel : public Channel 
{
public:
  static TypeId GetTypeId (void);
  GSLChannel ();

  // Transmission
  virtual bool TransmitStart (Ptr<const Packet> p, Ptr<GSLNetDevice> src, Address dst_address, Time txTime);
  bool TransmitTo(Ptr<const Packet> p, Ptr<GSLNetDevice> srcNetDevice, Ptr<GSLNetDevice> dstNetDevice, Time txTime, bool isSameSystem);

  // Device management
  void Attach (Ptr<GSLNetDevice> device);
  virtual std::size_t GetNDevices (void) const;
  Ptr<GSLNetDevice> GetGSLDevice (std::size_t i) const;
  virtual Ptr<NetDevice> GetDevice (std::size_t i) const;

protected:
  Time GetDelay (Ptr<MobilityModel> senderMobility, Ptr<MobilityModel> receiverMobility) const;
  bool IsInitialized (void) const;
                    
private:

  Time   m_initialDelay;      //!< Propagation delay at the initial distance
                              //   used to give a delay estimate to the
                              //   distributed simulator
  double m_propagationSpeed;  //!< propagation speed on the channel
  double m_range;             //!< maximum distance two devices can communicate over this channel

  enum WireState
  {

    INITIALIZING,   /** Initializing state */
    IDLE,           /** Idle state (no transmission from NetDevice) */
    TRANSMITTING,   /** Transmitting state (data being transmitted from NetDevice. */
    PROPAGATING     /** Propagating state (data is being propagated in the channel. */
  };

  class GSLLink
  {
    public:
        GSLLink() : m_state (INITIALIZING), m_net_device (0) {}
        WireState           m_state;       //!< State of the link
        Ptr<GSLNetDevice>   m_net_device;  //!< First NetDevice
  };

  typedef sgi::hash_map<Mac48Address, GSLLink, Mac48AddressHash> MacToNetDevice;
  typedef sgi::hash_map<Mac48Address, GSLLink, Mac48AddressHash>::iterator MacToNetDeviceI;
  MacToNetDevice m_link;
  std::vector<Ptr<GSLNetDevice>> m_net_devices;

};

} // namespace ns3

#endif /* GSL_CHANNEL_H */
