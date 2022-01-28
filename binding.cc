#include <string>
#include <thread>
#include <chrono>
#include <iostream>
#include "napi.h"
#include "LeapC.h"

using namespace Napi;

class LeapAsyncWorker : public AsyncProgressQueueWorker<LEAP_CONNECTION_MESSAGE>
{
    public:
        LeapAsyncWorker(Function& errorCallback, Function &okCallback, Function& progressCallback) 
			: AsyncProgressQueueWorker(okCallback) {
			this->errorCallback.Reset(errorCallback, 1);
			this->progressCallback.Reset(progressCallback, 1);
			isRunning = false;
		}

        void Execute(const ExecutionProgress& progress) {
			while(true) {
				if(isRunning) {
					LEAP_CONNECTION_MESSAGE msg;
					LeapPollConnection(connectionHandle, 1000, &msg);
					progress.Send(&msg, 1);
				} else {
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
				}
			}
		}

		Object leapVectorToObject(const LEAP_VECTOR &vector) {
			Object vectorObject = Object::New(Env());

			vectorObject.Set("x", vector.x);
			vectorObject.Set("y", vector.y);
			vectorObject.Set("z", vector.z);

			return vectorObject;
		}

		Object leapQuaternionToObject(const LEAP_QUATERNION &quaternion) {
			Object quaternionObject = Object::New(Env());

			quaternionObject.Set("x", quaternion.x);
			quaternionObject.Set("y", quaternion.y);
			quaternionObject.Set("z", quaternion.z);
			quaternionObject.Set("w", quaternion.w);

			return quaternionObject;
		}

		Object leapBoneToObject(const LEAP_BONE &bone) {
			Object boneObject = Object::New(Env());

			boneObject.Set("width", bone.width);
			boneObject.Set("rotation", leapQuaternionToObject(bone.rotation));
			boneObject.Set("prevJoint", leapVectorToObject(bone.prev_joint));
			boneObject.Set("nextJoint", leapVectorToObject(bone.next_joint));

			return boneObject;
		}

		Object leapPalmToObject(const LEAP_PALM &palm) {
			Object palmObject = Object::New(Env());

			palmObject.Set("direction", leapVectorToObject(palm.direction));
			palmObject.Set("normal", leapVectorToObject(palm.normal));
			palmObject.Set("orientation", leapQuaternionToObject(palm.orientation));
			palmObject.Set("position", leapVectorToObject(palm.position));
			palmObject.Set("stabilizedPosition", leapVectorToObject(palm.stabilized_position));
			palmObject.Set("velocity", leapVectorToObject(palm.velocity));
			palmObject.Set("width", palm.width);

			return palmObject;
		}

		Object leapFingerToObject(const LEAP_DIGIT &finger) {
			Object fingerObject = Object::New(Env());

			fingerObject.Set("fingerId", finger.finger_id);
			fingerObject.Set("isExtended", finger.is_extended);

			Array fingerBones = Array::New(Env(), 4);
			fingerObject.Set("bones", fingerBones);

			for(int i = 0; i < 4; ++i) {
				const LEAP_BONE leapFingerBone = finger.bones[i];
				fingerBones[i] = leapBoneToObject(leapFingerBone);
			}

			return fingerObject;
		}

		Object leapHandToObject(const LEAP_HAND &hand) {
			Object handObject = Object::New(Env());

			handObject.Set("handId", hand.id);
			handObject.Set("confidence", hand.confidence);
			handObject.Set("flags", hand.flags);
			handObject.Set("type", static_cast<int>(hand.type));
			handObject.Set("grabStrength", hand.grab_strength);
			handObject.Set("grabAngle", hand.grab_angle);
			handObject.Set("pinchDistance", hand.pinch_distance);
			handObject.Set("pinchStrength", hand.pinch_strength);
			handObject.Set("visibleTime", hand.visible_time);
			handObject.Set("arm", leapBoneToObject(hand.arm));
			handObject.Set("palm", leapPalmToObject(hand.palm));

			Array fingers = Array::New(Env(), 5);
			handObject.Set("fingers", fingers);
			for(int i = 0; i < 5; ++i) {
				fingers[i] = leapFingerToObject(hand.digits[i]);
			}

			return handObject;
		}

		void OnProgress(const LEAP_CONNECTION_MESSAGE* data, size_t /* count */) {
			HandleScope scope(Env());

			Object event = Object::New(Env());
			event.Set("deviceId", data->device_id);
			event.Set("type", static_cast<int>(data->type));
			Object eventData = Object::New(Env());
			event.Set("data", eventData);

			bool somethingToEmit = true;

			switch(data->type) {
				case eLeapEventType_Tracking: {
					const LEAP_TRACKING_EVENT trackingData = *data->tracking_event;
					uint32_t numHands = trackingData.nHands;
					eventData.Set("frameId", trackingData.tracking_frame_id);
					eventData.Set("framerate", trackingData.framerate);
					eventData.Set("numberOfHands", numHands);
					eventData.Set("timestamp", trackingData.info.timestamp);

					Array hands = Array::New(Env(), numHands);
					eventData.Set("hands", hands);
					for(unsigned int i = 0; i < numHands; ++i) {
						hands[i] = leapHandToObject(trackingData.pHands[i]);
					}
				}
				break;

				case eLeapEventType_Device:
					const LEAP_DEVICE_EVENT deviceData = *data->device_event;
					eventData.Set("deviceId", deviceData.device.id);
					eventData.Set("status", deviceData.status);
					eventData.Set("flags", deviceData.flags);
				break;

				case eLeapEventType_DeviceFailure:
					const LEAP_DEVICE_FAILURE_EVENT deviceFailureData = *data->device_failure_event;
					eventData.Set("status", static_cast<int>(deviceFailureData.status));
				break;

				case eLeapEventType_DeviceLost:
					const LEAP_DEVICE_EVENT deviceEventData = *data->device_event;
					eventData.Set("deviceId", deviceEventData.device.id);
					eventData.Set("status", deviceEventData.status);
				break;

				case eLeapEventType_Connection:
					const LEAP_CONNECTION_EVENT connectionData = *data->connection_event;
					eventData.Set("flags", connectionData.flags);
				break;

				case eLeapEventType_ConnectionLost:
					const LEAP_CONNECTION_LOST_EVENT connectionLostData = *data->connection_lost_event;
					eventData.Set("flags", connectionLostData.flags);
				break;

				case eLeapEventType_Policy:
					const LEAP_POLICY_EVENT policyData = *data->policy_event;
					eventData.Set("currentPolicy", policyData.current_policy);
				break;

				case eLeapEventType_None:
				default:
					somethingToEmit = false;
				break;
			}

			if(!this->progressCallback.IsEmpty() && somethingToEmit) {
                this->progressCallback.Call(Receiver().Value(), {event});
            }
		}

		void OnOk() {
			HandleScope scope(Env());

			Callback().Call({Env().Undefined()});
		}

		void OnError(const Error& e) {
			HandleScope scope(Env());

            if(!this->errorCallback.IsEmpty()) {
                this->errorCallback.Call(Receiver().Value(), {String::New(Env(), e.Message())});
            }
		}

		eLeapRS OpenConnection() {
			if(!isRunning) {
				eLeapRS result = eLeapRS_Success;
				if(connectionHandle == NULL) {
					result = LeapCreateConnection(NULL, &connectionHandle);
				}
				if(result == eLeapRS_Success) {
					result = LeapOpenConnection(connectionHandle);
					if(result == eLeapRS_Success){
						isRunning = true;
					}
				}
			}

			return eLeapRS_Success;
		}

		void CloseConnection() {
			isRunning = false;
			if(connectionHandle != NULL) {
				LeapCloseConnection(connectionHandle);
			}
		}

		~LeapAsyncWorker() {
			isRunning = false;
			if(connectionHandle != NULL) {
				LeapDestroyConnection(connectionHandle);
			}
		}

    private:
		FunctionReference progressCallback;
        FunctionReference errorCallback;
		LEAP_CONNECTION connectionHandle = NULL;
		bool isRunning = false;
} *worker = NULL;

Value CallInit(const CallbackInfo& info) {
    Function errorCb = info[0].As<Function>();
    Function okCb = info[1].As<Function>();
    Function progressCb = info[2].As<Function>();

	if(worker != NULL) {
		delete(worker);
		worker = NULL;
	}

	worker = new LeapAsyncWorker(errorCb, okCb, progressCb);
    worker->Queue();

	return info.Env().Undefined();
}

Value CallOpenConnection(const CallbackInfo& info) {
    if(worker != NULL) {
		Number result = Number::New(info.Env(), static_cast<int>(worker->OpenConnection()));
		return result;
	}
	return Number::New(info.Env(), static_cast<int>(eLeapRS_Success));
}

Value CallCloseConnection(const CallbackInfo& info) {
    if(worker != NULL) {
		worker->CloseConnection();
	}
	return Number::New(info.Env(), static_cast<int>(eLeapRS_Success));
}

Object Init(Env env, Object exports) {
    exports.Set(String::New(env, "init"), Function::New(env, CallInit));
	exports.Set(String::New(env, "openConnection"), Function::New(env, CallOpenConnection));
    exports.Set(String::New(env, "closeConnection"), Function::New(env, CallCloseConnection));

    return exports;
}

NODE_API_MODULE(nodeleap, Init)
