# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2019 Scipp contributors (https://github.com/scipp)
# @file
# @author Neil Vaytet


# The plotting backend: possible choices are "interactive", "static" and
# "matplotlib"
backend = "interactive"

# The list of default line colors
color_list = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd',
              '#8c564b', '#e377c2', '#7f7f7f', '#bcbd22', '#17becf']

# The colorbar properties
cb = {"name": "viridis", "log": False, "min": None, "max": None,
      "min_var": None, "max_var": None}

# The default image height (in pixels)
height = 600

# The default image width (in pixels)
width = 950

# The size threshold above which an image will automatically be rasterized
rasterize_threshold = 100000
