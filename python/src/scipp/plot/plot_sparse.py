# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2019 Scipp contributors (https://github.com/scipp)
# @author Neil Vaytet

# Scipp imports
from . import config
from .tools import edges_to_centers, render_plot, get_1d_axes
from .._scipp import core as sc

# Other imports
import numpy as np
import plotly.graph_objs as go
from itertools import product


def flatten_sparse_data():
    x = []
    y = []
    for i in range(d['a'].shape[0]):
        yy = d['a'][Dim.X, i].coords[Dim.Tof].values
        y.append(np.ones_like(yy) * d['a'].coords[Dim.X].values[i])
        x.append(np.array(yy))
    x = np.concatenate(x)
    y = np.concatenate(y)




def histogram_sparse_data(input_data, sparse_dim, bins):
    if isinstance(bins, bool):
        bins = 256
    if isinstance(bins, int):
        # Find min and max
        xmin =  1.0e30
        xmax = -1.0e30
        ndims = len(input_data.dims)
        vslice = input_data
        dims = input_data.dims
        shapes = input_data.shape
        # Construct tuple of ranges over dimension shapes
        indices = tuple()
        for i in range(ndims - 1):
            indices += range(shapes[i]),
        # Now construct all indices combinations using itertools
        for ind in product(*indices):
            # And for each indices combination, slice the original
            # data down to the sparse dimension
            vslice = input_data
            for i in range(ndims - 1):
                vslice = vslice[dims[i], ind[i]]
            # We should now be left with the bare sparse array for
            # this particular pixel
            vals = vslice.coords[sparse_dim].values
            if len(vals) > 0:
                xmin = min(xmin, np.nanmin(vals))
                xmax = max(xmax, np.nanmax(vals))
        dx = (xmax - xmin) / float(bins)
        # Add padding
        xmin -= 0.5 * dx
        xmax += 0.5 * dx
        bins = sc.Variable([sparse_dim], values=np.linspace(xmin, xmax, bins + 1),
                           unit=input_data.coords[sparse_dim].unit)
    elif isinstance(bins, np.ndarray):
        bins = sc.Variable([sparse_dim], values=bins,
                           unit=input_data.coords[sparse_dim].unit)
    elif isinstance(bins, sc.Variable):
        pass

    return sc.histogram(input_data, bins)


def plot_sparse(input_data, backend=None, logx=False, logy=False, logxy=False,
                   color=None, filename=None, axes=None):
    """
    Plot a 1D spectrum.

    Input is a dictionary containing a list of DataProxy.
    If the coordinate of the x-axis contains bin edges, then a bar plot is
    made.

    TODO: find a more general way of handling arguments to be sent to plotly,
    probably via a dictionay of arguments
    """

    data = []
    for i, (name, var) in enumerate(input_data.items()):

        xlab, ylab, x, y = get_1d_axes(var, axes, name)

        nx = x.shape[0]
        ny = y.shape[0]
        histogram = False
        if nx == ny + 1:
            histogram = True

        # Define trace
        trace = dict(x=x, y=y, name=name, type="scattergl")
        if histogram:
            trace["line"] = {"shape": "hv"}
            trace["y"] = np.concatenate((trace["y"], [0.0]))
            trace["fill"] = "tozeroy"
            trace["mode"] = "lines"
        if color is not None:
            trace["marker"] = {"color": color[i]}
        # Include variance if present
        if var.variances is not None:
            err_dict = dict(
                    type="data",
                    array=np.sqrt(var.variances),
                    visible=True,
                    color=color[i])
            if histogram:
                trace2 = dict(x=edges_to_centers(x), y=y, showlegend=False,
                              type="scattergl", mode="markers",
                              error_y=err_dict,
                              marker={"color": color[i]})
                data.append(trace2)
            else:
                trace["error_y"] = err_dict

        data.append(trace)

    layout = dict(
        xaxis=dict(title=xlab),
        yaxis=dict(title=ylab),
        showlegend=True,
        legend=dict(x=0.0, y=1.15, orientation="h"),
        height=config.height
    )
    if histogram:
        layout["barmode"] = "overlay"
    if logx or logxy:
        layout["xaxis"]["type"] = "log"
    if logy or logxy:
        layout["yaxis"]["type"] = "log"

    fig = go.Figure(data=data, layout=layout)
    render_plot(static_fig=fig, interactive_fig=fig, backend=backend,
                filename=filename)
    return

