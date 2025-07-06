#include <iostream>
#include "chip8.h"
#include "platform.h"

int main(int argc, char** argv)
{
    if (argc != 4){
        std::cerr << "Uso: " << argv[0] << "<Escala> <Delay> <ROM>\n";
        std::exit(EXIT_FAILURE);
    }

    int videoScale = std::stoi(argv[1]); //we need to scale 64x32 pixels to an adequate number for modern screens
    int cycleDelay = std::stoi(argv[2]);
    char const* romFilename = argv[3];

    Platform platform("Ch83mu", VIDEO_WIDTH * videoScale, VIDEO_HEIGHT * videoScale, VIDEO_WIDTH, VIDEO_HEIGHT);

    Chip8 Emulator;

    Emulator.LoadROM(romFilename);

    int videoPitch = sizeof(Emulator.MEM_BUFFER[0]) * VIDEO_WIDTH;

    auto lastCycleTime = std::chrono::high_resolution_clock::now();
    bool quit = false;

    while (!quit)
    {
        quit = platform.ProcessInput(Emulator.KEYPAD);
        auto currentTime = std::chrono::high_resolution_clock::now();
        float dt = std::chrono::duration<float, std::chrono::milliseconds::period>(currentTime - lastCycleTime).count(); //delay time set up for the clock
        if (dt > cycleDelay)
		{
			lastCycleTime = currentTime;

			Emulator.Cycle();

			platform.Update(Emulator.MEM_BUFFER, videoPitch);
		}

    }

    return 0;
}