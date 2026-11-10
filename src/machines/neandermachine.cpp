#include "neandermachine.h"

NeanderMachine::NeanderMachine()
{
    identifier = "NDR";

    //////////////////////////////////////////////////
    // Initialize registers
    //////////////////////////////////////////////////

    registers.append(new Register("AC", "........", 8));
    registers.append(new Register("PC", "", 8, false));

    PC = registers.last();



    //////////////////////////////////////////////////
    // Initialize memory
    //////////////////////////////////////////////////

    setMemorySize(256);



    //////////////////////////////////////////////////
    // Initialize flags
    //////////////////////////////////////////////////

    flags.append(new Flag(Flag::NEGATIVE, "N"));
    flags.append(new Flag(Flag::ZERO, "Z", true));



    //////////////////////////////////////////////////
    // Initialize instructions
    //////////////////////////////////////////////////

    instructions.append(new Instruction(1, "0000....", Instruction::NOP, "nop"));
    instructions.append(new Instruction(2, "0001....", Instruction::STR, "sta a"));
    instructions.append(new Instruction(2, "0010....", Instruction::LDR, "lda a"));
    instructions.append(new Instruction(2, "0011....", Instruction::ADD, "add a"));
    instructions.append(new Instruction(2, "0100....", Instruction::OR,  "or a"));
    instructions.append(new Instruction(2, "0101....", Instruction::AND, "and a"));
    instructions.append(new Instruction(1, "0110....", Instruction::NOT, "not"));
    instructions.append(new Instruction(2, "0111....", Instruction::SUB, "sub a"));
    instructions.append(new Instruction(2, "1000....", Instruction::JMP, "jmp a"));
    instructions.append(new Instruction(2, "1001....", Instruction::JN,  "jn a"));
    instructions.append(new Instruction(2, "1010....", Instruction::JZ,  "jz a"));
    instructions.append(new Instruction(1, "1111....", Instruction::HLT, "hlt"));



    //////////////////////////////////////////////////
    // Initialize addressing modes
    //////////////////////////////////////////////////

    addressingModes.append(new AddressingMode("........", AddressingMode::DIRECT, AddressingMode::NO_PATTERN));
}



//////////////////////////////////////////////////
// Disassembler
//////////////////////////////////////////////////

namespace
{
    enum ByteKind { BYTE_DATA, BYTE_CODE_START, BYTE_CODE_OPERAND };

    QString hexByte(int value)
    {
        return QString("%1").arg(value & 0xFF, 2, 16, QChar('0')).toUpper();
    }
}

// Code is everything reachable from address 0 following the control flow
// (JMP: target only; JN/JZ: target and next; HLT: stop). Everything else is data.
// Assembling the generated source reproduces the current memory byte by byte.
QString NeanderMachine::disassemble()
{
    const int size = getMemorySize();
    QVector<int> kind(size, BYTE_DATA);



    //////////////////////////////////////////////////
    // Find code
    //////////////////////////////////////////////////

    QVector<int> pendingAddresses;
    pendingAddresses.append(0);

    while (!pendingAddresses.isEmpty())
    {
        int address = pendingAddresses.takeLast();

        while (address < size && kind[address] == BYTE_DATA)
        {
            int value = getMemoryValue(address);
            Instruction *instruction = getInstructionFromValue(value);

            // Unknown opcodes and opcodes with non-zero low bits would not assemble back to the same byte
            if (instruction == nullptr || value != instruction->getByteValue())
                break;

            int numBytes = instruction->getNumBytes();

            if (address + numBytes > size)
                break; // Operand would wrap around memory
            if (numBytes == 2 && kind[address + 1] != BYTE_DATA)
                break; // Overlaps code already found

            kind[address] = BYTE_CODE_START;
            if (numBytes == 2)
                kind[address + 1] = BYTE_CODE_OPERAND;

            Instruction::InstructionCode code = instruction->getInstructionCode();

            if (code == Instruction::HLT)
                break;

            if (code == Instruction::JMP)
            {
                address = getMemoryValue(address + 1);
                continue;
            }

            if (code == Instruction::JN || code == Instruction::JZ)
                pendingAddresses.append(getMemoryValue(address + 1));

            address += numBytes;
        }
    }



    //////////////////////////////////////////////////
    // Create labels for referenced addresses
    //////////////////////////////////////////////////

    QVector<QString> labels(size);

    for (int address = 0; address < size; address++)
    {
        if (kind[address] != BYTE_CODE_START || getInstructionFromValue(getMemoryValue(address))->getNumBytes() != 2)
            continue;

        int target = getMemoryValue(address + 1);

        if (kind[target] == BYTE_CODE_OPERAND) // Referenced as "label.op"
            labels[target - 1] = "L_" + hexByte(target - 1);
        else if (kind[target] == BYTE_CODE_START)
            labels[target] = "L_" + hexByte(target);
        else
            labels[target] = "D_" + hexByte(target);
    }

    auto operandReference = [&](int target) -> QString
    {
        if (kind[target] == BYTE_CODE_OPERAND)
            return labels[target - 1] + OPERAND_LABEL_SUFFIX;
        else
            return labels[target];
    };

    // Unlabeled zeros (data or NOP) don't need to be written: unassembled memory is zero
    auto isSkippable = [&](int address) -> bool
    {
        return getMemoryValue(address) == 0 && labels[address].isEmpty() && kind[address] != BYTE_CODE_OPERAND;
    };



    //////////////////////////////////////////////////
    // Generate source code
    //////////////////////////////////////////////////

    const int MIN_SKIPPED_ZEROS = 4;
    const int LABEL_WIDTH = 8;
    const int MNEMONIC_WIDTH = 4;

    QString source;
    source += "; Gerado pelo Hidra a partir da memória (Neander).\n";
    source += "; Comentários e nomes de labels originais não podem ser recuperados.\n";
    source += "; Código: tudo que é alcançável a partir do endereço 0 seguindo os desvios.\n";
    source += "; Dados: todo o resto. Posições de memória omitidas valem 0.\n";

    enum Section { SECTION_NONE, SECTION_CODE, SECTION_DATA };
    Section currentSection = SECTION_NONE;
    int nextAssemblerAddress = 0;
    int address = 0;

    while (address < size)
    {
        // Skip long runs of zeros (and zeros up to the end of memory)
        int runEnd = address;
        while (runEnd < size && isSkippable(runEnd))
            runEnd++;

        if (runEnd > address && (runEnd - address >= MIN_SKIPPED_ZEROS || runEnd == size))
        {
            address = runEnd;
            continue;
        }

        bool isCode = (kind[address] == BYTE_CODE_START);
        Section section = isCode ? SECTION_CODE : SECTION_DATA;

        if (section != currentSection)
        {
            source += isCode ? "\n; ===== código =====\n" : "\n; ===== dados =====\n";
            currentSection = section;
        }

        if (address != nextAssemblerAddress)
            source += QString(LABEL_WIDTH, ' ') + "ORG 0x" + hexByte(address) + "\n";

        QString line = labels[address].isEmpty() ? QString() : labels[address] + ":";
        line = line.leftJustified(LABEL_WIDTH - 1) + " ";

        if (isCode)
        {
            Instruction *instruction = getInstructionFromValue(getMemoryValue(address));
            line += instruction->getMnemonic().toUpper();

            if (instruction->getNumBytes() == 2)
                line = line.leftJustified(LABEL_WIDTH + MNEMONIC_WIDTH) + operandReference(getMemoryValue(address + 1));

            address += instruction->getNumBytes();
        }
        else
        {
            line += "DB " + QString::number(getMemoryValue(address));
            address += 1;
        }

        source += line + "\n";
        nextAssemblerAddress = address;
    }

    return source;
}
