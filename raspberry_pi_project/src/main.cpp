#include "CameraStreamer.h"
#include "MjpegHttpServer.h"
#include "Connection/RPiPacket.h"
#include "Connection/UartConnection.h"

#include <opencv2/opencv.hpp>

#include <iostream>

// #include <fcntl.h>
#include <unistd.h>
// #include <termios.h>

// int main()
// {
//     UartConnection uartConnection;

//     uartConnection.Init();

//     while (true)
//     {
//         uartConnection.SendPacket(0, MessageTypes::Command, CommandTypes::SetLED, 1);
//         sleep(1);
        
//         uartConnection.SendPacket(0, MessageTypes::Command, CommandTypes::SetLED, 0);
//         sleep(1);
//     }
// }


int main()
{
    UartConnection uartConnection;
    CameraStreamer cameraStreamer(uartConnection);
    MjpegHttpServer httpServer(cameraStreamer);

    uartConnection.Init();

    if (!cameraStreamer.Start())
    {
        return 1;
    }

    if (!httpServer.Start())
    {
        cameraStreamer.Stop();
        return 1;
    }

    std::cout << "Open on Mac: http://<raspberry-pi-ip>:8080" << std::endl;

    int frameCount = 0;

    while (true)
    {
        cameraStreamer.Update(frameCount);

        // int key = cv::waitKey(1);
        // if (key == 'q' || key == 'Q')
        // {
        //     break;
        // }

        frameCount++;
    }

    // httpServer.Stop();
    cameraStreamer.Stop();
    cv::destroyAllWindows();

    return 0;
}
