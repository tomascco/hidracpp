#ifndef NEANDERMACHINE_H
#define NEANDERMACHINE_H

#include "core/machine.h"

class NeanderMachine : public Machine
{
public:
    NeanderMachine();

    bool supportsDisassembly() const override { return true; }
    QString disassemble() override;
};

#endif // NEANDERMACHINE_H
