"""
Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).

SPDX-License-Identifier: Apache-2.0 OR MIT
"""
# -------------------------------------------------------------------------


import az_qt_helpers
import azlmbr.editor as editor
from camera_animator import CameraAnimatorDialog

if __name__ == "__main__":
    print("VolumetricClouds.boostrap")

    # Register our custom widget as a dockable tool with the Editor under an Examples sub-menu
    options = editor.ViewPaneOptions()
    options.showOnToolsToolbar = True
    options.toolbarIcon = ":/${Name}/toolbar_icon.svg"
    az_qt_helpers.register_view_pane('CameraAnimatorDialog', CameraAnimatorDialog, category="Examples", options=options)
