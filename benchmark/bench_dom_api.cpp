#include <benchmark/benchmark.h>
#include "simdjson/document.h"
#include "simdjson/jsonparser.h"
using namespace simdjson;
using namespace benchmark;
using namespace std;

#ifndef JSON_TEST_PATH
#define JSON_TEST_PATH "jsonexamples/twitter.json"
#endif

const padded_string EMPTY_ARRAY("[]", 2);

static void twitter_count(State& state) {
  // Prints the number of results in twitter.json
  document doc = document::parse(get_corpus(JSON_TEST_PATH));
  for (auto _ : state) {
    uint64_t result_count = doc["search_metadata"]["count"];
    if (result_count != 100) { return; }
  }
}
BENCHMARK(twitter_count);

static void iterator_twitter_count(State& state) {
  // Prints the number of results in twitter.json
  document doc = document::parse(get_corpus(JSON_TEST_PATH));
  for (auto _ : state) {
    document::iterator iter(doc);
    // uint64_t result_count = doc["search_metadata"]["count"];
    if (!iter.move_to_key("search_metadata")) { return; }
    if (!iter.move_to_key("count")) { return; }
    if (!iter.is_integer()) { return; }
    int64_t result_count = iter.get_integer();

    if (result_count != 100) { return; }
  }
}
BENCHMARK(iterator_twitter_count);

static void twitter_default_profile(State& state) {
  // Count unique users with a default profile.
  document doc = document::parse(get_corpus(JSON_TEST_PATH));
  for (auto _ : state) {
    set<string_view> default_users;
    for (document::object tweet : doc["statuses"].as_array()) {
      document::object user = tweet["user"];
      if (user["default_profile"]) {
        default_users.insert(user["screen_name"]);
      }
    }
    if (default_users.size() != 86) { return; }
  }
}
BENCHMARK(twitter_default_profile);

static void iterator_twitter_default_profile(State& state) {
  // Count unique users with a default profile.
  document doc = document::parse(get_corpus(JSON_TEST_PATH));
  for (auto _ : state) {
    set<string_view> default_users;
    document::iterator iter(doc);

    // for (document::object tweet : doc["statuses"].as_array()) {
    if (!(iter.move_to_key("statuses") && iter.is_array())) { return; }
    if (iter.down()) { // first status
      do {

        // document::object user = tweet["user"];
        if (!(iter.move_to_key("user") && iter.is_object())) { return; }

        // if (user["default_profile"]) {
        if (iter.move_to_key("default_profile")) {
          if (iter.is_true()) {
            if (!iter.up()) { return; } // back to user

            // default_users.insert(user["screen_name"]);
            if (!(iter.move_to_key("screen_name") && iter.is_string())) { return; }
            default_users.insert(string_view(iter.get_string(), iter.get_string_length()));
          }
          if (!iter.up()) { return; } // back to user
        }

        if (!iter.up()) { return; } // back to status

      } while (iter.next()); // next status
    }

    if (default_users.size() != 86) { return; }
  }
}
BENCHMARK(iterator_twitter_default_profile);

static void twitter_image_sizes(State& state) {
  // Count unique image sizes
  document doc = document::parse(get_corpus(JSON_TEST_PATH));
  for (auto _ : state) {
    set<tuple<uint64_t, uint64_t>> image_sizes;
    for (document::object tweet : doc["statuses"].as_array()) {
      auto [media, not_found] = tweet["entities"]["media"];
      if (!not_found) {
        for (document::object image : media.as_array()) {
          for (auto [key, size] : image["sizes"].as_object()) {
            image_sizes.insert({ size["w"], size["h"] });
          }
        }
      }
    }
    if (image_sizes.size() != 15) { return; };
  }
}
BENCHMARK(twitter_image_sizes);

static void iterator_twitter_image_sizes(State& state) {
  // Count unique image sizes
  document doc = document::parse(get_corpus(JSON_TEST_PATH));
  for (auto _ : state) {
    set<tuple<uint64_t, uint64_t>> image_sizes;
    document::iterator iter(doc);

    // for (document::object tweet : doc["statuses"].as_array()) {
    if (!(iter.move_to_key("statuses") && iter.is_array())) { return; }
    if (iter.down()) { // first status
      do {

        // auto [media, not_found] = tweet["entities"]["media"];
        // if (!not_found) {
        if (iter.move_to_key("entities")) {
          if (!iter.is_object()) { return; }
          if (iter.move_to_key("media")) {
            if (!iter.is_array()) { return; }

            //   for (document::object image : media.as_array()) {
            if (iter.down()) { // first media
              do {

                // for (auto [key, size] : image["sizes"].as_object()) {
                if (!(iter.move_to_key("sizes") && iter.is_object())) { return; }
                if (iter.down()) { // first size
                  do {
                    iter.move_to_value();

                    // image_sizes.insert({ size["w"], size["h"] });
                    if (!(iter.move_to_key("w")) && !iter.is_integer()) { return; }
                    uint64_t width = iter.get_integer();
                    if (!iter.up()) { return; } // back to size
                    if (!(iter.move_to_key("h")) && !iter.is_integer()) { return; }
                    uint64_t height = iter.get_integer();
                    if (!iter.up()) { return; } // back to size
                    image_sizes.insert({ width, height });

                  } while (iter.next()); // next size
                  if (!iter.up()) { return; } // back to sizes
                }
                if (!iter.up()) { return; } // back to image
              } while (iter.next()); // next image
              if (!iter.up()) { return; } // back to media
            }
            if (!iter.up()) { return; } // back to entities
          }
          if (!iter.up()) { return; } // back to status
        } 
      } while (iter.next()); // next status
    }

    if (image_sizes.size() != 15) { return; };
  }
}
BENCHMARK(iterator_twitter_image_sizes);

BENCHMARK_MAIN();