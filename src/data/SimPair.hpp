#ifndef SIM_PAIR_HPP
#define SIM_PAIR_HPP

#include <stdint.h>

class SimPair
{
        public:
                SimPair();
                SimPair(uint32_t id, uint32_t similarity);

                uint32_t getIndex() const;
                uint32_t getSim() const;
                void setIndex(uint32_t id);
                void setSim(uint32_t similarity);

#ifdef SIMPAIR_DEBUG
                void print_debug() const;
#endif

        private:
                // 32 Bits of storage
                uint32_t _data;
                // Data Layout:
                // Bit 10..31: id (max id: 2^22 or 4194303)
                // Bit   0..9: similarity value (between 0 and 999)
};

#endif // SIM_PAIR
