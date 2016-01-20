/*
 * Copyright 2016 Open Connectome Project (http://openconnecto.me)
 * Written by Disa Mhembere (disa@jhu.edu)
 *
 * This file is part of FlashGraph.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY CURRENT_KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __PRUNE_STATS_H__
#define __PRUNE_STATS_H__

#include <vector>

namespace {
    // Class to hold stats on the effectiveness of pruning
    class prune_stats
    {
        private:
            // Counts per iteration
            unsigned lemma1, _3a, _3b, _3c, _4;

            // Total counts
            unsigned long tot_lemma1, tot_3a, tot_3b, tot_3c, tot_4, iter;
            unsigned nrow;
            unsigned nclust;

            prune_stats(const unsigned nrows, const unsigned nclust) {
                _3a = 0; _3b = 0; lemma1 = 0; _3c = 0; _4 = 0;
                tot_lemma1 = 0; tot_3a = 0; tot_3b = 0;
                tot_3c = 0; tot_4 = 0; iter = 0;

                this->nrow = nrows;
                this->nclust = nclust;
            }

        public:
            typedef std::shared_ptr<prune_stats> ptr;

            static ptr create(const unsigned nrows, const unsigned nclust) {
                return ptr(new prune_stats(nrows, nclust));
            }
            void pp_lemma1(const unsigned var=1) { lemma1 += var; }
            void pp_3a() { _3a++; }
            void pp_3b() { _3b++; }
            void pp_3c() { _3c++; }
            void pp_4() { _4++; }

            const unsigned get_lemma1() const { return lemma1; }
            const unsigned get_3a() const { return _3a; }
            const unsigned get_3b() const { return _3b; }
            const unsigned get_3c() const { return _3c; }
            const unsigned get_4() const { return _4; }

            prune_stats& operator+=(prune_stats& other) {
                lemma1 += other.get_lemma1();
                _3a += other.get_3a();
                _3b += other.get_3b();
                _3c += other.get_3c();
                _4 += other.get_4();
                return *this;
            }

            void finalize() {
                iter++;
                BOOST_VERIFY((lemma1 + _3a + _3b + _3c + _4) <=  nrow*nclust);
                BOOST_LOG_TRIVIAL(info) << "\n\nPrune stats count:\n"
                    "lemma1 = " << lemma1 << ", 3a = " << _3a
                    << ", 3b = " << _3b << ", 3c = " << _3c << ", 4 = " << _4;

                BOOST_LOG_TRIVIAL(info) << "\n\nPrune stats \%s:\n"
                    "lemma1 = " << (lemma1 == 0 ? 0 : ((double)lemma1/(nrow*nclust))*100) <<
                    "\%, 3a = " << (_3a == 0 ? 0 : ((double)_3a/(nrow*nclust))*100) <<
                    "\%, 3b = " << (_3b == 0 ? 0 : ((double) _3b/(nrow*nclust))*100) <<
                    "\%, 3c = " << (_3c == 0 ? 0 : ((double) _3c/(nrow*nclust))*100) <<
                    "\%, 4 = " << (_4 == 0 ? 0 : ((double) _4/(nrow*nclust))*100) << "\%";

                tot_lemma1 += lemma1;
                tot_3a += _3a;
                tot_3b += _3b;
                tot_3c += _3c;
                tot_4 += _4;

                lemma1 = 0; _3a = 0; _3b = 0; _3c = 0; _4 = 0; // reset
            }

            std::vector<double> get_stats() {
                double perc_lemma1 = (tot_lemma1 / ((double)(nrow*iter*nclust)))*100;
                double perc_3a = (tot_3a / ((double)(nrow*iter*nclust)))*100;
                double perc_3b = (tot_3b / ((double)(nrow*iter*nclust)))*100;
                double perc_3c = (tot_3c / ((double)(nrow*iter*nclust)))*100;
                double perc_4 = (tot_4 / ((double)(nrow*iter*nclust)))*100;
                // Total percentage
                double perc = ((tot_3b + tot_3a + tot_3c + tot_4 + tot_lemma1) /
                        ((double)(nrow*iter*nclust)))*100;

                printf("tot_lemma1 = %lu, tot_3a = %lu, tot_3b = %lu, tot_3c = %lu, tot_4 = %lu\n",
                        tot_lemma1, tot_3a, tot_3b, tot_3c, tot_4);

                BOOST_LOG_TRIVIAL(info) << "\n\nPrune stats total:\n"
                    "Tot = " << perc << "\%, 3a = " << perc_3a <<
                    "\%, 3b = " << perc_3b << "\%, 3c = " << perc_3c
                    << "\%, 4 = " << perc_4 << "\%, lemma1 = " << perc_lemma1 << "\%";

                std::vector<double> ret {perc_lemma1, perc_3a, perc_3b, perc_3c, perc_4, perc};
                return ret;
            }
    };
}
#endif
