#include <QtTest>
#include <QTemporaryDir>
#include <QFile>

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

    // Memory file tests
    void test_hexImportExport();

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

static void writeTextFile(QString path, QString contents)
{
    QFile file(path);
    QVERIFY(file.open(QFile::WriteOnly | QFile::Truncate));
    file.write(contents.toUtf8());
}

static void fillMemory(Machine &machine, int value)
{
    for (int i = 0; i < machine.getMemorySize(); i++)
        machine.setMemoryValue(i, value);
}

void NeanderMachineTest::test_hexImportExport()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    NeanderMachine source, target;
    for (int i = 0; i < source.getMemorySize(); i++)
        source.setMemoryValue(i, (i * 7 + 3) & 0xFF);

    // $writememh -> $readmemh round trip
    QString hexPath = dir.filePath("roundtrip.hex");
    QCOMPARE((int)source.exportMemoryHex(hexPath), (int)FileErrorCode::noError);
    fillMemory(target, 0xAA);
    QCOMPARE((int)target.importMemory(hexPath, 0, 256, 0), (int)FileErrorCode::noError);
    for (int i = 0; i < 256; i++)
        QCOMPARE(target.getMemoryValue(i), source.getMemoryValue(i));

    // Hidra binary format is still detected and imported
    QString memPath = dir.filePath("roundtrip.mem");
    QCOMPARE((int)source.exportMemory(memPath), (int)FileErrorCode::noError);
    fillMemory(target, 0xAA);
    QCOMPARE((int)target.importMemory(memPath, 0, 256, 0), (int)FileErrorCode::noError);
    for (int i = 0; i < 256; i++)
        QCOMPARE(target.getMemoryValue(i), source.getMemoryValue(i));

    // Handwritten file: comments, @address, '_' separators; unspecified addresses become 0
    QString handPath = dir.filePath("hand.hex");
    writeTextFile(handPath, "// comentário\n"
                            "@10 0A 0b /* bloco\n"
                            "   em várias linhas */ f_f\n"
                            "@80\n"
                            "1\n");
    fillMemory(target, 0x55);
    QCOMPARE((int)target.importMemory(handPath, 0, 256, 0), (int)FileErrorCode::noError);
    for (int i = 0; i < 256; i++)
    {
        int expected = 0;
        if (i == 0x10) expected = 0x0A;
        if (i == 0x11) expected = 0x0B;
        if (i == 0x12) expected = 0xFF;
        if (i == 0x80) expected = 0x01;
        QCOMPARE(target.getMemoryValue(i), expected);
    }

    // Partial import: file addresses [0x10, 0x14) to memory 0x40; only the destination range changes
    fillMemory(target, 0x55);
    QCOMPARE((int)target.importMemory(handPath, 0x10, 0x14, 0x40), (int)FileErrorCode::noError);
    QCOMPARE(target.getMemoryValue(0x3F), 0x55);
    QCOMPARE(target.getMemoryValue(0x40), 0x0A);
    QCOMPARE(target.getMemoryValue(0x41), 0x0B);
    QCOMPARE(target.getMemoryValue(0x42), 0xFF);
    QCOMPARE(target.getMemoryValue(0x43), 0x00); // Not specified in the file
    QCOMPARE(target.getMemoryValue(0x44), 0x55);

    // Invalid files return invalidFormat and leave memory untouched
    const char *invalidContents[] = {
        "0A ZZ\n",              // Not hexadecimal
        "100\n",                // Value larger than a byte
        "@100 01\n",            // Address beyond memory
        "@FF 01 02\n",          // Writes beyond memory
        "01 /* unterminated\n", // Unterminated block comment
    };

    for (const char *contents : invalidContents)
    {
        QString badPath = dir.filePath("bad.hex");
        writeTextFile(badPath, contents);
        fillMemory(target, 0x55);
        QCOMPARE((int)target.importMemory(badPath, 0, 256, 0), (int)FileErrorCode::invalidFormat);
        for (int i = 0; i < 256; i++)
            QCOMPARE(target.getMemoryValue(i), 0x55);
    }
}

#include "tst_neander.moc"
QTEST_APPLESS_MAIN(NeanderMachineTest)

