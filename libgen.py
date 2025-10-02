#!/usr/bin/env python3

import re

def read_file(filename):
    """Read the input C file"""
    with open(filename, 'r') as f:
        return f.readlines()


def write_file(filename, lines):
    """Write the output to a file"""
    with open(filename, 'w') as f:
        f.writelines(lines)


def transform_line(line):
    new_line = line.rstrip('\n') + "\\\n"
    new_line = new_line.replace("HASHMAP_DEFINE_PANIC(hashmap)", "HASHMAP_DEFINE_PANIC(Functions_Prefix_)")
    tokenized = tokenize_code_line(new_line)
    new_tokenized = []
    for token in tokenized:
        if token.startswith('"'):
            token = token.replace("Hashmap", "\"#Struct_Name_\"")
            token = token.replace("hashmap", "\"#Functions_Prefix_\"")
            token = token.replace("CustomKey", "\"#Custom_Key_Type_\"")
            token = token.replace("CustomValue", "\"#Custom_Value_Type_\"")
        else:
            token = token.replace("HashmapList", "Struct_Name_##List")
            token = token.replace("Hashmap", "Struct_Name_")
            token = token.replace("hashmap", "Functions_Prefix_##")
            token = token.replace("CustomKey", "Custom_Key_Type_")
            token = token.replace("CustomValue", "Custom_Value_Type_")
            token = token.replace("HASH_CALLBACK", "Custom_Hash_Func_")
            token = token.replace("COMPARISON_CALLBACK", "Custom_Comparison_Func_")
        new_tokenized.append(token)

    return "".join(new_tokenized)


def main():
    lines = read_file("hashmap.in.h")
    result = []
    in_macro = False
    for i, line in enumerate(lines):
        if "typedef int CustomValue;" in line:
            continue
        if "typedef const char *CustomKey;" in line:
            continue
        if "#ifndef HASH_CALLBACK" in line:
            continue
        if "#define HASH_CALLBACK NULL" in line:
            continue
        if "#endif /* HASH_CALLBACK */" in line:
            continue
        if "#ifndef COMPARISON_CALLBACK" in line:
            continue
        if "#define COMPARISON_CALLBACK NULL" in line:
            continue
        if "#endif /* COMPARISON_CALLBACK */" in line:
            continue
        if "/* Declarations start here */" in line:
            in_macro = True
            result.append("""#define HASHMAP_DECLARE(Struct_Name_, Functions_Prefix_, Custom_Key_Type_, Custom_Value_Type_, Custom_Hash_Func_, Custom_Comparison_Func_)\\
""")
            continue
        elif "/* Declarations stop here */" in line:
            in_macro = False
            last_value = result.pop()
            last_value = last_value[:-2] + last_value[-1]
            result.append(last_value)
            continue
        elif "/* Definitions start here */" in line:
            in_macro = True
            result.append("""#define HASHMAP_DEFINE(Struct_Name_, Functions_Prefix_, Custom_Key_Type_, Custom_Value_Type_, Custom_Hash_Func_, Custom_Comparison_Func_)\\
""")
            continue
        elif "/* Definitions stop here */" in line:
            in_macro = False
            last_value = result.pop()
            last_value = last_value[:-2] + last_value[-1]
            result.append(last_value)
            continue

        if in_macro:
            result.append(transform_line(line))
        else:
            result.append(line)

    write_file("hashmap.h", result)

def tokenize_code_line(line):
    """
    Tokenizes a line of code, keeping string literals intact.
    
    Args:
        line (str): A line of code to tokenize
        
    Returns:
        list: List of tokens where strings are preserved as complete tokens
    """
    pattern = r'("(?:[^"\\]|\\.)*")|' + r'([^"]+)'
    
    tokens = []
    matches = re.finditer(pattern, line)
    
    for match in matches:
        token = match.group(0)
        if token:
            tokens.append(token)
    
    return tokens

if __name__ == "__main__":
    main()
