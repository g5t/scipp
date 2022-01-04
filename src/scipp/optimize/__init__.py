# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 Scipp contributors (https://github.com/scipp)
# @author Simon Heybrock
"""Sub-package for optimization such as curve fitting."""

from ..core import scalar, stddevs, Variable, DataArray
from ..interpolate import _drop_masked

from typing import Callable, List, Tuple
from inspect import signature


def _as_scalar(obj, unit):
    if unit is None:
        return obj
    return scalar(value=obj, unit=unit)


def _wrap_func(f, x_prototype, p_units):
    x_prototype = x_prototype.copy()

    def func(x, *args):
        x_prototype.values = x
        p = [_as_scalar(v, u) for v, u in zip(args, p_units)]
        return f(x_prototype, *p).values

    return func


def curve_fit(f: Callable,
              da: DataArray,
              p0: List[Variable] = None,
              **kwargs) -> Tuple[List[Variable], List[List[Variable]]]:
    """Use non-linear least squares to fot a function, f, to data."""
    for arg in ['xdata', 'ydata', 'sigma']:
        if arg in kwargs:
            raise TypeError(
                f"Invalid argument '{arg}', already defined by the input data array.")
    import scipy.optimize as opt
    da = _drop_masked(da, da.dim)
    sigma = stddevs(da) if da.variances is not None else None
    x = da.coords[da.dim]
    sig = signature(f)
    if p0 is None:
        p0 = [1] * (len(sig.parameters) - 1)
    p_units = [p.unit if isinstance(p, Variable) else None for p in p0]
    p0 = [p.value if isinstance(p, Variable) else p for p in p0]
    popt, pcov = opt.curve_fit(f=_wrap_func(f, x, p_units),
                               xdata=x.values,
                               ydata=da.values,
                               sigma=sigma.values,
                               p0=p0)
    # TODO units for pcov
    return [_as_scalar(v, u) for v, u in zip(popt, p_units)], pcov


__all__ = ['curve_fit']
