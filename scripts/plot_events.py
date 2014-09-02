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
# exported using the @ref PROFILER "profiler module".
#
# Usage: ccl_plot_events.py file.tsv

import sys
import numpy
import pylab
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import matplotlib.ticker as ticker

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
    # Determine the color for current event (which is the same for the associated event name)
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





