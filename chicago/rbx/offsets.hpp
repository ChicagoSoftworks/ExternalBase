#pragma once
#include <windows.h>
#include <winhttp.h>
#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <cstdio>
#include <dxgi.h>
#include "../json.hpp"
#include "../util/loglib.hpp"
#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "dxgi.lib")

struct offsets_t {
    std::string version;
    json::value raw;

    struct {
        uint64_t AirDensity, GlobalWind;
    } AirProperties;
    struct {
        uint64_t Animation, Animator, IsPlaying, Looped, Speed, TimePosition;
    } AnimationTrack;
    struct {
        uint64_t ActiveAnimations;
    } Animator;
    struct {
        uint64_t Color, Decay, Density, Glare, Haze, Offset;
    } Atmosphere;
    struct {
        uint64_t Position;
    } Attachment;
    struct {
        uint64_t Key, Size, Value;
    } Attribute;
    struct {
        uint64_t Attributes, Length;
    } AttributesMap;
    struct {
        uint64_t CastShadow, Color3, Locked, Massless, Primitive, Reflectance, Shape, Transparency;
    } BasePart;
    struct {
        uint64_t Attachment0, Attachment1, Brightness, CurveSize0, CurveSize1, LightEmission, LightInfluence, Texture, TextureLength, TextureSpeed, Width0, Width1, ZOffset;
    } Beam;
    struct {
        uint64_t Enabled, Intensity, Size, Threshold;
    } BloomEffect;
    struct {
        uint64_t Enabled, Size;
    } BlurEffect;
    struct {
        uint64_t Pointer, Size;
    } ByteCode;
    struct {
        uint64_t CameraSubject, CameraType, FieldOfView, ImagePlaneDepth, Position, Rotation, Viewport, ViewportSize;
    } Camera;
    struct {
        uint64_t BaseTextureId, BodyPart, MeshId, OverlayTextureId;
    } CharacterMesh;
    struct {
        uint64_t MaxActivationDistance, MouseIcon;
    } ClickDetector;
    struct {
        uint64_t Color3, Template;
    } Clothing;
    struct {
        uint64_t Brightness, Contrast, Enabled, TintColor;
    } ColorCorrectionEffect;
    struct {
        uint64_t Enabled, TonemapperPreset;
    } ColorGradingEffect;
    struct {
        uint64_t CreatorId, GameId, GameLoaded, JobId, PlaceId, PlaceVersion, PrimitiveCount, ScriptContext, ServerIP, ToRenderView1, ToRenderView2, ToRenderView3, Workspace;
    } DataModel;
    struct {
        uint64_t Enabled, FarIntensity, FocusDistance, InFocusRadius, NearIntensity;
    } DepthOfFieldEffect;
    struct {
        uint64_t ActivatedCursorIcon, CursorIcon, MaxActivationDistance, MaxDragAngle, MaxDragTranslation, MaxForce, MaxTorque, MinDragAngle, MinDragTranslation, ReferenceInstance, Responsiveness;
    } DragDetector;
    struct {
        uint64_t Pointer, RealDataModel;
    } FakeDataModel;
    struct {
        uint64_t AbsolutePosition, AbsoluteRotation, AbsoluteSize;
    } GuiBase2D;
    struct {
        uint64_t BackgroundColor3, BackgroundTransparency, BorderColor3, Image, LayoutOrder, Position, RichText, Rotation, ScreenGui_Enabled, Size, Text, TextColor3, Visible, ZIndex;
    } GuiObject;
    struct {
        uint64_t AutoJumpEnabled, AutoRotate, AutomaticScalingEnabled, BreakJointsOnDeath, CameraOffset, DisplayDistanceType, DisplayName, EvaluateStateMachine, FloorMaterial, Health, HealthDisplayDistance, HealthDisplayType, HipHeight, HumanoidRootPart, HumanoidState, HumanoidStateID, IsWalking, Jump, JumpHeight, JumpPower, MaxHealth, MaxSlopeAngle, MoveDirection, MoveToPart, MoveToPoint, NameDisplayDistance, NameOcclusion, PlatformStand, RequiresNeck, RigType, SeatPart, Sit, TargetPoint, UseJumpPower, WalkTimer, Walkspeed, WalkspeedCheck;
    } Humanoid;
    struct {
        uint64_t ChildrenEnd, ChildrenStart, ClassBase, ClassDescriptor, ClassName, ComponentMap, Name, Parent, This;
    } Instance;
    struct {
        uint64_t Ambient, Brightness, ClockTime, ColorShift_Bottom, ColorShift_Top, EnvironmentDiffuseScale, EnvironmentSpecularScale, ExposureCompensation, FogColor, FogEnd, FogStart, GeographicLatitude, GlobalShadows, GradientBottom, GradientTop, LightColor, LightDirection, MoonPosition, OutdoorAmbient, Sky, Source, SunPosition;
    } Lighting;
    struct {
        uint64_t ByteCode, GUID, Hash;
    } LocalScript;
    struct {
        uint64_t Asphalt, Basalt, Brick, Cobblestone, Concrete, CrackedLava, Glacier, Grass, Ground, Ice, LeafyGrass, Limestone, Mud, Pavement, Rock, Salt, Sand, Sandstone, Slate, Snow, WoodPlanks;
    } MaterialColors;
    struct {
        uint64_t AssetID, Cache, LRUCache, MeshData, ToMeshData;
    } MeshContentProvider;
    struct {
        uint64_t FaceEnd, FaceStart, VertexEnd, VertexStart;
    } MeshData;
    struct {
        uint64_t MeshId, Texture;
    } MeshPart;
    struct {
        uint64_t Adornee, AnimationId, StringLength, Value;
    } Misc;
    struct {
        uint64_t PrimaryPart, Scale;
    } Model;
    struct {
        uint64_t ByteCode, GUID, Hash, IsCoreScript;
    } ModuleScript;
    struct {
        uint64_t InputObject, InputObject2, MousePosition, SensitivityPointer;
    } MouseService;
    struct {
        uint64_t Acceleration, Brightness, Drag, Lifetime, LightEmission, LightInfluence, Rate, RotSpeed, Rotation, Speed, SpreadAngle, Texture, TimeScale, VelocityInheritance, ZOffset;
    } ParticleEmitter;
    struct {
        uint64_t AccountAge, CameraMode, DisplayName, HealthDisplayDistance, LocalPlayer, LocaleId, MaxZoomDistance, MinZoomDistance, ModelInstance, Mouse, NameDisplayDistance, Team, TeamColor, UserId;
    } Player;
    struct {
        uint64_t Pointer;
    } PlayerConfigurer;
    struct {
        uint64_t Icon, Workspace, Target;
    } PlayerMouse;
    struct {
        uint64_t AssemblyAngularVelocity, AssemblyLinearVelocity, Flags, Material, Owner, Position, Rotation, Size, Validate;
    } Primitive;
    struct {
        uint64_t Anchored, CanCollide, CanQuery, CanTouch;
    } PrimitiveFlags;
    struct {
        uint64_t ActionText, Enabled, GamepadKeyCode, HoldDuration, KeyCode, MaxActivationDistance, ObjectText, RequiresLineOfSight;
    } ProximityPrompt;
    struct {
        uint64_t FakeDataModel, RealDataModel, RenderView;
    } RenderJob;
    struct {
        uint64_t DeviceD3D11, LightingValid, SkyValid, VisualEngine;
    } RenderView;
    struct {
        uint64_t HeartbeatFPS, HeartbeatTask;
    } RunService;
    struct {
        uint64_t ByteCode, GUID, Hash;
    } Script;
    struct {
        uint64_t RequireBypass;
    } ScriptContext;
    struct {
        uint64_t Occupant;
    } Seat;
    struct {
        uint64_t MoonAngularSize, MoonTextureId, SkyboxBk, SkyboxDn, SkyboxFt, SkyboxLf, SkyboxOrientation, SkyboxRt, SkyboxUp, StarCount, SunAngularSize, SunTextureId;
    } Sky;
    struct {
        uint64_t Looped, PlaybackSpeed, RollOffMaxDistance, RollOffMinDistance, SoundGroup, SoundId, Volume;
    } Sound;
    struct {
        uint64_t AllowTeamChangeOnTouch, Enabled, ForcefieldDuration, Neutral, TeamColor;
    } SpawnLocation;
    struct {
        uint64_t MeshId, Scale;
    } SpecialMesh;
    struct {
        uint64_t Value;
    } StatsItem;
    struct {
        uint64_t Enabled, Intensity, Spread;
    } SunRaysEffect;
    struct {
        uint64_t AlphaMode, Color, ColorMap, EmissiveMaskContent, EmissiveStrength, EmissiveTint, MetalnessMap, NormalMap, RoughnessMap;
    } SurfaceAppearance;
    struct {
        uint64_t JobEnd, JobName, JobStart, MaxFPS, Pointer;
    } TaskScheduler;
    struct {
        uint64_t BrickColor;
    } Team;
    struct {
        uint64_t GrassLength, MaterialColors, WaterColor, WaterReflectance, WaterTransparency, WaterWaveSize, WaterWaveSpeed;
    } Terrain;
    struct {
        uint64_t Decal_Texture, Texture_Texture;
    } Textures;
    struct {
        uint64_t CanBeDropped, Enabled, Grip, ManualActivationOnly, RequiresHandle, TextureId, Tooltip;
    } Tool;
    struct {
        uint64_t AssetId;
    } UnionOperation;
    struct {
        uint64_t WindowInputState;
    } UserInputService;
    struct {
        uint64_t MaxSpeed, SteerFloat, ThrottleFloat, Torque, TurnSpeed;
    } VehicleSeat;
    struct {
        uint64_t Dimensions, FakeDataModel, Pointer, RenderView, ViewMatrix;
    } VisualEngine;
    struct {
        uint64_t Part0, Part1;
    } Weld;
    struct {
        uint64_t Part0, Part1;
    } WeldConstraint;
    struct {
        uint64_t CapsLock, CurrentTextBox;
    } WindowInputState;
    struct {
        uint64_t CurrentCamera, DistributedGameTime, ReadOnlyGravity, World;
    } Workspace;
    struct {
        uint64_t AirProperties, FallenPartsDestroyHeight, Gravity, Primitives, worldStepsPerSec;
    } World;
};

inline std::string http_get(const wchar_t* host, const wchar_t* path) {
    std::string body;
    HINTERNET hSession = WinHttpOpen(L"luvrt/0", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return body;
    HINTERNET hConnect = WinHttpConnect(hSession, host, INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) { WinHttpCloseHandle(hSession); return body; }
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path, nullptr, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);
    if (!hRequest) { WinHttpCloseHandle(hConnect); WinHttpCloseHandle(hSession); return body; }
    if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0) && WinHttpReceiveResponse(hRequest, nullptr)) {
        DWORD status = 0, statusSize = sizeof(status);
        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &status, &statusSize, WINHTTP_NO_HEADER_INDEX);
        if (status == 200) {
            DWORD available = 0;
            while (WinHttpQueryDataAvailable(hRequest, &available) && available > 0) {
                std::vector<char> buf(available);
                DWORD read = 0;
                if (WinHttpReadData(hRequest, buf.data(), available, &read)) body.append(buf.data(), read);
            }
        }
    }
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return body;
}

#define OFF(_cat, _fld) if (cat.is_obj() && cat[#_fld].is_num()) r._cat._fld = (uint64_t)cat[#_fld].as_int64();

#define OFF_CAT(_name) \
    if (offs[#_name].is_obj()) { \
        auto& cat = offs[#_name];

#define OFF_END() }

inline std::string getgpu() {
    IDXGIFactory* factory = nullptr;
    if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory)))
        return "NVIDIA";
    IDXGIAdapter* adapter = nullptr;
    if (FAILED(factory->EnumAdapters(0, &adapter))) { factory->Release(); return "NVIDIA"; }
    DXGI_ADAPTER_DESC desc;
    adapter->GetDesc(&desc);
    adapter->Release();
    factory->Release();
    switch (desc.VendorId) {
        case 0x1002: return "AMD";
        case 0x8086: return "INTEL";
        default:     return "NVIDIA";
    }
}

inline offsets_t load_offsets() {
    offsets_t r = {};
    char* appdata = nullptr;
    _dupenv_s(&appdata, nullptr, "APPDATA");
    std::string cache = std::string(appdata ? appdata : "") + "/" + getgpu() + "/ComputeCache";
    free(appdata);
    CreateDirectoryA(cache.c_str(), nullptr);
    loglib::gpu("%s\n", getgpu().c_str());
    loglib::log("getting offsets\n");
    std::string live = http_get(L"offsets.imtheo.lol", L"/roblox/version");
    if (live.empty()) { loglib::err("failed to get version\n"); return r; }
    live.erase(live.find_last_not_of(" \n\r\t") + 1);
    loglib::log("live version: %s\n", live.c_str());
    std::string cached;
    std::ifstream cf((cache + "/cubin_loader.bin").c_str());
    if (cf) std::getline(cf, cached);
    std::string raw;
    if (live != cached) {
        loglib::log("version changed, downloading\n");
        raw = http_get(L"offsets.imtheo.lol", L"/offsets.json");
        if (raw.empty()) { loglib::err("failed to download offsets\n"); return r; }
        std::ofstream of((cache + "/driver_store.bin").c_str(), std::ios::trunc | std::ios::binary);
        of << raw;
        std::ofstream vf((cache + "/cubin_loader.bin").c_str(), std::ios::trunc);
        vf << live;
    } else {
        std::ifstream jf((cache + "/driver_store.bin").c_str());
        if (!jf) { loglib::err("driver_store.bin missing\n"); return r; }
        raw.assign(std::istreambuf_iterator<char>(jf), std::istreambuf_iterator<char>());
    }
    r.raw = json::parse(raw);
    if (r.raw.is_null() || r.raw["Offsets"].is_null()) { loglib::err("invalid offsets json\n"); r = {}; return r; }
    auto& offs = r.raw["Offsets"];
    r.version = r.raw["Roblox Version"].is_str() ? r.raw["Roblox Version"].as_str() : live;
    OFF_CAT(AirProperties) OFF(AirProperties, AirDensity) OFF(AirProperties, GlobalWind) OFF_END()
    OFF_CAT(AnimationTrack) OFF(AnimationTrack, Animation) OFF(AnimationTrack, Animator) OFF(AnimationTrack, IsPlaying) OFF(AnimationTrack, Looped) OFF(AnimationTrack, Speed) OFF(AnimationTrack, TimePosition) OFF_END()
    OFF_CAT(Animator) OFF(Animator, ActiveAnimations) OFF_END()
    OFF_CAT(Atmosphere) OFF(Atmosphere, Color) OFF(Atmosphere, Decay) OFF(Atmosphere, Density) OFF(Atmosphere, Glare) OFF(Atmosphere, Haze) OFF(Atmosphere, Offset) OFF_END()
    OFF_CAT(Attachment) OFF(Attachment, Position) OFF_END()
    OFF_CAT(Attribute) OFF(Attribute, Key) OFF(Attribute, Size) OFF(Attribute, Value) OFF_END()
    OFF_CAT(AttributesMap) OFF(AttributesMap, Attributes) OFF(AttributesMap, Length) OFF_END()
    OFF_CAT(BasePart) OFF(BasePart, CastShadow) OFF(BasePart, Color3) OFF(BasePart, Locked) OFF(BasePart, Massless) OFF(BasePart, Primitive) OFF(BasePart, Reflectance) OFF(BasePart, Shape) OFF(BasePart, Transparency) OFF_END()
    OFF_CAT(Beam) OFF(Beam, Attachment0) OFF(Beam, Attachment1) OFF(Beam, Brightness) OFF(Beam, CurveSize0) OFF(Beam, CurveSize1) OFF(Beam, LightEmission) OFF(Beam, LightInfluence) OFF(Beam, Texture) OFF(Beam, TextureLength) OFF(Beam, TextureSpeed) OFF(Beam, Width0) OFF(Beam, Width1) OFF(Beam, ZOffset) OFF_END()
    OFF_CAT(BloomEffect) OFF(BloomEffect, Enabled) OFF(BloomEffect, Intensity) OFF(BloomEffect, Size) OFF(BloomEffect, Threshold) OFF_END()
    OFF_CAT(BlurEffect) OFF(BlurEffect, Enabled) OFF(BlurEffect, Size) OFF_END()
    OFF_CAT(ByteCode) OFF(ByteCode, Pointer) OFF(ByteCode, Size) OFF_END()
    OFF_CAT(Camera) OFF(Camera, CameraSubject) OFF(Camera, CameraType) OFF(Camera, FieldOfView) OFF(Camera, ImagePlaneDepth) OFF(Camera, Position) OFF(Camera, Rotation) OFF(Camera, Viewport) OFF(Camera, ViewportSize) OFF_END()
    OFF_CAT(CharacterMesh) OFF(CharacterMesh, BaseTextureId) OFF(CharacterMesh, BodyPart) OFF(CharacterMesh, MeshId) OFF(CharacterMesh, OverlayTextureId) OFF_END()
    OFF_CAT(ClickDetector) OFF(ClickDetector, MaxActivationDistance) OFF(ClickDetector, MouseIcon) OFF_END()
    OFF_CAT(Clothing) OFF(Clothing, Color3) OFF(Clothing, Template) OFF_END()
    OFF_CAT(ColorCorrectionEffect) OFF(ColorCorrectionEffect, Brightness) OFF(ColorCorrectionEffect, Contrast) OFF(ColorCorrectionEffect, Enabled) OFF(ColorCorrectionEffect, TintColor) OFF_END()
    OFF_CAT(ColorGradingEffect) OFF(ColorGradingEffect, Enabled) OFF(ColorGradingEffect, TonemapperPreset) OFF_END()
    OFF_CAT(DataModel) OFF(DataModel, CreatorId) OFF(DataModel, GameId) OFF(DataModel, GameLoaded) OFF(DataModel, JobId) OFF(DataModel, PlaceId) OFF(DataModel, PlaceVersion) OFF(DataModel, PrimitiveCount) OFF(DataModel, ScriptContext) OFF(DataModel, ServerIP) OFF(DataModel, ToRenderView1) OFF(DataModel, ToRenderView2) OFF(DataModel, ToRenderView3) OFF(DataModel, Workspace) OFF_END()
    OFF_CAT(DepthOfFieldEffect) OFF(DepthOfFieldEffect, Enabled) OFF(DepthOfFieldEffect, FarIntensity) OFF(DepthOfFieldEffect, FocusDistance) OFF(DepthOfFieldEffect, InFocusRadius) OFF(DepthOfFieldEffect, NearIntensity) OFF_END()
    OFF_CAT(DragDetector) OFF(DragDetector, ActivatedCursorIcon) OFF(DragDetector, CursorIcon) OFF(DragDetector, MaxActivationDistance) OFF(DragDetector, MaxDragAngle) OFF(DragDetector, MaxDragTranslation) OFF(DragDetector, MaxForce) OFF(DragDetector, MaxTorque) OFF(DragDetector, MinDragAngle) OFF(DragDetector, MinDragTranslation) OFF(DragDetector, ReferenceInstance) OFF(DragDetector, Responsiveness) OFF_END()
    OFF_CAT(FakeDataModel) OFF(FakeDataModel, Pointer) OFF(FakeDataModel, RealDataModel) OFF_END()
    OFF_CAT(GuiBase2D) OFF(GuiBase2D, AbsolutePosition) OFF(GuiBase2D, AbsoluteRotation) OFF(GuiBase2D, AbsoluteSize) OFF_END()
    OFF_CAT(GuiObject) OFF(GuiObject, BackgroundColor3) OFF(GuiObject, BackgroundTransparency) OFF(GuiObject, BorderColor3) OFF(GuiObject, Image) OFF(GuiObject, LayoutOrder) OFF(GuiObject, Position) OFF(GuiObject, RichText) OFF(GuiObject, Rotation) OFF(GuiObject, ScreenGui_Enabled) OFF(GuiObject, Size) OFF(GuiObject, Text) OFF(GuiObject, TextColor3) OFF(GuiObject, Visible) OFF(GuiObject, ZIndex) OFF_END()
    OFF_CAT(Humanoid) OFF(Humanoid, AutoJumpEnabled) OFF(Humanoid, AutoRotate) OFF(Humanoid, AutomaticScalingEnabled) OFF(Humanoid, BreakJointsOnDeath) OFF(Humanoid, CameraOffset) OFF(Humanoid, DisplayDistanceType) OFF(Humanoid, DisplayName) OFF(Humanoid, EvaluateStateMachine) OFF(Humanoid, FloorMaterial) OFF(Humanoid, Health) OFF(Humanoid, HealthDisplayDistance) OFF(Humanoid, HealthDisplayType) OFF(Humanoid, HipHeight) OFF(Humanoid, HumanoidRootPart) OFF(Humanoid, HumanoidState) OFF(Humanoid, HumanoidStateID) OFF(Humanoid, IsWalking) OFF(Humanoid, Jump) OFF(Humanoid, JumpHeight) OFF(Humanoid, JumpPower) OFF(Humanoid, MaxHealth) OFF(Humanoid, MaxSlopeAngle) OFF(Humanoid, MoveDirection) OFF(Humanoid, MoveToPart) OFF(Humanoid, MoveToPoint) OFF(Humanoid, NameDisplayDistance) OFF(Humanoid, NameOcclusion) OFF(Humanoid, PlatformStand) OFF(Humanoid, RequiresNeck) OFF(Humanoid, RigType) OFF(Humanoid, SeatPart) OFF(Humanoid, Sit) OFF(Humanoid, TargetPoint) OFF(Humanoid, UseJumpPower) OFF(Humanoid, WalkTimer) OFF(Humanoid, Walkspeed) OFF(Humanoid, WalkspeedCheck) OFF_END()
    OFF_CAT(Instance) OFF(Instance, ChildrenEnd) OFF(Instance, ChildrenStart) OFF(Instance, ClassBase) OFF(Instance, ClassDescriptor) OFF(Instance, ClassName) OFF(Instance, ComponentMap) OFF(Instance, Name) OFF(Instance, Parent) OFF(Instance, This) OFF_END()
    OFF_CAT(Lighting) OFF(Lighting, Ambient) OFF(Lighting, Brightness) OFF(Lighting, ClockTime) OFF(Lighting, ColorShift_Bottom) OFF(Lighting, ColorShift_Top) OFF(Lighting, EnvironmentDiffuseScale) OFF(Lighting, EnvironmentSpecularScale) OFF(Lighting, ExposureCompensation) OFF(Lighting, FogColor) OFF(Lighting, FogEnd) OFF(Lighting, FogStart) OFF(Lighting, GeographicLatitude) OFF(Lighting, GlobalShadows) OFF(Lighting, GradientBottom) OFF(Lighting, GradientTop) OFF(Lighting, LightColor) OFF(Lighting, LightDirection) OFF(Lighting, MoonPosition) OFF(Lighting, OutdoorAmbient) OFF(Lighting, Sky) OFF(Lighting, Source) OFF(Lighting, SunPosition) OFF_END()
    OFF_CAT(LocalScript) OFF(LocalScript, ByteCode) OFF(LocalScript, GUID) OFF(LocalScript, Hash) OFF_END()
    OFF_CAT(MaterialColors) OFF(MaterialColors, Asphalt) OFF(MaterialColors, Basalt) OFF(MaterialColors, Brick) OFF(MaterialColors, Cobblestone) OFF(MaterialColors, Concrete) OFF(MaterialColors, CrackedLava) OFF(MaterialColors, Glacier) OFF(MaterialColors, Grass) OFF(MaterialColors, Ground) OFF(MaterialColors, Ice) OFF(MaterialColors, LeafyGrass) OFF(MaterialColors, Limestone) OFF(MaterialColors, Mud) OFF(MaterialColors, Pavement) OFF(MaterialColors, Rock) OFF(MaterialColors, Salt) OFF(MaterialColors, Sand) OFF(MaterialColors, Sandstone) OFF(MaterialColors, Slate) OFF(MaterialColors, Snow) OFF(MaterialColors, WoodPlanks) OFF_END()
    OFF_CAT(MeshContentProvider) OFF(MeshContentProvider, AssetID) OFF(MeshContentProvider, Cache) OFF(MeshContentProvider, LRUCache) OFF(MeshContentProvider, MeshData) OFF(MeshContentProvider, ToMeshData) OFF_END()
    OFF_CAT(MeshData) OFF(MeshData, FaceEnd) OFF(MeshData, FaceStart) OFF(MeshData, VertexEnd) OFF(MeshData, VertexStart) OFF_END()
    OFF_CAT(MeshPart) OFF(MeshPart, MeshId) OFF(MeshPart, Texture) OFF_END()
    OFF_CAT(Misc) OFF(Misc, Adornee) OFF(Misc, AnimationId) OFF(Misc, StringLength) OFF(Misc, Value) OFF_END()
    OFF_CAT(Model) OFF(Model, PrimaryPart) OFF(Model, Scale) OFF_END()
    OFF_CAT(ModuleScript) OFF(ModuleScript, ByteCode) OFF(ModuleScript, GUID) OFF(ModuleScript, Hash) OFF(ModuleScript, IsCoreScript) OFF_END()
    OFF_CAT(MouseService) OFF(MouseService, InputObject) OFF(MouseService, InputObject2) OFF(MouseService, MousePosition) OFF(MouseService, SensitivityPointer) OFF_END()
    OFF_CAT(ParticleEmitter) OFF(ParticleEmitter, Acceleration) OFF(ParticleEmitter, Brightness) OFF(ParticleEmitter, Drag) OFF(ParticleEmitter, Lifetime) OFF(ParticleEmitter, LightEmission) OFF(ParticleEmitter, LightInfluence) OFF(ParticleEmitter, Rate) OFF(ParticleEmitter, RotSpeed) OFF(ParticleEmitter, Rotation) OFF(ParticleEmitter, Speed) OFF(ParticleEmitter, SpreadAngle) OFF(ParticleEmitter, Texture) OFF(ParticleEmitter, TimeScale) OFF(ParticleEmitter, VelocityInheritance) OFF(ParticleEmitter, ZOffset) OFF_END()
    OFF_CAT(Player) OFF(Player, AccountAge) OFF(Player, CameraMode) OFF(Player, DisplayName) OFF(Player, HealthDisplayDistance) OFF(Player, LocalPlayer) OFF(Player, LocaleId) OFF(Player, MaxZoomDistance) OFF(Player, MinZoomDistance) OFF(Player, ModelInstance) OFF(Player, Mouse) OFF(Player, NameDisplayDistance) OFF(Player, Team) OFF(Player, TeamColor) OFF(Player, UserId) OFF_END()
    OFF_CAT(PlayerConfigurer) OFF(PlayerConfigurer, Pointer) OFF_END()
    OFF_CAT(PlayerMouse) OFF(PlayerMouse, Icon) OFF(PlayerMouse, Workspace) OFF(PlayerMouse, Target) OFF_END()
    OFF_CAT(Primitive) OFF(Primitive, AssemblyAngularVelocity) OFF(Primitive, AssemblyLinearVelocity) OFF(Primitive, Flags) OFF(Primitive, Material) OFF(Primitive, Owner) OFF(Primitive, Position) OFF(Primitive, Rotation) OFF(Primitive, Size) OFF(Primitive, Validate) OFF_END()
    OFF_CAT(PrimitiveFlags) OFF(PrimitiveFlags, Anchored) OFF(PrimitiveFlags, CanCollide) OFF(PrimitiveFlags, CanQuery) OFF(PrimitiveFlags, CanTouch) OFF_END()
    OFF_CAT(ProximityPrompt) OFF(ProximityPrompt, ActionText) OFF(ProximityPrompt, Enabled) OFF(ProximityPrompt, GamepadKeyCode) OFF(ProximityPrompt, HoldDuration) OFF(ProximityPrompt, KeyCode) OFF(ProximityPrompt, MaxActivationDistance) OFF(ProximityPrompt, ObjectText) OFF(ProximityPrompt, RequiresLineOfSight) OFF_END()
    OFF_CAT(RenderJob) OFF(RenderJob, FakeDataModel) OFF(RenderJob, RealDataModel) OFF(RenderJob, RenderView) OFF_END()
    OFF_CAT(RenderView) OFF(RenderView, DeviceD3D11) OFF(RenderView, LightingValid) OFF(RenderView, SkyValid) OFF(RenderView, VisualEngine) OFF_END()
    OFF_CAT(RunService) OFF(RunService, HeartbeatFPS) OFF(RunService, HeartbeatTask) OFF_END()
    OFF_CAT(Script) OFF(Script, ByteCode) OFF(Script, GUID) OFF(Script, Hash) OFF_END()
    OFF_CAT(ScriptContext) OFF(ScriptContext, RequireBypass) OFF_END()
    OFF_CAT(Seat) OFF(Seat, Occupant) OFF_END()
    OFF_CAT(Sky) OFF(Sky, MoonAngularSize) OFF(Sky, MoonTextureId) OFF(Sky, SkyboxBk) OFF(Sky, SkyboxDn) OFF(Sky, SkyboxFt) OFF(Sky, SkyboxLf) OFF(Sky, SkyboxOrientation) OFF(Sky, SkyboxRt) OFF(Sky, SkyboxUp) OFF(Sky, StarCount) OFF(Sky, SunAngularSize) OFF(Sky, SunTextureId) OFF_END()
    OFF_CAT(Sound) OFF(Sound, Looped) OFF(Sound, PlaybackSpeed) OFF(Sound, RollOffMaxDistance) OFF(Sound, RollOffMinDistance) OFF(Sound, SoundGroup) OFF(Sound, SoundId) OFF(Sound, Volume) OFF_END()
    OFF_CAT(SpawnLocation) OFF(SpawnLocation, AllowTeamChangeOnTouch) OFF(SpawnLocation, Enabled) OFF(SpawnLocation, ForcefieldDuration) OFF(SpawnLocation, Neutral) OFF(SpawnLocation, TeamColor) OFF_END()
    OFF_CAT(SpecialMesh) OFF(SpecialMesh, MeshId) OFF(SpecialMesh, Scale) OFF_END()
    OFF_CAT(StatsItem) OFF(StatsItem, Value) OFF_END()
    OFF_CAT(SunRaysEffect) OFF(SunRaysEffect, Enabled) OFF(SunRaysEffect, Intensity) OFF(SunRaysEffect, Spread) OFF_END()
    OFF_CAT(SurfaceAppearance) OFF(SurfaceAppearance, AlphaMode) OFF(SurfaceAppearance, Color) OFF(SurfaceAppearance, ColorMap) OFF(SurfaceAppearance, EmissiveMaskContent) OFF(SurfaceAppearance, EmissiveStrength) OFF(SurfaceAppearance, EmissiveTint) OFF(SurfaceAppearance, MetalnessMap) OFF(SurfaceAppearance, NormalMap) OFF(SurfaceAppearance, RoughnessMap) OFF_END()
    OFF_CAT(TaskScheduler) OFF(TaskScheduler, JobEnd) OFF(TaskScheduler, JobName) OFF(TaskScheduler, JobStart) OFF(TaskScheduler, MaxFPS) OFF(TaskScheduler, Pointer) OFF_END()
    OFF_CAT(Team) OFF(Team, BrickColor) OFF_END()
    OFF_CAT(Terrain) OFF(Terrain, GrassLength) OFF(Terrain, MaterialColors) OFF(Terrain, WaterColor) OFF(Terrain, WaterReflectance) OFF(Terrain, WaterTransparency) OFF(Terrain, WaterWaveSize) OFF(Terrain, WaterWaveSpeed) OFF_END()
    OFF_CAT(Textures) OFF(Textures, Decal_Texture) OFF(Textures, Texture_Texture) OFF_END()
    OFF_CAT(Tool) OFF(Tool, CanBeDropped) OFF(Tool, Enabled) OFF(Tool, Grip) OFF(Tool, ManualActivationOnly) OFF(Tool, RequiresHandle) OFF(Tool, TextureId) OFF(Tool, Tooltip) OFF_END()
    OFF_CAT(UnionOperation) OFF(UnionOperation, AssetId) OFF_END()
    OFF_CAT(UserInputService) OFF(UserInputService, WindowInputState) OFF_END()
    OFF_CAT(VehicleSeat) OFF(VehicleSeat, MaxSpeed) OFF(VehicleSeat, SteerFloat) OFF(VehicleSeat, ThrottleFloat) OFF(VehicleSeat, Torque) OFF(VehicleSeat, TurnSpeed) OFF_END()
    OFF_CAT(VisualEngine) OFF(VisualEngine, Dimensions) OFF(VisualEngine, FakeDataModel) OFF(VisualEngine, Pointer) OFF(VisualEngine, RenderView) OFF(VisualEngine, ViewMatrix) OFF_END()
    OFF_CAT(Weld) OFF(Weld, Part0) OFF(Weld, Part1) OFF_END()
    OFF_CAT(WeldConstraint) OFF(WeldConstraint, Part0) OFF(WeldConstraint, Part1) OFF_END()
    OFF_CAT(WindowInputState) OFF(WindowInputState, CapsLock) OFF(WindowInputState, CurrentTextBox) OFF_END()
    OFF_CAT(Workspace) OFF(Workspace, CurrentCamera) OFF(Workspace, DistributedGameTime) OFF(Workspace, ReadOnlyGravity) OFF(Workspace, World) OFF_END()
    OFF_CAT(World) OFF(World, AirProperties) OFF(World, FallenPartsDestroyHeight) OFF(World, Gravity) OFF(World, Primitives) OFF(World, worldStepsPerSec) OFF_END()
    loglib::log("offsets loaded\n");
    return r;
}

#undef OFF
#undef OFF_CAT
#undef OFF_END
