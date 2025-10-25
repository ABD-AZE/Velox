#pragma once
#include <utility>

constexpr bool isDigit(char c) { return c >= '0' && c <= '9'; }

constexpr bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

constexpr bool isAlphanumeric(char c) { return isAlpha(c) || isDigit(c); }

constexpr bool isWhitespace(char c) {
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

constexpr std::pair<char, bool> processEscapeSequence(char c) {
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
  case 'b':
    return {'\b', true};
  case 'a':
    return {'\a', true};
  case '?':
    return {'\?', true};
  case '0':
    return {'\0', true};
  default:
    return {c, false};
  }
}
