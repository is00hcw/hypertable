#
# Copyright (C) 2007-2013 Hypertable, Inc.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 3
# of the License, or any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.
#

include_directories(${Qfs_INCLUDE_DIR})

# qfsBroker
add_executable(qfsBroker main.cc QfsBroker.cc)
target_link_libraries(qfsBroker ${Qfs_LIBRARIES} HyperDfsBroker
                      ${MALLOC_LIBRARY} -lpthread)

install(TARGETS qfsBroker RUNTIME DESTINATION bin)
