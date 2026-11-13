#include <iostream>
#include <cstdint>
#include <format>
#include <fstream>
#include <vector>
#include <array>
#include <string_view>
#include <algorithm>
#include <charconv>
#include <random>
#include <nlohmann/json.hpp>

using namespace std::literals;

constexpr std::array WORD_T_STR_MAP{
    "unknown"sv,
    "Adjective"sv,
    "Adverb"sv,
    "Noun"sv,
    "Verb"sv,
    "Determiner"sv,
    "Conjunction"sv,
    "Preposition"sv,
    "Interjection"sv,
    "Pronoun"sv
};

enum Word_Type:uint16_t //word type must have same value as index in WORD_T_STR_MAP
{
    unknown,
    Adjective,
    Adverb,
    Noun,
    Verb,
    Determiner,
    Conjunction,
    Preposition,
    Interjection,
    Pronoun
};

uint16_t to_word_type(std::string_view str) {
    for (uint16_t idx = 1; idx < WORD_T_STR_MAP.size();++idx) {
        if (WORD_T_STR_MAP[idx] == str) {
            return idx;
        }
    }
    return Word_Type::unknown;
}

std::string_view to_word_type_str(uint16_t t) {
    if (t >= WORD_T_STR_MAP.size()) {
        return WORD_T_STR_MAP[0];
    }
    return WORD_T_STR_MAP[t];
}

struct Word {
    std::string str;
    uint16_t typ;
    uint16_t syllables;
};

inline const Word NULLWORD{ "null", 0, 0 };

struct Dict {
    
    const auto& str(size_t i) const {
        return data[i].str;
    }
    auto syllables(size_t i) const {
        return data[i].syllables;
    }
    auto typ(size_t i) const {
        return data[i].typ;
    }

    std::vector<Word> data;
};

std::ostream& operator<<(std::ostream& os, const Dict& d) {
    std::string buf;
    auto inserter = std::back_inserter(buf);
    for (size_t i = 0; i < d.data.size(); i++) {
        std::format_to(inserter, "{:3}. {:12} {:2} {}\n", i, to_word_type_str(d.typ(i)), d.syllables(i), d.str(i));
        os << buf;
        buf.clear();
    }
    return os;
}

enum ErrorCode
{
    noerror,
    genernal_error,
    parse_fail_open_file,
    parse_fail_unknow_type,
    alg_invalid_first_idx,
    alg_invalid_first_syl,
    alg_unsolvable,
    haiku_invalid_line_syllables,
    haiku_invalid_length,
    haiku_duplicate_word,
    haiku_incomplete_compose
};

struct Parser {
    Parser(std::string_view fname) :ifs{fname.data()}{}

    int operator()(Dict& dict) {
        nlohmann::json json;

        if (!ifs.is_open()) {
            return ErrorCode::parse_fail_open_file;
        }
        ifs >> json; //not sure that do i need handle some error here. 

        dict.data.reserve(json.size() + 1);
        dict.data.push_back(NULLWORD);//index 0 reserve for null word

        for (const auto& item : json) {
            auto& w = dict.data.emplace_back();
            w.typ = to_word_type(item.at("part_of_speech").get<std::string>());
            if (w.typ == Word_Type::unknown) {
                return ErrorCode::parse_fail_unknow_type;
            }
            w.str = item.at("word").get<std::string>();
            w.syllables = item.at("syllables").get<uint16_t>();
        }

        return 0;
    }
    std::ifstream ifs;
};

constexpr size_t HAIKU_LINE_N = 3;
constexpr std::array<size_t, HAIKU_LINE_N> HAIKU_LINE_SYl{ 5,7,5 };
constexpr size_t HAIKU_MAX_SYl = *std::max_element(HAIKU_LINE_SYl.begin(), HAIKU_LINE_SYl.end());

struct Haiku { 
    using Line = std::array<size_t, HAIKU_MAX_SYl>; // store index of word in dict for easier to process, use 0 (NULLWORD in dict) for unsolve or line break.

    auto line_n() const {
        return HAIKU_LINE_N;
    }

    auto max_syl() const {
        return HAIKU_MAX_SYl;
    }

    size_t& first()  {
        return lines[0][0];
    }

    size_t first() const {
        return lines[0][0];
    }

    std::array<Line, HAIKU_LINE_N> lines;
    std::array<size_t, HAIKU_LINE_N> line_syl{ HAIKU_LINE_SYl };
};

/// @brief validate following the bussiness rules 
/// - syllables for each line
/// - no repeat word
/// - Adjective, Adverb, Noun, Verb in Haiku
/// and the logical rule
/// - word number in Haiku
int validate(const Haiku& h, const Dict& d) {
    
    std::array<int, WORD_T_STR_MAP.size()> type_count{};
    std::vector<size_t> words;
    words.reserve(h.line_n() * h.max_syl());

    for (size_t l = 0;l < h.lines.size(); ++l) {
        uint16_t line_syllables = 0;
        for (auto idx: h.lines[l]) {
            if (idx == 0) {
                break;
            }
            line_syllables += d.syllables(idx);
            ++type_count[d.typ(idx)];
            words.push_back(idx);
        }
        if (line_syllables != h.line_syl[l]) {
            return ErrorCode::haiku_invalid_line_syllables;
        }
    }
    if (words.size() < 1) {
        return ErrorCode::haiku_invalid_length;
    }
    std::sort(words.begin(), words.end());

    for (size_t i = 1;i < words.size(); ++i) {
        if (words[i] == words[i - 1]) {
            return ErrorCode::haiku_duplicate_word;
        }
    }
    if (type_count[Word_Type::Adjective] == 0
        || type_count[Word_Type::Adverb] == 0
        || type_count[Word_Type::Noun] == 0
        || type_count[Word_Type::Verb] == 0
        ) {
        return ErrorCode::haiku_incomplete_compose;
    }
    return 0;
}

std::ostream& print(std::ostream& os, const Haiku& h, const Dict& d) {
    auto input = h.first();

    os << std::format("Input: {} ({})\n\nOutput:\n", d.str(input), d.syllables(input));

    for (auto& line:h.lines) {
        uint16_t n = 0;
        for (auto idx : line) {
            if (idx == 0) {
                break;
            }
            os << d.str(idx) << " ";
            n += d.syllables(idx);
        }
        os << std::format("({})\n", n);
    }
    
    return os;
}

struct Alg_Backtrack {
   
    Alg_Backtrack(Dict& dict) :m_dict{ dict } {}

    int operator()(Haiku& h) {
        size_t n = m_dict.data.size();
        size_t first = h.first();//input is first value

        if (first == 0 || first >= n) {
            return ErrorCode::alg_invalid_first_idx;
        }
        size_t first_syl = m_dict.syllables(first);
        if (first_syl == 0 || first_syl > h.line_syl[0]) {
            return ErrorCode::alg_invalid_first_syl;
        }

        //for reuse object
        m_used.assign(n, false);
        m_type_count.fill(0);

        //init stage
        m_used[first] = true;
        ++m_type_count[m_dict.typ(first)];

        if (solve(h, 0, 1, h.line_syl[0] - first_syl)) {
            return 0;
        }

        return ErrorCode::alg_unsolvable;
    }

    bool complete() const {
        return m_type_count[Word_Type::Adjective] > 0
            && m_type_count[Word_Type::Adverb] > 0
            && m_type_count[Word_Type::Noun] > 0
            && m_type_count[Word_Type::Verb] > 0;
    }

    bool solve(Haiku& h, size_t l, size_t p, size_t remain_syl) {
        if (remain_syl == 0) { //current line is complete
            auto next_l = l + 1;
            if (next_l == h.line_n()) { 
                return complete();//all lines is complete check for types in Haiku
            }
            return solve(h, next_l, 0, h.line_syl[next_l]); //go solve for next line
        }

        if (p >= h.max_syl()) { //number of words in line should never over max syllables.
            return false; //should never reach here because there is no word with 0 syllable in this world.
        }

        for (size_t i = 1; i < m_dict.data.size(); ++i) {
            size_t syl = m_dict.syllables(i);
            if (m_used[i] || syl == 0 || syl > remain_syl) { //skip invalid word
                continue;
            }
            auto typ = m_dict.typ(i);

            //try solove by this word
            m_used[i] = true;
            ++m_type_count[typ];
            h.lines[l][p] = i;

            if (solve(h, l, p + 1, remain_syl - syl)) {
                return true;
            }

            //backtrace is trigger restore to previous stage
            h.lines[l][p] = 0;
            --m_type_count[typ];
            m_used[i] = false;
        }
        return false; //trigger backtrace
    }

    Dict& m_dict;
    std::vector<bool> m_used;
    std::array<int, WORD_T_STR_MAP.size()> m_type_count{};
};


int cmd(std::string_view idx_str, std::string_view shuffle_str) {

    size_t idx;

    if (std::from_chars(idx_str.data(), idx_str.data() + idx_str.size(), idx).ec !=  std::errc{}) {
        std::cerr << std::format("can't parse idx to size_t: {}\n", idx_str);
        return 1;
    }

    int shuffle;

    if (std::from_chars(shuffle_str.data(), shuffle_str.data() + shuffle_str.size(), shuffle).ec != std::errc{}) {
        std::cerr << std::format("can't parse shuffle to int: {}\n", shuffle_str);
        return 1;
    }

    Parser ps{ R"(in.json)" };

    Dict dict{};
    int err = ps(dict);

    if (err) {
        std::cerr << std::format("parse error code={}\n", err);
        return err;
    }

    if (shuffle) {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::shuffle(dict.data.begin() + 1, dict.data.end(), rng);
    }

    Haiku h{};
    h.first() = idx;

    Alg_Backtrack alg{ dict };

    err = alg(h);
    if (err) {
        std::cerr << std::format("resolve error code={}\n", err);
        return err;
    }

    err = validate(h, dict);

    if (err) {
        std::cerr << std::format("invalid haiku output error code={}\n", err);
        return err;
    }

    print(std::cout, h, dict) <<  "\ndict:\n" << dict;

    return 0;
}

constexpr auto help_text = R"(usage:
    <exetable name> [int:input index in dict (default: 1)] [int:shuffle dict (default: 0)]
)";

int main(int argc, const char** argv) {
    switch(argc){
    case 1:
        return cmd("1", "0");
    case 2:
        return cmd(argv[1], "0");
    case 3:
        return cmd(argv[1], argv[2]);
    default:
        std::cerr << std::format("invalid arguments\n====================\n{}", help_text);
        return 1;
    }
}