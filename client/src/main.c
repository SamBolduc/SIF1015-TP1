#include "system.h"
#include "tui.h"

int main(int argc, char** argv) {
    ConnectToServer();

    StartTui();
    WaitTui();

    CloseConnection();
    return 0;
}