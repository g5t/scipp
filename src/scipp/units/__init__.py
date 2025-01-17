# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2022 Scipp contributors (https://github.com/scipp)
# @author Jan-Lukas Wynen
"""
Predefined units.

The following predefined units are available:

Dimensionless (those two are aliases for each other):
 - dimensionless
 - one

Common units:
 - angstrom
 - counts
 - deg
 - kg
 - K
 - meV
 - m
 - rad
 - s
 - us
 - ns
 - mm

Special:
 - default_unit (used by some functions to deduce a unit)

.. seealso::
    :py:class:`scipp.Unit` to construct other units.
"""

from .._scipp.core.units import (angstrom, counts, default_unit, deg, dimensionless, kg,
                                 K, meV, m, one, rad, s, us, ns, mm)

from .._scipp.core.units import clearUserDefinedUnits
from .._scipp.core import Unit as _Unit
from typing import Union


def addUserDefinedUnit(name: str, unit: Union[str, _Unit]) -> _Unit:
    from .._scipp.core.units import addUserDefinedUnit as implementation
    if isinstance(unit, str):
        unit = _Unit(unit)
    return implementation(name, unit)


__all__ = [
    'angstrom', 'counts', 'default_unit', 'deg', 'dimensionless', 'kg', 'K', 'meV', 'm',
    'one', 'rad', 's', 'us', 'ns', 'mm', 'addUserDefinedUnit', 'clearUserDefinedUnits'
]
