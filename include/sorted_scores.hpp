#pragma once
#include <vector>

struct SavedScore {
    int position;
    int score;

    bool operator<(SavedScore other) const {
        return position < other.position;
    }
};

class SortedScores {
    std::vector<SavedScore> entries;

    int lowerIndex(int position) {
        int first = 0;
        int last = int(entries.size());
        while (first < last) {
            int middle = first + (last - first) / 2;
            if (entries[middle].position < position) first = middle + 1;
            else last = middle;
        }
        return first;
    }

public:
    const SavedScore* findValue(SavedScore key) {
        int index = lowerIndex(key.position);
        if (index == int(entries.size()) || entries[index].position != key.position) return nullptr;
        return &entries[index];
    }

    bool insert(SavedScore entry) {
        int index = lowerIndex(entry.position);
        if (index < int(entries.size()) && entries[index].position == entry.position) return false;
        entries.insert(entries.begin() + index, entry);
        return true;
    }
};
