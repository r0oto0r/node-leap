const nodeleap = require('bindings')('nodeleap');

export interface NodeLeap {
    init(onErrorCallback: Function, onOkCallback: Function, onLeapEventCallback: Function): void;
	openConnection(): number;
	closeConnection(): number;
}

export const NodeLeap: NodeLeap = nodeleap;
