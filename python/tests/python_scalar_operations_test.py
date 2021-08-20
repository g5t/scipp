# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
import scipp as sc


def test_operation_with_python_float():
    a = sc.scalar(1.23456789)
    b = 9.87654321
    assert (a / b).value == (a / sc.scalar(b)).value


def test_inplace_operation_with_python_float():
    a = sc.scalar(1.23456789)
    b = 9.87654321
    expected = a.value / b
    a /= b
    assert a.value == expected


def test_operation_with_python_int32():
    a = sc.scalar(3, dtype='int32')
    b = 2
    assert sc.identical(a + b, a + sc.scalar(b))


def test_inplace_operation_with_python_int32():
    a = sc.scalar(3, dtype='int32')
    b = 2
    expected = a.value + b
    a += b
    assert a.value == expected
