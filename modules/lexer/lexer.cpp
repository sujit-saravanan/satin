#include <asio/write.hpp>
#include "lexer.h"

const char* map_file(const char* filepath, size_t& length){
    int fd = open(filepath, O_RDONLY);

    struct stat sb{};
    fstat(fd, &sb);

    length = sb.st_size;
    const char* addr = static_cast<const char*>(mmap(nullptr, length, PROT_READ, MAP_PRIVATE, fd, 0u));
    return addr;
}

eToken chr_to_enum(char chr){
    switch (chr){
        case '(':
            return eToken::eLParen;
        case ')':
            return eToken::eRParen;
        case '{':
            return eToken::eLBrace;
        case '}':
            return eToken::eRBrace;
        case ',':
            return eToken::eComma;
        case '$':
            return eToken::eDollar;
        case ' ':
        case '\n':
        case '\t':
        case '\f':
        case '\r':
        case '\v':
            return eToken::eWhiteSpace;
        default:
            return eToken::eMisc;
    }
}


void Lexer::lex(){
    auto curr = m_start;

    while (curr && curr != m_end){

        while (std::isspace(curr[0])){
            m_tokens.push_back({std::string{curr[0]}, eToken::eWhiteSpace});
            curr++;
        }

        if (std::isalpha(curr[0])){

            auto word_start = curr;
            while (std::isalpha(curr[0]) || curr[0] == '_')
                curr++;
            auto word_end  = curr;

            std::string word(word_start, word_end);
            if (word == "def")
                m_tokens.push_back({"def", eToken::eDef});
            else
                m_tokens.push_back({word, eToken::eMisc});
            curr--;

        } else {
            m_tokens.push_back({std::string{curr[0]}, chr_to_enum(curr[0])});
        }

        curr++;
    }
}

void Lexer::build(const char* input_path, const char* output_path) {
    size_t length;
    m_start = map_file(input_path, length);
    m_end = m_start + length;

    lex();
    getMacros();
    getInstances();

    std::string output;
    output.reserve(m_tokens.size() * 8);

    int macro_skip_index = 0;
    int instance_skip_index = 0;
    for (size_t i = 0; i < m_tokens.size(); i++){
        if (i == m_macroSkips[macro_skip_index].m_start)
            i = m_macroSkips[macro_skip_index++].m_end;

        if (i == m_instanceSkips[instance_skip_index].m_start){
            for (const auto& instance : m_instances){
                if (instance.m_token_index == i){
                    output += replaceInstance(*m_macros.find({instance.m_name}), instance);
                    break;
                }
            }
            i = m_instanceSkips[instance_skip_index++].m_end;
        }
        output += m_tokens[i].m_value;
    }

    FILE* out = ::fopen(output_path, "wb");
    fwrite(output.c_str(), output.size(), 1, out);
    fclose(out);
}

void Lexer::getAndConsumeWhitespace(size_t &index) {
    index++;
    while (m_tokens[index].m_token == eToken::eWhiteSpace)
        index++;
}

Macro Lexer::parseMacro(size_t &index, Skip &skip) {
    Macro ret_macro;

    skip.m_start = index;
    getAndConsumeWhitespace(index);
    ret_macro.m_name = m_tokens[index].m_value;
    getAndConsumeWhitespace(index);
    getAndConsumeWhitespace(index);

    // Get parameters
    while(m_tokens[index].m_token != eToken::eRParen){
        if (m_tokens[index].m_token == eToken::eWhiteSpace || m_tokens[index].m_token == eToken::eComma){
            getAndConsumeWhitespace(index);
            continue;
        }
        ret_macro.m_params.push_back(m_tokens[index].m_value);
        index++;
    }

    getAndConsumeWhitespace(index);

    ret_macro.m_content_start = index + 1;
    getAndConsumeWhitespace(ret_macro.m_content_start);

    // Get replacements
    while(m_tokens[index].m_token != eToken::eRBrace){
        if (m_tokens[index].m_token == eToken::eDollar && m_tokens[++index].m_token == eToken::eDollar){
            ret_macro.m_replacements.push_back({m_tokens[++index].m_value, index});
        }
        index++;
    }

    ret_macro.m_content_end = index;
    skip.m_end = ++index;

    return ret_macro;
}


void Lexer::getMacros() {

    for (size_t i = 0; i < m_tokens.size(); i++){
        if (m_tokens[i].m_token == eToken::eDef){
            Skip skip;
            auto macro = parseMacro(i, skip);
            m_macroSkips.push_back(skip);
            m_macros.insert(macro);
        }
    }
}


Instance Lexer::parseInstance(size_t &index, Skip &skip) {
    skip.m_start = index;
    Instance ret_instance;
    ret_instance.m_name = m_tokens[index].m_value;
    ret_instance.m_token_index = index;
    getAndConsumeWhitespace(index);
    getAndConsumeWhitespace(index);

    // Get parameters
    while(m_tokens[index].m_token != eToken::eRParen){
        if (m_tokens[index].m_token == eToken::eComma)
            index++;

        if (m_tokens[index].m_token == eToken::eLBrace){
            std::string parameter;
            index++;
            while (m_tokens[index].m_token != eToken::eRBrace){
                parameter += m_tokens[index].m_value;
                index++;
            }
            ret_instance.m_params.push_back(parameter);
        } else  if (m_tokens[index].m_token != eToken::eWhiteSpace){
            ret_instance.m_params.push_back(m_tokens[index].m_value);
        }

        index++;
    }
    skip.m_end = index + 1;
    return ret_instance;
}

void Lexer::getInstances() {
    bool deffed = false;
    for (size_t i = 0; i < m_tokens.size(); i++){
        if (m_tokens[i].m_token == eToken::eDef)
            deffed = true;

        if (m_tokens[i].m_token == eToken::eMisc && !deffed && m_macros.find({m_tokens[i].m_value}) != m_macros.end()){
            size_t temp = i;
            getAndConsumeWhitespace(temp);

            if (m_tokens[temp].m_token == eToken::eLParen){
                Skip skip;
                m_instances.emplace_back(parseInstance(i, skip));
                m_instanceSkips.push_back(skip);
            }
        }

        if (m_tokens[i].m_token == eToken::eMisc)
            deffed = false;
    }

}

size_t Lexer::findParamIndex(const std::vector<std::string> &parameters, const std::string &param) {
    for(int i = 0; i < parameters.size(); i++){
        if (parameters[i] == param)
            return i;
    }
    return -1;
}
bool Lexer::insertReplacement(const Macro &macro, const Instance &instance, const size_t index,
                              std::string &ret_replacement) {
    for (const auto& replacement : macro.m_replacements){
        if (replacement.m_token_index == index){
            ret_replacement.erase(ret_replacement.length()-2);
            ret_replacement += instance.m_params[findParamIndex(macro.m_params, m_tokens[index].m_value)];
            return true;
        }
    }
    return false;
}

std::string Lexer::replaceInstance(const Macro &macro, const Instance &instance) {
    std::string ret_replacement;

    for (size_t index = macro.m_content_start; index < macro.m_content_end; index++)
        if (!insertReplacement(macro, instance, index, ret_replacement))
            ret_replacement += m_tokens[index].m_value;


    return ret_replacement;
}

