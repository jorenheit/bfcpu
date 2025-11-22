/*
  Source: https://tnu.me/brainfuck/generator.js
  Deobuscated and translated to C++ using ChatGPT.
  I don't know how it works and I don't care.
  I just need it to work.
 */

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>
#include <climits>

using namespace std;

const string BF_CHARS = "><+-.,[]";

string repeat(char c, int n) {
    return string(max(n, 0), c);
}

int compute_base_block(const vector<int>& values) {
    vector<int> vals = values;
    sort(vals.begin(), vals.end());
    double mid = vals.size() / 2.0 - 0.5;
    double avg = (vals[(int)ceil(mid)] + vals[(int)floor(mid)]) / 2.0;
    return (int)round(sqrt(avg));
}

string move_pointer(int cur, int target) {
    if (cur < target)
        return repeat('>', target - cur);
    else if (cur > target)
        return repeat('<', cur - target);
    return "";
}

string adjust_cell(int cur, int target) {
    if (target > cur)
        return repeat('+', target - cur);
    else if (target < cur)
        return repeat('-', cur - target);
    return "";
}

pair<int,int> best_base_cell(int current_index, const vector<int>& base_values, int target_value) {
    int best_index = -1;
    int best_cost = INT_MAX;

    for (int i = 0; i < (int)base_values.size(); i++) {
        int v = base_values[i];
        int cost = abs(v - target_value) + abs(i - current_index);
        if (cost < best_cost) {
            best_cost = cost;
            best_index = i;
        }
    }
    return {best_index, best_cost};
}

string gen_bf(const string& utf8) {
    if (utf8.empty())
        return "";

    // raw bytes = UTF-8 code units
    vector<int> codes(utf8.begin(), utf8.end());

    vector<int> anchors;
    vector<int> bases;
    int pointer = 0;

    // Build anchor set
    for (int val : codes) {
        auto [idx, cost] = best_base_cell(pointer, bases, val);
        int threshold = min(val, (int)ceil(sqrt(val)) + 1);

        if (idx == -1 || cost >= threshold) {
            // create new anchor
            pointer = anchors.size();
            anchors.push_back(val);
            bases.push_back(val);
        } else {
            // reuse
            pointer = idx;
            bases[idx] = val;
        }
    }

    int base_block = compute_base_block(anchors);

    vector<int> factors;
    vector<int> rounded_anchors;

    for (int v : anchors) {
        int f = (int)round((double)v / base_block);
        factors.push_back(f);
        rounded_anchors.push_back(f * base_block);
    }

    // Build initializer loop
    string bf;
    bf += repeat('+', base_block);
    bf += "[";

    for (int f : factors) {
        bf += ">";
        bf += repeat('+', f);
    }

    bf += repeat('<', factors.size());
    bf += "-]";
    bf += ">";

    // Emit chars
    pointer = 0;
    int pointer_max = 0;
    vector<int> current = rounded_anchors;

    for (int val : codes) {
        auto [idx, _] = best_base_cell(pointer, current, val);

        bf += move_pointer(pointer, idx);
        bf += adjust_cell(current[idx], val);
        bf += ".";

        pointer = idx;
	pointer_max = max(pointer_max, pointer);
        current[pointer] = val;
    }

    bf += move_pointer(pointer, pointer_max);
    return bf;
}
