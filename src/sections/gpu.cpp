class GPU_Core {
public:
	u16 registers[32];
	u16 cache[32768];

	void execute() {
		u32 epc = (this->registers[0x1E] << 16) + this->registers[0x1F]; // Effective Program Counter
		u16 ins = ram[this->registers[0x1F] + 0];                  // Instruction
		u16 p1  = ram[this->registers[0x1F] + 1];                  // Parameter 1
		u16 p2  = ram[this->registers[0x1F] + 2];                  // Parameter 2
		u16 p3  = ram[this->registers[0x1F] + 3];                  // Parameter 3
		if   (ins == 0x0000) {      // No Operation
			epc += 1;
		}

		if (epc >= (ramWidth / 2)) epc = 0;
		this->registers[0x1F] = epc & 0x0000FFFF;
		this->registers[0x1E] = (epc & 0xFFFF0000) >> 16;
		return;
	}
};
