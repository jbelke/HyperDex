// Copyright (c) 2011, Cornell University
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//     * Redistributions of source code must retain the above copyright notice,
//       this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//     * Neither the name of HyperDex nor the names of its contributors may be
//       used to endorse or promote products derived from this software without
//       specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef hyperdaemon_searches_h_
#define hyperdaemon_searches_h_

// po6
#include <po6/threads/mutex.h>

// e
#include <e/intrusive_ptr.h>
#include <e/lockfree_hash_map.h>

// HyperspaceHashing
#include <hyperspacehashing/equality_wildcard.h>

// HyperDex
#include <hyperdex/ids.h>

// Forward Declarations
namespace hyperdex
{
class coordinatorlink;
}
namespace hyperdaemon
{
class datalayer;
class logical;
}

namespace hyperdaemon
{

class searches
{
    public:
        searches(hyperdex::coordinatorlink* cl, datalayer* data, logical* comm);
        ~searches() throw ();

    public:
        void prepare(const hyperdex::configuration& newconfig, const hyperdex::instance& us);
        void reconfigure(const hyperdex::configuration& newconfig, const hyperdex::instance& us);
        void cleanup(const hyperdex::configuration& newconfig, const hyperdex::instance& us);

    public:
        void start(const hyperdex::entityid& client, uint32_t nonce, const hyperdex::regionid& r, const hyperdex::search& s);
        void start(const hyperdex::entityid& client, uint32_t nonce,
                   const hyperdex::regionid& r,
                   const hyperspacehashing::equality_wildcard& wc);
        void next(const hyperdex::entityid& client, uint32_t nonce);
        void stop(const hyperdex::entityid& client, uint32_t nonce);

    private:
        static uint64_t hash(const std::pair<hyperdex::entityid, uint32_t>&);

    private:
        class search_state
        {
            public:
                search_state(const hyperdex::regionid& region,
                             const hyperspacehashing::equality_wildcard& wc,
                             e::intrusive_ptr<hyperdisk::snapshot> snap);
                ~search_state() throw ();

            public:
                po6::threads::mutex lock;
                const hyperdex::regionid region;
                const hyperspacehashing::equality_wildcard wc;
                e::intrusive_ptr<hyperdisk::snapshot> snap;
                uint64_t count;

            private:
                friend class e::intrusive_ptr<search_state>;

            private:
                void inc() { __sync_add_and_fetch(&m_ref, 1); }
                void dec() { if (__sync_sub_and_fetch(&m_ref, 1) == 0) delete this; }

            private:
                size_t m_ref;
        };

    private:
        searches(const searches&);

    private:
        searches& operator = (const searches&);

    private:
        hyperdex::coordinatorlink* m_cl;
        datalayer* m_data;
        logical* m_comm;
        hyperdex::configuration m_config;
        e::lockfree_hash_map<std::pair<hyperdex::entityid, uint32_t>, e::intrusive_ptr<search_state>, hash> m_searches;
};

} // namespace hyperdaemon

#endif // hyperdaemon_searches_h_
