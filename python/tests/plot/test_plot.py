# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (c) 2020 Scipp contributors (https://github.com/scipp)
# @file
# @author Neil Vaytet

from itertools import product

import numpy as np
import pytest

import scipp as sc
from scipp.plot import plot

# TODO: For now we are just checking that the plot does not throw any errors.
# In the future it would be nice to check the output by either comparing
# checksums or by using tools like squish.


def make_dense_dataset(ndim=1,
                       variances=False,
                       binedges=False,
                       labels=False,
                       masks=False):

    dim_list = ['tof', 'x', 'y', 'z', 'Q_x']

    N = 50
    M = 10

    d = sc.Dataset()
    shapes = []
    dims = []
    for i in range(ndim):
        n = N - (i * M)
        d.coords[dim_list[i]] = sc.Variable(
            dims=[dim_list[i]],
            values=np.arange(n + binedges).astype(np.float64))
        dims.append(dim_list[i])
        shapes.append(n)
    a = np.sin(np.arange(np.prod(shapes)).reshape(*shapes).astype(np.float64))
    d["Sample"] = sc.Variable(dims, values=a, unit=sc.units.counts)
    if variances:
        d["Sample"].variances = np.abs(np.random.normal(a * 0.1, 0.05))
    if labels:
        d.coords["somelabels"] = sc.Variable([dim_list[0]],
                                             values=np.linspace(
                                                 101., 105., shapes[0]),
                                             unit=sc.units.s)
    if masks:
        d.masks["mask"] = sc.Variable(dims,
                                      values=np.where(a > 0, True, False))
    return d


def make_sparse_dataset(ndim=1):

    dim_list = ['x', 'y', 'z', 'Q_x']

    N = 50
    M = 10

    dims = []
    shapes = []
    for i in range(ndim):
        n = N - (i * M)
        dims.append(dim_list[i])
        shapes.append(n)

    var = sc.Variable(dims=dims,
                      shape=shapes,
                      unit=sc.units.us,
                      dtype=sc.dtype.event_list_float64)
    dat = sc.Variable(dims=dims,
                      unit=sc.units.counts,
                      values=np.ones(shapes),
                      variances=np.ones(shapes))

    indices = tuple()
    for i in range(ndim):
        indices += range(shapes[i]),
    # Now construct all indices combinations using itertools
    for ind in product(*indices):
        # And for each indices combination, slice the original data
        vslice = var
        for i in range(ndim):
            vslice = vslice[dims[i], ind[i]]
        v = np.random.normal(float(N),
                             scale=2.0 * M,
                             size=int(np.random.rand() * N))
        vslice.values = v

    d = sc.Dataset()
    for i in range(ndim):
        d.coords[dim_list[i]] = sc.Variable([dim_list[i]],
                                            values=np.arange(N - (i * M),
                                                             dtype=np.float),
                                            unit=sc.units.m)
    d["a"] = sc.DataArray(data=dat, coords={'tof': var})
    return d


def test_plot_1d():
    d = make_dense_dataset(ndim=1)
    plot(d)


def test_plot_1d_with_variances():
    d = make_dense_dataset(ndim=1, variances=True)
    plot(d)


def test_plot_1d_bin_edges():
    d = make_dense_dataset(ndim=1, binedges=True)
    plot(d)


def test_plot_1d_with_labels():
    d = make_dense_dataset(ndim=1, labels=True)
    plot(d, axes=["somelabels"])


def test_plot_1d_log_axes():
    d = make_dense_dataset(ndim=1)
    plot(d, logx=True)
    plot(d, logy=True)
    plot(d, logxy=True)


def test_plot_1d_bin_edges_with_variances():
    d = make_dense_dataset(ndim=1, variances=True, binedges=True)
    plot(d)


def test_plot_1d_two_separate_entries():
    d = make_dense_dataset(ndim=1)
    d["Background"] = sc.Variable(['tof'],
                                  values=2.0 * np.random.rand(50),
                                  unit=sc.units.kg)
    plot(d)


def test_plot_1d_two_entries_on_same_plot():
    d = make_dense_dataset(ndim=1)
    d["Background"] = sc.Variable(['tof'],
                                  values=2.0 * np.random.rand(50),
                                  unit=sc.units.counts)
    plot(d)


def test_plot_1d_two_entries_hide_variances():
    d = make_dense_dataset(ndim=1, variances=True)
    d["Background"] = sc.Variable(['tof'],
                                  values=2.0 * np.random.rand(50),
                                  unit=sc.units.counts)
    plot(d, variances=False)
    # When variances are not present, the plot does not fail, is silently does
    # not show variances
    print(d)
    plot(d, variances={"Sample": False, "Background": True})


def test_plot_1d_three_entries_with_labels():
    N = 50
    d = make_dense_dataset(ndim=1, labels=True)
    d["Background"] = sc.Variable(['tof'],
                                  values=2.0 * np.random.rand(N),
                                  unit=sc.units.counts)
    d.coords['x'] = sc.Variable(['x'],
                                values=np.arange(N).astype(np.float64),
                                unit=sc.units.m)
    d["Sample2"] = sc.Variable(['x'],
                               values=10.0 * np.random.rand(N),
                               unit=sc.units.counts)
    d.coords["Xlabels"] = sc.Variable(['x'],
                                      values=np.linspace(151., 155., N),
                                      unit=sc.units.s)
    plot(d, axes={'x': "Xlabels", 'tof': "somelabels"})


def test_plot_1d_with_masks():
    d = make_dense_dataset(ndim=1, masks=True)
    plot(d)


def test_plot_2d_image():
    d = make_dense_dataset(ndim=2)
    plot(d)


def test_plot_2d_image_with_axes():
    d = make_dense_dataset(ndim=2)
    plot(d, axes=['tof', 'x'])


def test_plot_2d_image_with_labels():
    d = make_dense_dataset(ndim=2, labels=True)
    plot(d, axes=['x', "somelabels"])


def test_plot_2d_image_with_variances():
    d = make_dense_dataset(ndim=2, variances=True)
    plot(d, variances=True)


def test_plot_2d_image_with_filename():
    d = make_dense_dataset(ndim=2)
    plot(d, filename="image.pdf")


def test_plot_2d_image_with_variances_with_filename():
    d = make_dense_dataset(ndim=2, variances=True)
    plot(d, variances=True, filename="val_and_var.pdf")


def test_plot_2d_image_with_bin_edges():
    d = make_dense_dataset(ndim=2, binedges=True)
    plot(d)


def test_plot_2d_with_masks():
    d = make_dense_dataset(ndim=2, masks=True)
    plot(d)


def test_plot_collapse():
    d = make_dense_dataset(ndim=2)
    plot(d, collapse='tof')


def test_plot_sliceviewer():
    d = make_dense_dataset(ndim=3)
    plot(d)


def test_plot_sliceviewer_with_variances():
    d = make_dense_dataset(ndim=3, variances=True)
    plot(d, variances=True)


def test_plot_sliceviewer_with_two_sliders():
    d = make_dense_dataset(ndim=4)
    plot(d)


def test_plot_sliceviewer_with_axes():
    d = make_dense_dataset(ndim=3)
    plot(d, axes=['x', 'tof', 'y'])


def test_plot_sliceviewer_with_labels():
    d = make_dense_dataset(ndim=3, labels=True)
    plot(d, axes=['x', 'y', "somelabels"])


def test_plot_sliceviewer_with_3d_projection():
    d = make_dense_dataset(ndim=3)
    plot(d, projection="3d")


def test_plot_sliceviewer_with_3d_projection_with_variances():
    d = make_dense_dataset(ndim=3, variances=True)
    plot(d, projection="3d", variances=True)


@pytest.mark.skip(reason="3D plotting with labels is currently broken after"
                  "dims API refactor.")
def test_plot_sliceviewer_with_3d_projection_with_labels():
    d = make_dense_dataset(ndim=3, labels=True)
    plot(d, projection="3d", axes=['x', 'y', "somelabels"])


def test_plot_3d_with_filename():
    d = make_dense_dataset(ndim=3)
    plot(d, projection="3d", filename="a3dplot.html")


def test_plot_sliceviewer_with_1d_projection():
    d = make_dense_dataset(ndim=3)
    plot(d, projection="1d")


@pytest.mark.skip(reason="Volume rendering is not yet supported.")
def test_plot_volume():
    d = make_dense_dataset(ndim=3)
    plot(d, projection="volume")


def test_plot_convenience_methods():
    d = make_dense_dataset(ndim=3)
    sc.plot.image(d)
    sc.plot.threeslice(d)
    # sc.plot.volume(d)
    sc.plot.superplot(d)


def test_plot_1d_sparse_data_with_bool_bins():
    d = make_sparse_dataset(ndim=1)
    plot(d, bins={'tof': True})


def test_plot_1d_sparse_data_with_int_bins():
    d = make_sparse_dataset(ndim=1)
    plot(d, bins={'tof': 50})


def test_plot_1d_sparse_data_with_nparray_bins():
    d = make_sparse_dataset(ndim=1)
    plot(d, bins={'tof': np.linspace(0.0, 105.0, 50)})


def test_plot_1d_sparse_data_with_Variable_bins():
    d = make_sparse_dataset(ndim=1)
    bins = sc.Variable(['tof'],
                       values=np.linspace(0.0, 105.0, 50),
                       unit=sc.units.us)
    plot(d, bins={'tof': bins})


@pytest.mark.skip(reason="This is currently broken with error "
                  "`VariableConstView Coord/Label has more than one "
                  "dimension associated with tof and will not be "
                  "reduced by the operation dimension tof Terminating "
                  "operation.` when trying to histogram the data "
                  "on-the-fly.")
def test_plot_2d_sparse_data_with_int_bins():
    d = make_sparse_dataset(ndim=2)
    plot(d, bins=50)


@pytest.mark.skip(reason="This is currently broken with error "
                  "`VariableConstView Coord/Label has more than one "
                  "dimension associated with tof and will not be "
                  "reduced by the operation dimension tof Terminating "
                  "operation.` when trying to histogram the data "
                  "on-the-fly.")
def test_plot_2d_sparse_data_with_nparray_bins():
    d = make_sparse_dataset(ndim=2)
    plot(d, bins=np.linspace(0.0, 105.0, 50))


@pytest.mark.skip(reason="This is currently broken with error "
                  "`VariableConstView Coord/Label has more than one "
                  "dimension associated with tof and will not be "
                  "reduced by the operation dimension tof Terminating "
                  "operation.` when trying to histogram the data "
                  "on-the-fly.")
def test_plot_2d_sparse_data_with_Variable_bins():
    d = make_sparse_dataset(ndim=2)
    bins = sc.Variable(['tof'],
                       values=np.linspace(0.0, 105.0, 50),
                       unit=sc.units.us)
    plot(d, bins=bins)


@pytest.mark.skip(reason="RuntimeError: Only the simple case histograms may "
                  "be constructed for now: 2 dims including sparse.")
def test_plot_3d_sparse_data_with_int_bins():
    d = make_sparse_dataset(ndim=3)
    plot(d, bins=50)


@pytest.mark.skip(reason="RuntimeError: Only the simple case histograms may "
                  "be constructed for now: 2 dims including sparse.")
def test_plot_3d_sparse_data_with_nparray_bins():
    d = make_sparse_dataset(ndim=3)
    plot(d, bins=np.linspace(0.0, 105.0, 50))


@pytest.mark.skip(reason="RuntimeError: Only the simple case histograms may "
                  "be constructed for now: 2 dims including sparse.")
def test_plot_3d_sparse_data_with_Variable_bins():
    d = make_sparse_dataset(ndim=3)
    bins = sc.Variable(['tof'],
                       values=np.linspace(0.0, 105.0, 50),
                       unit=sc.units.us)
    plot(d, bins=bins)


def test_plot_variable():
    N = 50
    v1d = sc.Variable(['tof'], values=np.random.rand(N), unit=sc.units.counts)
    v2d = sc.Variable(['tof', 'x'],
                      values=np.random.rand(N, N),
                      unit=sc.units.K)
    v3d = sc.Variable(['tof', 'x', 'y'],
                      values=np.random.rand(N, N, N),
                      unit=sc.units.m)
    plot(v1d)
    plot(v2d)
    plot(v3d)


def test_plot_dataset_view():
    d = make_dense_dataset(ndim=2)
    plot(d['x', 0])


def test_plot_data_array():
    d = make_dense_dataset(ndim=1)
    plot(d["Sample"])


def test_plot_vector_axis_labels_1d():
    d = sc.Dataset()
    N = 10
    vecs = []
    for i in range(N):
        vecs.append(np.random.random(3))
    d.coords['x'] = sc.Variable(['x'],
                                values=vecs,
                                unit=sc.units.m,
                                dtype=sc.dtype.vector_3_float64)
    d["Sample"] = sc.Variable(['x'],
                              values=np.random.rand(N),
                              unit=sc.units.counts)
    plot(d)


def test_plot_string_axis_labels_1d():
    d = sc.Dataset()
    N = 10
    d.coords['x'] = sc.Variable(
        dims=['x'],
        values=["a", "b", "c", "d", "e", "f", "g", "h", "i", "j"],
        unit=sc.units.m)
    d["Sample"] = sc.Variable(['x'],
                              values=np.random.rand(N),
                              unit=sc.units.counts)
    plot(d)


def test_plot_string_axis_labels_1d_short():
    d = sc.Dataset()
    N = 5
    d.coords['x'] = sc.Variable(dims=['x'],
                                values=["a", "b", "c", "d", "e"],
                                unit=sc.units.m)
    d["Sample"] = sc.Variable(['x'],
                              values=np.random.rand(N),
                              unit=sc.units.counts)
    plot(d)


def test_plot_string_and_vector_axis_labels_2d():
    N = 10
    M = 5
    vecs = []
    for i in range(N):
        vecs.append(np.random.random(3))
    d = sc.Dataset()
    d.coords['x'] = sc.Variable(['x'],
                                values=vecs,
                                unit=sc.units.m,
                                dtype=sc.dtype.vector_3_float64)
    d.coords['y'] = sc.Variable(['y'],
                                values=["a", "b", "c", "d", "e"],
                                unit=sc.units.m)
    d["Signal"] = sc.Variable(['y', 'x'],
                              values=np.random.random([M, N]),
                              unit=sc.units.counts)
    plot(d)


def test_plot_2d_with_dimension_of_size_1():
    N = 10
    M = 1
    x = np.arange(N, dtype=np.float64)
    y = np.arange(M, dtype=np.float64)
    z = np.arange(M + 1, dtype=np.float64)
    d = sc.Dataset()
    d.coords['x'] = sc.Variable(['x'], values=x, unit=sc.units.m)
    d.coords['y'] = sc.Variable(['y'], values=y, unit=sc.units.m)
    d.coords['z'] = sc.Variable(['z'], values=z, unit=sc.units.m)
    d["a"] = sc.Variable(['y', 'x'],
                         values=np.random.random([M, N]),
                         unit=sc.units.counts)
    d["b"] = sc.Variable(['z', 'x'],
                         values=np.random.random([M, N]),
                         unit=sc.units.counts)
    plot(d["a"])
    plot(d["b"])


@pytest.mark.skip(reason="Requires aligned coords to insert two sparse data "
                  "entries.")
def test_sparse_data_slice_with_on_the_fly_histogram():
    N = 50
    M = 10
    var = sc.Variable(dims=['x'],
                      shape=[M],
                      dtype=sc.dtype.event_list_float64,
                      unit=sc.units.us)
    dat = sc.Variable(dims=['x'],
                      unit=sc.units.counts,
                      values=np.ones(M),
                      variances=np.ones(M))
    for i in range(M):
        v = np.random.normal(50.0, scale=20.0, size=int(np.random.rand() * N))
        var['x', i].values = v

    d = sc.Dataset()
    d.coords['x'] = sc.Variable(['x'], values=np.arange(M), unit=sc.units.m)
    d['a'] = sc.DataArray(data=dat, coords={'tof': var})
    d['b'] = sc.DataArray(data=dat * 1.1, coords={'tof': var * 1.1})
    plot(d['x', 4], bins=100)
