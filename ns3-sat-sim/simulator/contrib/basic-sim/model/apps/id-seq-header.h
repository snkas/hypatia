/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 INRIA
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
 * Based on SeqTsHeader by:
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */

#ifndef ID_SEQ_HEADER_H
#define ID_SEQ_HEADER_H

#include "ns3/header.h"

namespace ns3 {
    
class IdSeqHeader : public Header
{
public:
  static TypeId GetTypeId (void);
    
  IdSeqHeader ();
  void SetId (uint64_t id);
  void SetSeq (uint64_t seq);
  uint64_t GetId (void) const;
  uint64_t GetSeq (void) const;

  virtual TypeId GetInstanceTypeId (void) const;
  virtual void Print (std::ostream &os) const;
  virtual uint32_t GetSerializedSize (void) const;
  virtual void Serialize (Buffer::Iterator start) const;
  virtual uint32_t Deserialize (Buffer::Iterator start);

private:
  uint64_t m_id;  //!< Identifier of what these sequences belong to
  uint64_t m_seq; //!< Sequence number
};

} // namespace ns3

#endif /* ID_SEQ_HEADER_H */
