// Copyright 2010-2014 RethinkDB, all rights reserved.
#ifndef TIMESTAMPS_HPP_
#define TIMESTAMPS_HPP_

#include <inttypes.h>

#include "containers/archive/archive.hpp"
#include "repli_timestamp.hpp"
#include "rpc/serialize_macros.hpp"

class printf_buffer_t;


/* These are the timestamp types used by the clustering code.
`repli_timestamp_t`, which is used internally within the btree code, is defined
elsewhere. */

/* `state_timestamp_t` is a unique identifier of a particular point in a series of
writes.  Writes carry the identifier of the new point in time that will exist when
said write is applied. */

class state_timestamp_t {
public:
    bool operator==(state_timestamp_t t) const { return num == t.num; }
    bool operator!=(state_timestamp_t t) const { return num != t.num; }
    bool operator<(state_timestamp_t t) const { return num < t.num; }
    bool operator>(state_timestamp_t t) const { return num > t.num; }
    bool operator<=(state_timestamp_t t) const { return num <= t.num; }
    bool operator>=(state_timestamp_t t) const { return num >= t.num; }

    static state_timestamp_t zero() {
        state_timestamp_t t;
        t.num = 0;
        return t;
    }

    state_timestamp_t next() const {
        state_timestamp_t t;
        t.num = num + 1;
        return t;
    }

    // (It wouldn't be the end of the world if you had to remove this NDEBUG wrapper
    // for some reason.  Right now we only use pred() in certain assertions that the
    // preceding state has the appropriate timestamp, when doing a write operation.
    // But beware: I suspect that certain assertions (of the metainfo or something)
    // might be invalid in the face of canceled write operations?  There is some
    // peculiar code in the broadcaster.)
#ifndef NDEBUG
    state_timestamp_t pred() const {
        state_timestamp_t t;
        t.num = num - 1;
        return t;
    }
#endif  // NDEBUG

    // Converts a "state_timestamp_t" to a repli_timestamp_t.  Really the only
    // difference is that repli_timestamp_t::invalid exists (you shouldn't use it).
    // Also, repli_timestamp_t's are generally used in the cache and serializer,
    // where they don't necessarily come in a linear sequence -- state timestamps
    // sort of live in their shards.
    repli_timestamp_t to_repli_timestamp() const {
        repli_timestamp_t ts;
        ts.longtime = num;
        return ts;
    }

    friend void debug_print(printf_buffer_t *buf, state_timestamp_t ts);

    RDB_MAKE_ME_SERIALIZABLE_1(state_timestamp_t, num);

private:
    uint64_t num;
};

void debug_print(printf_buffer_t *buf, state_timestamp_t ts);


#endif /* TIMESTAMPS_HPP_ */
