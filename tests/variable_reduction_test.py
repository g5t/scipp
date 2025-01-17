# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 Scipp contributors (https://github.com/scipp)
# @file
# @author Simon Heybrock
import scipp as sc
import numpy as np


def test_all():
    var = sc.array(dims=['x', 'y'], values=[[True, True], [True, False]])
    assert sc.identical(sc.all(var), sc.scalar(False))


def test_all_with_dim():
    var = sc.array(dims=['x', 'y'], values=[[True, True], [True, False]])
    assert sc.identical(sc.all(var, 'x'), sc.array(dims=['y'], values=[True, False]))
    assert sc.identical(sc.all(var, 'y'), sc.array(dims=['x'], values=[True, False]))


def test_any():
    var = sc.array(dims=['x', 'y'], values=[[True, True], [True, False]])
    assert sc.identical(sc.any(var), sc.scalar(True))


def test_any_with_dim():
    var = sc.array(dims=['x', 'y'], values=[[True, True], [False, False]])
    assert sc.identical(sc.any(var, 'x'), sc.array(dims=['y'], values=[True, True]))
    assert sc.identical(sc.any(var, 'y'), sc.array(dims=['x'], values=[True, False]))


def test_min():
    var = sc.array(dims=['x'], values=[1.0, 2.0, 3.0])
    assert sc.identical(sc.min(var, 'x'), sc.scalar(1.0))


def test_max():
    var = sc.array(dims=['x'], values=[1.0, 2.0, 3.0])
    assert sc.identical(sc.max(var, 'x'), sc.scalar(3.0))


def test_nanmin():
    var = sc.array(dims=['x'], values=[np.nan, 2.0, 3.0])
    assert sc.identical(sc.nanmin(var, 'x'), sc.scalar(2.0))


def test_nanmax():
    var = sc.array(dims=['x'], values=[1.0, 2.0, np.nan])
    assert sc.identical(sc.nanmax(var, 'x'), sc.scalar(2.0))


def test_sum():
    var = sc.array(dims=['x', 'y'], values=np.arange(4.0).reshape(2, 2))
    assert sc.identical(sc.sum(var), sc.scalar(6.0))
    assert sc.identical(sc.sum(var, 'x'), sc.array(dims=['y'], values=[2.0, 4.0]))


def test_nansum():
    var = sc.array(dims=['x', 'y'], values=[[1.0, 1.0], [1.0, np.nan]])
    assert sc.identical(sc.nansum(var), sc.scalar(3.0))
    assert sc.identical(sc.nansum(var, 'x'), sc.array(dims=['y'], values=[2.0, 1.0]))


def test_mean():
    var = sc.arange('r', 4.0).fold('r', sizes={'x': 2, 'y': 2})
    assert sc.identical(sc.mean(var), sc.scalar(6.0 / 4))
    assert sc.identical(sc.mean(var, 'x'), sc.array(dims=['y'], values=[1.0, 2.0]))


def test_nanmean():
    var = sc.array(dims=['x', 'y'], values=[[1.0, 1.0], [1.0, 1.0]])
    assert sc.identical(sc.nanmean(var), sc.scalar(3.0 / 3))
    assert sc.identical(sc.nanmean(var, 'x'), sc.array(dims=['y'], values=[1.0, 1.0]))
