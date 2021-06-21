# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
# @author Neil Vaytet


class PlotView:
    """
    Base class for a plot view.
    It holds a `figure`, which can be either a Matplotlib based figure (1d and
    2d) or a pythreejs scene (3d).

    The difference between a `PlotView` and a `PlotFigure` is that the
    `PlotView` also handles the communications with the `PlotController` that
    are to do with the `PlotProfile` plot displayed below the `PlotFigure`.
    """
    def __init__(self, figure, formatters):
        self._dims = None
        self._scale = None
        self.figure = figure
        self.formatters = formatters
        self.controller = {}
        self.profile_hover_connection = None
        self.profile_pick_connection = None
        self.profile_update_lock = False
        self.profile_scatter = None
        self.profile_counter = -1
        self.profile_ids = []

    @property
    def axes(self):
        return self._axes

    @property
    def dims(self):
        return self._dims

    def set_scale(self, scale):
        """
        Set new scales for dims. Takes effect after update_data() is called.
        """
        self._dims = None  # flag for axis change
        self._scale = scale

    def _ipython_display_(self):
        """
        IPython display representation for Jupyter notebooks.
        """
        return self._to_widget()._ipython_display_()

    def _to_widget(self):
        """
        The `view` as a widget is just the `figure` as a widget.
        """
        return self.figure._to_widget()

    def close(self):
        """
        Close the figure.
        """
        return self.figure._to_image()

    def show(self):
        """
        Forward the call to show() to the figure.
        """
        self.figure.show()

    def savefig(self, *args, **kwargs):
        """
        Forward figure saving to the `figure`.
        """
        self.figure.savefig(*args, **kwargs)

    def connect(self, controller=None):
        """
        Connect the view interface to the callbacks provided by the
        `controller`.
        """
        self.controller = controller
        self.figure.connect(controller=controller)

    def home_view(self, *args, **kwargs):
        self.figure.home_view(*args, **kwargs)

    def pan_view(self, *args, **kwargs):
        self.figure.pan_view(*args, **kwargs)

    def zoom_view(self, *args, **kwargs):
        self.figure.zoom_view(*args, **kwargs)

    def save_view(self, *args, **kwargs):
        self.figure.save_view(*args, **kwargs)

    def rescale_to_data(self, vmin, vmax):
        """
        Forward rescaling to the `figure`.
        """
        self.figure.rescale_to_data(vmin.value, vmax.value)

    def toggle_mask(self, change=None):
        """
        Dummy toggle_mask function.
        """
        return

    def toggle_norm(self, norm, vmin, vmax):
        """
        Forward norm change to the `figure`.
        """
        self.figure.toggle_norm(norm, vmin.value, vmax.value)

    def _update_axes(self):
        """
        Forward axes update to the `figure`.
        """
        self.figure.initialize({
            axis: self.formatters[dim]
            for axis, dim in zip(self._axes, self._dims)
        })
        scale = {
            axis: self._scale[dim]
            for axis, dim in zip(self._axes, self._dims)
        }
        self.figure.update_axes(scale=scale, unit=f'[{self._data.unit}]')

    def _make_data(self, new_values, mask_info):
        return new_values

    def refresh(self, mask_info):
        self.figure.update_data(self._make_data(self._data, mask_info))

    def update_data(self, new_values, mask_info=None):
        """
        Forward data update to the `figure`.
        """
        self._data = new_values
        if self._dims != new_values.dims:
            self._dims = new_values.dims
            self._update_axes()
        if self.figure.toolbar is not None:
            self.figure.toolbar.dims = self._dims
        self.refresh(mask_info)

    def update_profile_connection(self, visible):
        """
        Connect or disconnect profile pick and hover events.
        """
        if visible:
            self.profile_pick_connection, self.profile_hover_connection = \
                self.figure.connect_profile(
                    self.keep_or_remove_profile, self.update_profile)
        else:
            self.figure.disconnect_profile(self.profile_pick_connection,
                                           self.profile_hover_connection)
            self.profile_pick_connection = None
            self.profile_hover_connection = None

    def update_norm_button(self, *args, **kwargs):
        """
        Forward norm button update to the `figure`.
        """
        self.figure.update_norm_button(*args, **kwargs)

    def set_draw_no_delay(self, *args, **kwargs):
        """
        Forward set_draw_no_delay to the `figure`.
        """
        self.figure.set_draw_no_delay(*args, **kwargs)
