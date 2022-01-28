
import { LeapConnectionData, LeapConnectionLostData, LeapDeviceData, LeapDeviceFailureData, LeapDeviceLostData, LeapTrackingData, LeapEventType, LeapEvent, LeapServerEvent } from "./Intrerfaces";
import { Server } from 'socket.io';
import { NodeLeap } from "./NodeLeap";

export class NodeLeapServer {
	private static port = 5000;
	private static socketServer: Server;
	private static leapConnected: boolean = false;
	private static connectionFlags: number = 0;
	private static deviceId: number = -1;
	private static deviceStatus: number = 0;
	private static deviceFlags: number = 0;

	public static start() {
		this.startSocketServer();
		this.startLeapConnection();
	}

	private static startSocketServer() {
		this.socketServer = new Server({ transports: [ 'websocket' ] })
		this.socketServer.on('connection', (client: any) => {
            const ip = client.handshake.address;
            console.log(`${ip} connected`);

            client.on('disconnect', () => {
                console.log(`${ip} disconnected`);
				client.offAny();
            });
        });
        this.socketServer.listen(this.port);
		console.log(`Socket server listing on port ${this.port}`);
	}

	private static startLeapConnection() {
		NodeLeap.init(this.onError, this.onOk, this.onLeapEvent);
		NodeLeap.openConnection();
	}

	private static onError = (msg: string) => {
		console.log('error', msg);
	};
	
	private static onOk = () => {
		// Signals module end of execution which should never happen
		throw "FATAL: MODULE STOPPED";
	};

	private static processTrackingData(trackingData: LeapTrackingData) {
		this.socketServer.volatile.emit(LeapServerEvent.TRACKING, trackingData);
	}

	private static onLeapEvent = (event: LeapEvent) => {
		switch(event.type) {
			case LeapEventType.Tracking: {
				const data: LeapTrackingData = event.data as LeapTrackingData;
				this.processTrackingData(data);
			}
			break;
			case LeapEventType.Connection: {
				console.log(`Connected to Leap Tracking Service.`);
				const { flags }: LeapConnectionData = event.data as LeapConnectionData;
				this.connectionFlags = flags;
				this.leapConnected = true;
			}
			break;
			case LeapEventType.ConnectionLost: {
				console.log(`Disconnected from Leap Tracking Service.`);
				const { flags }: LeapConnectionLostData = event.data as LeapConnectionLostData;
				this.connectionFlags = flags;
				this.leapConnected = false;
			}
			break;
			case LeapEventType.Device: {
				const { deviceId, status, flags }: LeapDeviceData = event.data as LeapDeviceData;
				this.deviceId = deviceId;
				this.deviceStatus = status;
				this.deviceFlags = flags;
				console.log(`Device found: id ${this.deviceId}`);
			}
			break;
			case LeapEventType.DeviceLost: {
				console.log(`Device lost: id ${this.deviceId}`);
				const { status }: LeapDeviceLostData = event.data as LeapDeviceLostData;
				this.deviceId = -1;
				this.deviceStatus = status;
			}
			break;
			case LeapEventType.DeviceFailure: {
				const { status }: LeapDeviceFailureData = event.data as LeapDeviceFailureData;
				console.log(`DeviceFailure ${status}`);
				this.deviceStatus = status;
			}
			break;
			case LeapEventType.Policy:
			case LeapEventType.TrackingMode:
			case LeapEventType.None:
				break;
			default:
				console.log(event.type);
				break;
		}
	};
}
