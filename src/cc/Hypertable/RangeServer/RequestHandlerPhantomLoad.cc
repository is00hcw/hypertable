/** -*- c++ -*-
 * Copyright (C) 2007-2012 Hypertable, Inc.
 *
 * This file is part of Hypertable.
 *
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; version 3 of the
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

#include "Common/Compat.h"
#include "Common/Error.h"
#include "Common/Logger.h"

#include "AsyncComm/ResponseCallback.h"
#include "Common/Serialization.h"

#include "Hypertable/Lib/Types.h"

#include "RangeServer.h"
#include "RequestHandlerPhantomLoad.h"

using namespace Hypertable;
using namespace Serialization;

/**
 *
 */
void RequestHandlerPhantomLoad::run() {
  ResponseCallback cb(m_comm, m_event);
  String location;
  int plan_generation;
  vector<uint32_t> fragments;
  vector<QualifiedRangeSpec> specs;
  vector<RangeState> states;
  uint32_t nn;

  const uint8_t *decode_ptr = m_event->payload;
  size_t decode_remain = m_event->payload_len;
  QualifiedRangeSpec spec;
  RangeState state;

  try {
    location = Serialization::decode_vstr(&decode_ptr, &decode_remain);
    plan_generation = Serialization::decode_i32(&decode_ptr, &decode_remain);
    nn = Serialization::decode_i32(&decode_ptr, &decode_remain);
    for (uint32_t ii=0; ii<nn; ++ii)
      fragments.push_back(Serialization::decode_i32(&decode_ptr, 
                  &decode_remain));
    nn = Serialization::decode_i32(&decode_ptr, &decode_remain);
    for (uint32_t ii=0; ii<nn; ++ii) {
      spec.decode(&decode_ptr, &decode_remain);
      specs.push_back(spec);
    }
    for (uint32_t ii=0; ii<nn; ++ii) {
      state.decode(&decode_ptr, &decode_remain);
      states.push_back(state);
    }
  }
  catch (Exception &e) {
    HT_ERROR_OUT << e << HT_END;
    cb.error(e.code(), e.what());
    return;
  }

  try {
    m_range_server->phantom_load(&cb, location, plan_generation, fragments, specs, states);
  }
  catch (Exception &e) {
    HT_ERROR_OUT << e << HT_END;
    cb.error(e.code(), e.what());
  }

}
