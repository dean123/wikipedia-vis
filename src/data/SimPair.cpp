#include "SimPair.hpp"

#include <cassert>

#ifdef SIMPAIR_DEBUG
#include <iostream>
#include <bitset>
#endif

SimPair::SimPair()
 : _data(0)
{}

SimPair::SimPair(uint32_t id, uint32_t similarity)
 : _data(0)
{
//        assert(id < 1 << 22 && similarity < 1 << 10);
        _data = (id << 10);
        _data |= similarity;

}

uint32_t
SimPair::getIndex() const
{
        return _data >> 10; // do we need to mask with 0x003FFFFF ?
                           // rsh on unsigned should fill with zero
                           // and not carry the sign bit
}

uint32_t
SimPair::getSim() const
{
        return _data & 0x3FF;
}

void
SimPair::setIndex(uint32_t id)
{
        assert(id < 1 << 10);
        _data &= 0x3FF;
        _data |= id << 10;
}

void
SimPair::setSim(uint32_t similarity)
{
        assert(similarity < 1 << 22);
        _data &= 0xFFFFFC00;
        _data |= similarity;
}

#ifdef SIMPAIR_DEBUG
void
SimPair::print_debug() const
{
        std::bitset<32> bs(_data);
        std::cout << "_data: " << bs << std::endl;
        std::cout << "ID: " << this->getIndex() << " - Similarity: " << this->getSim() << std::endl;
}
#endif

