#include <gtest/gtest.h>

extern "C" {
#include <cfg.h>
#include <sylkie_config.h>
}

static struct cfg_parser parsers[] = {
    {'v', "verbosity", CFG_INT, "control the amound of logging"},
    {'b', "bar", CFG_WORD, "random word"},
    {'f', "foo", CFG_BYTE, "what would this be without foo"},
    {'o', "bool", CFG_BOOL, "throw a random bool in the middle"},
    {'z', "baz", CFG_STRING, "lesser known foobar"},
    {'q', "qux", CFG_HW_ADDRESS, "much lesser known than foobar"},
    {'e', "example", CFG_IPV6_ADDRESS, "random example"},
    {'!', "never-called", CFG_BOOL, "this is never called"}};

static size_t parsers_sz = sizeof(parsers) / sizeof(struct cfg_parser);

static const char *parser_names[] = {
    "verbosity", "qux", "foo", "example", "bool", "baz", "bar",
};
static size_t parser_names_sz = sizeof(parser_names) / sizeof(const char *);

static const char *argv[] = {"-v",
                             "42",
                             "--bool",
                             "-z",
                             "thisisastring",
                             "--example=ff02::1",
                             "--foo",
                             "255",
                             "-b",
                             "512",
                             "-q",
                             "00:00:00:00:00:00"};
static int argc = sizeof(argv) / sizeof(const char *);

static u_int8_t all_nodes[] = {0xff, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01};

static u_int8_t lo_mac[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

class Cfg : public ::testing::Test {
public:
  static const struct cfg_template templt;

protected:
  struct packet_command *pkt_cmd = NULL;
  struct cfg_set config_set;

  void test_order() {
    size_t i;
    // Note: never-called should not be in this list
    for (i = 0; i < parsers_sz && i < parser_names_sz; ++i) {
      EXPECT_EQ(strcmp(parser_names[i], config_set.options.map[i]->name), 0);
    }
  }

  void test_basic_find() {
    size_t i;
    const struct cfg_set_item *item = NULL;
    // Ensure that each item expected can be found and returns an item
    for (i = 0; i < parsers_sz && i < parser_names_sz; ++i) {
      item = cfg_set_find(&config_set, parser_names[i]);
      ASSERT_TRUE(item);
      ASSERT_EQ(strcmp(item->name, parser_names[i]), 0);
    }

    // Ensure never-called cannot be found
    item = cfg_set_find(&config_set, "never-called");
    ASSERT_FALSE(item);
  }

  void test_find() {
    const struct cfg_set_item *item = NULL;
    // Ensure each item returns the correct info with cfg_find_type
    item = cfg_set_find(&config_set, "verbosity");
    EXPECT_EQ(strcmp("verbosity", item->name), 0);
    EXPECT_EQ(CFG_INT, item->value.type);
    EXPECT_EQ(item->value.integer, 42);

    item = cfg_set_find(&config_set, "bool");
    EXPECT_EQ(strcmp("bool", item->name), 0);
    EXPECT_EQ(item->value.type, CFG_BOOL);
    EXPECT_EQ(item->value.boolean, true);

    item = cfg_set_find(&config_set, "baz");
    EXPECT_EQ(strcmp("baz", item->name), 0);
    EXPECT_EQ(item->value.type, CFG_STRING);
    EXPECT_EQ(strcmp((const char *)item->value.data, "thisisastring"), 0);

    item = cfg_set_find(&config_set, "example");
    EXPECT_EQ(strcmp("example", item->name), 0);
    EXPECT_EQ(item->value.type, CFG_IPV6_ADDRESS);
    EXPECT_EQ(memcmp(item->value.data, &all_nodes, sizeof(all_nodes)), 0);

    item = cfg_set_find(&config_set, "foo");
    EXPECT_EQ(strcmp("foo", item->name), 0);
    EXPECT_EQ(item->value.type, CFG_BYTE);
    EXPECT_EQ(item->value.byte, 255);

    item = cfg_set_find(&config_set, "bar");
    EXPECT_EQ(strcmp("bar", item->name), 0);
    EXPECT_EQ(item->value.type, CFG_WORD);
    EXPECT_EQ(item->value.word, 512);

    item = cfg_set_find(&config_set, "qux");
    EXPECT_EQ(strcmp("qux", item->name), 0);
    EXPECT_EQ(item->value.type, CFG_HW_ADDRESS);
    EXPECT_EQ(memcmp(item->value.data, &lo_mac, sizeof(lo_mac)), 0);
  }

  void test_find_type() {
    int integer = 0;
    u_int16_t word = 0;
    u_int8_t byte = 0;
    void *data = NULL;
    bool value = false;

    // Ensure that unused arguments return -1
    ASSERT_EQ(cfg_set_find_type(&config_set, "never-called", CFG_BOOL, &value),
              -1);
    // Ensure find_type does not mutate the initial value
    EXPECT_FALSE(value);

    // Ensure that when the wrong type is provided no data is returned
    ASSERT_EQ(cfg_set_find_type(&config_set, "verbosity", CFG_STRING, &integer),
              -1);
    //// The above call should not have mutated integer
    EXPECT_EQ(integer, 0);

    // Ensure each item returns the correct info via cfg_set_find_type

    ASSERT_EQ(cfg_set_find_type(&config_set, "verbosity", CFG_INT, &integer),
              0);
    EXPECT_EQ(integer, 42);

    ASSERT_EQ(cfg_set_find_type(&config_set, "bool", CFG_BOOL, &value), 0);
    EXPECT_TRUE(value);

    ASSERT_EQ(cfg_set_find_type(&config_set, "baz", CFG_STRING, &data), 0);
    EXPECT_EQ(strcmp((const char *)data, "thisisastring"), 0);

    ASSERT_EQ(
        cfg_set_find_type(&config_set, "example", CFG_IPV6_ADDRESS, &data), 0);
    EXPECT_EQ(memcmp(data, &all_nodes, sizeof(all_nodes)), 0);

    ASSERT_EQ(cfg_set_find_type(&config_set, "foo", CFG_BYTE, &byte), 0);
    EXPECT_EQ(byte, 255);

    ASSERT_EQ(cfg_set_find_type(&config_set, "bar", CFG_WORD, &word), 0);
    EXPECT_EQ(word, 512);

    ASSERT_EQ(cfg_set_find_type(&config_set, "qux", CFG_HW_ADDRESS, &data), 0);
    EXPECT_EQ(memcmp(data, &lo_mac, sizeof(lo_mac)), 0);
  }
};

const struct cfg_template Cfg::templt = {
    parsers,
    parsers_sz,
    "Usage: This is a test parser",
    "Summary of a unit test",
    NULL,
    0
};

class CfgCmdline : public Cfg {
protected:
  virtual void SetUp() {
    cfg_set_init_cmdline(&config_set, &Cfg::templt, argc, argv);
  }

  virtual void TearDown() {
    cfg_set_free(&config_set);
  }
};

#ifdef BUILD_JSON
class CfgJson : public Cfg {
protected:
  struct json_object *jobj;

  virtual void SetUp() {
    static char json_input[] = "{ \
                            \"verbosity\": 42, \
                            \"bool\": true, \
                            \"baz\": \
                            \"thisisastring\", \
                            \"example\": \"ff02::1\", \
                            \"foo\": 255, \
                            \"bar\": 512, \
                            \"qux\": \"00:00:00:00:00:00\" \
                            }";
    enum json_tokener_error jerr;
    struct json_tokener *tok = json_tokener_new();

    do {
      jobj = json_tokener_parse_ex(tok, json_input, sizeof(json_input));
      jerr = json_tokener_get_error(tok);
    } while (jerr == json_tokener_continue);

    json_tokener_free(tok);
    cfg_set_init_json(&config_set, &Cfg::templt, jobj);
  }

  virtual void TearDown() {
    json_object_put(jobj);
    cfg_set_free(&config_set);
  }
};
#endif

TEST_F(CfgCmdline, order) {
  test_order();
}

TEST_F(CfgCmdline, basic_find) {
  test_basic_find();
}

TEST_F(CfgCmdline, find) {
  test_find();
}

TEST_F(CfgCmdline, find_type) {
  test_find_type();
}

#ifdef BUILD_JSON

TEST_F(CfgJson, order) {
  test_order();
}

TEST_F(CfgJson, basic_find) {
  test_basic_find();
}

TEST_F(CfgJson, find) {
  test_find();
}

TEST_F(CfgJson, find_type) {
  test_find_type();
}

#endif
