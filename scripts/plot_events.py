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
"""
Plots a Gantt-like chart of OpenCL events using the profilling info
exported by CFProfiler.
"""

import sys
import numpy
import pylab
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import matplotlib.ticker as ticker

# Load profiling info in file given as first cli argument
pdat = numpy.genfromtxt(sys.argv[1], delimiter='\t', dtype=None, names=('queue','t_start','t_end','event'))

# Create matplotlib figure
plt.figure()
# Set relative dimensions of plotting area
ax = plt.axes([0.1, 0.1, 0.71, 0.8])
# Set Y axis (queues) as integers
ax.get_yaxis().set_major_locator(ticker.MaxNLocator(integer=True))
# Set a vertical grid
plt.grid(True, axis='x')
# Set axis absolute dimensions of plotting area (depends on profiling info)
plt.axis([min(pdat['t_start']), max(pdat['t_end']), min(pdat['queue'])-0.5, max(pdat['queue'])+0.5])

# Determine unique semantic events
uniq_evts = numpy.unique(pdat['event']).tolist()
# Determine number of unique semantic events
num_uniq_evts = len(uniq_evts)
# Associate a different color with each unique semantic event
cmap = pylab.cm.get_cmap('spectral')
uniq_colors = [x*cmap.N/num_uniq_evts for x in range(num_uniq_evts)]

# Create legend handles (one for each unique semantic event)
handles = []
for uniq_evt in uniq_evts:
    # Determine index of current unique semantic event
    uei = uniq_evts.index(uniq_evt)
    # Get color associated with current unique semantic event
    color = cmap(uniq_colors[uei])
    # Create a color patch for the legend of the current unique semantic event
    ptch = patches.Patch(edgecolor='black', facecolor=color, linestyle='solid', fill=True)
    # Add patch to legend handles
    handles.append(ptch)

# Plot events 
for event in pdat:
    # Determine plotting locations for current event
    x = event['t_start']
    y = event['queue'] - 0.4
    width = event['t_end'] - event['t_start']
    # Determine index of unique semantic event of which current event is an instance of
    uei = uniq_evts.index(event['event'])
    # Determine the color for current event (which is the same for the associated unique semantic event)
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





