# SPDX-License-Identifier: BSD-3-Clause
# Copyright (c) 2021 Scipp contributors (https://github.com/scipp)
# @author Neil Vaytet

from .controller import PlotController


class PlotController2d(PlotController):
    """
    Controller class for 2d plots.
    """
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)

    def update_viewport(self, xylims):
        """
        When the view requests a viewport udpate, we get new image data from
        the model and send them back to the view which then updates the figure.
        """
        new_values = self.model.update_viewport(xylims)
        if new_values is not None:
            self.view.update_data(new_values, mask_info=self.get_masks_info())

    def connect_view(self):
        """
        Connect the view interface to callbacks.
        """
        super().connect_view()
        self.view.connect(
            view_callbacks={
                "update_viewport": self.update_viewport,
                "rescale_to_data": self.rescale_to_data
            })

    def redraw(self):
        """
        Update the model data dicts and re-draw the figure.
        """
        self.model.reset_resampling_model()
        super().redraw()
