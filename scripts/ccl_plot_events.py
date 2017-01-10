#!/usr/bin/env python
#
# This file is part of cf4ocl (C Framework for OpenCL).
#
# cf4ocl is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# cf4ocl is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with cf4ocl.  If not, see <http://www.gnu.org/licenses/>.
#

## @file
# Plots a Gantt-like chart of OpenCL events using the profiling info
# exported using the @ref CCL_PROFILER "profiler module".
#
# @author Nuno Fachada
# @date 2016
# @copyright [GNU General Public License version 3 (GPLv3)](http://www.gnu.org/licenses/gpl.html)

##
# @page ccl_plot_events
#
# @brief Plots a Gantt-like chart of OpenCL events using the profiling info
# exported using the @ref CCL_PROFILER "profiler module".
#
# SYNOPSIS
# ========
#
# **ccl_plot_events.py** *FILE.TSV*
#
# DESCRIPTION
# ===========
#
# The `ccl_plot_events` script accepts a single parameter indicating
# a file containing profiling info exported using the
# @ref CCL_PROFILER "profiler module".
#
# This script requires a [Python](https://www.python.org/) installation,
# and depends on the [Matplotlib](http://matplotlib.org/) and
# [NumPy](http://www.numpy.org/) libraries.
#
# AUTHOR
# ======
#
# Written by Nuno Fachada.
#
# REPORTING BUGS
# ==============
#
# * Report ccl_plot_events.py bugs at https://github.com/fakenmc/cf4ocl/issues
# * cf4ocl home page: http://fakenmc.github.io/cf4ocl/
# * cf4ocl wiki: https://github.com/fakenmc/cf4ocl/wiki
#
# COPYRIGHT
# =========
#
# Copyright (C) 2016 Nuno Fachada<br/>
# License GPLv3+: GNU GPL version 3 or later
# <http://gnu.org/licenses/gpl.html>.<br/>
# This is free software: you are free to change and redistribute it.<br/>
# There is NO WARRANTY, to the extent permitted by law.
#

import sys
import os.path
import numpy
import pylab
import matplotlib.pyplot as plt
import matplotlib.patches as patches

# Check command-line arguments
if len(sys.argv) < 2:
	print 'Usage: ' + sys.argv[0] + ' <file.tsv>\n'
	exit(-1)
elif sys.argv[1] == '--version':
	print 'ccl_plot_events.py v2.0.0\n'
	exit(0)
elif not os.path.isfile(sys.argv[1]):
	print "File not found: '" + sys.argv[1] + "'\n"
	exit(-2)


# Load profiling info in file given as first cli argument
pdat = numpy.genfromtxt(sys.argv[1], delimiter='\t', dtype=None, names=('queue','t_start','t_end','event'))

# Get queues
queues = numpy.unique(pdat['queue']).tolist()
# Determine number of queues
num_queues = len(queues)

# Create matplotlib figure
plt.figure()
# Set relative dimensions of plotting area
ax = plt.axes([0.1, 0.1, 0.71, 0.8])
# Set a vertical grid
plt.grid(True, axis='x')
# Set axis absolute dimensions of plotting area (depends on profiling info)
plt.axis([min(pdat['t_start']), max(pdat['t_end']), -0.5, num_queues - 0.5])
# Set ticks to queue names
plt.yticks(range(num_queues), queues, size='small')

# Determine event names
uniq_evts = numpy.unique(pdat['event']).tolist()
# Determine number of event names
num_uniq_evts = len(uniq_evts)
# Associate a different color with each event name
cmap = pylab.cm.get_cmap('spectral')
uniq_colors = [x*cmap.N/num_uniq_evts for x in range(num_uniq_evts)]

# Create legend handles (one for each event name)
handles = []
for uniq_evt in uniq_evts:
    # Determine index of current event name
    uei = uniq_evts.index(uniq_evt)
    # Get color associated with current event name
    color = cmap(uniq_colors[uei])
    # Create a color patch for the legend of the current event name
    ptch = patches.Patch(edgecolor='black', facecolor=color, linestyle='solid', fill=True)
    # Add patch to legend handles
    handles.append(ptch)

# Plot events
for event in pdat:
    # Determine plotting locations for current event
    x = event['t_start']
    y = queues.index(event['queue']) - 0.4
    width = event['t_end'] - event['t_start']
    # Determine index of event name of which current event is an instance of
    uei = uniq_evts.index(event['event'])
    # Determine the color for current event (which is the same for the
    # associated event name)
    color = cmap(uniq_colors[uei])
    # Plot event
    rect = patches.Rectangle((x, y), width, 0.8, edgecolor='none', facecolor=color, fill=True)
    plt.gca().add_patch(rect)

# Add legend
plt.legend(handles, uniq_evts, borderaxespad=0, bbox_to_anchor=(1.02, 1), loc=2, prop={'size':'x-small'})

# Label axes
plt.ylabel('Queues')
plt.xlabel('Time')

# Show figure
plt.show()





