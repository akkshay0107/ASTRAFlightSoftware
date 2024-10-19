#include "Comms.h"
#include "Error.h"
#include <cstring>

/* ----- PRIVATE HELPERS ----- */

/* We want to define a serial helper for our fastmavlink
 * library so it can more efficiently write messages to serial
 * without requiring intermediate buffers
 */

/* Define serial interface to use at compile time */
#if IS_ARDUINO
#include <Arduino.h>
#define MAVLinkSerial Serial
#define FASTMAVLINK_SERIAL_WRITE_CHAR 1
void fmav_serial_write_char(char c) {
    MAVLinkSerial.write(c);
}
/* Helpers for data available and data read */
uint16_t availableBytes() {
    return MAVLinkSerial.available();
}
char readChar() {
    return MAVLinkSerial.read();
}
#else
#error Comms not implemented for non-Arduino interfaces - maybe you can add it :)
#endif

/* ----- PUBLIC INTERFACES ----- */

/**
 * We leave the constructor and init empty for now; most initialization
 * functions should be handled in init() in the style of most Arduino
 * libraries.
 */
CommsManager::CommsManager() { }
int CommsManager::init() { return NO_ERROR_CODE; }

/**
 * Spin function; this should be run every loop. Processes
 * new data from serial input and sends periodic messages
 */
void CommsManager::update(unsigned long time) {
    uint16_t available = availableBytes();
    for (uint16_t i = 0; i < available; i++) {
        char c = readChar();
        uint8_t res = fmav_parse_to_msg(&(this->message), &(this->status), c);
        if (res == FASTMAVLINK_PARSE_RESULT_OK) {
            this->processMessage(&(this->message));
        }
    }

    if (time - this->lastHeartbeat > (1000000 / HEARTBEAT_HZ)) {
        this->sendHeartbeat();
        this->lastHeartbeat = time;
    }

    if (time - lastTelem > (1000000 / TELEM_HZ)) {
        this->sendTelem();
        this->lastTelem = time;
    }

    if (time - lastHealth > (1000000 / HEALTH_HZ)) {
        this->sendHealth();
        this->lastHealth = time;
    }
}

/**
 * Send a status text message to ground control station. Can be
 * used to send debug messages, warnings, or errors.
 */
void CommsManager::sendStatusText(MAV_SEVERITY severity, const char *text) {
    int len = strlen(text);
    if (this->statusTextID == 0) {
        this->statusTextID = 1;
    }
    while (len > 0) {
        fmav_statustext_t data;
        data.severity = severity;
        // TODO: We should limit this to read less than 50 at the tail
        // But for some reason that duplicates the text
        // So for now we do this and assume memory security isn't an issue
        memcpy(data.text, text, 50);
        data.id = this->statusTextID;
        fmav_msg_statustext_encode_to_serial(this->sysid, this->compid,
                                             &data, &(this->status));
        len -= 50;
        text += 50;
    }

    this->statusTextID += 1; // Will underflow automatically, no round necessary
}

/* ----- PRIVATE HELPERS ----- */

/**
 * General message processor. Routes messages to respective handlers.
 * TODO: Consider whether a dynamic subscriber system would be worth the
 * overhead compared to manually routing based on switch statement
 */
void CommsManager::processMessage(fmav_message_t *msg) {
    switch (msg->msgid) {
        case FASTMAVLINK_MSG_ID_COMMAND_LONG:
            fmav_command_long_t command;
            fmav_msg_command_long_decode(&command, msg);
            this->processCommand(msg->sysid, msg->compid, &command);
            break;
        default:
            break; // Handling currently not implemented
    }
}

/**
 * Command processor. Further routes commands to respecting handlers.
 */
void CommsManager::processCommand(uint8_t sysid, uint8_t compid, fmav_command_long_t *cmd) {
    // TODO: More generic method of registering handlers
    float params[7];
    params[0] = cmd->param1;
    params[1] = cmd->param2;
    params[2] = cmd->param3;
    params[3] = cmd->param4;
    params[4] = cmd->param5;
    params[5] = cmd->param6;
    params[6] = cmd->param7;
    int ack = flightData::router->call(cmd->command, params);
    if (ack == -1) {
        // We return the generic acknowledgement UNSUPPORTED
        this->ackCommand(cmd->command, MAV_RESULT_UNSUPPORTED, sysid, compid);
        return;
    }
    this->ackCommand(cmd->command, MAV_RESULT_ACCEPTED, sysid, compid);
}

/**
 * Send telemetry
 */
void CommsManager::sendTelem() {
    fmav_control_system_state_t state;
    state.q[1] = flightData::estimatedStateX(0);
    state.q[2] = flightData::estimatedStateX(1);
    state.q[3] = flightData::estimatedStateX(2);
    
    // TODO: Figure out which axis is which
    state.roll_rate = flightData::estimatedStateX(3);
    state.pitch_rate = flightData::estimatedStateX(4);
    state.yaw_rate = flightData::estimatedStateX(5);
    fmav_msg_control_system_state_encode_to_serial(
        this->sysid, this->compid,
        &state, &(this->status)
    );
}

/**
 * Send health
 */
void CommsManager::sendHealth() {
    fmav_sys_status_t health;
    health.voltage_battery = (int) (flightData::voltage[0] * 1000);
    // TODO: Report error flags
    // health.errors_count1 = <bitmask>;
    // TODO: report sensor health
    // health.onboard_control_sensors_present = <bitmask>;
    // TODO: report load
    // health.load = <0-1000 int>;
    fmav_msg_sys_status_encode_to_serial(
        this->sysid, this->compid,
        &health, &(this->status)
    );
}

/** 
 * Send heartbeat message
 */
void CommsManager::sendHeartbeat() {
    fmav_heartbeat_t hb = {
        .custom_mode = 0,
        .type = MAV_TYPE_ROCKET,
        .autopilot = 69,
        .base_mode = 0,
        .system_status = MAV_STATE_ACTIVE, // TODO: Allow updating state
    };
    // fmav_status_t status;
    fmav_msg_heartbeat_encode_to_serial(
        this->sysid, this->compid,
        &hb, &(this->status)
    );
}

void CommsManager::ackCommand(uint16_t command, uint8_t code, uint8_t sysid, uint8_t compid) {
    fmav_command_ack_t ack;
    ack.result = code;
    ack.command = command;
    ack.target_system = sysid;
    ack.target_component = compid;
    fmav_msg_command_ack_encode_to_serial(this->sysid, this->compid,
                                      &ack, &(this->status));
}

/**
 * Reject command with FAILED, mostly used for missing handlers
 */
void CommsManager::rejectCommand(uint16_t command, const char *reason) {
    this->sendStatusText(MAV_SEVERITY_WARNING, reason);
    fmav_command_ack_t ack;
    ack.command = command;
    ack.result = MAV_RESULT_FAILED;
    fmav_msg_command_ack_encode_to_serial(this->sysid, this->compid,
                                      &ack, &(this->status));
}