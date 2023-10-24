#include "systemc.h"

class bus_interface : public sc_interface {
public:
    virtual void write(int data) = 0;
    virtual int read() = 0;
};
