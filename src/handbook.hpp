#pragma once

#include <string>
#include <vector>

struct HandbookEntry {
    std::string id;
    std::vector<std::string> aliases;
    std::string kind;
    std::string definition;
    std::string formula;
    std::string example;
    std::vector<std::string> related;
};

class Handbook {
public:
    Handbook();

    const HandbookEntry* find(const std::string& query) const;
    std::vector<const HandbookEntry*> search(const std::string& query) const;
    const std::vector<HandbookEntry>& all() const { return entries_; }

    std::string format(const HandbookEntry& e) const;
    std::string format_help_index() const;
    std::string format_list() const;

private:
    std::vector<HandbookEntry> entries_;
};
