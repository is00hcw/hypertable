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

#ifndef HYPERTABLE_SCANCONTEXT_H
#define HYPERTABLE_SCANCONTEXT_H

#include<re2/re2.h>

#include <cassert>
#include <utility>
#include <set>

#include "Common/ByteString.h"
#include "Common/Error.h"
#include "Common/ReferenceCount.h"
#include "Common/StringExt.h"

#include "Hypertable/Lib/Key.h"
#include "Hypertable/Lib/Schema.h"
#include "Hypertable/Lib/ScanSpec.h"
#include "Hypertable/Lib/Types.h"

namespace Hypertable {

  using namespace std;

  class CellFilterInfo {
  public:
    CellFilterInfo(): cutoff_time(0), max_versions(0), counter(false),
        has_index(false), has_qualifier_index(false),
        accept_empty_qualifier(false), filter_by_exact_qualifier(false), 
        filter_by_regexp_qualifier(false), filter_by_prefix_qualifier(false) {}

    CellFilterInfo(const CellFilterInfo& other) {
      cutoff_time = other.cutoff_time;
      max_versions = other.max_versions;
      counter = other.counter;
      regexp_qualifiers.clear();
      for (size_t ii=0; ii<other.regexp_qualifiers.size(); ++ii) {
        regexp_qualifiers.push_back(new RE2(other.regexp_qualifiers[ii]->pattern()));
      }
      exact_qualifiers = other.exact_qualifiers;
      foreach_ht (const String& qualifier, exact_qualifiers) {
        exact_qualifiers_set.insert(qualifier.c_str());
      }
      prefix_qualifiers = other.prefix_qualifiers;
      column_predicates = other.column_predicates;
      search_buf_column_predicates = other.search_buf_column_predicates;
      filter_by_exact_qualifier = other.filter_by_exact_qualifier;
      filter_by_prefix_qualifier = other.filter_by_prefix_qualifier;
      filter_by_regexp_qualifier = other.filter_by_regexp_qualifier;
      has_index = other.has_index;
      has_qualifier_index = other.has_qualifier_index;
      accept_empty_qualifier = other.accept_empty_qualifier;
    }

    ~CellFilterInfo() {
      for (size_t ii=0; ii<regexp_qualifiers.size(); ++ii)
        delete regexp_qualifiers[ii];
    }

    bool qualifier_matches(const char *qualifier, size_t qualifier_len) {
      if (accept_empty_qualifier ||
          (!filter_by_exact_qualifier && !filter_by_regexp_qualifier && 
              !filter_by_prefix_qualifier))
        return true;

      // check exact match first
      if (filter_by_exact_qualifier) {
        if (exact_qualifiers_set.find(qualifier) != exact_qualifiers_set.end())
          return true;
      }

      // check prefix filters
      if (filter_by_prefix_qualifier) {
	int cmp;
        foreach_ht (const String& qstr, prefix_qualifiers) {
          if (qstr.size() > qualifier_len)
            continue;
          if (0 == (cmp = memcmp(qstr.c_str(), qualifier, qstr.size())))
            return true;
	  else if (cmp > 0)
	    break;
        }
      }

      // check for regexp match
      if (filter_by_regexp_qualifier) {
        for (size_t ii=0; ii<regexp_qualifiers.size(); ++ii)
          if (RE2::PartialMatch(qualifier, *regexp_qualifiers[ii]))
            return true;
        return false;
      }

      return false;
    }

    void add_qualifier(const char *qualifier, bool is_regexp, bool is_prefix) {
      if (is_regexp) {
        RE2 *regexp = new RE2(qualifier);
        if (!regexp->ok())
          HT_THROW(Error::BAD_SCAN_SPEC, (String)"Can't convert qualifier " 
                  + qualifier + " to regexp -" + regexp->error_arg());
        regexp_qualifiers.push_back(regexp);
        filter_by_regexp_qualifier = true;
      }
      else if (is_prefix) {
        prefix_qualifiers.insert(qualifier);
        filter_by_prefix_qualifier = true;
      }
      else {
        pair<StringSet::iterator, bool> r = exact_qualifiers.insert(qualifier);
        if (r.second)
          exact_qualifiers_set.insert(r.first->c_str());
        filter_by_exact_qualifier = true;
      }
    }

    bool has_qualifier_filter() const {
      return filter_by_exact_qualifier || filter_by_regexp_qualifier
                || filter_by_prefix_qualifier;
    }
    bool has_qualifier_regexp_filter() const { return filter_by_regexp_qualifier;}

    bool column_predicate_matches(const char* value, uint32_t value_len) {
      int ncp = 0;
      foreach_ht (const ColumnPredicate& cp, column_predicates) {
        if (cp.value && value) {
          switch (cp.operation) {
            case ColumnPredicate::EXACT_MATCH:
              if (cp.value_len == value_len &&
                    memcmp(cp.value, value, cp.value_len) == 0)
                return true;
              break;
            case ColumnPredicate::PREFIX_MATCH:
              if (cp.value_len <= value_len &&
                      memcmp(cp.value, value, cp.value_len) == 0)
                return true;
              break;
            default:
              break;
          }
        }
        else if (!cp.value && !value)
          return true;
        ++ncp;
      }
      return false;
    }

    void add_column_predicate( const ColumnPredicate& cp ) {
      column_predicates.push_back( cp );
    }

    bool has_column_predicate_filter( ) const {
      return !column_predicates.empty();
    }

    int64_t  cutoff_time;
    uint32_t max_versions;
    bool counter;
    bool has_index;
    bool has_qualifier_index;
    bool accept_empty_qualifier;

  private:
    // disable assignment -- if needed then implement with deep copy of
    // qualifier_regexp
    CellFilterInfo& operator = (const CellFilterInfo&);
    vector<RE2 *> regexp_qualifiers;
    StringSet exact_qualifiers;
    CstrSet exact_qualifiers_set;
    StringSet prefix_qualifiers;
    std::vector<ColumnPredicate> column_predicates;
    struct search_buf_t {
      size_t shift[256];
      bool repeat;
      search_buf_t() : repeat( false ) { }
    };
    std::vector<search_buf_t> search_buf_column_predicates;
    bool filter_by_exact_qualifier;
    bool filter_by_regexp_qualifier;
    bool filter_by_prefix_qualifier;

    // Searches for a pattern in a block of memory using the Boyer- Moore-Horspool-Sunday algorithm
    static const uint8_t* memfind(
      const uint8_t* block,        // Block containing data
      size_t         block_size,   // Size of block in bytes
      const uint8_t* pattern,      // Pattern to search for
      size_t         pattern_size, // Size of pattern block
      size_t*        shift,        // Shift table (search buffer)
      bool*          repeat_find); // true, search buffer already initialized
  };

  /**
   * Scan context information
   */
  class ScanContext : public ReferenceCount {
  public:
    SchemaPtr schema;
    const ScanSpec *spec;
    ScanSpecBuilder scan_spec_builder;
    const RangeSpec *range;
    RangeSpecManaged range_managed;
    DynamicBuffer dbuf;
    SerializedKey start_serkey, end_serkey;
    Key start_key, end_key;
    String start_row, end_row;
    String start_qualifier, end_qualifier;
    bool start_inclusive, end_inclusive;
    bool single_row;
    bool has_cell_interval;
    bool has_start_cf_qualifier;
    bool restricted_range;
    int64_t revision;
    pair<int64_t, int64_t> time_interval;
    bool family_mask[256];
    vector<CellFilterInfo> family_info;
    RE2 *row_regexp;
    RE2 *value_regexp;
    typedef std::set<const char *, LtCstr, CstrAlloc> CstrRowSet;
    CstrRowSet rowset;
    uint32_t timeout_ms;

    /**
     * Constructor.
     *
     * @param rev scan revision
     * @param ss scan specification
     * @param range range specifier
     * @param schema smart pointer to schema object
     */
    ScanContext(int64_t rev, const ScanSpec *ss, const RangeSpec *range,
                SchemaPtr &schema) : family_info(256), row_regexp(0),
                                     value_regexp(0), timeout_ms(0) {
      initialize(rev, ss, range, schema);
    }

    /**
     * Constructor.
     *
     * @param rev scan revision
     * @param schema smart pointer to schema object
     */
    ScanContext(int64_t rev, SchemaPtr &schema)
      : family_info(256), row_regexp(0), value_regexp(0), timeout_ms(0) {
      initialize(rev, 0, 0, schema);
    }

    /**
     * Constructor.  Calls initialize() with an empty schema pointer.
     *
     * @param rev scan revision
     */
    ScanContext(int64_t rev=TIMESTAMP_MAX) 
      : family_info(256), row_regexp(0), value_regexp(0), timeout_ms(0) {
      SchemaPtr schema;
      initialize(rev, 0, 0, schema);
    }

    /**
     * Constructor.
     *
     * @param schema smart pointer to schema object
     */
    ScanContext(SchemaPtr &schema) 
      : family_info(256), row_regexp(0), value_regexp(0), timeout_ms(0) {
      initialize(TIMESTAMP_MAX, 0, 0, schema);
    }

    ~ScanContext() {
      if (row_regexp != 0) {
        delete row_regexp;
      }
      if (value_regexp != 0) {
        delete value_regexp;
      }
    }

    void deep_copy_specs() {
      scan_spec_builder = *spec;
      spec = &scan_spec_builder.get();
      range_managed = *range;
      range = &range_managed;
    }

  private:

    /**
     * Initializes the scan context.  Sets up the family_mask filter that
     * allows for quick lookups to see if a family is included in the scan.
     * Also sets up family_info entries for the column families that are
     * included in the scan which contains cell garbage collection info for
     * each family (e.g. cutoff timestamp and number of copies to keep).  Also
     * sets up end_row to be the last possible key in spec->end_row.
     *
     * @param rev scan revision
     * @param ss scan specification
     * @param range range specifier
     * @param sp shared pointer to schema object
     */
    void initialize(int64_t rev, const ScanSpec *ss, const RangeSpec *range,
                    SchemaPtr &sp);
    /**
     * Disable copy ctor and assignment op
     */
    ScanContext(const ScanContext&);
    ScanContext& operator = (const ScanContext&);

    CharArena arena;
  };

  typedef intrusive_ptr<ScanContext> ScanContextPtr;

} // namespace Hypertable

#endif // HYPERTABLE_SCANCONTEXT_H
