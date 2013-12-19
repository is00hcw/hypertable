/* -*- c++ -*-
 * Copyright (C) 2007-2013 Hypertable, Inc.
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

/// @file
/// Declarations for BlockCompressionCodec.
/// This file contains declarations for BlockCompressionCodec, an abstract base
/// class for block compressors.

#ifndef HYPERTABLE_BLOCKCOMPRESSIONCODEC_H
#define HYPERTABLE_BLOCKCOMPRESSIONCODEC_H

#include <Common/ReferenceCount.h>
#include <Common/String.h>

#include <Hypertable/Lib/BlockHeader.h>

#include <vector>

namespace Hypertable {

  class DynamicBuffer;

  /// @addtogroup libHypertable
  /// @{

  /// Abstract base class for block compression codecs.
  class BlockCompressionCodec : public ReferenceCount {
  public:

    /// Enumeration for compression type.
    enum Type {
      UNKNOWN=-1, ///< Unknown compression type
      NONE=0,     ///< No compression
      BMZ=1,      ///< Bentley-McIlroy large common substring compression
      ZLIB=2,     ///< ZLIB compression
      LZO=3,      ///< LZO compression
      QUICKLZ=4,  ///< QuickLZ 1.5 compession
      SNAPPY=5,   ///< Snappy compression
      COMPRESSION_TYPE_LIMIT=6  ///< Limit of compression types
    };

    /// Compression codec argument vector.
    typedef std::vector<String> Args;

    /// Returns string mnemonic for compression type.
    /// @param algo Compression type (see BlockCompressionCodec::Type)
    /// @return %String mnemonic representing name of compression algorithm
    static const char *get_compressor_name(uint16_t algo);

    /// Destructor.
    virtual ~BlockCompressionCodec() { }

    /// Compresses a buffer.
    /// @param intput Input buffer
    /// @param output Output buffer
    /// @param header Block header populated by function
    /// @param reserve Additional space to reserve at end of <code>output</code>
    virtual void deflate(const DynamicBuffer &input, DynamicBuffer &output,
                         BlockHeader &header, size_t reserve=0) = 0;

    /// Decompresses a buffer.
    /// @param intput Input buffer
    /// @param output Output buffer
    /// @param header Block header
    virtual void inflate(const DynamicBuffer &input, DynamicBuffer &output,
                         BlockHeader &header) = 0;

    /// Sets arguments to control compression behavior
    /// @param args Compressor specific arguments
    virtual void set_args(const Args &args) {}

    /// Returns compression type enum.
    /// Returns the enum value that represents the compressoion type
    /// @see BlockCompressionCodec::Type
    /// @return Compressor type
    virtual int get_type() = 0;

  };

  /// Smart pointer to BlockCompressionCodec
  typedef boost::intrusive_ptr<BlockCompressionCodec> BlockCompressionCodecPtr;

  /// @}

} // namespace Hypertable

#endif // HYPERTABLE_BLOCKCOMPRESSIONCODEC_H
