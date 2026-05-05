#include <stdio.h>
#include <stdbool.h>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <numeric>
#include <fstream>
#include <string>
#include <SDL2/SDL.h>

typedef float     f32;
typedef double    f64;
typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;
typedef size_t    usize;
typedef ssize_t   isize;

// Emulator Configs
#define screenWIDTH     1280
#define screenHEIGHT    720

// Memory Widths (in Bytes)
#define ramWidth        2000000000
#define vramWidth       256000000
#define ssdSectorWidth  512

using namespace std;

struct {
	// Emulator Stuff
	SDL_Window* window;
	SDL_Texture* texture;
	SDL_Renderer* renderer;
	u32 pixels[screenWIDTH * screenHEIGHT];
	bool quit = false;
	bool stepThrough = true;
	bool stepThroughCooldown = false;
} state;

class Profiler {

public:
	u32 currentFrame;
	float lastFrame;
	float deltaTime;
	u32 FPS;

	void updateFrame() {
		currentFrame = static_cast<float>(SDL_GetTicks()) / 1000;
		deltaTime = currentFrame - lastFrame;
		lastFrame = static_cast<float>(SDL_GetTicks()) / 1000;
		FPS = 1 / deltaTime;
	}
};

u16 ram[ramWidth / 2];
u16 vram[vramWidth / 2];

#include "sections/cpu.hpp"
#include "sections/gpu.hpp"

/*
Processor Instruction Set
 - 00 // No Operation
 - 01 // Store Value to Register
 - 02 // Store Register to Register
 - 03 // Store Register to Register if Zero
 - 04 // Store Register to Register if Equal
 - 05 // Store Register to Register if Less
 - 06 // Store Register to Register if More
 - 07 // Jump to Register
 - 08 // Jump to Register if Zero
 - 09 // 
 - 0A // Add Register to Register
 - 0B // Subract Register to Register
 - 0C // Multiply Register to Register
 - 0D // Divide Register to Register
 - 0E // Remainder Register to Register
 - 0F // 
 - 10 // AND Register to Register
 - 11 // OR Register to Register
 - 12 // XOR Register to Register
 - 13 // NOT Register to Register
 - 14 // Shift Register Left
 - 15 // Shift Register Right
 - 16 // Push Register to Stack
 - 17 // Pop Stack to Register
 - 18 // 
 - 19 // 
 - 1A // Store Register to RAM
 - 1B // Store RAM to Register
 - 1C // Push Packet to Bus Device
 - 1D // Request Packet from Bus Device
 - 1E // 
 - 1F // 

Processor Registers
 - 00 // General Purpose
 - 01 // General Purpose
 - 02 // General Purpose
 - 03 // General Purpose
 - 04 // General Purpose
 - 05 // General Purpose
 - 06 // General Purpose
 - 07 // General Purpose
 - 08 // General Purpose
 - 09 // General Purpose
 - 0A // General Purpose
 - 0B // General Purpose
 - 0C // General Purpose
 - 0D // General Purpose
 - 0E // General Purpose
 - 0F // General Purpose
 - 10 // Unused - General Purpose (for now, don't use unless these absolutely needed)
 - 11 // Unused - General Purpose
 - 12 // Unused - General Purpose
 - 13 // Unused - General Purpose
 - 14 // Unused - General Purpose
 - 15 // Unused - General Purpose
 - 16 // Unused - General Purpose
 - 17 // Unused - General Purpose
 - 18 // Unused - General Purpose
 - 19 // BIOS Mode
 - 1A // Bus Heartbeats
 - 1B // RAM Address Top
 - 1C // RAM Address Bottom
 - 1D // Stack Pointer
 - 1E // Program Counter Top
 - 1F // Program Counter Bottom
*/

CPU_Core    cpu_core;
GPU_Core    gpu_core;

void runDataGrabber();
void loadBIOS();

int main(int argc, char *argv[]) {
	memset(cpu_core.registers       , 0, sizeof(cpu_core.registers       ));
	memset(cpu_core.stack           , 0, sizeof(cpu_core.stack           ));
	memset(ram                      , 0, sizeof(ram                      ));
	memset(vram                     , 0, sizeof(vram                     ));
	loadBIOS();


	if (SDL_Init(SDL_INIT_EVERYTHING)  != 0) {
		std::cout << "[Main] - SDL failed to initialize. Error: " << SDL_GetError() << std::endl;
		return -1;
	}

	state.window =
		SDL_CreateWindow(
			"Lemon-16 Emulator",
			SDL_WINDOWPOS_CENTERED_DISPLAY(0),
			SDL_WINDOWPOS_CENTERED_DISPLAY(0),
			screenWIDTH,
			screenHEIGHT,
			SDL_WINDOW_ALLOW_HIGHDPI);

	if (state.window == NULL) {
		printf("[Main] - SDL Window failed to initialize.\n");
		return -1;
	}

	state.renderer = SDL_CreateRenderer(state.window, -1, SDL_RENDERER_PRESENTVSYNC);
	if (state.renderer == NULL) {
		printf("[Main] - SDL Renderer failed to initialize.\n");
		return -1;
	}
	state.texture =
		SDL_CreateTexture(
			state.renderer,
			SDL_PIXELFORMAT_RGBA8888,
			SDL_TEXTUREACCESS_STREAMING,
			screenWIDTH,
			screenHEIGHT);

	Profiler profiler;
	while (!state.quit) {
		profiler.updateFrame();
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				std::cout << "[Main] - Window Closed. Stopping." << std::endl;
				state.quit = true;
				break;
			} else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        state.quit = true;
						std::cout << "[Main] - Escape Pressed. Stopping." << std::endl;
                        break;
                    case SDLK_o:
						runDataGrabber();
                        break;
					case SDLK_p:
						if (!state.stepThroughCooldown && state.stepThrough) cpu_core.execute();
						state.stepThroughCooldown = true;
                        break;
					case SDLK_UP:
						state.stepThrough = true;
                        break;
					case SDLK_DOWN:
						state.stepThrough = false;
                        break;
                }
				if (event.key.keysym.sym != SDLK_p) {
					state.stepThroughCooldown = false;
				}
            }
		}
		if (!state.stepThrough) {
			for (int executionStep = 0; executionStep < 266666; executionStep++) {
				cpu_core.execute();
				if (state.quit) break;
			}
		}

		memset(state.pixels, 0, sizeof(state.pixels));
		for (int i = 0; i < screenWIDTH * screenHEIGHT * 2; i += 2) {
			state.pixels[i / 2] = (vram[i] << 16) + vram[i + 1];
		}
		SDL_UpdateTexture(state.texture, NULL, state.pixels, screenWIDTH * sizeof(u32));
		SDL_RenderCopyEx(state.renderer, state.texture, NULL, NULL, 0.0, NULL, SDL_FLIP_VERTICAL);
		SDL_RenderPresent(state.renderer);
	}
	SDL_Delay(1000);
	SDL_DestroyTexture(state.texture);
	SDL_DestroyRenderer(state.renderer);
	SDL_DestroyWindow(state.window);
	return 0;
}

void loadBIOS() {
	streampos size;
	char * memblock;

	ifstream file ("bios.bin", ios::in | ios::binary | ios::ate);
	if (file.is_open()) {
    	size = file.tellg();
    	memblock = new char [size];
    	file.seekg (0, ios::beg);
    	file.read (memblock, size);
    	file.close();

		int length = sizeof(memblock) / sizeof(memblock[0]);
		for (int i = 0; i < length / 2; i++) {
			ram[i] = (memblock[i*2] << 8) + memblock[(i*2)+1];
		}

    	delete[] memblock;
	} else cout << "Unable to open file";
	cpu_core.registers[0xFD] = 0x0000;
	cpu_core.registers[0xFE] = 0x0000;
	cpu_core.registers[0xFF] = 0x0000;
	return;
}

void runDataGrabber() {
	std::cout << std::endl;
	std::cout << "[Main] - Debug Data: {" << std::endl;
	std::cout << " - Registers ----" << std::endl;
	std::cout << " - " << std::hex << cpu_core.registers[0x00] << " - " << cpu_core.registers[0x01] << " - " << cpu_core.registers[0x02] << " - " << cpu_core.registers[0x03] << std::endl;
	std::cout << " - " << std::hex << cpu_core.registers[0x04] << " - " << cpu_core.registers[0x05] << " - " << cpu_core.registers[0x06] << " - " << cpu_core.registers[0x07] << std::endl;
	std::cout << " - " << std::hex << cpu_core.registers[0x08] << " - " << cpu_core.registers[0x09] << " - " << cpu_core.registers[0x0A] << " - " << cpu_core.registers[0x0B] << std::endl;
	std::cout << " - " << std::hex << cpu_core.registers[0x0C] << " - " << cpu_core.registers[0x0D] << " - " << cpu_core.registers[0x0E] << " - " << cpu_core.registers[0x0F] << std::endl;
	std::cout << " - " << std::hex << cpu_core.registers[0x10] << " - " << cpu_core.registers[0x11] << " - " << cpu_core.registers[0x12] << " - " << cpu_core.registers[0x13] << std::endl;
	std::cout << " - " << std::hex << cpu_core.registers[0x14] << " - " << cpu_core.registers[0x15] << " - " << cpu_core.registers[0x16] << " - " << cpu_core.registers[0x17] << std::endl;
	std::cout << " - " << std::hex << cpu_core.registers[0x18] << " - " << cpu_core.registers[0x19] << " - " << cpu_core.registers[0x1A] << " - " << cpu_core.registers[0x1B] << std::endl;
	std::cout << " - " << std::hex << cpu_core.registers[0x1C] << " - " << cpu_core.registers[0x1D] << " - " << cpu_core.registers[0x1E] << " - " << cpu_core.registers[0x1F] << std::endl;
	std::cout << std::endl;
	std::cout << " - RAM ------------" << std::endl;
	int effectiveAddress = cpu_core.registers[0x1F]; //std::floor(registers[0x1F] / 4) * 4;
	for (int i = 0; i < 8; i++) {
		std::cout << " - " << std::hex << effectiveAddress + (i * 4) << " | " << ram[effectiveAddress + (i * 4)] << " - " << ram[effectiveAddress + 1 + (i * 4)] << " - " << ram[effectiveAddress + 2 + (i * 4)] << " - " << ram[effectiveAddress + 3 + (i * 4)] << std::endl;
	}
	std::cout << std::endl;
	/*std::cout << " - Video RAM ------------" << std::endl;
	effectiveAddress = 0;
	for (int i = 0; i < 8; i++) {
		std::cout << " - " << std::hex << effectiveAddress + (i * 4) << " | " << vram[effectiveAddress + (i * 4)] << " - " << vram[effectiveAddress + 1 + (i * 4)] << " - " << vram[effectiveAddress + 2 + (i * 4)] << " - " << vram[effectiveAddress + 3 + (i * 4)] << std::endl;
	}
	std::cout << "}" << std::endl;*/
}
