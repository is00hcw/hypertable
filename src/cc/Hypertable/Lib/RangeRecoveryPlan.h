/** -*- c++ -*-
 * Copyright (C) 2007-2012 Hypertable, Inc.
 *
 * This file is part of Hypertable.
 *
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 2 of the
 * License, or any later version.
 *
 * Hypertable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#ifndef HYPERTABLE_RANGERECOVERYPLAN_H
#define HYPERTABLE_RANGERECOVERYPLAN_H

#include "Common/Compat.h"
#include <vector>
#include <map>

#include "Common/String.h"
#include "Common/Serialization.h"
#include "Types.h"
#include "RangeRecoveryReceiverPlan.h"
#include "RangeRecoveryReplayPlan.h"

namespace Hypertable {

  class RangeRecoveryPlan : public ReferenceCount {
  public:
    RangeRecoveryPlan() : type(RangeSpec::UNKNOWN) { }
    RangeRecoveryPlan(int type_) : type(type_) { }

    size_t encoded_length() const {
      return 4 + replay_plan.encoded_length() + receiver_plan.encoded_length();
    }

    void encode(uint8_t **bufp) const {
      Serialization::encode_i32(bufp, type);
      replay_plan.encode(bufp);
      receiver_plan.encode(bufp);
    }

    void decode(const uint8_t **bufp, size_t *remainp) {
      type = Serialization::decode_i32(bufp, remainp);
      replay_plan.decode(bufp, remainp);
      receiver_plan.decode(bufp, remainp);
    }

    void clear() {
      type = RangeSpec::UNKNOWN;
      replay_plan.clear();
      receiver_plan.clear();
    }

    friend std::ostream &operator<<(std::ostream &os,
            const RangeRecoveryPlan &plan) {
      os << "{RangeRecoveryPlan: type=" << plan.type << ", replay_plan="
         << plan.replay_plan << ", receiver_plan=" << plan.receiver_plan << "}";
      return os;
    }

    int type;
    RangeRecoveryReplayPlan replay_plan;
    RangeRecoveryReceiverPlan receiver_plan;
  };
  
  typedef intrusive_ptr<RangeRecoveryPlan> RangeRecoveryPlanPtr;

} // namespace Hypertable

#endif // HYPERTABLE_RANGERECOVERYPLAN_H
