"""
Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).

SPDX-License-Identifier: Apache-2.0 OR MIT
"""
# -------------------------------------------------------------------------

from PySide2.QtCore import Qt
from PySide2.QtWidgets import QDialog, QLabel, QVBoxLayout, QHBoxLayout, QLineEdit, QPushButton
from PySide2.QtGui import QDoubleValidator

import azlmbr.bus as ebus
import azlmbr.editor as editor
import azlmbr.math as azmath
import azlmbr.components as azcomponents
import azlmbr.legacy.general as azgeneral

# Animates the current viewport camera
# by updating Translation and/or angular rotation as defined
# by the user.
# This script is registered into the Tools menu thanks to the companion bootstrap.py.
# it can also be executed manually by using:
#  pyRunFile <path to this script>
# in the Editor command console.

# A Widget for the azlmbr.math.Vector3 class
class MyVector3Widget:
    def __init__(self, absMax):
        self.hlayout = QHBoxLayout()
        self.editAxisX = self.CreateAxis('X', absMax)
        self.editAxisY = self.CreateAxis('Y', absMax)
        self.editAxisZ = self.CreateAxis('Z', absMax)

    def CreateAxis(self, name, absMax):
        self.hlayout.addWidget(QLabel(name))
        editAxis = QLineEdit('0.0')
        editAxis.setValidator(QDoubleValidator(-absMax, absMax, 3))
        self.hlayout.addWidget(editAxis)
        return editAxis

    def GetLayout(self):
        return self.hlayout

    def GetAsVector3(self):
        x = float(self.editAxisX.text())
        y = float(self.editAxisY.text())
        z = float(self.editAxisZ.text())
        return azmath.Vector3(x,y,z)

    def SetVector3(self, vector3):
        self.editAxisX.setText("{:0.3f}".format(vector3.x))
        self.editAxisY.setText("{:0.3f}".format(vector3.y))
        self.editAxisZ.setText("{:0.3f}".format(vector3.z))


# This is the main UI.
class CameraAnimatorDialog(QDialog):
    def __init__(self, parent=None):
        super(CameraAnimatorDialog, self).__init__(parent)

        self.mainLayout = QVBoxLayout(self)
        layout = self.mainLayout

        layout.addWidget(QLabel('Linear Velocity [m/s]'))
        self._linearVelocityWidget = MyVector3Widget(100.0)
        layout.addLayout(self._linearVelocityWidget.GetLayout())
        layout.addWidget(QLabel(' ')) #space

        layout.addWidget(QLabel('Angular Velocity [deg/s]'))
        self._angularVelocityWidget = MyVector3Widget(359.0)
        layout.addLayout(self._angularVelocityWidget.GetLayout())
        layout.addWidget(QLabel(' ')) #space

        button = QPushButton("Animate")
        button.clicked.connect(self.OnAnimateCamera)  
        layout.addWidget(button)

        button = QPushButton("Stop")
        button.clicked.connect(self.OnStopCamera)  
        layout.addWidget(button)

        self.setLayout(layout)

    # Animates the camera by using the TickBus. It also makes sure the camera keeps animating
    # even if the Editor loses focus, thanks to the 'ed_backgroundUpdatePeriod' CVAR.  
    def OnAnimateCamera(self):
        self._camEntityId = editor.EditorCameraRequestBus(ebus.Broadcast, "GetCurrentViewEntityId")
        self._camVelocity = self._linearVelocityWidget.GetAsVector3()
        self._camAngularVelocity = self._angularVelocityWidget.GetAsVector3()
        print(f"linear velocity={self._camVelocity}")
        print(f"angular velocity={self._camAngularVelocity}")
        #camPos = azcomponents.TransformBus(ebus.Event, "GetWorldTranslation", self._camEntityId)
        #print(f"Camera is located at {camPos}")
        self._onTickHandler = ebus.NotificationHandler('TickBus')
        myself = self
        def onTick(args):
            nonlocal myself
            myself.OnTick(args)
        self._onTickHandler.connect() #must be called before add_callback
        self._onTickHandler.add_callback('OnTick', onTick)
        # Even if we lose focus, keep the tickBus ticking:
        azgeneral.set_cvar_integer("ed_backgroundUpdatePeriod", -1)
        print("Started camera animation!")


    def OnStopCamera(self):
        if not self._onTickHandler:
            return
        self._onTickHandler.disconnect()
        self._onTickHandler = None
        # if we lose focus do not tick:
        azgeneral.set_cvar_integer("ed_backgroundUpdatePeriod", 0)
        print("Stopped camera animation!")


    def OnTick(self, args):
        #print(f"I'm self ticking {len(args)}, {type(args[0])}")
        camTM = azcomponents.TransformBus(ebus.Event, "GetWorldTM", self._camEntityId)
        rotQuat = camTM.GetRotation()
        camPos = camTM.GetTranslation()
        #print(f"Camera is located at {camPos}")
    
        # Apply linear velocity changes
        deltaTime = args[0]
        deltaDistance = self._camVelocity.MultiplyFloat(deltaTime)
        camPos = camPos.Add(deltaDistance)

        # Apply Rotation changes.
        deltaAngles = self._camAngularVelocity.MultiplyFloat(deltaTime)
        deltaQuat = azmath.Quaternion_CreateFromEulerAnglesDegrees(deltaAngles)
        rotQuat = rotQuat.MultiplyQuaternion(deltaQuat)

        camTM = azmath.Transform_CreateFromQuaternionAndTranslation(rotQuat, camPos)
        azcomponents.TransformBus(ebus.Event, "SetWorldTM", self._camEntityId, camTM)



if __name__ == "__main__":
    dialog = CameraAnimatorDialog()
    dialog.setWindowTitle("Camera Animator Dialog")
    dialog.show()
    dialog.adjustSize()
