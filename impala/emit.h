#ifndef IMPALA_EMIT_H
#define IMPALA_EMIT_H

#include "impala/sema/world.h"

namespace impala {

template<class T> using Ptr = std::unique_ptr<const T>;
template<class T> using Ptrs = std::deque<Ptr<T>>;

struct Stmnt;

class Emitter : public World {
public:
    Emitter() {}

    void emit_stmnts(const Ptrs<Stmnt>&);
};

}

#endif
