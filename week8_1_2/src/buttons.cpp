#include "buttons.hpp"
#include <stdio.h>
#include <linux/gpio.h> // GPIO character device API
#include <string.h>     // memset() and strlen() defined here
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <thread>
#include <iostream>

/* Buttons Layout
 *                     0
 * 5  4              3   2
 *                     1
 */

/* GPIOs
 *                     12
 * 27  22            5    6
 *                     16
 */

/* Buttons are ACTIVE LOW */

// Define GPIO lines
#define BTN_A 27
#define BTN_B 22
#define BTN_UP 12
#define BTN_DOWN 16
#define BTN_LEFT 5
#define BTN_RIGHT 6

Buttons::Buttons(GameState *game) : game(game)
{
    /* TODO: Request GPIO inputs
     * 1) Open gpio device
     */
    // Open gpiochip0 as read-only device
    int fd_ = open("/dev/gpiochip0", O_RDONLY);
    if (fd_ < 0)
    {
        perror("open() failed in buttons contructor");
    }

    // Fill struct elements with 1's to clear old values
    // Vi havde et problem, da vi startede med at sætte values til 0. Så skød vores rumskib i starten af spillet af sig selv
    // Derfor sættes den til 1 for at simuleren, at alle "ikke" er trykket
    memset(&req_in, 1, sizeof(req_in));

    // Specify GPIO lines we want to access
    req_in.lineoffsets[0] = BTN_A;
    req_in.lineoffsets[1] = BTN_B;
    req_in.lineoffsets[2] = BTN_UP;
    req_in.lineoffsets[3] = BTN_DOWN;
    req_in.lineoffsets[4] = BTN_LEFT;
    req_in.lineoffsets[5] = BTN_RIGHT;

    // Access all lines as inputs
    req_in.flags = GPIOHANDLE_REQUEST_INPUT;
    // Number of gpio lines to access
    req_in.lines = 6;
    // Set a name to be shown when gpioinfo is called
    strcpy(req_in.consumer_label, "GPIO read example");
    /*
     * 2) Request inputs (IOCTL)
     */
    // Use ioctl() to get GPIO line handle from kernel
    int ret_val = ioctl(fd_, GPIO_GET_LINEHANDLE_IOCTL, &req_in);
    if (ret_val < 0)
    {
        perror("ioctl() failed i step 1 contructor");
        close(req_in.fd);
        close(fd_);
    }
}

Buttons::~Buttons()
{
    /* TODO: Release GPIO inputs
     * 1) Close File descriptors (x2)
     */
    close(req_in.fd);
    close(fd_);
}

Vec2 Buttons::readMoveButtons()
{
    for (int i = 2; i < (int)req_in.lines; ++i)
        if (data_in.values[i] == 0)
        {
            switch (i)
            {
            case 2:
                return {0, -1};
                break;
            case 3:
                return {0, 1};
                break;
            case 4:
                return {-1, 0};
                break;
            case 5:
                return {1, 0};
                break;
            default:
                return {0, 0};
                break;
            }
        }
    /* TODO: ReadMoveButtons
     * returns latest value of move buttons:
     * - up: {0, -1}
     * - down {0, 1}
     * - left: {-1 ,0}
     * - right: {1, 0}
     * - None pressed: {0, 0}
     */
    return {0, 0};
}

class EdgeDetector
{
private:
    int lastValue = 1;

public:
    EdgeDetector(int initialValue) : lastValue(initialValue) {};
    bool valueChanged(int value)
    {
        bool falling = (value != lastValue && value == 0);
        lastValue = value;
        return falling;
    }
};

void Buttons::buttonsThread()
{
    while (game->running)
    {
        /* TODO: Buttons Thread
         * 1) Run loop at fixed update speed @ 25 ms
         * 2) Update GPIO input data buffer
         * 3) Notify if shoot button has changed
         */
        readGPIOs();                                                // read først for at opdatere vores data struct og opdatere vores shoot knapper
        readMoveButtons();                                          // læser alle move knapper
        std::this_thread::sleep_for(std::chrono::milliseconds(25)); // sover i 25 ms
    }
}

void Buttons::readGPIOs()
{
    /* TODO: Read GPIOs
     * 1) Update input data buffer with current GPIO state
     */
    // Use ioctl() to get GPIO line value from kernel

    // Her gemmer vi de værdier, som knapperne havde i sidste while loop
    buttonLeft_last = data_in.values[0];
    buttonRight_last = data_in.values[1];

    // her opdateres data_in med de nye værdier.
    int ret_val = ioctl(req_in.fd, GPIOHANDLE_GET_LINE_VALUES_IOCTL, &data_in);
    if (ret_val < 0)
    {
        perror("ioctl failed in readGPIOs");
        close(req_in.fd);
        close(fd_);
    }

    // hvis en knap er trykket vil dens value være 0 og hvis sidste værdi var 1 vil det betyde tryk
    //  Derved sender vi et skud afsted og butten_last opdateres til 0. Derved skyder vi kun 1 gang
    // selvom vi holder knappen nede.

    if (data_in.values[0] == 0 && buttonLeft_last == 1)
    {
        {
        std::lock_guard<std::mutex> lock(game->gameMutex);
        game->shootButtonLeft_flag = true;
        }
        game->shootButton_cv.notify_one(); // signalere til vores bullet_input_thread, at knappen er trykket
    }
    else if (data_in.values[1] == 0 && buttonRight_last == 1)
    {
        {
        std::lock_guard<std::mutex> lock(game->gameMutex);
        game->shootButtonRight_flag = true;
        }
        game->shootButton_cv.notify_one(); // signalere til vores bullet_input_thread, at knappen er trykket
    }
}