#pragma once

bool isDigit(char c)
{
  return c >= '0' && c <= '9';
}

bool isAlpha(char c)
{
  return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c == '_');
}

bool isAlphanumeric(char c)
{
  return isAlpha(c) || isDigit(c);
}

bool isWhitespace(char c)
{
  return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}
