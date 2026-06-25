#pragma once
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>

class MqttClient {
public:
  void begin(const String &host, uint16_t port,
             const String &user, const String &pass,
             const String &prefix, const String &clientId);
  void reconfigure(const String &host, uint16_t port,
                   const String &user, const String &pass,
                   const String &prefix, const String &clientId);
  void loop();
  bool publish(const char *subtopic, const String &payload, bool retain = false);
  bool publishRaw(const char *topic, const String &payload, bool retain = false);
  bool connected();
  bool newConnection();
  const String &prefix() const { return _prefix; }
  String statusLabel();

private:
  void tryConnect();

  WiFiClient _wifi;
  PubSubClient _mqtt;
  String _host;
  uint16_t _port = 1883;
  String _user;
  String _pass;
  String _prefix;
  String _clientId;
  bool _initialized = false;
  bool _newConn = false;
  uint32_t _lastAttemptMs = 0;
  uint32_t _backoffMs = 5000;
};
