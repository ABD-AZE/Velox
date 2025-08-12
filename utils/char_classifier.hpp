#pragma once
#include <utility>

bool isDigit(char c) { return c >= '0' && c <= '9'; }

bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

bool isAlphanumeric(char c) { return isAlpha(c) || isDigit(c); }

bool isWhitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// Utility function to handle escape sequences
// Returns a pair: <escaped_character, success>
// If success is false, the character should not be processed as an escape
std::pair<char, bool> processEscapeSequence(char c) {
  switch (c) {
  case '"':
    return {'"', true};
  case '\'':
    return {'\'', true};
  case '\\':
    return {'\\', true};
  case 'n':
    return {'\n', true};
  case 't':
    return {'\t', true};
  case 'r':
    return {'\r', true};
  case 'f':
    return {'\f', true};
  case 'v':
    return {'\v', true};
  case '0':
    return {'\0', true};
  default:
    return {c, false};
  }
}
