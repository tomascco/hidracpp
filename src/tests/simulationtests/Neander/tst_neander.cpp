#include <QtTest>

#include "simulationtestcase.h"
#include "neandermachine.h"

class NeanderMachineTest : public QObject
{

    Q_OBJECT

private slots:
    void init();

    // Execution tests
    void test_STA();
    void test_LDA();
    void test_ADD();
    void test_OR();
    void test_AND();
    void test_NOT();
    void test_SUB();
    void test_JMP();
    void test_JN();
    void test_JZ();
    void test_HLT();
    void test_NOP();

    // Assembler tests
    void test_operandLabel();
    void test_numberLiterals();

    private:
    NeanderMachine testedMachine;

};

void NeanderMachineTest::init()
{
    testedMachine.clearFlags();
    testedMachine.setPCValue(0);
    testedMachine.setRegisterValue("AC", 0);
    testedMachine.setRunning(true);
}

void NeanderMachineTest::test_STA()
{
    int value; 
    PUT_INSTRUCTION_IN_MEMORY(testedMachine, 16, 128);
    testedMachine.setMemoryValue(128,5);
    testedMachine.step();
    value = testedMachine.getMemoryValue(128);
    QCOMPARE(value, 0);
}
void NeanderMachineTest::test_LDA()
{
    int value; 
    PUT_INSTRUCTION_IN_MEMORY(testedMachine, 32, 128);
    testedMachine.setMemoryValue(128,5);
    testedMachine.step();
    value = testedMachine.getRegisterValue("AC");
    QCOMPARE(value, 5);
}
void NeanderMachineTest::test_ADD()
{
    int value; 
    PUT_INSTRUCTION_IN_MEMORY(testedMachine, 48, 128);
    testedMachine.setRegisterValue("AC", 7);
    testedMachine.setMemoryValue(128,5);
    testedMachine.step();
    value = testedMachine.getRegisterValue("AC");
    QCOMPARE(value, 12);
}

void NeanderMachineTest::test_OR()
{
    int value; 
    PUT_INSTRUCTION_IN_MEMORY(testedMachine, 64, 128);
    testedMachine.setRegisterValue("AC", 7);
    testedMachine.setMemoryValue(128,5);
    testedMachine.step();
    value = testedMachine.getRegisterValue("AC");
    QCOMPARE(value, 7);
}

void NeanderMachineTest::test_AND()
{
     int value; 
    PUT_INSTRUCTION_IN_MEMORY(testedMachine, 80, 128);
    testedMachine.setRegisterValue("AC", 7);
    testedMachine.setMemoryValue(128, 5);
    testedMachine.step();
    value = testedMachine.getRegisterValue("AC");
    QCOMPARE(value, 5);
}

void NeanderMachineTest::test_NOT()
{   
     int value; 
    PUT_INSTRUCTION_IN_MEMORY(testedMachine, 96, 0);
    testedMachine.setRegisterValue("AC", 7);
    testedMachine.step();
    value = testedMachine.getRegisterValue("AC");
    QCOMPARE(value, 248);
}

void NeanderMachineTest::test_SUB()
{
    // 7 - 5 = 2 (positive, non-zero)
    PUT_INSTRUCTION_IN_MEMORY(testedMachine, 0x70, 128);
    testedMachine.setRegisterValue("AC", 7);
    testedMachine.setMemoryValue(128, 5);
    testedMachine.step();
    QCOMPARE(testedMachine.getRegisterValue("AC"), 2);
    QCOMPARE(testedMachine.getFlagValue("N"), 0);
    QCOMPARE(testedMachine.getFlagValue("Z"), 0);

    // 5 - 5 = 0 (zero)
    testedMachine.setPCValue(0);
    testedMachine.setRegisterValue("AC", 5);
    testedMachine.step();
    QCOMPARE(testedMachine.getRegisterValue("AC"), 0);
    QCOMPARE(testedMachine.getFlagValue("N"), 0);
    QCOMPARE(testedMachine.getFlagValue("Z"), 1);

    // 5 - 7 = -2 = 254 (negative)
    testedMachine.setPCValue(0);
    testedMachine.setRegisterValue("AC", 5);
    testedMachine.setMemoryValue(128, 7);
    testedMachine.step();
    QCOMPARE(testedMachine.getRegisterValue("AC"), 254);
    QCOMPARE(testedMachine.getFlagValue("N"), 1);
    QCOMPARE(testedMachine.getFlagValue("Z"), 0);
}

void NeanderMachineTest::test_JMP()
{   
    int value; 
    PUT_INSTRUCTION_IN_MEMORY(testedMachine, 128, 30);
    testedMachine.step();
    value = testedMachine.getRegisterValue("PC");
    QCOMPARE(value, 30);
}

void NeanderMachineTest::test_JN()
{   
    int value; 
    PUT_INSTRUCTION_IN_MEMORY(testedMachine, 144, 30);
    testedMachine.setRegisterValue("AC", 7);
    testedMachine.updateFlags(7);
    testedMachine.step();
    value = testedMachine.getRegisterValue("PC");
    QCOMPARE(value, 2);

    testedMachine.setRegisterValue("PC", 0);
    PUT_INSTRUCTION_IN_MEMORY(testedMachine, 144, 30);
    testedMachine.setRegisterValue("AC", 248);
    testedMachine.updateFlags(248);
    testedMachine.step();
    value = testedMachine.getRegisterValue("PC");
    QCOMPARE(value, 30);

}

void NeanderMachineTest::test_JZ()
{
    int value; 
    PUT_INSTRUCTION_IN_MEMORY(testedMachine, 160, 30);
    testedMachine.setRegisterValue("AC", 7);
    testedMachine.updateFlags(7);
    testedMachine.step();
    value = testedMachine.getRegisterValue("PC");
    QCOMPARE(value, 2);

    testedMachine.setRegisterValue("PC", 0);
    PUT_INSTRUCTION_IN_MEMORY(testedMachine, 160, 30);
    testedMachine.setRegisterValue("AC", 0);
    testedMachine.updateFlags(0);
    testedMachine.step();
    value = testedMachine.getRegisterValue("PC");
    QCOMPARE(value, 30);

}

void NeanderMachineTest::test_HLT()
{
    PUT_INSTRUCTION_IN_MEMORY(testedMachine, 240, 0);
    testedMachine.step();
    QCOMPARE(testedMachine.isRunning(), false);
}

void NeanderMachineTest::test_NOP()
{
    PUT_INSTRUCTION_IN_MEMORY(testedMachine, 0, 0);
    testedMachine.step();
    QCOMPARE(testedMachine.isRunning(), true);
}

void NeanderMachineTest::test_operandLabel()
{
    // "x.op" points to the operand byte of the instruction labeled "x"
    {
        NeanderMachine machine;
        machine.assemble("org 0\n"
                         "x: lda 128\n"   // 0, 1
                         "sta x.op\n"     // 2, 3
                         "hlt\n");        // 4
        QVERIFY(machine.getBuildSuccessful());
        QCOMPARE(machine.getMemoryValue(3), 1);
        QCOMPARE(machine.getAddressCorrespondingLabel(0), QString("x"));
        QCOMPARE(machine.getAddressCorrespondingLabel(1), QString("x.op"));
    }

    // Label alone on its line, case insensitive, with offset
    {
        NeanderMachine machine;
        machine.assemble("org 16\n"
                         "Loop:\n"
                         "  lda 128\n"       // 16, 17
                         "  sta LOOP.OP\n"   // 18, 19
                         "  jmp loop.op+1\n" // 20, 21
                         );
        QVERIFY(machine.getBuildSuccessful());
        QCOMPARE(machine.getMemoryValue(19), 17);
        QCOMPARE(machine.getMemoryValue(21), 18);
    }

    // One-byte instructions have no operand label
    {
        NeanderMachine machine;
        machine.assemble("y: not\n"
                         "lda y.op\n");
        QVERIFY(!machine.getBuildSuccessful());
    }

    // Directives have no operand label
    {
        NeanderMachine machine;
        machine.assemble("lda z.op\n"
                         "z: db 5\n");
        QVERIFY(!machine.getBuildSuccessful());
    }
}

void NeanderMachineTest::test_numberLiterals()
{
    {
        NeanderMachine machine;
        machine.assemble("        org 0\n"
                         "        lda x\n"
                         "        sub y\n"
                         "        sta r\n"
                         "        hlt\n"
                         "        org 0x80        ; data from 128\n"
                         "x:      db 0b00001010\n"
                         "y:      db 0X03\n"
                         "r:      db h7F\n"
                         "        db -0x01\n"
                         "        db -0b10\n");
        QVERIFY(machine.getBuildSuccessful());
        QCOMPARE(machine.getMemoryValue(1), 128);     // lda x -> x = 0x80
        QCOMPARE(machine.getMemoryValue(0x80), 10);
        QCOMPARE(machine.getMemoryValue(0x81), 3);
        QCOMPARE(machine.getMemoryValue(0x82), 0x7F);
        QCOMPARE(machine.getMemoryValue(0x83), 0xFF);
        QCOMPARE(machine.getMemoryValue(0x84), 0xFE);
    }

    // Offset with hexadecimal literal
    {
        NeanderMachine machine;
        machine.assemble("x: lda x+0x10\n");
        QVERIFY(machine.getBuildSuccessful());
        QCOMPARE(machine.getMemoryValue(1), 16);
    }

    // Invalid literals and out-of-range addresses
    const char *invalidSources[] = {
        "org 0x100\n",   // Beyond Neander's memory
        "db 0x\n",       // Prefix without digits
        "db 0b102\n",    // Invalid binary digit
        "db 0xG1\n",     // Invalid hex digit
        "db 0x+1\n",     // Sign after prefix
    };

    for (const char *source : invalidSources)
    {
        NeanderMachine machine;
        machine.assemble(source);
        QVERIFY2(!machine.getBuildSuccessful(), source);
    }

    QCOMPARE(Machine::parseNumber("h80", nullptr), 128);
    QCOMPARE(Machine::parseNumber("0x80", nullptr), 128);
    QCOMPARE(Machine::parseNumber("0b1111", nullptr), 15);
    QCOMPARE(Machine::parseNumber("-12", nullptr), -12);
}

#include "tst_neander.moc"
QTEST_APPLESS_MAIN(NeanderMachineTest)

