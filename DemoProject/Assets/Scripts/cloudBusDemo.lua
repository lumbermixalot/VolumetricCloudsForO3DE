-- Just  basic template that you can quickly copy/paste
-- and start writing your component class
local cloud_bus_demo = {
    Properties = {
        waitTime = { default = 3.0
                   , description = "The amount of seconds to wait before using the Volumetric Clouds API."
                   , suffix = "[s]"},
        shaderConstants = {
            p00_uvwScale = { default = 0.5
                       , description="See Volumetric Cloudscape component"},
            p01_maxMipLevels = { default = 10
                       , description="See Volumetric Cloudscape component"},
            p02_rayMarchingSteps = {
                p00_min = 32,
                p01_max = 64 
            },
        },
        planetaryData = {
            p00_planetRadiusKm = {
                default = 6371.0,
                suffix = "[Km]"
            },
            p01_distanceToCloudSlabKm = {
                default = 1.5,
                suffix = "[Km]"
            },
            p02_cloudSlabThickness = {
                default = 3.5,
                suffix = "[Km]"
            },
            p03_sunLightColor = {
                default = Color(1.0, 1.0, 1.0, 1.0)
            },
            p04_sunLightIntensity = {
                default = 1.0
            },
            p05_ambientLightColor = {
                default = Color(1.0, 1.0, 1.0, 1.0)
            },
            p06_ambientLightIntensity = {
                default = 1.0
            },
        },
        weatherData = {
            p00_weatherMapSizeKm = 100.0,
            p01_globalCloudCoverage = 0.75,
            p02_globalCloudDensity= 1.0,
            p03_windVelocity=Vector3(0.0, 0.0, 0.0),
            p04_cloudTopShiftOffsetKm = 0.0
        },
        cloudMaterialProperties = {
            p00_absorptionCoefficient = 0.01,
            p01_scatteringCoefficient = 0.03,
            p02_excentricity = 0.2,
            p03_multiScatteringABC = Vector3(0.5, 0.5, 0.5)
        }
    }
}

function cloud_bus_demo:OnActivate()
    self._elapsedTime = 0.0
    self._currentState = self._OnTickState_Waiting
    if self.Properties.waitTime >= 0.0001 then
	    self.tickBusHandler = TickBus.Connect(self)
    else
        Debug.Log("cloud_bus_demo:OnActivate The waitTime is close to 0, the Volumetric Clouds API wont be used.")
    end
end

function cloud_bus_demo:OnDeactivate()
    if self.tickBusHandler then
        self.tickBusHandler:Disconnect()
    end
end

function cloud_bus_demo:OnTick(deltaTime, timePoint)
	self._currentState(self, deltaTime, timePoint)
end

local function CloudMaterialPropertiesToString(cmpTable)
    local firstPart = "<cmp table: aCoeff=" .. tostring(cmpTable.p00_absorptionCoefficient) .. ", sCoeff=" .. tostring(cmpTable.p01_scatteringCoefficient)
    return firstPart .. ", G=" .. tostring(cmpTable.p02_excentricity) .. ", abc=" .. tostring(cmpTable.p03_multiScatteringABC) .. ">"
end
---------------------------------------------------------------
-- The states start
function cloud_bus_demo:_OnTickState_Waiting(deltaTime, timePoint)
    self._elapsedTime = self._elapsedTime + deltaTime
    if self._elapsedTime >= self.Properties.waitTime then
        self._currentState = self._OnTickState_Running
    end
end

function cloud_bus_demo:_OnTickState_Running(deltaTime, timePoint)
    -- Modify the state of the cloud component and stop the Tick
    local isCallBatching = VolumetricCloudsRequestBus.Broadcast.IsCallBatching()
    Debug.Warning(not isCallBatching, "VolumetricCloudsRequestBus was already call batching. Weird!")
    VolumetricCloudsRequestBus.Broadcast.BeginCallBatch()

    local scProps = self.Properties.shaderConstants

    local uvwScale = VolumetricCloudsRequestBus.Broadcast.GetUVWScale()
    Debug.Log("Current uvwScale=" .. tostring(uvwScale) .. ", will be changed to " .. tostring(scProps.p00_uvwScale))
    VolumetricCloudsRequestBus.Broadcast.SetUVWScale(scProps.p00_uvwScale)

    local maxMipLevels = VolumetricCloudsRequestBus.Broadcast.GetMaxMipLevels()
    Debug.Log("Current maxMipLevels=" .. tostring(maxMipLevels) .. ", will be changed to " .. tostring(scProps.p01_maxMipLevels))
    VolumetricCloudsRequestBus.Broadcast.SetMaxMipLevels(scProps.p01_maxMipLevels)

    local tuple = VolumetricCloudsRequestBus.Broadcast.GetRayMarchingSteps()
    local rayMarchStepsMin = tuple:Get0()
    local rayMarchStepsMax = tuple:Get1()
    Debug.Log("Current RayMarchingStepsMin=" .. tostring(rayMarchStepsMin) .. ", will be changed to " .. tostring(scProps.p02_rayMarchingSteps.p00_min))
    Debug.Log("Current RayMarchingStepsMax=" .. tostring(rayMarchStepsMax) .. ", will be changed to " .. tostring(scProps.p02_rayMarchingSteps.p01_max))
    rayMarchStepsMin = scProps.p02_rayMarchingSteps.p00_min
    rayMarchStepsMax = scProps.p02_rayMarchingSteps.p01_max
    VolumetricCloudsRequestBus.Broadcast.SetRayMarchingSteps(rayMarchStepsMin, rayMarchStepsMax)

    -- Planetary data
    local pdProps = self.Properties.planetaryData

    local planetRadiusKm = VolumetricCloudsRequestBus.Broadcast.GetPlanetRadiusKm()
    Debug.Log("Current planetRadiusKm=" .. tostring(planetRadiusKm) .. ", will be changed to " .. tostring(pdProps.p00_planetRadiusKm))
    VolumetricCloudsRequestBus.Broadcast.SetPlanetRadiusKm(pdProps.p00_planetRadiusKm)

    local distanceToCloudSlabKm = VolumetricCloudsRequestBus.Broadcast.GetDistanceToCloudSlabKm()
    Debug.Log("Current distanceToCloudSlabKm=" .. tostring(distanceToCloudSlabKm) .. ", will be changed to " .. tostring(pdProps.p01_distanceToCloudSlabKm))
    VolumetricCloudsRequestBus.Broadcast.SetDistanceToCloudSlabKm(pdProps.p01_distanceToCloudSlabKm)

    local cloudSlabThickness = VolumetricCloudsRequestBus.Broadcast.GetCloudSlabThicknessKm()
    Debug.Log("Current cloudSlabThickness=" .. tostring(cloudSlabThickness) .. ", will be changed to " .. tostring(pdProps.p02_cloudSlabThickness))
    VolumetricCloudsRequestBus.Broadcast.SetCloudSlabThicknessKm(pdProps.p02_cloudSlabThickness)

    local sunLightColorAndIntensity = VolumetricCloudsRequestBus.Broadcast.GetSunLightColorAndIntensity()
    Debug.Log("Current sunLightColor=" .. tostring(sunLightColorAndIntensity) .. ", will be changed to " .. tostring(pdProps.p03_sunLightColor))
    Debug.Log("Current sunLightIntensity=" .. tostring(sunLightColorAndIntensity.a) .. ", will be changed to " .. tostring(pdProps.p04_sunLightIntensity))
    sunLightColorAndIntensity = pdProps.p03_sunLightColor:Clone()
    sunLightColorAndIntensity.a = pdProps.p04_sunLightIntensity
    VolumetricCloudsRequestBus.Broadcast.SetSunLightColorAndIntensity(sunLightColorAndIntensity)

    local ambientLightColorAndIntensity = VolumetricCloudsRequestBus.Broadcast.GetAmbientLightColorAndIntensity()
    Debug.Log("Current ambientLightColor=" .. tostring(ambientLightColorAndIntensity) .. ", will be changed to " .. tostring(pdProps.p05_ambientLightColor))
    Debug.Log("Current ambientLightIntensity=" .. tostring(ambientLightColorAndIntensity.a) .. ", will be changed to " .. tostring(pdProps.p06_ambientLightIntensity))
    ambientLightColorAndIntensity = pdProps.p05_ambientLightColor:Clone()
    ambientLightColorAndIntensity.a = pdProps.p06_ambientLightIntensity
    VolumetricCloudsRequestBus.Broadcast.SetAmbientLightColorAndIntensity(ambientLightColorAndIntensity)

    -- Weather data
    local wdProps = self.Properties.weatherData

    local weatherMapSizeKm = VolumetricCloudsRequestBus.Broadcast.GetWeatherMapSizeKm()
    Debug.Log("Current weatherMapSizeKm=" .. tostring(weatherMapSizeKm) .. ", will be changed to " .. tostring(wdProps.p00_weatherMapSizeKm))
    VolumetricCloudsRequestBus.Broadcast.SetWeatherMapSizeKm(wdProps.p00_weatherMapSizeKm)

    local globalCloudCoverage = VolumetricCloudsRequestBus.Broadcast.GetCloudCoverage()
    Debug.Log("Current globalCloudCoverage=" .. tostring(globalCloudCoverage) .. ", will be changed to " .. tostring(wdProps.p01_globalCloudCoverage))
    VolumetricCloudsRequestBus.Broadcast.SetCloudCoverage(wdProps.p01_globalCloudCoverage)
    
    local globalCloudDensity = VolumetricCloudsRequestBus.Broadcast.GetCloudDensity()
    Debug.Log("Current globalCloudDensity=" .. tostring(globalCloudDensity) .. ", will be changed to " .. tostring(wdProps.p02_globalCloudDensity))
    VolumetricCloudsRequestBus.Broadcast.SetCloudDensity(wdProps.p02_globalCloudDensity)
    
    local windVelocity = VolumetricCloudsRequestBus.Broadcast.GetWindVelocity()
    Debug.Log("Current windVelocity=" .. tostring(windVelocity) .. ", will be changed to " .. tostring(wdProps.p03_windVelocity))
    VolumetricCloudsRequestBus.Broadcast.SetWindVelocity(wdProps.p03_windVelocity)
    
    local cloudTopShiftOffsetKm = VolumetricCloudsRequestBus.Broadcast.GetCloudTopShiftKm()
    Debug.Log("Current cloudTopShiftOffsetKm=" .. tostring(cloudTopShiftOffsetKm) .. ", will be changed to " .. tostring(wdProps.p04_cloudTopShiftOffsetKm))
    VolumetricCloudsRequestBus.Broadcast.SetCloudTopShiftKm(wdProps.p04_cloudTopShiftOffsetKm)

    -- Cloud Material Properties
    local cmProps = self.Properties.cloudMaterialProperties

    local cmp = VolumetricCloudsRequestBus.Broadcast.GetCloudMaterialProperties()
    local cmPropsStr = CloudMaterialPropertiesToString(cmProps)
    Debug.Log("Current cloudMaterialProperties=" .. tostring(cmp) .. ", will be changed to " .. cmPropsStr)
    cmp.absorptionCoefficient = cmProps.p00_absorptionCoefficient
    cmp.scatteringCoefficient = cmProps.p01_scatteringCoefficient
    cmp.excentricity = cmProps.p02_excentricity
    cmp.multiScatteringABC = cmProps.p03_multiScatteringABC
    VolumetricCloudsRequestBus.Broadcast.SetCloudMaterialProperties(cmp)

    VolumetricCloudsRequestBus.Broadcast.EndCallBatch()

    Debug.Log("automation:_OnTickState_Running Successfully demonstrated the VolumetricCloudsRequestBus API.")
    self.tickBusHandler:Disconnect()
    self.tickBusHandler = nil
end
-- The states end
---------------------------------------------------------------




return cloud_bus_demo