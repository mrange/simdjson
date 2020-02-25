#include <iostream>
#include "simdjson/document.h"
#include "simdjson/jsonioutil.h"
using namespace std;
using namespace simdjson;

void document_parse_error_code() {
  string json("[ 1, 2, 3 ]");
  auto [doc, error] = document::parse(json);
  if (error) { cerr << "Error: " << error_message(error) << endl; exit(1); }
  doc.print_json(cout);
  cout << endl;
}

void document_parse_exception() {
  string json("[ 1, 2, 3 ]");
  document doc = document::parse(json);
  doc.print_json(cout);
  cout << endl;
}

void document_parse_padded_string() {
  padded_string json(string("[ 1, 2, 3 ]"));
  document doc = document::parse(json);
  doc.print_json(cout);
  cout << endl;
}

void document_parse_get_corpus() {
  padded_string json(get_corpus("jsonexamples/small/demo.json"));
  document doc = document::parse(json);
  doc.print_json(cout);
  cout << endl;
}

void parser_parse() {
  // Allocate a parser big enough for all files
  document::parser parser;
  if (!parser.allocate_capacity(1024*1024)) { exit(1); }

  // Read files with the parser, one by one
  for (padded_string json : { string("[1, 2, 3]"), string("true"), string("[ true, false ]") }) {
    cout << "Parsing " << json.data() << " ..." << endl;
    auto [doc, error] = parser.parse(json);
    if (error) { cerr << "Error: " << error_message(error) << endl; exit(1); }
    doc.print_json(cout);
    cout << endl;
  }
}

void object_iterator() {
  string json(R"({ "a": 1, "b": 2, "c": 3 })");
  const char* expected_key[] = { "a", "b", "c" };
  uint64_t expected_value[] = { 1, 2, 3 };
  int i = 0;

  document doc = document::parse(json);
  for (auto [key, value] : document::object(doc)) {
    if (key != expected_key[i] || uint64_t(value) != expected_value[i]) { cerr << "Expected " << expected_key[i] << " = " << expected_value[i] << ", got " << key << "=" << uint64_t(value) << endl; exit(1); }
    i++;
  }
  if (i*sizeof(uint64_t) != sizeof(expected_value)) { cout << "Expected " << sizeof(expected_value) << " values, got " << i << endl; exit(1); }
}

void array_iterator() {
  string json(R"([ 1, 10, 100 ])");
  uint64_t expected_value[] = { 1, 10, 100 };
  int i=0;

  document doc = document::parse(json);
  for (uint64_t value : document::array(doc)) {
    if (value != expected_value[i]) { cerr << "Expected " << expected_value[i] << ", got " << value << endl; exit(1); }
    i++;
  }
  if (i*sizeof(uint64_t) != sizeof(expected_value)) { cout << "Expected " << sizeof(expected_value) << " values, got " << i << endl; exit(1); }
}

void object_iterator_empty() {
  string json(R"({})");
  uint64_t expected_value[] = {};
  int i = 0;

  document doc = document::parse(json);
  for (auto [key, value] : document::object(doc)) {
    cout << "Unexpected " << key << " = " << uint64_t(value) << endl;
    i++;
  }
  if (i*sizeof(uint64_t) != sizeof(expected_value)) { cout << "Expected " << sizeof(expected_value) << " values, got " << i << endl; exit(1); }
}

void array_iterator_empty() {
  string json(R"([])");
  uint64_t expected_value[] = {};
  int i=0;

  document doc = document::parse(json);
  for (uint64_t value : document::array(doc)) {
    cout << "Unexpected value " << value << endl;
    i++;
  }
  if (i*sizeof(uint64_t) != sizeof(expected_value)) { cout << "Expected " << sizeof(expected_value) << " values, got " << i << endl; exit(1); }
}

void string_value() {
  string json(R"([ "hi", "has backslash\\" ])");
  document doc = document::parse(json);
  auto val = document::array(doc).begin();
  if (strcmp((const char*)*val, "hi")) { cerr << "Expected const char*(\"hi\") to be \"hi\", was " << (const char*)*val << endl; exit(1); }
  if (string_view(*val) != "hi") { cerr << "Expected string_view(\"hi\") to be \"hi\", was " << string_view(*val) << endl; exit(1); }
  ++val;
  if (strcmp((const char*)*val, "has backslash\\")) { cerr << "Expected const char*(\"has backslash\\\\\") to be \"has backslash\\\", was " << (const char*)*val << endl; exit(1); }
  if (string_view(*val) != "has backslash\\") { cerr << "Expected string_view(\"has backslash\\\\\") to be \"has backslash\\\", was " << string_view(*val) << endl; exit(1); }
}

void numeric_values() {
  string json(R"([ 0, 1, -1, 1.1 ])");
  document doc = document::parse(json);
  auto val = document::array(doc).begin();
  if (uint64_t(*val) != 0) { cerr << "Expected uint64_t(0) to be 0, was " << uint64_t(*val) << endl; exit(1); }
  if (int64_t(*val) != 0) { cerr << "Expected int64_t(0) to be 0, was " << int64_t(*val) << endl; exit(1); }
  if (double(*val) != 0) { cerr << "Expected double(0) to be 0, was " << double(*val) << endl; exit(1); }
  ++val;
  if (uint64_t(*val) != 1) { cerr << "Expected uint64_t(1) to be 1, was " << uint64_t(*val) << endl; exit(1); }
  if (int64_t(*val) != 1) { cerr << "Expected int64_t(1) to be 1, was " << int64_t(*val) << endl; exit(1); }
  if (double(*val) != 1) { cerr << "Expected double(1) to be 1, was " << double(*val) << endl; exit(1); }
  ++val;
  if (int64_t(*val) != -1) { cerr << "Expected int64_t(-1) to be -1, was " << int64_t(*val) << endl; exit(1); }
  if (double(*val) != -1) { cerr << "Expected double(-1) to be -1, was " << double(*val) << endl; exit(1); }
  ++val;
  if (double(*val) != 1.1) { cerr << "Expected double(1.1) to be 1.1, was " << double(*val) << endl; exit(1); }
}

void boolean_values() {
  string json(R"([ true, false ])");
  document doc = document::parse(json);
  auto val = document::array(doc).begin();
  if (bool(*val) != true) { cerr << "Expected bool(true) to be true, was " << bool(*val) << endl; exit(1); }
  ++val;
  if (bool(*val) != false) { cerr << "Expected bool(false) to be false, was " << bool(*val) << endl; exit(1); }
}

void null_value() {
  string json(R"([ null ])");
  document doc = document::parse(json);
  auto val = document::array(doc).begin();
  if (!(*val).is_null()) { cerr << "Expected null to be null!" << endl; exit(1); }
}


int main() {
  cout << "Running examples." << endl;
  document_parse_error_code();
  document_parse_exception();
  document_parse_padded_string();
  document_parse_get_corpus();
  parser_parse();
  object_iterator();
  array_iterator();
  object_iterator_empty();
  array_iterator_empty();
  string_value();
  numeric_values();
  boolean_values();
  null_value();
  cout << "Ran to completion!" << endl;
  return 0;
}
