--[[
* Copyright (c) Galib Arrieta (aka lumbermixalot@github, aka galibzon@github).
*
* SPDX-License-Identifier: Apache-2.0 OR MIT
*
]]

local camera_mover = {
	Properties = {
        cameraEntity = { default = EntityId()
                        , description="The camera entity that we will be animated. If not defined will be set to self."},
        linearVelocity = { default = Vector3(0.0, 0.0, 0.0)
                   , description="The camera position will be updated according to this vector."
                   , suffix="[m/s]"},
        angularVelocity = { default = Vector3(0.0, 0.0, 0.0)
                   , description="The camera orientation will be updated according to these Euler deg/secs."
                   , suffix="[deg/s]"},
        waitTime = { default = 3.0
                   , description="Time to wait before starting to animate."
                   , suffix = "[s]"},
	},
}

function camera_mover:OnActivate()
    self.cameraEntity = self.entityId
    if self.Properties.cameraEntity:IsValid() then
        self.cameraEntity = self.Properties.cameraEntity
    end
    --local targetEntity = self.Properties.TargetEntity
    --if (not targetEntity) or (not targetEntity:IsValid()) then
    --    targetEntity = self.entityId
    --end
    --self.targetEntity = targetEntity

    -- This module is not a listener, but to avoid crashes we require the Input_Event
    -- Script Events in case no one else has loaded the library.
    -- require("Scripts.ScriptEvents.input_event")
    	-- Connect to tick bus to receive time updates
	self.tickBusHandler = TickBus.Connect(self)

    self._waitTime = 0.0
    self._tickState = self._OnTickWaiting
    
    Debug.Log("camera_mover:OnActivate")
end

function camera_mover:OnDeactivate()
    if self.tickBusHandler then
        self.tickBusHandler:Disconnect()
    end
end


function camera_mover:OnTick(deltaTime, timePoint)
    self:_tickState(deltaTime, timePoint)
end

function camera_mover:_OnTickWaiting(deltaTime, timePoint)
    self._waitTime = self._waitTime + deltaTime
    if self._waitTime >= self.Properties.waitTime then
        self._tickState = self._OnTickAnimating
    end
end

function camera_mover:_OnTickAnimating(deltaTime, timePoint)
	local camTM = TransformBus.Event.GetWorldTM(self.cameraEntity)
    
    local rotQuat = camTM:GetRotation()
    local deltaAngles = self.Properties.angularVelocity * deltaTime
    local deltaQuat = Quaternion.CreateFromEulerAnglesDegrees(deltaAngles)
    rotQuat = rotQuat * deltaQuat

    local camPos = camTM:GetTranslation()
    camPos =  camPos + self.Properties.linearVelocity * deltaTime

    camTM = Transform.CreateFromQuaternionAndTranslation(rotQuat, camPos)
    TransformBus.Event.SetWorldTM(self.cameraEntity, camTM)
end


return camera_mover