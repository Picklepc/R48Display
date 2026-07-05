#pragma once

#include <Arduino.h>

namespace R48Web {

String dashboardBody();
String batteryBody();
PGM_P maintenanceBody();
PGM_P settingsBody();
String updateBody();
PGM_P appScript();

}  // namespace R48Web
