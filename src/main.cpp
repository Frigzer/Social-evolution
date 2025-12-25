#define NOMINMAX
#include <Windows.h>
#include "SimulationApp.hpp"

int main() {
    SetProcessDPIAware();
    SimulationApp app;
    app.run();
    return 0;
}