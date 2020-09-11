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
 * Author: Andre Aguas    March 2020
 * 
 */


#ifndef POINT_TO_POINT_LASER_CHANNEL_H
#define POINT_TO_POINT_LASER_CHANNEL_H

#include "ns3/channel.h"
#include "ns3/data-rate.h"
#include "ns3/mobility-model.h"
#include "ns3/node.h"
#include "ns3/point-to-point-laser-net-device.h"


namespace ns3 {

class PointToPointLaserNetDevice;
class Packet;

/**
 * \brief Point to Point Laser Channel
 * 
 * Channel connecting two satellites 
 *
 * This class represents a very simple point to point channel.  Think full
 * duplex RS-232 or RS-422 with null modem and no handshaking.  There is no
 * multi-drop capability on this channel -- there can be a maximum of two 
 * point-to-point net devices connected.
 *
 * There are two "wires" in the channel.  The first device connected gets the
 * [0] wire to transmit on.  The second device gets the [1] wire.  There is a
 * state (IDLE, TRANSMITTING) associated with each wire.
 * 
 * (PointToPointChannel with mobile nodes)
 *
 */
class PointToPointLaserChannel : public Channel 
{
public:
  /**
   * \brief Get the TypeId
   *
   * \return The TypeId for this class
   */
  static TypeId GetTypeId (void);

  /**
   * \brief Create a PointToPointLaserChannel
   * 
   */
  PointToPointLaserChannel ();

  /**
   * \brief Attach a given netdevice to this channel
   * 
   * \param device pointer to the netdevice to attach to the channel
   */
  void Attach (Ptr<PointToPointLaserNetDevice> device);

  /**
   * \brief Transmit a packet over this channel
   * 
   * \param p Packet to transmit
   * \param src source PointToPointLaserNetDevice
   * \param node_other_end node at the other end of the channel
   * \param txTime transmission time
   * \returns true if successful (always true)
   */
  virtual bool TransmitStart (Ptr<const Packet> p, Ptr<PointToPointLaserNetDevice> src, Ptr<Node> node_other_end, Time txTime);

  /**
   * \brief Write the traffic sent to each node (link utilization) to a stringstream 
   * 
   * \param str the string stream
   * \param node_id the source node of the traffic
   */
  void WriteTraffic(std::stringstream& str, uint32_t node_id);

  /**
   * \brief Get number of devices on this channel
   * 
   * \returns number of devices on this channel
   */
  virtual std::size_t GetNDevices (void) const;

  /**
   * \brief Get PointToPointLaserNetDevice corresponding to index i on this channel
   * 
   * \param i Index number of the device requested
   * 
   * \returns Ptr to PointToPointLaserNetDevice requested
   */
  Ptr<PointToPointLaserNetDevice> GetPointToPointLaserDevice (std::size_t i) const;

  /**
   * \brief Get NetDevice corresponding to index i on this channel
   * 
   * \param i Index number of the device requested
   * 
   * \returns Ptr to NetDevice requested
   */
  virtual Ptr<NetDevice> GetDevice (std::size_t i) const;

protected:
  /**
   * \brief Get the delay between two nodes on this channel
   * 
   * \param senderMobility location of the sender
   * \param receiverMobility location of the receiver
   * 
   * \returns Time delay
   */
  Time GetDelay (Ptr<MobilityModel> senderMobility, Ptr<MobilityModel> receiverMobility) const;

  /**
   * \brief Check to make sure the link is initialized
   * 
   * \returns true if initialized, asserts otherwise
   */
  bool IsInitialized (void) const;

  /**
   * \brief Get the source net-device 
   * 
   * \param i the link (direction) requested
   * 
   * \returns Ptr to source PointToPointLaserNetDevice for the 
   *          specified link
   */
  Ptr<PointToPointLaserNetDevice> GetSource (uint32_t i) const;

  /**
   * \brief Get the destination net-device
   * 
   * \param i the link requested
   * \returns Ptr to destination PointToPointLaserNetDevice for 
   *          the specified link
   */
  Ptr<PointToPointLaserNetDevice> GetDestination (uint32_t i) const;

  /**
   * \brief stores the number of bytes send between every pair of nodes 
   * 
   * \param wire the direction to where the bytes were sent
   * \param n_bytes the amount of bytes sent
   */
  void BookkeepBytes (uint32_t wire, uint32_t n_bytes);

  /**
   * TracedCallback signature for packet transmission animation events.
   *
   * \param [in] packet The packet being transmitted.
   * \param [in] txDevice the TransmitTing NetDevice.
   * \param [in] rxDevice the Receiving NetDevice.
   * \param [in] duration The amount of time to transmit the packet.
   * \param [in] lastBitTime Last bit receive time (relative to now)
   * \deprecated The non-const \c Ptr<NetDevice> argument is deprecated
   * and will be changed to \c Ptr<const NetDevice> in a future release.
   */
  typedef void (* TxRxAnimationCallback)
    (Ptr<const Packet> packet,
     Ptr<NetDevice> txDevice, Ptr<NetDevice> rxDevice,
     Time duration, Time lastBitTime);
                    
private:
  /** Each point to point link has exactly two net devices. */
  static const std::size_t N_DEVICES = 2;

  Time               m_initial_delay;     //!< Propagation delay at the initial distance
                                          //   used to give a delay estimate to the
                                          //   distributed simulator
  double             m_propagationSpeed;  //!< propagation speed on the channel
  std::size_t        m_nDevices;          //!< Devices of this channel

  /**
   * The trace source for the packet transmission animation events that the 
   * device can fire.
   * Arguments to the callback are the packet, transmitting
   * net device, receiving net device, transmission time and 
   * packet receipt time.
   *
   * \see class CallBackTraceSource
   * \deprecated The non-const \c Ptr<NetDevice> argument is deprecated
   * and will be changed to \c Ptr<const NetDevice> in a future release.
   */
  TracedCallback<Ptr<const Packet>,     // Packet being transmitted
                 Ptr<NetDevice>,  // Transmitting NetDevice
                 Ptr<NetDevice>,  // Receiving NetDevice
                 Time,                  // Amount of time to transmit the pkt
                 Time                   // Last bit receive time (relative to now)
                 > m_txrxPointToPoint;

  /** \brief Wire states
   *
   */
  enum WireState
  {
    /** Initializing state */
    INITIALIZING,
    /** Idle state (no transmission from NetDevice) */
    IDLE,
    /** Transmitting state (data being transmitted from NetDevice. */
    TRANSMITTING,
    /** Propagating state (data is being propagated in the channel. */
    PROPAGATING
  };

  /**
   * \brief Wire model for the PointToPointLaserChannel
   */
  class Link
  {
public:
    /** \brief Create the link, it will be in INITIALIZING state
     *
     */
    Link() : m_state (INITIALIZING), m_src (0), m_dst (0) {}

    WireState                       m_state; //!< State of the link
    Ptr<PointToPointLaserNetDevice> m_src;   //!< First NetDevice
    Ptr<PointToPointLaserNetDevice> m_dst;   //!< Second NetDevice
  };

  Link    m_link[N_DEVICES]; //!< Link model
};

} // namespace ns3

#endif /* POINT_TO_POINT_LASER_CHANNEL_H */
