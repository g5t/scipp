import numpy as np
from scipp.spatial import translation, translations
import scipp as sc


def test_translation():
    transform = translation(value=[1, -5, 5], unit=sc.units.m)
    vector = sc.vector(value=[1, 2, 3], unit=sc.units.m)

    assert sc.allclose(transform * vector, sc.vector(value=[2, -3, 8], unit=sc.units.m))


def test_translations():
    transforms = translations(dims=["x"],
                              values=[[1, 0, -2], [3, 4, 5]],
                              unit=sc.units.m)
    vectors = sc.vectors(dims=["x"], values=[[1, 2, 3], [4, 5, 6]], unit=sc.units.m)

    assert sc.allclose(
        transforms * vectors,
        sc.vectors(dims=["x"], values=[[2, 2, 1], [7, 9, 11]], unit=sc.units.m))


def test_translation_default_unit_is_dimensionless():
    var = translation(value=np.ones(shape=(3, )))
    assert var.unit == sc.units.one


def test_translations_default_unit_is_dimensionless():
    var = translations(dims=['x'], values=np.ones(shape=(2, 3)))
    assert var.unit == sc.units.one
