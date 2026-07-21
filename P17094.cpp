#include <algorithm>
#include <array>
#include <cstdint>
#include <iostream>
#include <vector>

using u64 = unsigned long long;

struct State {
    // money[0] is cash now; money[i] is paid back i years later.
    std::array<u64, 7> money{};
};

struct Choice {
    int used = 0;
    // put[i] cents into a deposit lasting i years.
    std::array<int, 7> put{};
};

// A can reproduce every strategy available to B: by every future time its
// accumulated available money is at least B's.
bool dominates(const State& a, const State& b, int d) {
    u64 sum_a = 0, sum_b = 0;
    for (int i = 0; i <= d; ++i) {
        sum_a += a.money[i];
        sum_b += b.money[i];
        if (sum_a < sum_b) return false;
    }
    return true;
}

bool same_state(const State& a, const State& b) {
    return a.money == b.money;
}

int main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    int T;
    std::cin >> T;
    while (T--) {
        u64 n;
        int d, y;
        std::cin >> n >> d >> y;

        // Long deposits that can possibly be useful.  Deposits of more than
        // d years can be split at year d, and a long deposit of more than d
        // cents can be improved by separating a d-cent part into yearly
        // deposits.  A long deposit that can itself be split without loss is
        // discarded as well.
        std::array<std::vector<int>, 7> amount_for_term;
        for (int term = 2; term <= d; ++term) {
            for (int amount = 1; amount <= d; ++amount) {
                const int once = amount + amount * term / d;
                bool splittable = false;
                for (int first = 1; first < term; ++first) {
                    const int middle = amount + amount * first / d;
                    const int split = middle + middle * (term - first) / d;
                    if (split >= once) {
                        splittable = true;
                        break;
                    }
                }
                if (!splittable) amount_for_term[term].push_back(amount);
            }
        }

        // choices[rem] enumerates all possible long deposits made now when
        // rem years remain.  There is at most one deposit for each term: two
        // deposits with the same term should be merged before interest is
        // rounded independently.
        std::array<std::vector<Choice>, 7> choices;
        for (int rem = 1; rem <= d; ++rem) {
            Choice current;
            auto dfs = [&](auto&& self, int term) -> void {
                if (term > d) {
                    choices[rem].push_back(current);
                    return;
                }

                self(self, term + 1);  // no deposit of this term
                if (term <= rem) {
                    for (int amount : amount_for_term[term]) {
                        current.used += amount;
                        current.put[term] = amount;
                        self(self, term + 1);
                        current.put[term] = 0;
                        current.used -= amount;
                    }
                }
            };
            dfs(dfs, 2);
        }

        std::vector<State> states(1);
        states[0].money[0] = n;

        for (int year = 0; year < y; ++year) {
            const int remaining = y - year;
            const auto& available_choices = choices[std::min(d, remaining)];
            std::vector<State> next_states;

            for (const State& state : states) {
                for (const Choice& choice : available_choices) {
                    if (static_cast<u64>(choice.used) > state.money[0]) continue;

                    State next;
                    const u64 yearly = state.money[0] - choice.used;
                    next.money[0] = yearly + yearly / d + state.money[1];

                    for (int i = 1; i < d; ++i) {
                        next.money[i] = state.money[i + 1];
                    }

                    for (int term = 2; term <= d; ++term) {
                        const int amount = choice.put[term];
                        if (amount == 0) continue;
                        next.money[term - 1] += amount + amount * term / d;
                    }
                    next_states.push_back(next);
                }
            }

            std::sort(next_states.begin(), next_states.end(), [](const State& a, const State& b) {
                return a.money < b.money;
            });
            next_states.erase(
                std::unique(next_states.begin(), next_states.end(), same_state),
                next_states.end());

            states.clear();
            for (const State& candidate : next_states) {
                bool discarded = false;
                for (const State& kept : states) {
                    if (dominates(kept, candidate, d)) {
                        discarded = true;
                        break;
                    }
                }
                if (discarded) continue;

                states.erase(
                    std::remove_if(states.begin(), states.end(), [&](const State& kept) {
                        return dominates(candidate, kept, d);
                    }),
                    states.end());
                states.push_back(candidate);
            }
        }

        u64 answer = 0;
        for (const State& state : states) answer = std::max(answer, state.money[0]);
        std::cout << answer << '\n';
    }
    return 0;
}
