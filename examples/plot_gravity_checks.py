#!/usr/bin/env python

import sys
import glob
import re
import numpy as np
import matplotlib.pyplot as plt

params = {'axes.labelsize': 14,
'axes.titlesize': 18,
'font.size': 12,
'legend.fontsize': 12,
'xtick.labelsize': 14,
'ytick.labelsize': 14,
'text.usetex': True,
'figure.figsize': (10, 10),
'figure.subplot.left'    : 0.06,
'figure.subplot.right'   : 0.99  ,
'figure.subplot.bottom'  : 0.06  ,
'figure.subplot.top'     : 0.985  ,
'figure.subplot.wspace'  : 0.14  ,
'figure.subplot.hspace'  : 0.14  ,
'lines.markersize' : 6,
'lines.linewidth' : 3.,
'text.latex.unicode': True
}
plt.rcParams.update(params)
plt.rc('font',**{'family':'sans-serif','sans-serif':['Times']})

min_error = 1e-6
max_error = 1e-1
num_bins = 51

# Construct the bins
bin_edges = np.linspace(np.log10(min_error), np.log10(max_error), num_bins + 1)
bin_size = (np.log10(max_error) - np.log10(min_error)) / num_bins
bins = 0.5*(bin_edges[1:] + bin_edges[:-1])
bin_edges = 10**bin_edges
bins = 10**bins

# Colours
cols = ['b', 'g', 'r', 'm']

# Time-step to plot
step = int(sys.argv[1])

# Find the files for the different expansion orders
order_list = glob.glob("gravity_checks_swift_step%d_order*.dat"%step)
num_order = len(order_list)

# Get the multipole orders
order = np.zeros(num_order)
for i in range(num_order):
    order[i] = int(order_list[i][32])

# Read the exact accelerations first
data = np.loadtxt('gravity_checks_exact_step%d.dat'%step)
exact_ids = data[:,0]
exact_pos = data[:,1:4]
exact_a = data[:,4:7]
# Sort stuff
sort_index = np.argsort(exact_ids)
exact_ids = exact_ids[sort_index]
exact_pos = exact_pos[sort_index, :]
exact_a = exact_a[sort_index, :]        
exact_a_norm = np.sqrt(exact_a[:,0]**2 + exact_a[:,1]**2 + exact_a[:,2]**2)
    
# Start the plot
plt.figure()

# Get the Gadget-2 data if existing
gadget2_file_list = glob.glob("forcetest_gadget2.txt")
if len(gadget2_file_list) != 0:

    gadget2_data = np.loadtxt(gadget2_file_list[0])
    gadget2_ids = gadget2_data[:,0]
    gadget2_pos = gadget2_data[:,1:4]
    gadget2_a_exact = gadget2_data[:,4:7]
    gadget2_a_grav = gadget2_data[:, 7:10]

    # Sort stuff
    sort_index = np.argsort(gadget2_ids)
    gadget2_ids = gadget2_ids[sort_index]
    gadget2_pos = gadget2_pos[sort_index, :]
    gadget2_a_exact = gadget2_a_exact[sort_index, :]
    gadget2_a_grav = gadget2_a_grav[sort_index, :]

    # Cross-checks
    if not np.array_equal(exact_ids, gadget2_ids):
        print "Comparing different IDs !"

    if np.max(np.abs(exact_pos - gadget2_pos)/np.abs(gadget2_pos)) > 1e-6:
        print "Comparing different positions ! max difference:"
        index = np.argmax(exact_pos[:,0]**2 + exact_pos[:,1]**2 + exact_pos[:,2]**2 - gadget2_pos[:,0]**2 - gadget2_pos[:,1]**2 - gadget2_pos[:,2]**2)
        print "Gadget2 (id=%d):"%gadget2_ids[index], gadget2_pos[index,:], "exact (id=%d):"%exact_ids[index], exact_pos[index,:], "\n"

    if np.max(np.abs(a_exact - gadget2_a_exact) / np.abs(gadget2_a_exact)) > 2e-6:
        print "Comparing different exact accelerations ! max difference:"
        index = np.argmax(exact_a[:,0]**2 + exact_a[:,1]**2 + exact_a[:,2]**2 - gadget2_a_exact[:,0]**2 - gadget2_a_exact[:,1]**2 - gadget2_a_exact[:,2]**2)
        print "a_exact --- Gadget2:", gadget2_a_exact[index,:], "exact:", a_exact[index,:]
        print "a_grav ---  Gadget2:", gadget2_a_grav[index,:], "exact:", a_grav[index,:],"\n"
        print "pos ---     Gadget2: (id=%d):"%gadget2_ids[index], gadget2_pos[index,:], "exact (id=%d):"%ids[index], pos[index,:],"\n"

    
    # Compute the error norm
    diff = gadget2_a_exact - gadget2_a_grav

    norm_diff = np.sqrt(diff[:,0]**2 + diff[:,1]**2 + diff[:,2]**2)
    norm_a = np.sqrt(gadget2_a_exact[:,0]**2 + gadget2_a_exact[:,1]**2 + gadget2_a_exact[:,2]**2)

    norm_error = norm_diff / norm_a
    error_x = abs(diff[:,0]) / norm_a
    error_y = abs(diff[:,1]) / norm_a
    error_z = abs(diff[:,2]) / norm_a
    
    # Bin the error
    norm_error_hist,_ = np.histogram(norm_error, bins=bin_edges, density=False) / (np.size(norm_error) * bin_size)
    error_x_hist,_ = np.histogram(error_x, bins=bin_edges, density=False) / (np.size(norm_error) * bin_size)
    error_y_hist,_ = np.histogram(error_y, bins=bin_edges, density=False) / (np.size(norm_error) * bin_size)
    error_z_hist,_ = np.histogram(error_z, bins=bin_edges, density=False) / (np.size(norm_error) * bin_size)
    
    norm_median = np.median(norm_error)
    median_x = np.median(error_x)
    median_y = np.median(error_y)
    median_z = np.median(error_z)

    norm_per95 = np.percentile(norm_error,95)
    per95_x = np.percentile(error_x,95)
    per95_y = np.percentile(error_y,95)
    per95_z = np.percentile(error_z,95)

    plt.subplot(221)    
    plt.semilogx(bins, norm_error_hist, 'k--', label="Gadget-2")
    plt.plot([norm_median, norm_median], [2.7, 3], 'k-', lw=1)
    plt.plot([norm_per95, norm_per95], [2.7, 3], 'k:', lw=1)
    plt.subplot(222)
    plt.semilogx(bins, error_x_hist, 'k--', label="Gadget-2")
    plt.plot([median_x, median_x], [1.8, 2], 'k-', lw=1)
    plt.plot([per95_x, per95_x], [1.8, 2], 'k:', lw=1)
    plt.subplot(223)    
    plt.semilogx(bins, error_y_hist, 'k--', label="Gadget-2")
    plt.plot([median_y, median_y], [1.8, 2], 'k-', lw=1)
    plt.plot([per95_y, per95_y], [1.8, 2], 'k:', lw=1)
    plt.subplot(224)    
    plt.semilogx(bins, error_z_hist, 'k--', label="Gadget-2")
    plt.plot([median_z, median_z], [1.8, 2], 'k-', lw=1)
    plt.plot([per95_z, per95_z], [1.8, 2], 'k:', lw=1)


# Plot the different histograms
for i in range(num_order-1, -1, -1):
    data = np.loadtxt(order_list[i])
    ids = data[:,0]
    pos = data[:,1:4]
    a_grav = data[:, 4:7]

    # Sort stuff
    sort_index = np.argsort(ids)
    ids = ids[sort_index]
    pos = pos[sort_index, :]
    a_grav = a_grav[sort_index, :]        

    # Cross-checks
    if not np.array_equal(exact_ids, ids):
        print "Comparing different IDs !"

    if np.max(np.abs(exact_pos - pos)/np.abs(pos)) > 1e-6:
        print "Comparing different positions ! max difference:"
        index = np.argmax(exact_pos[:,0]**2 + exact_pos[:,1]**2 + exact_pos[:,2]**2 - pos[:,0]**2 - pos[:,1]**2 - pos[:,2]**2)
        print "SWIFT (id=%d):"%ids[index], pos[index,:], "exact (id=%d):"%exact_ids[index], exact_pos[index,:], "\n"

    
    # Compute the error norm
    diff = exact_a - a_grav

    norm_diff = np.sqrt(diff[:,0]**2 + diff[:,1]**2 + diff[:,2]**2)

    norm_error = norm_diff / exact_a_norm
    error_x = abs(diff[:,0]) / exact_a_norm
    error_y = abs(diff[:,1]) / exact_a_norm
    error_z = abs(diff[:,2]) / exact_a_norm
    
    # Bin the error
    norm_error_hist,_ = np.histogram(norm_error, bins=bin_edges, density=False) / (np.size(norm_error) * bin_size)
    error_x_hist,_ = np.histogram(error_x, bins=bin_edges, density=False) / (np.size(norm_error) * bin_size)
    error_y_hist,_ = np.histogram(error_y, bins=bin_edges, density=False) / (np.size(norm_error) * bin_size)
    error_z_hist,_ = np.histogram(error_z, bins=bin_edges, density=False) / (np.size(norm_error) * bin_size)

    norm_median = np.median(norm_error)
    median_x = np.median(error_x)
    median_y = np.median(error_y)
    median_z = np.median(error_z)

    norm_per95 = np.percentile(norm_error,95)
    per95_x = np.percentile(error_x,95)
    per95_y = np.percentile(error_y,95)
    per95_z = np.percentile(error_z,95)
    
    plt.subplot(221)
    plt.semilogx(bins, norm_error_hist, color=cols[i],label="SWIFT m-poles order %d"%order[i])
    plt.plot([norm_median, norm_median], [2.7, 3],'-', color=cols[i], lw=1)
    plt.plot([norm_per95, norm_per95], [2.7, 3],':', color=cols[i], lw=1)
    plt.subplot(222)    
    plt.semilogx(bins, error_x_hist, color=cols[i],label="SWIFT m-poles order %d"%order[i])
    plt.plot([median_x, median_x], [1.8, 2],'-', color=cols[i], lw=1)
    plt.plot([per95_x, per95_x], [1.8, 2],':', color=cols[i], lw=1)
    plt.subplot(223)    
    plt.semilogx(bins, error_y_hist, color=cols[i],label="SWIFT m-poles order %d"%order[i])
    plt.plot([median_y, median_y], [1.8, 2],'-', color=cols[i], lw=1)
    plt.plot([per95_y, per95_y], [1.8, 2],':', color=cols[i], lw=1)
    plt.subplot(224)    
    plt.semilogx(bins, error_z_hist, color=cols[i],label="SWIFT m-poles order %d"%order[i])
    plt.plot([median_z, median_z], [1.8, 2],'-', color=cols[i], lw=1)
    plt.plot([per95_z, per95_z], [1.8, 2],':', color=cols[i], lw=1)

    
plt.subplot(221)
plt.xlabel("$|\delta \overrightarrow{a}|/|\overrightarrow{a}_{exact}|$")
plt.ylabel("Density")
plt.xlim(min_error, 2*max_error)
plt.ylim(0,3)
plt.legend(loc="upper left")
plt.subplot(222)    
plt.xlabel("$\delta a_x/|\overrightarrow{a}_{exact}|$")
plt.ylabel("Density")
plt.xlim(min_error, 2*max_error)
plt.ylim(0,2)
#plt.legend(loc="center left")
plt.subplot(223)    
plt.xlabel("$\delta a_y/|\overrightarrow{a}_{exact}|$")
plt.ylabel("Density")
plt.xlim(min_error, 2*max_error)
plt.ylim(0,2)
#plt.legend(loc="center left")
plt.subplot(224)    
plt.xlabel("$\delta a_z/|\overrightarrow{a}_{exact}|$")
plt.ylabel("Density")
plt.xlim(min_error, 2*max_error)
plt.ylim(0,2)
#plt.legend(loc="center left")



plt.savefig("gravity_checks_step%d.png"%step)
