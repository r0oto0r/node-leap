
export enum LeapHandType {
	LEFT = 0,
	RIGHT = 1
}

export enum LeapServiceDisposition {
	LowFpsDetected = 0x00000001,
	PoorPerformancePause     = 0x00000002,
	TrackingErrorUnknown     = 0x00000004,
	ALL = LowFpsDetected | PoorPerformancePause | TrackingErrorUnknown
};

export enum LeapDeviceStatus {
	Streaming = 0x00000001,
	Paused = 0x00000002,
	Robust = 0x00000004,
	Smudged = 0x00000008,
	LowResource = 0x00000010,
	UnknownFailure = 0xE8010000,
	BadCalibration = 0xE8010001,
	BadFirmware = 0xE8010002,
	BadTransport = 0xE8010003,
	BadControl = 0xE8010004
}

export enum LeapPolicy {
	BackgroundFrames = 0x00000001,
	Images = 0x00000002,
	OptimizeHMD = 0x00000004,
	AllowPauseResume = 0x00000008,
	MapPoints = 0x00000080,
	OptimizeScreenTop = 0x00000100
}

export interface LeapQuaternion {
	x: number;
	y: number;
	z: number;
	w: number;
}

export interface LeapVector {
	x: number;
	y: number;
	z: number;
}

export interface LeapBone {
	width: number;
	rotation: LeapQuaternion;
	prevJoint: LeapVector;
	nextJoint: LeapVector;
}

export interface LeapArm extends LeapBone {}

export interface LeapPalm {
	direction: LeapVector;
	normal: LeapVector;
	orientation: LeapQuaternion;
	position: LeapVector;
	stabilizedPosition: LeapVector;
	velocity: LeapVector;
	width: number;
}

export interface LeapFinger {
	fingerId: number;
	isExtended: number;
	bones: Array<LeapBone>;
}

export interface LeapHand {
	handId: number;
	confidence: number;
	flags: number;
	type: LeapHandType;
	grabStrength: number;
	grabAngle: number;
	pinchDistance: number;
	pinchStrength: number;
	visibleTime: number;
	arm: LeapArm;
	palm: LeapPalm;
	fingers: Array<LeapFinger>;
}

export interface LeapTrackingData {
	frameId: number;
	framerate: number;
	numberOfHands: number;
	timestamp: number;
	hands: Array<LeapHand>;
}

export interface LeapDeviceData {
	deviceId: number;
	status: LeapDeviceStatus;
	flags: number; // not used
}

export interface LeapDeviceFailureData {
	status: LeapDeviceStatus;
}

export interface LeapDeviceLostData {
	deviceId: number;
	status: LeapDeviceStatus;
}

export interface LeapConnectionData {
	flags: number; // combo of LeapServiceDisposition
}

export interface LeapConnectionLostData {
	flags: number; // not used
}

export interface LeapPolicyData {
	currentPolicy: number; // combo of LeapPolicy
}

export enum LeapEventType {
	None = 0,
	Connection = 1,
	ConnectionLost = 2,
	Device = 3,
	DeviceFailure = 4,
	Policy = 5,
	Tracking = 256,
	DeviceLost = 260,
	TrackingMode = 267
}

export interface LeapEvent {
	type: LeapEventType;
	data: LeapTrackingData | LeapDeviceData | LeapDeviceFailureData | LeapDeviceLostData | LeapConnectionData | LeapConnectionLostData | LeapPolicyData
}

export enum LeapServerEvent {
	TRACKING = 'TRACKING'
}
