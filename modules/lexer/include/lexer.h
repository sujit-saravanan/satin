#pragma once

#include "pch.h"

const char* map_file(const char* filepath, size_t& length);

enum class eToken{
    eMisc,
    eDef,
    eLParen,
    eRParen,
    eLBrace,
    eRBrace,
    eComma,
    eDollar,
    eWhiteSpace
};
struct Token{
    std::string m_value;
    eToken      m_token;
};

eToken chr_to_enum(char chr);

struct Skip{
    size_t m_start{};
    size_t m_end{};
};

struct Instance{
    std::string m_name{};
    std::vector<std::string> m_params{};
    size_t m_token_index{};
};
struct Replacement{
    std::string m_value{};
    size_t m_token_index{};
};
struct Macro{
    std::string m_name{};
    std::vector<std::string> m_params{};
    std::vector<Replacement> m_replacements{};

    size_t m_content_start{};
    size_t m_content_end{};

    bool operator==(const Macro& macro) const
    {
        return (this->m_name == macro.m_name);
    }
};
struct MacroHashFunction{
    size_t operator()(const Macro& macro) const
    {
        return std::hash<std::string>{}(macro.m_name);
    }
};


struct Lexer{
public:
    void build(const char* input_path, const char* output_path);

private:
    inline void getAndConsumeWhitespace(size_t &index);

    void lex();

    Macro parseMacro(size_t &index, Skip &skip);
    void getMacros();

    Instance parseInstance(size_t &index, Skip &skip);
    void getInstances();

    static inline size_t findParamIndex(const std::vector<std::string>& parameters, const std::string& param);

    inline bool insertReplacement(const Macro &macro, const Instance &instance, size_t index, std::string &ret_replacement);

    std::string replaceInstance(const Macro &macro, const Instance &instance);
private:
    const char* m_start;
    const char* m_end;

    std::vector<Token> m_tokens;

    std::vector<Skip> m_macroSkips;
    std::vector<Skip> m_instanceSkips;
    std::unordered_set<Macro, MacroHashFunction> m_macros;
    std::vector<Instance> m_instances;
};