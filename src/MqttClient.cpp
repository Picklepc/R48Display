#include "MqttClient.h"

void MqttClient::begin(const String &host, uint16_t port,
                       const String &user, const String &pass,
                       const String &prefix, const String &clientId) {
  _host = host; _port = port; _user = user; _pass = pass;
  _prefix = prefix; _clientId = clientId;
  _mqtt.setClient(_wifi);
  _mqtt.setServer(_host.c_str(), _port);
  _mqtt.setBufferSize(1024);
  _mqtt.setKeepAlive(60);
  _initialized = true;
  _lastAttemptMs = 0;
  _backoffMs = 5000;
}

void MqttClient::reconfigure(const String &host, uint16_t port,
                              const String &user, const String &pass,
                              const String &prefix, const String &clientId) {
  if (_mqtt.connected()) _mqtt.disconnect();
  _newConn = false;
  begin(host, port, user, pass, prefix, clientId);
}

void MqttClient::loop() {
  if (!_initialized || WiFi.status() != WL_CONNECTED) return;
  if (_mqtt.connected()) { _mqtt.loop(); return; }
  const uint32_t now = millis();
  if (_lastAttemptMs > 0 && now - _lastAttemptMs < _backoffMs) return;
  tryConnect();
}

void MqttClient::tryConnect() {
  _lastAttemptMs = millis();
  const String lwt = _prefix + "/availability";
  bool ok = _user.isEmpty()
    ? _mqtt.connect(_clientId.c_str(), lwt.c_str(), 0, true, "offline")
    : _mqtt.connect(_clientId.c_str(), _user.c_str(), _pass.c_str(),
                    lwt.c_str(), 0, true, "offline");
  if (ok) {
    _mqtt.publish(lwt.c_str(), "online", true);
    _backoffMs = 5000;
    _newConn = true;
  } else {
    _backoffMs = (_backoffMs < 5001) ? 30000UL : 60000UL;
    Serial.printf("[MQTT] connect failed (rc=%d), retry in %lus\n",
                  _mqtt.state(), _backoffMs / 1000);
  }
}

bool MqttClient::publish(const char *subtopic, const String &payload, bool retain) {
  if (!_mqtt.connected()) return false;
  return _mqtt.publish((_prefix + "/" + subtopic).c_str(), payload.c_str(), retain);
}

bool MqttClient::publishRaw(const char *topic, const String &payload, bool retain) {
  if (!_mqtt.connected()) return false;
  return _mqtt.publish(topic, payload.c_str(), retain);
}

bool MqttClient::connected() { return _mqtt.connected(); }

bool MqttClient::newConnection() {
  if (_newConn) { _newConn = false; return true; }
  return false;
}

String MqttClient::statusLabel() {
  if (!_initialized) return "off";
  return _mqtt.connected() ? "connected" : "offline";
}
